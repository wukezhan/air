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
#include "Zend/zend_smart_str.h"

#include "main/php_output.h"

#include "php_air.h"

#include "src/air_config.h"
#include "src/air_loader.h"
#include "src/air_view.h"

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
	zval data;
	array_init(&data);
	zend_update_property(air_view_ce, self, ZEND_STRL("_data"), &data);
	zval_ptr_dtor(&data);
}

PHP_METHOD(air_view, assign) {
	AIR_INIT_THIS;

	int argc = ZEND_NUM_ARGS();
	zval *data = zend_read_property(air_view_ce, self, ZEND_STRL("_data"), 0, NULL);
	if(argc == 1){
		zval *arr = NULL;
		if( zend_parse_parameters(argc, "a", &arr) == FAILURE ){
			AIR_NEW_EXCEPTION(1, "param 1 must be an array when only 1 param passed in");
		}
		if(Z_TYPE_P(arr) == IS_ARRAY){
			zend_hash_copy(Z_ARRVAL_P(data), Z_ARRVAL_P(arr), (copy_ctor_func_t) zval_add_ref);
		}
	}else{
		zend_string *key = NULL;
		zval *val;
		if( zend_parse_parameters(argc, "Sz", &key, &val) == FAILURE ){
			AIR_NEW_EXCEPTION(1, "invalid params");
		}
		Z_TRY_ADDREF_P(val);
		zend_hash_update(Z_ARRVAL_P(data), key, val);
	}

	AIR_RET_THIS;
}

PHP_METHOD(air_view, render){
	AIR_INIT_THIS;

	zend_string *tpl_path = NULL;
	zend_bool ret_res = 0;
	if(zend_parse_parameters(ZEND_NUM_ARGS(), "S|b", &tpl_path, &ret_res) == FAILURE)
	{
		RETURN_FALSE;
	}

	smart_str ss_path = {0};
	if(ZSTR_VAL(tpl_path)[0] != '/'){
		zval *root_path = NULL;
		if((root_path = zend_get_constant_str(ZEND_STRL("ROOT_PATH"))) == NULL){
			php_error_docref(NULL, E_ERROR,  "ROOT_PATH not defined");
		}
		smart_str_appendl(&ss_path, Z_STRVAL_P(root_path), Z_STRLEN_P(root_path));
		smart_str_appendc(&ss_path, '/');

		zval *config = zend_read_property(air_view_ce, self, ZEND_STRL("_config"), 0, NULL);
		if(config != NULL && Z_TYPE_P(config) == IS_ARRAY){
			zval *tmp = zend_hash_str_find(Z_ARRVAL_P(config), ZEND_STRL("path"));
			if(tmp){
				smart_str_appendl(&ss_path, Z_STRVAL_P(tmp), Z_STRLEN_P(tmp));
			}
		}else{
			zval *app_conf;
			if((app_conf = air_config_str_get(NULL, ZEND_STRL("app"))) == NULL){
				AIR_NEW_EXCEPTION(1, "@error config: app");
			}
			zval *app_path = NULL;
			if((app_path = air_config_str_get(app_conf, ZEND_STRL("path"))) == NULL){
				AIR_NEW_EXCEPTION(1, "@error config: app.path");
			}
			zval *view_path = NULL;
			if((view_path = air_config_str_path_get(app_conf, ZEND_STRL("view.path"))) == NULL){
				AIR_NEW_EXCEPTION(1, "@view config not found");
			}
			smart_str_appendl(&ss_path, Z_STRVAL_P(app_path), Z_STRLEN_P(app_path));
			smart_str_appendc(&ss_path, '/');
			smart_str_appendl(&ss_path, Z_STRVAL_P(view_path), Z_STRLEN_P(view_path));
		}
		smart_str_appendc(&ss_path, '/');
	}
	smart_str_appendl(&ss_path, ZSTR_VAL(tpl_path), ZSTR_LEN(tpl_path));
	smart_str_0(&ss_path);

	if (ret_res) {
		if(php_output_start_user(NULL, 0, PHP_OUTPUT_HANDLER_STDFLAGS) == FAILURE)
		{
			php_error_docref("ref.outcontrol", E_NOTICE, "failed to create buffer");
			smart_str_free(&ss_path);
			RETURN_FALSE;
		}
	}

	zval *data = zend_read_property(air_view_ce, self, ZEND_STRL("_data"), 0, NULL);
	if(EXPECTED(zend_set_local_var_str(ZEND_STRL("var"), data, 1) == SUCCESS)){
		Z_TRY_ADDREF_P(data);
	}

	zend_class_entry *origin_scope = EG(scope);
	EG(scope) = air_view_ce;
	if(air_loader_execute_file(ZEND_INCLUDE, ss_path.s, NULL) == FAILURE){
		EG(scope) = origin_scope;
		air_throw_exception_ex(1, "tpl %s render failed", ZSTR_VAL(ss_path.s));
		return ;
	}
	EG(scope) = origin_scope;

	if(ret_res){
		php_output_get_contents(return_value);
		php_output_discard();
	}

	smart_str_free(&ss_path);
}
PHP_METHOD(air_view, set_config) {
	AIR_INIT_THIS;

	zval *config = NULL;
	if( zend_parse_parameters(ZEND_NUM_ARGS(), "z", &config) == FAILURE )
	{
		php_error(E_ERROR, "error view config");
	}
	zend_update_property(air_view_ce, self, ZEND_STRL("_config"), config);

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

	air_view_ce = zend_register_internal_class_ex(&ce, NULL);

	zend_declare_property_null(air_view_ce, ZEND_STRL("_config"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(air_view_ce, ZEND_STRL("_data"), ZEND_ACC_PROTECTED);
	return SUCCESS;
}
/* }}} */

