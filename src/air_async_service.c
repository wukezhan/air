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
#include "src/air_async_service.h"

zend_class_entry *air_async_service_ce;

/* {{{ ARG_INFO */
ZEND_BEGIN_ARG_INFO_EX(air_async_service_construct_arginfo, 0, 0, 2)
	ZEND_ARG_INFO(0, waiter)
	ZEND_ARG_INFO(0, request)
ZEND_END_ARG_INFO()

/* }}} */

/* {{{ PHP METHODS */
PHP_METHOD(air_async_service, __construct) {
	AIR_INIT_THIS;

	zval *waiter = NULL;
	zval *request = NULL;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz", &waiter, &request) == FAILURE){
		AIR_NEW_EXCEPTION(1, "invalid __construct params");
	}
	//check if waiter is instance of air\waiter
	if(Z_TYPE_P(waiter) != IS_OBJECT || !instanceof_function(Z_OBJCE_P(waiter), air_async_waiter_ce TSRMLS_CC)) {
		air_throw_exception(1, "param waiter must be a instance of air\\waiter");
	}
	zend_update_property(air_async_service_ce, self, ZEND_STRL("_waiter"), waiter TSRMLS_CC);
	zend_update_property(air_async_service_ce, self, ZEND_STRL("_request"), request TSRMLS_CC);
	zval *__id = zend_read_static_property(air_async_service_ce, ZEND_STRL("__id"), 0 TSRMLS_CC);
	(*__id).value.lval++;
	zend_update_property_long(air_async_service_ce, self, ZEND_STRL("_id"), Z_LVAL_P(__id) TSRMLS_CC);
}

//call the service
PHP_METHOD(air_async_service, call) {
	AIR_INIT_THIS;

	zval *waiter = zend_read_property(air_async_service_ce, self, ZEND_STRL("_waiter"), 0 TSRMLS_CC);
	zval *self_id = zend_read_property(air_async_service_ce, self, ZEND_STRL("_id"), 0 TSRMLS_CC);
	zval *ret = NULL;
	zend_call_method_with_1_params(&waiter, Z_OBJCE_P(waiter), NULL, "response", &ret, self_id);
	if(ret){
		RETURN_ZVAL(ret, 1, 1);
	}else{
		RETURN_NULL();
	}
}
/* }}} */

/* {{{ air_async_service_methods */
zend_function_entry air_async_service_methods[] = {
	PHP_ME(air_async_service, __construct, air_async_service_construct_arginfo,  ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(air_async_service, call, NULL,  ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};
/* }}} */

/* {{{ AIR_MINIT_FUNCTION */
AIR_MINIT_FUNCTION(air_async_service) {
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "air\\async\\service", air_async_service_methods);

	air_async_service_ce = zend_register_internal_class_ex(&ce, NULL, NULL TSRMLS_CC);
	air_async_service_ce->ce_flags |= ZEND_ACC_FINAL_CLASS;

	zend_declare_property_long(air_async_service_ce, ZEND_STRL("__id"), 0, ZEND_ACC_PROTECTED | ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_null(air_async_service_ce, ZEND_STRL("_id"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(air_async_service_ce, ZEND_STRL("_waiter"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(air_async_service_ce, ZEND_STRL("_request"), ZEND_ACC_PUBLIC TSRMLS_CC);
	return SUCCESS;
}
/* }}} */

