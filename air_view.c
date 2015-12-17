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

#include "main/php_output.h"
#include "ext/standard/php_smart_str.h"

#include "php_air.h"

#include "air_config.h"
#include "air_loader.h"
#include "air_view.h"

zend_class_entry *air_view_ce;

/* {{{ ARG_INFO */
ZEND_BEGIN_ARG_INFO_EX(air_view_set_config_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, config)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_view_render_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, tpl)
	ZEND_ARG_INFO(0, ret_res)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_view_assign_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, o1)
	ZEND_ARG_INFO(0, o2)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ PHP METHODS */
PHP_METHOD(air_view, __construct) {
	AIR_INIT_THIS;
	zval *data;
	MAKE_STD_ZVAL(data);
	array_init(data);
	zend_update_property(air_view_ce, self, ZEND_STRL("_data"), data TSRMLS_CC);
	zval_ptr_dtor(&data);
}

PHP_METHOD(air_view, assign) {
	AIR_INIT_THIS;

	int argc = ZEND_NUM_ARGS();
	zval *data = zend_read_property(air_view_ce, self, ZEND_STRL("_data"), 0 TSRMLS_CC);
	if(argc == 1){
		zval *arr;
		if( zend_parse_parameters(argc TSRMLS_CC, "a", &arr) == FAILURE ){
			AIR_NEW_EXCEPTION(1, "param 1 must be an array when only 1 param passed in");
		}
		if(Z_TYPE_P(arr) == IS_ARRAY){
			zend_hash_copy(Z_ARRVAL_P(data), Z_ARRVAL_P(arr), (copy_ctor_func_t) zval_add_ref, NULL, sizeof(zval *));
		}
	}else{
		char *key;
		int key_len = 0;
		zval *val;
		if( zend_parse_parameters(argc TSRMLS_CC, "sz", &key, &key_len, &val) == FAILURE ){
			AIR_NEW_EXCEPTION(1, "invalid params");
		}
		Z_ADDREF_P(val);
		zend_hash_update(Z_ARRVAL_P(data), key, key_len+1, &val, sizeof(zval *), NULL);
	}

	AIR_RET_THIS;
}

