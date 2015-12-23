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
#include "ext/standard/php_smart_str.h"

#include "php_air.h"

#include "air_config.h"
#include "air_loader.h"
#include "air_view.h"

#include "air_controller.h"

zend_class_entry *air_controller_ce;

int air_controller_set_meta(air_controller_t *self, smart_str controller, smart_str action TSRMLS_DC) {
	zend_update_property_stringl(air_controller_ce, self, ZEND_STRL("_c"), controller.c, controller.len TSRMLS_CC);
	zend_update_property_stringl(air_controller_ce, self, ZEND_STRL("_a"), action.c, action.len TSRMLS_CC);
	return SUCCESS;
}

int air_controller_set_route(air_controller_t *self, zval *route TSRMLS_DC) {
	zend_update_property(air_controller_ce, self, ZEND_STRL("_route"), route TSRMLS_CC);
	return SUCCESS;
}

/* {{{ ARG_INFO */
ZEND_BEGIN_ARG_INFO_EX(air_controller_assign_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

/* }}} */

/* {{{ PHP METHODS */
PHP_METHOD(air_controller, __construct) {
	AIR_INIT_THIS;

	zval *data;
	MAKE_STD_ZVAL(data);
	array_init(data);
	zend_update_property(air_controller_ce, self, ZEND_STRL("_data"), data TSRMLS_CC);
	zval_ptr_dtor(&data);
}

PHP_METHOD(air_controller, init) {
}

PHP_METHOD(air_controller, before_action) {
}

PHP_METHOD(air_controller, assign) {
	AIR_INIT_THIS;

	int argc = ZEND_NUM_ARGS();
	zval *data = zend_read_property(air_controller_ce, self, ZEND_STRL("_data"), 0 TSRMLS_CC);
	if(argc == 1){
		zval *arr;
		if( zend_parse_parameters(argc TSRMLS_CC, "z", &arr) == FAILURE ){
			AIR_NEW_EXCEPTION(1, "invalid invalid param");
		}
		if(Z_TYPE_P(arr) == IS_ARRAY){
			zend_hash_copy(Z_ARRVAL_P(data), Z_ARRVAL_P(arr), (copy_ctor_func_t) zval_add_ref, NULL, sizeof(zval *));
		}
	}else{
		char *key;
		int key_len = 0;
		zval *val;
		if( zend_parse_parameters(argc TSRMLS_CC, "sz", &key, &key_len, &val) == FAILURE ){
			AIR_NEW_EXCEPTION(1, "invalid assign param");
		}
		Z_ADDREF_P(val);
		zend_hash_update(Z_ARRVAL_P(data), key, key_len+1, &val, sizeof(zval *), NULL);
	}

	AIR_RET_THIS;
}

PHP_METHOD(air_controller, set_view) {
	AIR_INIT_THIS;

	char *view_path;
	int len = 0;
	if( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &view_path, &len) == FAILURE ){
		AIR_NEW_EXCEPTION(1, "invalid set_view param");
	}

	zend_update_property_stringl(air_controller_ce, self, ZEND_STRL("_view_path"), view_path, len);

	AIR_RET_THIS;
}

PHP_METHOD(air_controller, init_view) {
	AIR_INIT_THIS;

	zend_class_entry *view_ce = NULL;
	zval *view_conf = NULL;
	zval *ve_conf = NULL;
	if(air_config_path_get(NULL, ZEND_STRL("app.view"), &view_conf TSRMLS_CC) == FAILURE){
		AIR_NEW_EXCEPTION(1, "error config: app.view");
	}
	if(air_config_get(view_conf, ZEND_STRS("engine"), &ve_conf) == FAILURE){
		if(ve_conf == NULL){
			MAKE_STD_ZVAL(ve_conf);
		}
		ZVAL_STRINGL(ve_conf, "air\\view", 8, 1);
	}
	view_ce = air_loader_lookup_class(Z_STRVAL_P(ve_conf), Z_STRLEN_P(ve_conf));
	if (!view_ce) {
		php_error(E_ERROR, "view engine %s not found", Z_STRVAL_P(ve_conf));
		return ;
	}
	if (!instanceof_function(view_ce, air_view_ce TSRMLS_CC)) {
		AIR_NEW_EXCEPTION(1, "view engine must be a instance of air\\view");
	}

	zval *ve = NULL;
	MAKE_STD_ZVAL(ve);
	object_init_ex(ve, view_ce);
	zend_call_method_with_0_params(&ve, view_ce, NULL, "__construct", NULL);
	zval *vc_conf = NULL;
	if(air_config_get(view_conf, ZEND_STRS("config"), &vc_conf TSRMLS_CC) == SUCCESS){
		zend_call_method_with_1_params(&ve, view_ce, NULL, "set_config", NULL, vc_conf);
	}

	zend_update_property(air_controller_ce, self, ZEND_STRL("_view_engine"), ve TSRMLS_CC);
	zval_ptr_dtor(&ve);

	AIR_RET_THIS;
}

