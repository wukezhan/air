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
#include "ext/standard/php_smart_str.h"

#include "php_air.h"

#include "air_config.h"
#include "air_loader.h"

zend_class_entry *air_loader_ce;

zend_class_entry *air_loader_lookup_class(const char *classname, int len TSRMLS_DC) {
	zend_class_entry **pp_ce = NULL;
	int status = zend_lookup_class(classname, len, &pp_ce TSRMLS_CC);
	if (status == FAILURE || ((*pp_ce)->ce_flags & (ZEND_ACC_INTERFACE | (ZEND_ACC_TRAIT - ZEND_ACC_EXPLICIT_ABSTRACT_CLASS))) != 0){
		return NULL;
	}
	return *pp_ce;
}

int air_loader_include_file(char *filename TSRMLS_DC) {
	char realpath[MAXPATHLEN];
	if(!VCWD_REALPATH(filename, realpath)){
		return FAILURE;
	}

	zend_op_array *orig_op_array = EG(active_op_array);
	zend_op ** orig_opline_ptr = EG(opline_ptr);
	zval **orig_retval_ptr_ptr = EG(return_value_ptr_ptr);

	zend_file_handle file_handle;

	file_handle.filename = filename;
	file_handle.free_filename = 0;
	file_handle.type = ZEND_HANDLE_FILENAME;
	file_handle.opened_path = NULL;
	file_handle.handle.fp = NULL;

	EG(active_op_array) = zend_compile_file(&file_handle, ZEND_INCLUDE TSRMLS_CC);
	if (file_handle.opened_path) {
		int dummy = 1;
		zend_hash_add(&EG(included_files), file_handle.opened_path, strlen(file_handle.opened_path) + 1, (void *)&dummy, sizeof(int), NULL);
	}
	zend_destroy_file_handle(&file_handle TSRMLS_CC);
	if (EG(active_op_array)) {
		EG(return_value_ptr_ptr) = NULL;
		zend_execute(EG(active_op_array) TSRMLS_CC);
		zend_exception_restore(TSRMLS_C);
		if (EG(exception)) {
			if (EG(user_exception_handler)) {
				zval *orig_user_exception_handler;
				zval **params[1], *retval2, *old_exception;
				old_exception = EG(exception);
				EG(exception) = NULL;
				params[0] = &old_exception;
				orig_user_exception_handler = EG(user_exception_handler);
				if (call_user_function_ex(CG(function_table), NULL, orig_user_exception_handler, &retval2, 1, params, 1, NULL TSRMLS_CC) == SUCCESS) {
					if (retval2 != NULL) {
						zval_ptr_dtor(&retval2);
					}
					if (EG(exception)) {
						zval_ptr_dtor(&EG(exception));
						EG(exception) = NULL;
					}
					zval_ptr_dtor(&old_exception);
				} else {
					EG(exception) = old_exception;
					zend_exception_error(EG(exception), E_ERROR TSRMLS_CC);
				}
			} else {
				zend_exception_error(EG(exception), E_ERROR TSRMLS_CC);
			}
		}
		destroy_op_array(EG(active_op_array) TSRMLS_CC);
		efree(EG(active_op_array));
	}

	EG(active_op_array) = orig_op_array;
	EG(opline_ptr) = orig_opline_ptr;
	EG(return_value_ptr_ptr) = orig_retval_ptr_ptr;

	return SUCCESS;
}

int air_loader_load_class_from_dir(char *classname, int len, zval *dir TSRMLS_DC) {
	char *filename = estrndup(classname, len);
	int idx = 0;
	while(idx < len){
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
	smart_str_appendl(&buf, filename, len);
	smart_str_appendl(&buf, ".php", 4);
	smart_str_0(&buf);
	int status = air_loader_include_file(buf.c TSRMLS_CC);
	smart_str_free(&buf);
	efree(filename);
	return status;
}

int air_loader_autoload_class(char *classname, int len TSRMLS_DC) {
	char *class_str = classname;
	smart_str ss_app_ns = {0};
	zval *app_path = NULL;
	if(air_config_path_get(NULL, ZEND_STRS("app.path"), &app_path TSRMLS_CC) == FAILURE || Z_TYPE_P(app_path) != IS_STRING){
		smart_str_appendl(&ss_app_ns, "app\\", 4);
	}else{
		smart_str_appendl(&ss_app_ns, Z_STRVAL_P(app_path), Z_STRLEN_P(app_path));
		smart_str_appendc(&ss_app_ns, '\\');
	}
	smart_str_0(&ss_app_ns);
	int is_app_class = 1;
	int idx = 0;
	int max_idx = MIN(ss_app_ns.len, len);
	while(is_app_class == 1 && idx < max_idx) {
		if(class_str[idx] != ss_app_ns.c[idx]){
			is_app_class = 0;
			break;
		}
		idx++;
	}
	smart_str_free(&ss_app_ns);
	zval root_dir, lib_dir;
	if(zend_get_constant(ZEND_STRL("ROOT_PATH"), &root_dir TSRMLS_CC) == FAILURE){
		php_error(E_ERROR, "ROOT_PATH is not defined");
	}
	int status = FAILURE;
	if(is_app_class){
		status = air_loader_load_class_from_dir(classname, len, &root_dir TSRMLS_CC);
	}else{
		if(zend_get_constant(ZEND_STRL("LIB_PATH"), &lib_dir TSRMLS_CC) == FAILURE){
			php_error(E_ERROR, "LIB_PATH is not defined");
		}
		status = air_loader_load_class_from_dir(classname, len, &lib_dir TSRMLS_CC);
		if(status == FAILURE){
			status = air_loader_load_class_from_dir(classname, len, &root_dir);
		}
	}
	zval_dtor(&root_dir);
	zval_dtor(&lib_dir);
	return status;
}

/* {{{ ARG_INFO */
ZEND_BEGIN_ARG_INFO_EX(air_loader_load_arginfo, 0, 0, 0)
	ZEND_ARG_INFO(0, classname)
ZEND_END_ARG_INFO()

/* }}} */

/* {{{ PHP METHODS */
PHP_METHOD(air_loader, autoload) {
	zval *spl_autoload_register;
	zval *loader;

	MAKE_STD_ZVAL(spl_autoload_register);
	ZVAL_STRING(spl_autoload_register, "spl_autoload_register", 1);

	MAKE_STD_ZVAL(loader);
	array_init(loader);
	add_next_index_string(loader, "air\\loader", 1);
	add_next_index_string(loader, "load", 1);
	zval *params[1] = {loader};
	zval *ret;
	MAKE_STD_ZVAL(ret);
	if(call_user_function(EG(function_table), NULL, spl_autoload_register, ret, 1, params TSRMLS_CC) == FAILURE){
		//should not happen
		php_error(E_ERROR, "register error");
	}
	if(ret){
		zval_ptr_dtor(&ret);
	}
	zval_ptr_dtor(&spl_autoload_register);
	zval_ptr_dtor(&loader);
}

PHP_METHOD(air_loader, load) {
	char *classname = NULL;
	int len = 0;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &classname, &len) == FAILURE ){
		AIR_NEW_EXCEPTION(1, "invalid air\\loader::load param");
	}
	if(classname == NULL){
		AIR_NEW_EXCEPTION(1, "air\\loader::load param can not be empty");
	}else{
		int status = air_loader_autoload_class(classname, len TSRMLS_CC);
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

	air_loader_ce = zend_register_internal_class_ex(&ce, NULL, NULL TSRMLS_CC);
	return SUCCESS;
}
/* }}} */