PHP_METHOD(air_view, render){
	AIR_INIT_THIS;

	char *tpl_str;
	int tpl_len = 0;
	zend_bool ret_res = 0;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|b", &tpl_str, &tpl_len, &ret_res) == FAILURE)
	{
		RETURN_FALSE;
	}

	smart_str ss_path = {0};
	if(tpl_str[0] != '/'){
		zval *root_path = NULL;
		MAKE_STD_ZVAL(root_path);
		if(zend_get_constant(ZEND_STRL("ROOT_PATH"), root_path) == FAILURE){
			php_error_docref(NULL TSRMLS_CC, E_ERROR,  "ROOT_PATH not defined");
		}
		smart_str_appendl(&ss_path, Z_STRVAL_P(root_path), Z_STRLEN_P(root_path));
		smart_str_appendc(&ss_path, '/');
		if(root_path != NULL){
			zval_ptr_dtor(&root_path);
		}

		zval *tmp = NULL;
		zval **tmp_pp;
		zval *config = zend_read_property(air_view_ce, getThis(), ZEND_STRL("_config"), 0 TSRMLS_CC);
		if(config != NULL && Z_TYPE_P(config) == IS_ARRAY
				&& zend_hash_find(Z_ARRVAL_P(config), ZEND_STRL("path"), (void **)&tmp_pp) == SUCCESS){
			smart_str_appendl(&ss_path, Z_STRVAL_PP(tmp_pp), Z_STRLEN_PP(tmp_pp));
		}else{
			zval *app_conf;
			zval *view_conf;
			if(air_config_get(NULL, ZEND_STRS("app"), &app_conf TSRMLS_CC) == FAILURE){
			}
			if(air_config_get(NULL, ZEND_STRS("app"), &app_conf TSRMLS_CC) == FAILURE){
				AIR_NEW_EXCEPTION(1, "@error config: app");
			}
			zval *app_path = NULL;
			if(air_config_get(app_conf, ZEND_STRS("path"), &app_path) == FAILURE){
				AIR_NEW_EXCEPTION(1, "@error config: app.path");
			}
			zval *view_path = NULL;
			if(air_config_get_path(app_conf, ZEND_STRS("view.path"), &view_path) == FAILURE){
				AIR_NEW_EXCEPTION(1, "@view config not found");
			}
			smart_str_appendl(&ss_path, Z_STRVAL_P(app_path), Z_STRLEN_P(app_path));
			smart_str_appendc(&ss_path, '/');
			smart_str_appendl(&ss_path, Z_STRVAL_P(view_path), Z_STRLEN_P(view_path));
		}
		smart_str_appendc(&ss_path, '/');
	}
	smart_str_appendl(&ss_path, tpl_str, tpl_len);
	smart_str_0(&ss_path);

	//构造运行时所需基本变量
	HashTable *origin_symbol_table = NULL;
	zval *output_handler = NULL;
	long chunk_size = 0;
	long flags = PHP_OUTPUT_HANDLER_STDFLAGS;
	zval *view_ret = NULL;

	//尝试缓存当前符号表
	if(EG(active_symbol_table)){
		origin_symbol_table = EG(active_symbol_table);
	}

	if (ret_res) {
		MAKE_STD_ZVAL(view_ret);
		if(php_output_start_user(output_handler, chunk_size, flags TSRMLS_CC) == FAILURE)
		{
			php_error_docref("ref.outcontrol" TSRMLS_CC, E_NOTICE, "failed to create buffer");
			RETURN_FALSE;
		}
	}
	ALLOC_HASHTABLE(EG(active_symbol_table));
	zval *data = zend_read_property(air_view_ce, getThis(), ZEND_STRL("_data"), 0 TSRMLS_CC);
	zend_hash_init(EG(active_symbol_table), 0, NULL, ZVAL_PTR_DTOR, 0);
	//将当前的模板变量放到符号表去
	ZEND_SET_SYMBOL_WITH_LENGTH(EG(active_symbol_table), "var", 4, data, Z_REFCOUNT_P(data) + 1, PZVAL_IS_REF(data));
	if(air_loader_include_file(ss_path.c TSRMLS_CC) == FAILURE){
		air_throw_exception_ex(1, "tpl %s render failed!\n", ss_path.c);
		return ;
	}

	if(ret_res){
		php_output_get_contents(view_ret TSRMLS_CC);
		php_output_discard(TSRMLS_C);
		RETVAL_ZVAL(view_ret, 1, 0);
		zval_ptr_dtor(&view_ret);
	}

	zend_hash_destroy(EG(active_symbol_table));
	FREE_HASHTABLE(EG(active_symbol_table));
	EG(active_symbol_table) = origin_symbol_table;

	smart_str_free(&ss_path);
}
PHP_METHOD(air_view, set_config) {
	AIR_INIT_THIS;

	zval *config;
	if( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &config) == FAILURE )
	{
		php_error(E_ERROR, "error view config");
	}
	zend_update_property(air_view_ce, self, ZEND_STRL("_config"), config TSRMLS_CC);

	AIR_RET_THIS;
}
/* }}} */

/* {{{ air_view_methods */
zend_function_entry air_view_methods[] = {
	PHP_ME(air_view, __construct, NULL,  ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(air_view, set_config, air_view_set_config_arginfo,  ZEND_ACC_PUBLIC)
	PHP_ME(air_view, render, air_view_render_arginfo,  ZEND_ACC_PUBLIC)
	PHP_ME(air_view, assign, air_view_assign_arginfo,  ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};
/* }}} */

/* {{{ AIR_MINIT_FUNCTION */
AIR_MINIT_FUNCTION(air_view) {
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "air\\view", air_view_methods);

	air_view_ce = zend_register_internal_class_ex(&ce, NULL, NULL TSRMLS_CC);

	zend_declare_property_null(air_view_ce, ZEND_STRL("_config"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(air_view_ce, ZEND_STRL("_data"), ZEND_ACC_PROTECTED TSRMLS_CC);
	return SUCCESS;
}
/* }}} */

