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

#include "air_config.h"
#include "air_exception.h"

zend_class_entry *air_config_ce;

void air_config_merge_default(zval *data TSRMLS_DC){
	zval *_data = NULL;
	MAKE_STD_ZVAL(_data);
	array_init(_data);

	zval *app = NULL;
	MAKE_STD_ZVAL(app);
	array_init(app);
	add_assoc_stringl_ex(app, ZEND_STRS("path"), ZEND_STRL("app"), 1);

	zval *exec;
	MAKE_STD_ZVAL(exec);
	array_init(exec);
	add_assoc_stringl_ex(exec, ZEND_STRS("path"), ZEND_STRL("exec"), 1);
	add_assoc_zval_ex(app, ZEND_STRS("exec"), exec);


	zval *site;
	MAKE_STD_ZVAL(site);
	array_init(site);
	add_assoc_stringl_ex(site, ZEND_STRS("path"), ZEND_STRL("site"), 1);
	add_assoc_zval_ex(app, ZEND_STRS("site"), site);

	zval *view;
	MAKE_STD_ZVAL(view);
	array_init(view);
	add_assoc_stringl_ex(view, ZEND_STRS("engine"), ZEND_STRL("air\\view"), 1);
	add_assoc_stringl_ex(view, ZEND_STRS("path"), ZEND_STRL("view"), 1);
	add_assoc_stringl_ex(view, ZEND_STRS("type"), ZEND_STRL(".php"), 1);
	add_assoc_zval_ex(app, ZEND_STRS("view"), view);

	add_assoc_zval_ex(_data, ZEND_STRS("app"), app);
	if(data != NULL && Z_TYPE_P(data) == IS_ARRAY){
		php_array_replace_recursive(Z_ARRVAL_P(_data), Z_ARRVAL_P(data) TSRMLS_CC);
	}
	zend_update_static_property(air_config_ce, ZEND_STRL("_data"), _data TSRMLS_CC);
	zval_ptr_dtor(&_data);
}

zval *air_config_get_data(TSRMLS_DC){
	zval *data = zend_read_static_property(air_config_ce, ZEND_STRL("_data"), 1 TSRMLS_CC);
	if(Z_TYPE_P(data) == IS_NULL){
		air_config_merge_default(data TSRMLS_CC);
		data = zend_read_static_property(air_config_ce, ZEND_STRL("_data"), 1 TSRMLS_CC);
	}
	return data;
}

int air_config_get(zval *data, const char *key, int key_len, zval **val TSRMLS_DC) {
	zval *_data = data;
	if(data == NULL){
		_data = air_config_get_data(TSRMLS_CC);
	}
	int status = air_arr_get(_data, key, key_len, val);
	if(status == FAILURE) {
		php_error(E_NOTICE, "config key '%s' not found\n", key);
	}
	return status;
}

int air_config_path_get(zval *data, const char *path, int path_len, zval **val TSRMLS_DC) {
	zval *_data = data;
	if(data == NULL){
		_data = air_config_get_data(TSRMLS_CC);
	}
	zval **tmp = NULL;
	char *seg = NULL, *sptr = NULL;
	char *skey = estrndup(path, path_len);
	seg = php_strtok_r(skey, ".", &sptr);
	int status = SUCCESS;
	while(seg){
		if(Z_TYPE_P(_data) != IS_ARRAY) {
			status = FAILURE;
			break;
		}
		if(zend_hash_find(Z_ARRVAL_P(_data), seg, strlen(seg)+1, (void **)&tmp) == SUCCESS) {
			SEPARATE_ZVAL(tmp);
			_data = *tmp;
		}else{
			php_error(E_NOTICE, "config path '%s' not found\n", path);
			status = FAILURE;
			break;
		}
		seg = php_strtok_r(NULL, ".", &sptr);
	}
	if(status == SUCCESS){
		*val = _data;
	}
	efree(skey);
	return status;
}

/* {{{ ARG_INFO */
ZEND_BEGIN_ARG_INFO_EX(air_config_get_arginfo, 0, 0, 0)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, default_value)
ZEND_END_ARG_INFO()
ZEND_BEGIN_ARG_INFO_EX(air_config_set_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ PHP METHODS */
PHP_METHOD(air_config, get) {
	zval *data = air_config_get_data();
	char *key;
	int key_len = 0;
	zval *val = NULL;
	zval *def_val = NULL;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|sz", &key, &key_len, &def_val) == FAILURE)
	{
	}
	if(!key_len){
		RETURN_ZVAL(data, 1, 0);
	}
	if(air_arr_get(data, key, key_len, &val TSRMLS_CC) == FAILURE) {
		if(def_val){
			RETURN_ZVAL(def_val, 1, 0);
		}else{
			RETURN_NULL();
		}
	}else{
		RETURN_ZVAL(val, 1, 0);
	}
}

PHP_METHOD(air_config, path_get) {
	char *key;
	int key_len = 0;
	zval *val = NULL;
	zval *def_val = NULL;
	if( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|sz", &key, &key_len, &def_val) == FAILURE )
	{
		AIR_NEW_EXCEPTION(1, "invalid path_get params");
	}
	if(!key_len){
		val = air_config_get_data();
		RETURN_ZVAL(val, 1, 0);
	}
	if(air_config_path_get(NULL, key, key_len, &val TSRMLS_CC) == FAILURE) {
		if(def_val){
			RETURN_ZVAL(def_val, 1, 0);
		}
	}else{
		RETURN_ZVAL(val, 1, 0);
	}
}

PHP_METHOD(air_config, set) {
	zval *data;
	data = zend_read_static_property(air_config_ce, ZEND_STRL("_data"), 1 TSRMLS_CC);
	if(!ZVAL_IS_NULL(data)){
		AIR_NEW_EXCEPTION(1, "config can only be set once");
	}
	if( zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &data) == FAILURE )
	{
	}
	air_config_merge_default(data TSRMLS_CC);
}
/* }}} */

/* {{{ air_config_methods */
zend_function_entry air_config_methods[] = {
	PHP_ME(air_config, get,	air_config_get_arginfo,  ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(air_config, path_get,	air_config_get_arginfo,  ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(air_config, set,	air_config_set_arginfo,  ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};
/* }}} */

/* {{{ AIR_MINIT_FUNCTION */
AIR_MINIT_FUNCTION(air_config) {
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "air\\config", air_config_methods);

	air_config_ce = zend_register_internal_class_ex(&ce, NULL, NULL TSRMLS_CC);
	air_config_ce->ce_flags |= ZEND_ACC_FINAL_CLASS;

	zend_declare_property_null(air_config_ce, ZEND_STRL("_data"), ZEND_ACC_PROTECTED | ZEND_ACC_STATIC TSRMLS_CC);

	return SUCCESS;
}
/* }}} */

