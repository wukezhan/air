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

#include "src/air_async_scheduler.h"
#include "src/air_async_service.h"
#include "src/air_async_waiter.h"

zend_class_entry *air_async_waiter_ce;

/* {{{ ARG_INFO */
ZEND_BEGIN_ARG_INFO_EX(air_async_waiter_construct_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, config)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_async_waiter_serve_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, request)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_async_waiter_resp_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, service)
ZEND_END_ARG_INFO()

/* }}} */

/* {{{ PHP METHODS */
PHP_METHOD(air_async_waiter, __construct) {
	AIR_INIT_THIS;

	zval *services;
	MAKE_STD_ZVAL(services);
	array_init(services);
	zend_update_property(air_async_waiter_ce, self, ZEND_STRL("_services"), services TSRMLS_CC);
	zval_ptr_dtor(&services);

	zval *responses;
	MAKE_STD_ZVAL(responses);
	array_init(responses);
	zend_update_property(air_async_waiter_ce, self, ZEND_STRL("_responses"), responses TSRMLS_CC);
	zval_ptr_dtor(&responses);

	zval *context;
	MAKE_STD_ZVAL(context);
	array_init(context);
	add_assoc_long(context, "step", 0);
	zend_update_property(air_async_waiter_ce, self, ZEND_STRL("_context"), context TSRMLS_CC);
	zval_ptr_dtor(&context);
}

PHP_METHOD(air_async_waiter, serve) {
	AIR_INIT_THIS;
	zval *request = NULL;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &request) == FAILURE){
		AIR_NEW_EXCEPTION(1, "invalid serve param");
	}

	zval *service = NULL;
	MAKE_STD_ZVAL(service);
	object_init_ex(service, air_async_service_ce);
	zend_call_method_with_2_params(&service, air_async_service_ce, NULL, "__construct", NULL, self, request);
	zval *services = zend_read_property(Z_OBJCE_P(self), self, ZEND_STRL("_services"), 0 TSRMLS_CC);
	zval *service_id = zend_read_property(Z_OBJCE_P(service), service, ZEND_STRL("_id"), 1 TSRMLS_CC);
	add_index_zval(services, Z_LVAL_P(service_id), service);
	RETURN_ZVAL(service, 1, 0);
}

PHP_METHOD(air_async_waiter, acquire) {
	zval *class_name;
	MAKE_STD_ZVAL(class_name);
	ZVAL_STRING(class_name, EG(called_scope)->name, 0);
	zval **params[1] = {&class_name};
	zval *ret;
	air_call_static_method(air_async_scheduler_ce, "acquire", &ret, 1, params);
	zval_ptr_dtor(&class_name);
	RETURN_ZVAL(ret, 1, 1);
}

PHP_METHOD(air_async_waiter, response) {
	long id = 0;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &id) == FAILURE){
		AIR_NEW_EXCEPTION(1, "invalid response param");
	}
	AIR_INIT_THIS;
	zval *responses = zend_read_property(air_async_waiter_ce, self, ZEND_STRL("_responses"), 0 TSRMLS_CC);
	zval *data = air_arr_idx_find(responses, id);
	if(!data){
		zval *services = zend_read_property(air_async_waiter_ce, self, ZEND_STRL("_services"), 0 TSRMLS_CC);
		zval *service = air_arr_idx_find(services, id);
		if(service){
			air_call_static_method(air_async_scheduler_ce, "loop", NULL, 0, NULL);
			data = air_arr_idx_find(responses, id);
		}
	}
	if(data){
		RETURN_ZVAL(data, 1, 0);
	}
	RETURN_NULL();
}
/* }}} */

/* {{{ air_async_waiter_methods */
zend_function_entry air_async_waiter_methods[] = {
	PHP_ME(air_async_waiter, __construct, air_async_waiter_construct_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(air_async_waiter, serve, air_async_waiter_serve_arginfo, ZEND_ACC_PUBLIC)
	//PHP_ME(air_async_waiter, _response, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(air_async_waiter, acquire, NULL, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(air_async_waiter, response, air_async_waiter_resp_arginfo, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};
/* }}} */

/* {{{ AIR_MINIT_FUNCTION */
AIR_MINIT_FUNCTION(air_async_waiter) {
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "air\\async\\waiter", air_async_waiter_methods);

	air_async_waiter_ce = zend_register_internal_class_ex(&ce, NULL, NULL TSRMLS_CC);

	zend_declare_property_null(air_async_waiter_ce, ZEND_STRL("_services"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(air_async_waiter_ce, ZEND_STRL("_responses"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(air_async_waiter_ce, ZEND_STRL("_context"), ZEND_ACC_PROTECTED TSRMLS_CC);
	return SUCCESS;
}
/* }}} */

