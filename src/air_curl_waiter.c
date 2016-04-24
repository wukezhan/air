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

/* {{{ ARG_INFO */
/*
ZEND_BEGIN_ARG_INFO_EX(air_curl_waiter_construct_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, config)
ZEND_END_ARG_INFO()
*/
/* }}} */

/* {{{ PHP METHODS */

PHP_METHOD(air_curl_waiter, step_0) {
	AIR_INIT_THIS;
	zval *services = zend_read_property(air_curl_waiter_ce, self, ZEND_STRL("_services"), 1 TSRMLS_CC);
	zval *context = zend_read_property(air_curl_waiter_ce, self, ZEND_STRL("_context"), 1 TSRMLS_CC);
	if(!zend_hash_num_elements(Z_ARRVAL_P(services))){
		add_assoc_long(context, "step", -1);
		return ;
	}
	zval *mh = zend_read_property(air_curl_waiter_ce, self, ZEND_STRL("_mh"), 1 TSRMLS_CC);
	if(!mh || Z_TYPE_P(mh) == IS_NULL){
		zval *_mh = air_call_func("curl_multi_init", 0, NULL);
		zend_update_property(air_curl_waiter_ce, self, ZEND_STRL("_mh"), _mh TSRMLS_CC);
		zval_ptr_dtor(&_mh);
		mh = zend_read_property(air_curl_waiter_ce, self, ZEND_STRL("_mh"), 1 TSRMLS_CC);
	}
	zval *map;
	MAKE_STD_ZVAL(map);
	array_init(map);
	zval *service;
	ulong idx;
	char *key;
	int key_len;
	AIR_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(services), idx, key, key_len, service){
		zend_class_entry *service_ce = Z_OBJCE_P(service);
		zval *arr;
		MAKE_STD_ZVAL(arr);
		array_init(arr);
		zval *service_id = zend_read_property(service_ce, service, ZEND_STRL("_id"), 1 TSRMLS_CC);
		Z_ADDREF_P(service_id);
		add_next_index_zval(arr, service_id);
		zval *curl = zend_read_property(service_ce, service, ZEND_STRL("_request"), 1 TSRMLS_CC);
		air_call_object_method(&curl, Z_OBJCE_P(curl), "init", NULL, 0, NULL);
		zval *ch = zend_read_property(Z_OBJCE_P(curl), curl, ZEND_STRL("_ch"), 1 TSRMLS_CC);
		Z_ADDREF_P(ch);
		add_next_index_zval(arr, ch);
		add_next_index_zval(map, arr);
		zval *am_params[2] = {mh, ch};
		zval *tmp = air_call_func("curl_multi_add_handle", 2, am_params);
		zval_ptr_dtor(&tmp);
	}AIR_HASH_FOREACH_END();
	add_assoc_zval(context, "map", map);
	add_assoc_long(context, "step", 1);
}

PHP_METHOD(air_curl_waiter, step_1) {
	AIR_INIT_THIS;
	zval *context = zend_read_property(air_curl_waiter_ce, self, ZEND_STRL("_context"), 1 TSRMLS_CC);
	zval *mh = zend_read_property(air_curl_waiter_ce, self, ZEND_STRL("_mh"), 1 TSRMLS_CC);

	zval *mrc = NULL;
	zval *active = NULL;
	MAKE_STD_ZVAL(active);
	ZVAL_LONG(active, 0);
	Z_SET_ISREF_P(active);
	zval *mh_params[2] = {mh, active};
	mrc = air_call_func("curl_multi_exec", 2, mh_params);
	if(!(Z_LVAL_P(mrc) == CURLM_CALL_MULTI_PERFORM || (Z_LVAL_P(active) && Z_LVAL_P(mrc)==CURLM_OK))){
		add_assoc_long(context, "step", 2);
	}
	zval_ptr_dtor(&active);
	zval_ptr_dtor(&mrc);
}

