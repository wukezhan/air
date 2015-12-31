/*
  +----------------------------------------------------------------------+
  | air framework                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) wukezhan<wukezhan@gmail.com>                           |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: wukezhan<wukezhan@gmail.com>                                 |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "Zend/zend_interfaces.h"

#include "php_air.h"

#include "src/air_async_waiter.h"
#include "src/air_curl.h"
#include "src/air_curl_waiter.h"

#define CURLM_OK 0
#define CURLM_CALL_MULTI_PERFORM -1

zend_class_entry *air_curl_waiter_ce;

void air_curl_waiter_build_multi_curl(zval *self, zval *map){
	zval *mh = zend_read_property(air_curl_waiter_ce, self, ZEND_STRL("_mh"), 1, NULL);
	zval *services = zend_read_property(air_curl_waiter_ce, self, ZEND_STRL("_services"), 1, NULL);
	zval *service;
	ulong idx;
	zend_string *key;
	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(services), idx, key, service){
		zend_class_entry *service_ce = Z_OBJCE_P(service);
		zval arr;
		array_init(&arr);
		zval *service_id = zend_read_property(service_ce, service, ZEND_STRL("_id"), 1, NULL);
		Z_TRY_ADDREF_P(service_id);
		add_next_index_zval(&arr, service_id);
		zval *curl = zend_read_property(service_ce, service, ZEND_STRL("request"), 1, NULL);
		air_curl_set_opt_array(curl);
		zval *ch = zend_read_property(Z_OBJCE_P(curl), curl, ZEND_STRL("_ch"), 1, NULL);
		Z_TRY_ADDREF_P(ch);
		add_next_index_zval(&arr, ch);
		add_next_index_zval(map, &arr);
		zval am_params[2] = {*mh, *ch};
		air_call_func("curl_multi_add_handle", 2, am_params, NULL);
	}ZEND_HASH_FOREACH_END();
}

void air_curl_waiter_select(zval *self, zval *map){
	zval *mh = zend_read_property(air_curl_waiter_ce, self, ZEND_STRL("_mh"), 1, NULL);

	zval sleep_us;
	ZVAL_LONG(&sleep_us, 20);
	zval usleep_params[1] = { sleep_us };

	zval active;
	ZVAL_LONG(&active, 0);
	ZVAL_MAKE_REF(&active);
	zval mh_params[2] = {*mh, active};

	zval *active_value = NULL;
	zval mrc;
	do{
		air_call_func("curl_multi_exec", 2, mh_params, &mrc);
		zval sel_ret;
		air_call_func("curl_multi_select", 1, mh_params, &sel_ret);
		if(Z_LVAL(sel_ret) == -1){
			air_call_func("usleep", 1, usleep_params, NULL);
		}
		zval_ptr_dtor(&sel_ret);
		active_value = Z_REFVAL(active);
	}while(Z_LVAL(mrc) == CURLM_CALL_MULTI_PERFORM || (Z_LVAL_P(active_value) && Z_LVAL(mrc)==CURLM_OK));
	ZVAL_UNREF(&active);
	zval_ptr_dtor(&active);
	zval_ptr_dtor(&mrc);
	zval_ptr_dtor(&sleep_us);

	zval *services = zend_read_property(air_curl_waiter_ce, self, ZEND_STRL("_services"), 1, NULL);
	zval *responses = zend_read_property(air_curl_waiter_ce, self, ZEND_STRL("_responses"), 1, NULL);
	zval *arr = NULL;
	ulong idx;
	zend_string *key;
	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(map), idx, key, arr){
		zval *service_id = zend_hash_index_find(Z_ARRVAL_P(arr), 0);
		zval *ch = zend_hash_index_find(Z_ARRVAL_P(arr), 1);
		zval *service = zend_hash_index_find(Z_ARRVAL_P(services), Z_LVAL_P(service_id));
		zval *curl = zend_read_property(Z_OBJCE_P(service), service, ZEND_STRL("request"), 1, NULL);

		zval event_params;
		array_init(&event_params);
		Z_TRY_ADDREF_P(ch);
		add_next_index_zval(&event_params, ch);
		zval ch_params[1] = { *ch };
		zval curl_errno;
		air_call_func("curl_errno", 1, ch_params, &curl_errno);
		zval event;
		if(Z_LVAL(curl_errno)){
			ZVAL_STRING(&event, "error");
		}else{
			ZVAL_STRING(&event, "success");
			zval result;
			air_call_func("curl_multi_getcontent", 1, ch_params, &result);
			add_next_index_zval(&event_params, &result);
		}
		zval_ptr_dtor(&curl_errno);
		zval trigger_params[2] = { event, event_params};
		zval result;
		ZVAL_UNDEF(&result);
		air_call_object_method(curl, air_curl_ce, "trigger", &result, 2, trigger_params);
		if(!Z_ISUNDEF(result)){
			add_index_zval(responses, Z_LVAL_P(service_id), &result);
		}else{
			zval_ptr_dtor(&result);
			php_error(E_WARNING, "error on trigger event %s", Z_STRVAL(event));
		}
		zval_ptr_dtor(&event);
		zval_ptr_dtor(&event_params);

		zval rm_params[2] = {*mh, *ch};
		air_call_func("curl_multi_remove_handle", 2, rm_params, NULL);
		zend_hash_index_del(Z_ARRVAL_P(services), Z_LVAL_P(service_id));
	}ZEND_HASH_FOREACH_END();
}

/* {{{ ARG_INFO */
/*
ZEND_BEGIN_ARG_INFO_EX(air_curl_waiter_construct_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, config)
ZEND_END_ARG_INFO()
*/
/* }}} */

