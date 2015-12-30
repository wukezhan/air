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
#include "Zend/zend_hash.h"
#include "php_air.h"

#include "air_async_service.h"
#include "air_async_waiter.h"
#include "air_exception.h"
#include "air_mysqli.h"
#include "air_mysql_builder.h"
#include "air_mysql_keeper.h"
#include "air_mysql_waiter.h"

zend_class_entry *air_mysql_waiter_ce;

void air_mysql_waiter_pool_init(zval **services, zval *wait_pool, zval *m2s){
	zend_class_entry *mysqli_ce = air_get_ce(ZEND_STRL("mysqli"));
	zval async;
	ZVAL_LONG(&async, MYSQLI_ASYNC);

	zval *service;
	ulong idx;
	zend_string *key;
	zval *mysqli = NULL;
	HashTable *ah = Z_ARRVAL_P(*services);
	for(zend_hash_internal_pointer_reset(ah);
			zend_hash_has_more_elements(ah) == SUCCESS;){
		if ((service = zend_hash_get_current_data(ah)) == NULL) {
			continue;
		}
		if(zend_hash_get_current_key(ah, &key, &idx) != HASH_KEY_IS_STRING) {
			key = NULL;
		}
		if(Z_TYPE_P(service) == IS_NULL){
			zend_hash_index_del(ah, idx);
			continue;
		}
		zval *service_id = zend_read_property(air_async_service_ce, service, ZEND_STRL("_id"), 1, NULL);
		zval *builder = zend_read_property(air_async_service_ce, service, ZEND_STRL("request"), 1, NULL);
		zval *status = zend_read_property(air_mysql_builder_ce, builder, ZEND_STRL("_status"), 0, NULL);
		if(Z_LVAL_P(status)){
			//ignore if the builder's been executed
			zend_hash_index_del(ah, idx);
			continue;
		}
		zval *builder_config = zend_read_property(air_mysql_builder_ce, builder, ZEND_STRL("_config"), 1, NULL);
		zval *mode = zend_read_property(air_mysql_builder_ce, builder, ZEND_STRL("_mode"), 1, NULL);
		if(Z_TYPE_P(mode) == IS_NULL){
			air_mysql_builder_auto_mode(builder);
			mode = zend_read_property(air_mysql_builder_ce, builder, ZEND_STRL("_mode"), 1, NULL);
		}
		zval factory_params[2] = {*builder_config, *mode};
		zval mysqli;
		air_call_static_method(air_mysql_keeper_ce, "factory", &mysqli, 2, factory_params);
		if(Z_TYPE(mysqli) != IS_NULL){
			zval build_params[1] = {mysqli};
			zval sql;
			air_call_object_method(builder, air_mysql_builder_ce, "build", &sql, 1, build_params);
			if(!Z_ISUNDEF(sql) && !Z_ISUNDEF(sql)){
				zval query_params[2] = {sql, async};
				air_call_object_method(&mysqli, mysqli_ce, "query", NULL, 2, query_params);
				Z_TRY_ADDREF_P(&mysqli);
				add_next_index_zval(wait_pool, &mysqli);
				Z_TRY_ADDREF_P(service_id);
				add_index_zval(m2s, air_mysqli_get_id(&mysqli), service_id);
				zval_ptr_dtor(&sql);
			}else{
				//should not happen
				php_error(E_ERROR, "sql not found");
			}
		}
		zval_ptr_dtor(&mysqli);
		zend_hash_move_forward(ah);
	}
	zval_ptr_dtor(&async);
}

