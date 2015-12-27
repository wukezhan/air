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

extern PHPAPI void php_var_dump(zval *struc, int level);
extern PHPAPI void php_debug_zval_dump(zval *struc, int level);

#define AIR_DEBUG(msg) php_printf("\nADBG: %s:%d: %s\n\n", __FILE__, __LINE__, msg)

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

#define AIR_PZ_CTOR(pz) zval *pz; do{\
		zval pz##_zv;\
		pz = &(pz##_zv);\
	}while(0)

#define AIR_PZ_INIT(pz) do {\
		zval pz##_zv;\
		pz = &(pz##_zv);\
	}while(0)

#define AIR_PZ_DTOR(pz) do {\
		if(pz && !Z_ISUNDEF_P(pz)){\
			zval_ptr_dtor(pz);\
		}\
	}while(0)

#define AIR_OBJ_INIT(pz, classname) do {\
		zend_class_entry *ce = NULL;\
		if((ce = (zend_class_entry *)zend_hash_str_find_ptr(EG(class_table), classname, sizeof(classname)-1)) != NULL) {\
			object_init_ex((pz), ce);\
		}\
	}while(0)

static inline zend_class_entry *air_get_ce(char *classname, int len){
	zend_class_entry *ce = NULL;
	if((ce = (zend_class_entry *)zend_hash_str_find_ptr(EG(class_table), classname, len)) == NULL) {
		return NULL;
	}
	return ce;
}

static int air_call_func(const char *func_name, uint32_t param_count, zval params[], zval *retval){
	zval func;
	ZVAL_STRING(&func, func_name);
	zval *_retval = NULL;
	if(!retval){
		zval ret;
		_retval = &ret;
	}
	int status = call_user_function(EG(function_table), NULL, &func, retval?retval: _retval, param_count, params);
	zval_ptr_dtor(&func);
	if(_retval){
		zval_ptr_dtor(_retval);
	}
	return status;
}

#define AIR_METHOD_MAX_PARAM_SIZE 8

static inline zval* air_call_method(zval *object, zend_class_entry *obj_ce, zend_function **fn_proxy, const char *function_name, size_t function_name_len, zval *retval_ptr, int param_count, zval params[])
{
	int result;
	zend_fcall_info fci;
	zval retval;
	HashTable *function_table;
	if(param_count > AIR_METHOD_MAX_PARAM_SIZE){
		php_error(E_ERROR, "too many params");
	}

	zval args[AIR_METHOD_MAX_PARAM_SIZE];
	int i = 0;
	for(; i<param_count; i++){
		ZVAL_COPY_VALUE(&args[i], &params[i]);
	}

	fci.size = sizeof(fci);
	/*fci.function_table = NULL; will be read form zend_class_entry of object if needed */
	fci.object = (object && Z_TYPE_P(object) == IS_OBJECT) ? Z_OBJ_P(object) : NULL;
	ZVAL_STRINGL(&fci.function_name, function_name, function_name_len);
	fci.retval = retval_ptr ? retval_ptr : &retval;
	fci.param_count = param_count;
	fci.params = params;
	fci.no_separation = 1;
	fci.symbol_table = NULL;

	if (!fn_proxy && !obj_ce) {
		/* no interest in caching and no information already present that is
		 * needed later inside zend_call_function. */
		fci.function_table = !object ? EG(function_table) : NULL;
		result = zend_call_function(&fci, NULL);
		zval_ptr_dtor(&fci.function_name);
	} else {
		zend_fcall_info_cache fcic;

		fcic.initialized = 1;
		if (!obj_ce) {
			obj_ce = object ? Z_OBJCE_P(object) : NULL;
		}
		if (obj_ce) {
			function_table = &obj_ce->function_table;
		} else {
			function_table = EG(function_table);
		}
		if (!fn_proxy || !*fn_proxy) {
			if ((fcic.function_handler = zend_hash_find_ptr(function_table, Z_STR(fci.function_name))) == NULL) {
				/* error at c-level */
				zend_error_noreturn(E_CORE_ERROR, "Couldn't find implementation for method %s%s%s", obj_ce ? ZSTR_VAL(obj_ce->name) : "", obj_ce ? "::" : "", function_name);
			}
			if (fn_proxy) {
				*fn_proxy = fcic.function_handler;
			}
		} else {
			fcic.function_handler = *fn_proxy;
		}
		fcic.calling_scope = obj_ce;
		if (object) {
			fcic.called_scope = Z_OBJCE_P(object);
		} else {
			zend_class_entry *called_scope = zend_get_called_scope(EG(current_execute_data));

			if (obj_ce &&
			    (!called_scope ||
			     !instanceof_function(called_scope, obj_ce))) {
				fcic.called_scope = obj_ce;
			} else {
				fcic.called_scope = called_scope;
			}
		}
		fcic.object = object ? Z_OBJ_P(object) : NULL;
		result = zend_call_function(&fci, &fcic);
		zval_ptr_dtor(&fci.function_name);
	}
	if (result == FAILURE) {
		/* error at c-level */
		if (!obj_ce) {
			obj_ce = object ? Z_OBJCE_P(object) : NULL;
		}
		if (!EG(exception)) {
			zend_error_noreturn(E_CORE_ERROR, "Couldn't execute method %s%s%s", obj_ce ? ZSTR_VAL(obj_ce->name) : "", obj_ce ? "::" : "", function_name);
		}
	}
	/* copy arguments back, they might be changed by references */
	for(i=0; i<param_count; i++){
		if(Z_ISREF(args[i]) && !Z_ISREF(params[i])){
			ZVAL_COPY_VALUE(&params[i], &args[i]);
		}
	}
	if (!retval_ptr) {
		zval_ptr_dtor(&retval);
		return NULL;
	}
	return retval_ptr;
}

#define air_call_object_method(obj, obj_ce, function_name, retval_ptr, param_count, params) air_call_method(obj, obj_ce, NULL, function_name, sizeof(function_name)-1, retval_ptr, param_count, params)
#define air_call_static_method(obj_ce, function_name, retval_ptr, param_count, params) air_call_method(NULL, obj_ce, NULL, function_name, sizeof(function_name)-1, retval_ptr, param_count, params)
/** }}} common wrapper **/

#endif
/** }}} wrapper **/