/* {{{ PHP METHODS */

PHP_METHOD(air_curl_waiter, _response) {
	AIR_INIT_THIS;
	zval *mh = zend_read_property(air_curl_waiter_ce, self, ZEND_STRL("_mh"), 1, NULL);
	if(!mh || Z_TYPE_P(mh) == IS_NULL){
		zval _mh;
		air_call_func("curl_multi_init", 0, NULL, &_mh);
		zend_update_property(air_curl_waiter_ce, self, ZEND_STRL("_mh"), &_mh);
		zval_ptr_dtor(&_mh);
		mh = zend_read_property(air_curl_waiter_ce, self, ZEND_STRL("_mh"), 1, NULL);
	}
	zval map;
	array_init(&map);
	air_curl_waiter_build_multi_curl(self, &map);
	air_curl_waiter_select(self, &map);
	zval_ptr_dtor(&map);
}

PHP_METHOD(air_curl_waiter, __destruct) {
	AIR_INIT_THIS;
	zval *mh = zend_read_property(air_curl_waiter_ce, self, ZEND_STRL("_mh"), 1, NULL);
	if(mh){
		zval close_params[1] = {*mh};
		air_call_func("curl_multi_close", 1, close_params, NULL);
	}
}
/* }}} */

/* {{{ air_curl_waiter_methods */
zend_function_entry air_curl_waiter_methods[] = {
	PHP_ME(air_curl_waiter, _response, NULL,  ZEND_ACC_PUBLIC)
	PHP_ME(air_curl_waiter, __destruct, NULL,  ZEND_ACC_PUBLIC | ZEND_ACC_DTOR)
	{NULL, NULL, NULL}
};
/* }}} */

/* {{{ AIR_MINIT_FUNCTION */
AIR_MINIT_FUNCTION(air_curl_waiter) {
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "air\\curl\\waiter", air_curl_waiter_methods);

	air_curl_waiter_ce = zend_register_internal_class_ex(&ce, air_async_waiter_ce);
	zend_declare_property_null(air_curl_waiter_ce, ZEND_STRL("_mh"), ZEND_ACC_PROTECTED);
	return SUCCESS;
}
/* }}} */

