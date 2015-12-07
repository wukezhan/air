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
/** {{{ wrapper **/
#ifndef AIR_WRAPPER_H
#define AIR_WRAPPER_H

/** {{{ common wrapper **/
#define AIR_MINIT_FUNCTION(module)	ZEND_MINIT_FUNCTION(module)
#define AIR_RINIT_FUNCTION(module)	ZEND_RINIT_FUNCTION(module)
#define AIR_MODULE_STARTUP(module)	ZEND_MODULE_STARTUP_N(module)(INIT_FUNC_ARGS_PASSTHRU)
#define AIR_MSHUTDOWN_FUNCTION(module)	ZEND_MSHUTDOWN_FUNCTION(module)
#define AIR_MODULE_SHUTDOWN(module)	ZEND_MODULE_SHUTDOWN_N(module)(INIT_FUNC_ARGS_PASSTHRU)

#define AIR_INIT_THIS zval *self = getThis()
#define AIR_RET_THIS RETURN_ZVAL(self, 1, 0)

#define air_app_t   zval
#define air_router_t    zval
#define air_controller_t    zval
#define air_view_t  zval
#define air_mysqli_t    zval
#define air_config_t    zval

#define air_c_key "controller"
#define air_a_key "action"


#define AIR_SITE 1
#define AIR_EXEC 2
#define AIR_R 1
#define AIR_W 2

extern PHPAPI void php_var_dump(zval **struc, int level TSRMLS_DC);
extern PHPAPI void php_debug_zval_dump(zval **struc, int level TSRMLS_DC);

#define AIR_DEBUG(msg) php_printf("\nADBG: %s:%d: %s\n\n", __FILE__, __LINE__, msg)
/** }}} common wrapper **/

/** {{{ version wrapper **/
#if PHP_MAJOR_VERSION < 7
//less than php 7
#define AIR_HASH_FOREACH_KEY_VAL(ht, idx, key, key_len, pzval) do{\
		zval **___tmp = NULL;\
		for(zend_hash_internal_pointer_reset(ht);\
			zend_hash_has_more_elements(ht) == SUCCESS;\
			zend_hash_move_forward(ht)){\
			if (zend_hash_get_current_data(ht, (void**)&___tmp) == FAILURE) {\
				continue;\
			}\
			pzval = *___tmp;\
			if(zend_hash_get_current_key_ex(ht, &key, &key_len, &idx, 0, NULL) != HASH_KEY_IS_STRING) {\
				key = NULL; key_len = 0;\
			}

#define AIR_HASH_FOREACH_END()\
		}\
	}while(0)

static inline int air_arr_get(zval *data, const char *key, int key_len, zval **val TSRMLS_DC) {
	if(data == NULL){
		return FAILURE;
	}
	int status = SUCCESS;
	zval **tmp;
	if(zend_hash_find(Z_ARRVAL_P(data), key, key_len, (void **)&tmp) == SUCCESS) {
		SEPARATE_ZVAL(tmp);
		*val = *tmp;
	}else{
		status = FAILURE;
	}
	return status;
}
static inline int air_hash_get(HashTable *ht, const char *key, int key_len, void **val TSRMLS_DC) {
	if(ht == NULL){
		return FAILURE;
	}
	int status = SUCCESS;
	zval **tmp = NULL;
	if(zend_hash_find(ht, key, key_len, (void **)&tmp) == SUCCESS) {
		*val = *tmp;
	}else{
		status = FAILURE;
	}
	return status;
}

static inline zval *air_arr_find(zval *data, const char *key, int key_len) {
	if(data == NULL){
		return NULL;
	}
	zval **tmp;
	if(zend_hash_find(Z_ARRVAL_P(data), key, key_len, (void **)&tmp) == SUCCESS) {
		return *tmp;
	}else{
		return NULL;
	}
}
static inline zval *air_arr_idx_find(zval *data, int idx) {
	if(data == NULL){
		return NULL;
	}
	zval **tmp;
	if(zend_hash_index_find(Z_ARRVAL_P(data), idx, (void **)&tmp) == SUCCESS) {
		return *tmp;
	}else{
		return NULL;
	}
}

static inline zend_class_entry *air_get_ce(char *classname, int len){
	zend_class_entry *ce = NULL;
	if(air_hash_get(EG(class_table), classname, len + 1, (void **)&ce) == FAILURE) {
		return NULL;
	}
	return ce;
}

static inline zval *air_new_object(char *classname, int len){
	zend_class_entry *ce = air_get_ce(classname, len);
	if(!ce){
		php_error(E_NOTICE, "class %s not found", classname);
		return NULL;
	}
	zval *obj;
	MAKE_STD_ZVAL(obj);
	object_init_ex(obj, ce);
	return obj;
}

#define air_call_func(func, param_count, params) air_call_function((func), sizeof(func)-1, param_count, params TSRMLS_CC)