void air_mysql_waiter_pool_poll(zval **responses_pp, zval **services_pp, zval *wait_pool, zval *m2s){
	zval *responses = *responses_pp;
	zval *services = *services_pp;
	int wait_pool_size = zend_hash_num_elements(Z_ARRVAL_P(wait_pool));
	int processed = 0;
	zval timeout;
	ZVAL_LONG(&timeout, 2);
	zend_class_entry *mysqli_ce = air_get_ce(ZEND_STRL("mysqli"));
	while(processed < wait_pool_size){
		zval *mysqli;
		zval errors, reads, rejects;
		ZVAL_COPY(&errors, wait_pool);
		ZVAL_MAKE_REF(&errors);
		ZVAL_COPY(&reads, wait_pool);
		ZVAL_MAKE_REF(&reads);
		ZVAL_COPY(&rejects, wait_pool);
		ZVAL_MAKE_REF(&rejects);
		zval poll_params[4] = {reads, errors, rejects, timeout};
		zval count;
		air_call_static_method(mysqli_ce, "poll", &count, 4, poll_params);
		if(Z_LVAL(count)){
			ulong idx;
			zend_string *key;
			int key_len;
			zval *tmp = Z_REFVAL(reads);
			ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(tmp), idx, key, mysqli){
				zval *service_id = zend_hash_index_find(Z_ARRVAL_P(m2s), air_mysqli_get_id(mysqli));
				zval *service = zend_hash_index_find(Z_ARRVAL_P(services), Z_LVAL_P(service_id));
				zval *builder = zend_read_property(air_async_service_ce, service, ZEND_STRL("request"), 0, NULL);
				zval trigger_params[2];
				zval event;
				zval event_params;
				array_init(&event_params);
				Z_ADDREF_P(mysqli);
				add_next_index_zval(&event_params, mysqli);
				zval mysqli_result;
				if(air_mysqli_get_errno(mysqli)){
					ZVAL_STRING(&event, "error");
				}else{
					ZVAL_STRING(&event, "success");
					air_call_object_method(mysqli, mysqli_ce, "reap_async_query", &mysqli_result, 0, NULL);
					Z_ADDREF_P(&mysqli_result);
					add_next_index_zval(&event_params, &mysqli_result);
				}
				trigger_params[0] = event;
				trigger_params[1] = event_params;
				zval results;
				air_call_object_method(builder, air_mysql_builder_ce, "trigger", &results, 2, trigger_params);
				if(!Z_ISUNDEF(results)){
					add_index_zval(responses, Z_LVAL_P(service_id), &results);
				}else{
					php_error(E_WARNING, "error on trigger event %s with no results", Z_STRVAL(event));
				}
				zend_hash_index_del(Z_ARRVAL_P(services), Z_LVAL_P(service_id));
				zval_ptr_dtor(&event);
				zval_ptr_dtor(&event_params);
				zval_ptr_dtor(&mysqli_result);
				zval *builder_config = zend_read_property(air_mysql_builder_ce, builder, ZEND_STRL("_config"), 0, NULL);
				zval *mode = zend_read_property(air_mysql_builder_ce, builder, ZEND_STRL("_mode"), 0, NULL);
				zval release_params[3] = {*mysqli, *builder_config, *mode};
				air_call_static_method(air_mysql_keeper_ce, "release", NULL, 3, release_params);
			}ZEND_HASH_FOREACH_END();
			processed += Z_LVAL(count);
		}
		zval_ptr_dtor(&errors);
		zval_ptr_dtor(&reads);
		zval_ptr_dtor(&rejects);
		zval_ptr_dtor(&count);
	}
	zval_ptr_dtor(&timeout);
}

/* {{{ ARG_INFO */

/* }}} */

/* {{{ PHP METHODS */

PHP_METHOD(air_mysql_waiter, _response) {
	AIR_INIT_THIS;
	zval *services = zend_read_property(Z_OBJCE_P(self), self, ZEND_STRL("_services"), 0, NULL);
	zval *responses = zend_read_property(Z_OBJCE_P(self), self, ZEND_STRL("_responses"), 0, NULL);
	int cnt;

	while(cnt = zend_hash_num_elements(Z_ARRVAL_P(services))){
		zval wait_pool, m2s;
		array_init(&wait_pool);
		array_init(&m2s);
		air_mysql_waiter_pool_init(&services, &wait_pool, &m2s);
		air_mysql_waiter_pool_poll(&responses, &services, &wait_pool, &m2s);
		zval_ptr_dtor(&wait_pool);
		zval_ptr_dtor(&m2s);
	}
}

/* }}} */

/* {{{ air_mysql_waiter_methods */
zend_function_entry air_mysql_waiter_methods[] = {
	PHP_ME(air_mysql_waiter, _response, NULL,  ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};
/* }}} */

/* {{{ AIR_MINIT_FUNCTION */
AIR_MINIT_FUNCTION(air_mysql_waiter) {
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "air\\mysql\\waiter", air_mysql_waiter_methods);

	air_mysql_waiter_ce = zend_register_internal_class_ex(&ce, air_async_waiter_ce);
	return SUCCESS;
}
/* }}} */

