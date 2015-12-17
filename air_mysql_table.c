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

#include "air_exception.h"
#include "air_mysql_builder.h"
#include "air_mysql_table.h"

zend_class_entry *air_mysql_table_ce;

zval *air_mysql_table_get_builder(zval *self){
	zval *config = zend_read_property(air_mysql_table_ce, self, ZEND_STRL("_config"), 1 TSRMLS_CC);
	zval *db = zend_read_property(air_mysql_table_ce, self, ZEND_STRL("_db"), 1 TSRMLS_CC);
	zval *table = zend_read_property(air_mysql_table_ce, self, ZEND_STRL("_table"), 1 TSRMLS_CC);
	if(Z_TYPE_P(db) == IS_NULL || Z_TYPE_P(table) == IS_NULL){
		AIR_NEW_EXCEPTION(1, "error air\\mysql\\table");
	}
	char *str;
	int len = spprintf(&str, 0, "`%s`.`%s`", Z_STRVAL_P(db), Z_STRVAL_P(table));
	zval *db_table;
	MAKE_STD_ZVAL(db_table);
	ZVAL_STRINGL(db_table, str, len, 1);
	zval **params[2] = {&config, &db_table};
	zval *builder = air_new_object(ZEND_STRL("air\\mysql\\builder"));
	air_call_method(&builder, air_mysql_builder_ce, NULL, ZEND_STRL("__construct"), NULL, 2, params);
	zval_ptr_dtor(&db_table);
	efree(str);
	return builder;
}

/* {{{ ARG_INFO */
ZEND_BEGIN_ARG_INFO_EX(air_mysql_table_construct_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, config)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(air_mysql_table_get_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, fields)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(air_mysql_table_set_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ PHP METHODS */
PHP_METHOD(air_mysql_table, __construct) {
}

PHP_METHOD(air_mysql_table, async) {
	AIR_INIT_THIS;
	zval *builder = air_mysql_table_get_builder(self);
	air_call_method(&builder, air_mysql_builder_ce, NULL, ZEND_STRL("async"), NULL, 0, NULL);
	RETURN_ZVAL(builder, 1, 1);
}

PHP_METHOD(air_mysql_table, add) {
	AIR_INIT_THIS;
	zval *data;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &data) == FAILURE){
		return ;
	}
	zval **params[1] = {&data};
	zval *builder = air_mysql_table_get_builder(self);
	air_call_method(&builder, air_mysql_builder_ce, NULL, ZEND_STRL("add"), NULL, 1, params);
	RETURN_ZVAL(builder, 1, 1);
}

PHP_METHOD(air_mysql_table, get) {
	AIR_INIT_THIS;
	zval *fields;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &fields) == FAILURE){
		return ;
	}
	zval **params[1] = {&fields};
	zval *builder = air_mysql_table_get_builder(self);
	air_call_method(&builder, air_mysql_builder_ce, NULL, ZEND_STRL("get"), NULL, 1, params);
	RETURN_ZVAL(builder, 1, 1);
}

PHP_METHOD(air_mysql_table, set) {
	AIR_INIT_THIS;
	zval *data;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &data) == FAILURE){
		return ;
	}
	zval **params[1] = {&data};
	zval *builder = air_mysql_table_get_builder(self);
	air_call_method(&builder, air_mysql_builder_ce, NULL, ZEND_STRL("set"), NULL, 1, params);
	RETURN_ZVAL(builder, 1, 1);
}

PHP_METHOD(air_mysql_table, del) {
	AIR_INIT_THIS;
	zval *builder = air_mysql_table_get_builder(self);
	air_call_method(&builder, air_mysql_builder_ce, NULL, ZEND_STRL("del"), NULL, 0, NULL);
	RETURN_ZVAL(builder, 1, 1);
}

PHP_METHOD(air_mysql_table, __destruct) {
}

/* }}} */

/* {{{ air_mysql_table_methods */
zend_function_entry air_mysql_table_methods[] = {
	PHP_ME(air_mysql_table, __construct, NULL,  ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(air_mysql_table, async, NULL,  ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql_table, add, air_mysql_table_set_arginfo,  ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql_table, get, air_mysql_table_get_arginfo,  ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql_table, set, air_mysql_table_set_arginfo,  ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql_table, del, NULL,  ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql_table, __destruct, NULL,  ZEND_ACC_PUBLIC | ZEND_ACC_DTOR)
	{NULL, NULL, NULL}
};
/* }}} */

/* {{{ AIR_MINIT_FUNCTION */
AIR_MINIT_FUNCTION(air_mysql_table) {
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "air\\mysql\\table", air_mysql_table_methods);

	air_mysql_table_ce = zend_register_internal_class_ex(&ce, NULL, NULL TSRMLS_CC);
	zend_declare_property_null(air_mysql_table_ce, ZEND_STRL("_config"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(air_mysql_table_ce, ZEND_STRL("_db"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(air_mysql_table_ce, ZEND_STRL("_table"), ZEND_ACC_PROTECTED TSRMLS_CC);
	return SUCCESS;
}
/* }}} */