static inline zval *air_call_function(const char *func_name, int func_name_len, int param_count, zval **params TSRMLS_DC){
	zval *func;
	MAKE_STD_ZVAL(func);
	ZVAL_STRINGL(func, func_name, func_name_len, 1);
	zval *ret;
	MAKE_STD_ZVAL(ret);
	char *error;
	zend_bool is_callable = zend_is_callable_ex(func, NULL, 0, NULL, NULL, NULL, &error TSRMLS_CC);
	if(!is_callable){
		php_error(E_ERROR, "%s", error);
	}
	int status = call_user_function(EG(function_table), NULL, func, ret, param_count, params TSRMLS_CC);
	if(status == FAILURE){
		php_error(E_WARNING, "call user function error: %s", Z_STRVAL_P(func));
		zval_ptr_dtor(&func);
		zval_ptr_dtor(&ret);
		return NULL;
	}
	zval_ptr_dtor(&func);
	return ret;
}

static inline zval* air_call_method(zval **object_pp, zend_class_entry *obj_ce, zend_function **fn_proxy, const char *function_name, int function_name_len, zval **retval_ptr_ptr, int param_count, zval ***params  TSRMLS_DC)
{
	int result;
	zend_fcall_info fci;
	zval z_fname;
	zval *retval;
	HashTable *function_table;

	fci.size = sizeof(fci);
	/*fci.function_table = NULL; will be read form zend_class_entry of object if needed */
	fci.object_ptr = object_pp ? *object_pp : NULL;
	fci.function_name = &z_fname;
	fci.retval_ptr_ptr = retval_ptr_ptr ? retval_ptr_ptr : &retval;
	fci.param_count = param_count;
	fci.params = params;
	fci.no_separation = 1;
	fci.symbol_table = NULL;

	if (!fn_proxy && !obj_ce) {
		/* no interest in caching and no information already present that is
		 * needed later inside zend_call_function. */
		ZVAL_STRINGL(&z_fname, function_name, function_name_len, 0);
		fci.function_table = !object_pp ? EG(function_table) : NULL;
		result = zend_call_function(&fci, NULL TSRMLS_CC);
	} else {
		zend_fcall_info_cache fcic;

		fcic.initialized = 1;
		if (!obj_ce) {
			obj_ce = object_pp ? Z_OBJCE_PP(object_pp) : NULL;
		}
		if (obj_ce) {
			function_table = &obj_ce->function_table;
		} else {
			function_table = EG(function_table);
		}
		if (!fn_proxy || !*fn_proxy) {
			if (zend_hash_find(function_table, function_name, function_name_len+1, (void **) &fcic.function_handler) == FAILURE) {
				/* error at c-level */
				zend_error(E_CORE_ERROR, "Couldn't find implementation for method %s%s%s", obj_ce ? obj_ce->name : "", obj_ce ? "::" : "", function_name);
			}
			if (fn_proxy) {
				*fn_proxy = fcic.function_handler;
			}
		} else {
			fcic.function_handler = *fn_proxy;
		}
		fcic.calling_scope = obj_ce;
		if (object_pp) {
			fcic.called_scope = Z_OBJCE_PP(object_pp);
		} else if (obj_ce &&
		           !(EG(called_scope) &&
		             instanceof_function(EG(called_scope), obj_ce TSRMLS_CC))) {
			fcic.called_scope = obj_ce;
		} else {
			fcic.called_scope = EG(called_scope);
		}
		fcic.object_ptr = object_pp ? *object_pp : NULL;
		result = zend_call_function(&fci, &fcic TSRMLS_CC);
	}
	if (result == FAILURE) {
		/* error at c-level */
		if (!obj_ce) {
			obj_ce = object_pp ? Z_OBJCE_PP(object_pp) : NULL;
		}
		if (!EG(exception)) {
			zend_error(E_CORE_ERROR, "Couldn't execute method %s%s%s", obj_ce ? obj_ce->name : "", obj_ce ? "::" : "", function_name);
		}
	}
	if (!retval_ptr_ptr) {
		if (retval) {
			zval_ptr_dtor(&retval);
		}
		return NULL;
	}
	return *retval_ptr_ptr;
}

#define air_call_object_method(obj_pp, obj_ce, fn_proxy, function_name, retval_ptr_ptr, param_count, params) air_call_method(obj_pp, obj_ce, fn_proxy, function_name, sizeof(function_name)-1, retval_ptr_ptr, param_count, params TSRMLS_CC)
#define air_call_static_method(obj_ce, fn_proxy, function_name, retval_ptr_ptr, param_count, params) air_call_method(NULL, obj_ce, fn_proxy, function_name, sizeof(function_name)-1, retval_ptr_ptr, param_count, params TSRMLS_CC)

#else
//larger than php 70
#endif
/** }}} version wrapper **/

#endif
/** }}} wrapper **/