PHP_METHOD(air_controller, render_view) {
	AIR_INIT_THIS;
	zend_call_method_with_0_params(&self, air_controller_ce, NULL, "init_view", NULL);

	zval *view_path;
	MAKE_STD_ZVAL(view_path);
	zval *ret_res;
	MAKE_STD_ZVAL(ret_res);
	char *tpl_str;
	int tpl_len = 0;
	int _ret = 0;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|sl", &tpl_str, &tpl_len, &_ret) == FAILURE)
	{
		RETURN_FALSE;
	}
	if(tpl_len == 0){
		zval *_conf = NULL;
		zval *_view_path = NULL;
		zval *_view_type = NULL;
		if(air_config_path_get(NULL, ZEND_STRS("app.view"), &_conf) == FAILURE){
			AIR_NEW_EXCEPTION(1, "view config app.view not found");
		}
		if(air_config_get(_conf, ZEND_STRS("type"), &_view_type) == FAILURE){
			AIR_NEW_EXCEPTION(1, "view config app.view.type not found");
		}
		zval *route = zend_read_property(air_controller_ce, self, ZEND_STRL("_route"), 0 TSRMLS_CC);
		zval **_a, **_c;
		zend_hash_find(Z_ARRVAL_P(route), ZEND_STRS(air_c_key), (void **)&_c);
		zend_hash_find(Z_ARRVAL_P(route), ZEND_STRS(air_a_key), (void **)&_a);
		char *str;
		int len = spprintf(&str, 0, "%s%c%s%s",
				Z_STRVAL_PP(_c), '/', Z_STRVAL_PP(_a), Z_STRVAL_P(_view_type));
		ZVAL_STRINGL(view_path, str, len, 1);
		efree(str);
	}else{
		ZVAL_STRINGL(view_path, tpl_str, tpl_len, 0);
	}

	ZVAL_BOOL(ret_res, _ret);

	zval *data = zend_read_property(air_controller_ce, self, ZEND_STRL("_data"), 0 TSRMLS_CC);
	zval *view = zend_read_property(air_controller_ce, self, ZEND_STRL("_view_engine"), 0 TSRMLS_CC);

	zval *ret = NULL;
	zend_call_method_with_1_params(&view, Z_OBJCE_P(view), NULL, "assign", NULL, data);
	zend_call_method_with_2_params(&view, Z_OBJCE_P(view), NULL, "render", &ret, view_path, ret_res);
	zval_ptr_dtor(&view_path);
	zval_ptr_dtor(&ret_res);
	if(ret){
		RETURN_ZVAL(ret, 1, 1);
	}
}

PHP_METHOD(air_controller, after_action) {
}

PHP_METHOD(air_controller, __destruct) {
}

/* }}} */

/* {{{ air_controller_methods */
zend_function_entry air_controller_methods[] = {
	PHP_ME(air_controller, __construct, NULL,  ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(air_controller, init, NULL,  ZEND_ACC_PUBLIC)
	PHP_ME(air_controller, before_action, NULL,  ZEND_ACC_PUBLIC)
	PHP_ME(air_controller, assign, NULL,  ZEND_ACC_PUBLIC)
	PHP_ME(air_controller, set_view, NULL,  ZEND_ACC_PUBLIC)
	PHP_ME(air_controller, init_view, NULL,  ZEND_ACC_PUBLIC)
	PHP_ME(air_controller, render_view, NULL,  ZEND_ACC_PUBLIC)
	PHP_ME(air_controller, after_action, NULL,  ZEND_ACC_PUBLIC)
	PHP_ME(air_controller, __destruct, NULL,  ZEND_ACC_PUBLIC | ZEND_ACC_DTOR)
	{NULL, NULL, NULL}
};
/* }}} */

/* {{{ AIR_MINIT_FUNCTION */
AIR_MINIT_FUNCTION(air_controller) {
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "air\\controller", air_controller_methods);

	air_controller_ce = zend_register_internal_class_ex(&ce, NULL, NULL TSRMLS_CC);

	zend_declare_property_null(air_controller_ce, ZEND_STRL("_a"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(air_controller_ce, ZEND_STRL("_c"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(air_controller_ce, ZEND_STRL("_data"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(air_controller_ce, ZEND_STRL("_route"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(air_controller_ce, ZEND_STRL("_view_engine"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(air_controller_ce, ZEND_STRL("_view_path"), ZEND_ACC_PROTECTED TSRMLS_CC);

	return SUCCESS;
}
/* }}} */

