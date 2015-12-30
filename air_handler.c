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

#include "php_air.h"

#include "air_handler.h"

zend_class_entry *air_handler_ce;

/* {{{ ARG_INFO */
ZEND_BEGIN_ARG_INFO_EX(air_handler_construct_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, config)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_handler_on_error_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, callback)
	ZEND_ARG_INFO(0, error_type)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_handler_on_exception_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, callback)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_handler_on_shutdown_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, callback)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ PHP METHODS */
PHP_METHOD(air_handler, on_error) {
	zval params[2];
	zval *callback, *error_type = NULL;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "z|z", &callback, &error_type) == FAILURE) {
		return ;
	}
	//ZVAL_COPY(&params[0], callback);
	params[0] = *callback;
	if(error_type){
		//ZVAL_COPY(&params[1], error_type);
		params[1] = *error_type;
	}
	if(air_call_func("set_error_handler", error_type?2: 1, params, return_value) == FAILURE){
		php_error(E_ERROR, "air\\handler::on_error() call error");
	}
}

PHP_METHOD(air_handler, on_exception) {
	zval params[1];
	zval *callback;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &callback) == FAILURE) {
		return ;
	}
	params[0] = *callback;
	air_call_func("set_exception_handler", 1, params, return_value);
}

PHP_METHOD(air_handler, on_shutdown) {
	zval params[1];
	zval *callback;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &callback) == FAILURE) {
		return ;
	}
	params[0] = *callback;
	air_call_func("register_shutdown_function", 1, params, return_value);
}
/* }}} */

/* {{{ air_handler_methods */
zend_function_entry air_handler_methods[] = {
	PHP_ME(air_handler, on_error, air_handler_on_error_arginfo,  ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(air_handler, on_exception, air_handler_on_exception_arginfo,  ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(air_handler, on_shutdown, air_handler_on_shutdown_arginfo,  ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};
/* }}} */

/* {{{ AIR_MINIT_FUNCTION */
AIR_MINIT_FUNCTION(air_handler) {
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "air\\handler", air_handler_methods);

	air_handler_ce = zend_register_internal_class_ex(&ce, NULL);
	return SUCCESS;
}
/* }}} */

