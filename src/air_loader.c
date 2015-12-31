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

#include "php_air.h"

#include "src/air_config.h"
#include "src/air_loader.h"

zend_class_entry *air_loader_ce;

zend_class_entry *air_loader_lookup_class(zend_string *classname, int autoload) {
	return zend_lookup_class_ex(classname, NULL, autoload);
}

int air_loader_execute_file(int type, zend_string *filename, zval *retval) {
	char realpath[MAXPATHLEN];
	if(!VCWD_REALPATH(ZSTR_VAL(filename), realpath)){
		return FAILURE;
	}

	zend_file_handle file_handle;
	file_handle.filename = ZSTR_VAL(filename);
	file_handle.free_filename = 0;
	file_handle.type = ZEND_HANDLE_FILENAME;
	file_handle.opened_path = NULL;
	file_handle.handle.fp = NULL;

	zend_op_array *op_array = zend_compile_file(&file_handle, type);
	if (!file_handle.opened_path) {
		file_handle.opened_path = filename;
	}
	zend_hash_add_empty_element(&EG(included_files), file_handle.opened_path);
	zend_destroy_file_handle(&file_handle);
	if (op_array) {
		zend_execute(op_array, retval);
		zend_exception_restore();
		zend_try_exception_handler();
		if (EG(exception)) {
			zend_exception_error(EG(exception), E_ERROR);
		}
		destroy_op_array(op_array);
		efree_size(op_array, sizeof(zend_op_array));
	} else if (type==ZEND_REQUIRE) {
		return FAILURE;
	}

	return SUCCESS;
}

zend_class_entry *air_loader_load_class_from_dir(zend_string *classname, zval *dir) {
	char *filename = estrndup(ZSTR_VAL(classname), ZSTR_LEN(classname));
	int idx = 0;
	while(idx < ZSTR_LEN(classname)){
		if(filename[idx] == '\\'){
			filename[idx] = '/';
		}
		idx++;
	}
	smart_str buf = {0};
	char *dirname = Z_STRVAL_P(dir);
	int dir_len = Z_STRLEN_P(dir);
	if(dirname[dir_len-1] == '/') {
		dir_len--;
	}
	smart_str_appendl(&buf, dirname, dir_len);
	smart_str_appendc(&buf, '/');
	smart_str_appendl(&buf, filename, ZSTR_LEN(classname));
	smart_str_appendl(&buf, ".php", 4);
	smart_str_0(&buf);
	//?if retval can be NULL?
	zend_class_entry *ce = NULL;
	if(air_loader_execute_file(ZEND_INCLUDE, buf.s, NULL)){
		ce = air_loader_lookup_class(classname, 0);
	}
	smart_str_free(&buf);
	efree(filename);
	return ce;
}

zend_class_entry *air_loader_autoload_class(zend_string *classname) {
	zend_class_entry *ce = NULL;
	char *class_str = ZSTR_VAL(classname);
	smart_str ss_app_ns = {0};
	zval *app_path = air_config_path_get(NULL, zend_string_init(ZEND_STRS("app.path"), 1));
	if(!app_path){
		smart_str_appendl(&ss_app_ns, "app\\", 4);
	}else{
		smart_str_appendl(&ss_app_ns, Z_STRVAL_P(app_path), Z_STRLEN_P(app_path));
		smart_str_appendc(&ss_app_ns, '\\');
	}
	smart_str_0(&ss_app_ns);
	int is_app_class = 1;
	int idx = 0;
	int max_idx = MIN(ZSTR_LEN(ss_app_ns.s), ZSTR_LEN(classname));
	while(is_app_class == 1 && idx < max_idx) {
		if(class_str[idx] != ZSTR_VAL(ss_app_ns.s)[idx]){
			is_app_class = 0;
			break;
		}
		idx++;
	}
	smart_str_free(&ss_app_ns);
	zval *root_dir = NULL, *lib_dir = NULL;
	if((root_dir = zend_get_constant_str(ZEND_STRL("ROOT_PATH"))) == NULL){
		php_error(E_ERROR, "ROOT_PATH is not defined");
	}
	if(is_app_class){
		ce = air_loader_load_class_from_dir(classname, root_dir);
	}else{
		if((lib_dir = zend_get_constant_str(ZEND_STRL("LIB_PATH"))) == NULL){
			php_error(E_ERROR, "LIB_PATH is not defined");
		}
		ce = air_loader_load_class_from_dir(classname, lib_dir);
		if(!ce){
			ce = air_loader_load_class_from_dir(classname, root_dir);
		}
	}
	return ce;
}

/* {{{ ARG_INFO */
ZEND_BEGIN_ARG_INFO_EX(air_loader_load_arginfo, 0, 0, 0)
	ZEND_ARG_INFO(0, classname)
ZEND_END_ARG_INFO()

/* }}} */

/* {{{ PHP METHODS */
PHP_METHOD(air_loader, autoload) {
	zval loader;
	ZVAL_STRING(&loader, "air\\loader::load");

	zval params[1] = {loader};
	air_call_func("spl_autoload_register", 1, params, return_value);
	zval_ptr_dtor(&loader);
}

PHP_METHOD(air_loader, load) {
	zend_string *classname = NULL;
	if(zend_parse_parameters(ZEND_NUM_ARGS(), "S", &classname) == FAILURE ){
		AIR_NEW_EXCEPTION(1, "invalid air\\loader::load param");
	}
	if(classname == NULL){
		AIR_NEW_EXCEPTION(1, "air\\loader::load param can not be empty");
	}else{
		if(air_loader_autoload_class(classname)){
			RETURN_TRUE;
		}else{
			RETURN_FALSE;
		}
	}
}
/* }}} */

/* {{{ air_loader_methods */
zend_function_entry air_loader_methods[] = {
	PHP_ME(air_loader, autoload, NULL,  ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(air_loader, load, air_loader_load_arginfo,  ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};
/* }}} */

/* {{{ AIR_MINIT_FUNCTION */
AIR_MINIT_FUNCTION(air_loader) {
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "air\\loader", air_loader_methods);

	air_loader_ce = zend_register_internal_class_ex(&ce, NULL);
	return SUCCESS;
}
/* }}} */

