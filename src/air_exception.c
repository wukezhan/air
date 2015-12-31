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

#include "Zend/zend_exceptions.h"

#include "php_air.h"

#include "src/air_exception.h"

zend_class_entry *air_exception_ce;

zend_object *air_throw_exception_ex(long code, const char *format, ...){
	va_list arg;
	char *message;
	zend_object *ae;
	uint len;

	va_start(arg, format);
	len = vspprintf(&message, 0, format, arg);
	va_end(arg);

	ae = zend_throw_exception(air_exception_ce, message, code);
	efree(message);
	return ae;
}
zend_object *air_throw_exception(long code, const char* message){
	return zend_throw_exception(air_exception_ce, message, code);
}

/* {{{ ARG_INFO */
//ZEND_BEGIN_ARG_INFO_EX(air_exception_construct_arginfo, 0, 0, 1)
//	ZEND_ARG_INFO(0, config)
//ZEND_END_ARG_INFO()

/* }}} */

/* {{{ PHP METHODS */
/* }}} */

/* {{{ air_exception_methods */
zend_function_entry air_exception_methods[] = {
	//PHP_ME(air_exception, __construct, air_exception_construct_arginfo,  ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	{NULL, NULL, NULL}
};
/* }}} */

/* {{{ AIR_MINIT_FUNCTION */
AIR_MINIT_FUNCTION(air_exception) {
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "air\\exception", air_exception_methods);

	air_exception_ce = zend_register_internal_class_ex(&ce, zend_exception_get_default(TSRMLS_C));
	//air_exception_ce->ce_flags |= ZEND_ACC_FINAL_CLASS;
	return SUCCESS;
}
/* }}} */