PHP_METHOD(air_curl_waiter, step_2) {
	AIR_INIT_THIS;
	zval *services = zend_read_property(air_curl_waiter_ce, self, ZEND_STRL("_services"), 1 TSRMLS_CC);
	zval *responses = zend_read_property(air_curl_waiter_ce, self, ZEND_STRL("_responses"), 1 TSRMLS_CC);
	zval *mh = zend_read_property(air_curl_waiter_ce, self, ZEND_STRL("_mh"), 1 TSRMLS_CC);
	zval *context = zend_read_property(air_curl_waiter_ce, self, ZEND_STRL("_context"), 1 TSRMLS_CC);
	zval *map = air_arr_find(context, ZEND_STRS("map"));

	zval *arr;
	ulong idx;
	char *key;
	int key_len;
	AIR_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(map), idx, key, key_len, arr){
		zval *service_id = air_arr_idx_find(arr, 0);
		zval *ch = air_arr_idx_find(arr, 1);
		zval *service = air_arr_idx_find(services, Z_LVAL_P(service_id));
		zval *curl = zend_read_property(Z_OBJCE_P(service), service, ZEND_STRL("_request"), 1 TSRMLS_CC);

		zval **trigger_params[2];
		zval *event;
		MAKE_STD_ZVAL(event);
		zval *event_params;
		MAKE_STD_ZVAL(event_params);
		array_init(event_params);
		Z_ADDREF_P(ch);
		add_next_index_zval(event_params, ch);
		zval *ch_params[1] = {ch};
		zval *curl_errno = air_call_func("curl_errno", 1, ch_params);
		if(Z_LVAL_P(curl_errno)){
			ZVAL_STRING(event, "error", 1);
		}else{
			ZVAL_STRING(event, "success", 1);
			zval *result = air_call_func("curl_multi_getcontent", 1, ch_params);
			add_next_index_zval(event_params, result);
		}
		zval_ptr_dtor(&curl_errno);
		trigger_params[0] = &event;
		trigger_params[1] = &event_params;
		zval *result = NULL;
		air_call_method(&curl, air_curl_ce, NULL, ZEND_STRL("trigger"), &result, 2, trigger_params TSRMLS_CC);
		if(result){
			add_index_zval(responses, Z_LVAL_P(service_id), result);
		}else{
			php_error(E_WARNING, "error on trigger event %s", Z_STRVAL_P(event));
		}
		zval_ptr_dtor(&event);
		zval_ptr_dtor(&event_params);

		zval *rm_params[2] = {mh, ch};
		zval *rm_ret = air_call_func("curl_multi_remove_handle", 2, rm_params);
		zval_ptr_dtor(&rm_ret);
		zend_hash_index_del(Z_ARRVAL_P(services), Z_LVAL_P(service_id));
	}AIR_HASH_FOREACH_END();
	add_assoc_long(context, "step", -1);
}

PHP_METHOD(air_curl_waiter, __destruct) {
	AIR_INIT_THIS;
	zval *mh = zend_read_property(air_curl_waiter_ce, self, ZEND_STRL("_mh"), 1 TSRMLS_CC);
	if(!ZVAL_IS_NULL(mh)){
		zval *close_params[1] = {mh};
		zval *tmp = air_call_func("curl_multi_close", 1, close_params);
		zval_ptr_dtor(&tmp);
	}
}
/* }}} */

/* {{{ air_curl_waiter_methods */
zend_function_entry air_curl_waiter_methods[] = {
	PHP_ME(air_curl_waiter, step_0, NULL,  ZEND_ACC_PUBLIC)
	PHP_ME(air_curl_waiter, step_1, NULL,  ZEND_ACC_PUBLIC)
	PHP_ME(air_curl_waiter, step_2, NULL,  ZEND_ACC_PUBLIC)
	PHP_ME(air_curl_waiter, __destruct, NULL,  ZEND_ACC_PUBLIC | ZEND_ACC_DTOR)
	{NULL, NULL, NULL}
};
/* }}} */

/* {{{ AIR_MINIT_FUNCTION */
AIR_MINIT_FUNCTION(air_curl_waiter) {
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "air\\curl\\waiter", air_curl_waiter_methods);

	air_curl_waiter_ce = zend_register_internal_class_ex(&ce, air_async_waiter_ce, NULL TSRMLS_CC);
	zend_declare_property_null(air_curl_waiter_ce, ZEND_STRL("_mh"), ZEND_ACC_PROTECTED TSRMLS_CC);
	return SUCCESS;
}
/* }}} */

