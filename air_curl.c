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

#include "air_curl.h"
#include "air_curl_waiter.h"

zend_class_entry *air_curl_ce;

void air_curl_set_opt(zval *self, zval **opts, ulong ok, zval *ov){
	zval *_opts;
	if(!opts){
		_opts = zend_read_property(air_curl_ce, self, ZEND_STRL("_opts"), 1 TSRMLS_CC);
	}else{
		_opts = *opts;
	}
	Z_ADDREF_P(ov);
	add_index_zval(_opts, ok, ov);
}

void air_curl_update_result(zval *self, zval *ch, zval *result){
	zend_update_property(air_curl_ce, self, ZEND_STRL("_data"), result TSRMLS_CC);
	zend_update_property_long(air_curl_ce, self, ZEND_STRL("_status"), 1 TSRMLS_CC);
}

void air_curl_set_opt_array(zval *self){
	zval *ch = zend_read_property(air_curl_ce, self, ZEND_STRL("_ch"), 1 TSRMLS_CC);
	zval *opts = zend_read_property(air_curl_ce, self, ZEND_STRL("_opts"), 1 TSRMLS_CC);
	zval *params[2] = {ch, opts};
	zval *tmp = air_call_func("curl_setopt_array", 2, params);
	zval_ptr_dtor(&tmp);
}

void air_curl_execute(zval *self){
	zval *status = zend_read_property(air_curl_ce, self, ZEND_STRL("_status"), 1 TSRMLS_CC);
	if(Z_LVAL_P(status)){
		return ;
	}
	air_curl_set_opt_array(self);
	zval *ch = zend_read_property(air_curl_ce, self, ZEND_STRL("_ch"), 1 TSRMLS_CC);

	zval *data  = NULL;
	zval *service = zend_read_property(air_curl_ce, self, ZEND_STRL("_service"), 1 TSRMLS_CC);
	if(Z_TYPE_P(service) != IS_NULL){
		data = zend_read_property(air_curl_ce, self, ZEND_STRL("_data"), 1 TSRMLS_CC);
		if(Z_TYPE_P(data) != IS_NULL){
			return ;
		}
		air_call_method(&service, Z_OBJCE_P(service), NULL, ZEND_STRL("call"), &data, 0, NULL TSRMLS_CC);
		if(Z_TYPE_P(data) == IS_NULL){
			ZVAL_BOOL(data, 0);
		}
		air_curl_update_result(self, ch, data);
		zval_ptr_dtor(&data);
	}else{
		zval *params[1] = {ch};
		zval *result = air_call_func("curl_exec", 1, params);
		zval **trigger_params[2];
		zval *event;
		MAKE_STD_ZVAL(event);
		zval *event_params;
		MAKE_STD_ZVAL(event_params);
		array_init(event_params);
		Z_ADDREF_P(ch);
		add_next_index_zval(event_params, ch);
		zval *curl_errno = air_call_func("curl_errno", 1, params);
		if(Z_LVAL_P(curl_errno)){
			ZVAL_STRING(event, "error", 1);
			zval_ptr_dtor(&result);
		}else{
			ZVAL_STRING(event, "success", 1);
			add_next_index_zval(event_params, result);
		}
		zval_ptr_dtor(&curl_errno);
		trigger_params[0] = &event;
		trigger_params[1] = &event_params;
		air_call_method(&self, air_curl_ce, NULL, ZEND_STRL("trigger"), &data, 2, trigger_params TSRMLS_CC);
		air_curl_update_result(self, ch, data);
		zval_ptr_dtor(&data);
		zval_ptr_dtor(&event);
		zval_ptr_dtor(&event_params);
	}
}

/* {{{ ARG_INFO */
ZEND_BEGIN_ARG_INFO_EX(air_curl_construct_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, config)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_curl_url_data_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, url)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_curl_setopt_arginfo, 0, 0, 2)
	ZEND_ARG_INFO(0, ok)
	ZEND_ARG_INFO(0, ov)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_curl_k_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, k)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_curl_kv_arginfo, 0, 0, 2)
	ZEND_ARG_INFO(0, k)
	ZEND_ARG_INFO(0, v)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_curl_trigger_arginfo, 0, 0, 2)
	ZEND_ARG_INFO(0, event)
	ZEND_ARG_INFO(0, param_array)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_curl_on_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, handler)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ PHP METHODS */
PHP_METHOD(air_curl, __construct) {
	AIR_INIT_THIS;
	zval *ch = air_call_func("curl_init", 0, NULL);
	zend_update_property(air_curl_ce, self, ZEND_STRL("_ch"), ch);
	zval_ptr_dtor(&ch);

	zval *opts;
	MAKE_STD_ZVAL(opts);
	array_init(opts);
	zend_update_property(air_curl_ce, self, ZEND_STRL("_opts"), opts);
	zval_ptr_dtor(&opts);

	zval *cb_arr;
	MAKE_STD_ZVAL(cb_arr);
	array_init(cb_arr);
	zval *cb_succ;
	MAKE_STD_ZVAL(cb_succ);
	array_init(cb_succ);
	add_next_index_string(cb_succ, "air\\curl", 1);
	add_next_index_string(cb_succ, "on_success_default", 1);
	zval *cb_err;
	MAKE_STD_ZVAL(cb_err);
	array_init(cb_err);
	add_next_index_string(cb_err, "air\\curl", 1);
	add_next_index_string(cb_err, "on_error_default", 1);
	add_assoc_zval(cb_arr, "success", cb_succ);
	add_assoc_zval(cb_arr, "error", cb_err);
	zend_update_property(air_curl_ce, self, ZEND_STRL("_callback"), cb_arr);
	zval_ptr_dtor(&cb_arr);
}

PHP_METHOD(air_curl, offsetExists) {
	AIR_INIT_THIS;
	char *key;
	uint len;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &key, &len) == FAILURE) {
		return;
	} else {
		air_curl_execute(self);
		zval *data = zend_read_property(air_curl_ce, self, ZEND_STRL("_data"), 1 TSRMLS_CC);
		RETURN_BOOL(Z_TYPE_P(data) == IS_ARRAY && zend_hash_exists(Z_ARRVAL_P(data), key, len + 1));
	}
}

PHP_METHOD(air_curl, offsetSet) {
	php_error(E_WARNING, "curl data is not allowed to reset");
}

PHP_METHOD(air_curl, offsetGet) {
	AIR_INIT_THIS;
	char *key;
	int len;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &key, &len) == FAILURE) {
		return ;
	}
	air_curl_execute(self);
	zval *data = zend_read_property(air_curl_ce, self, ZEND_STRL("_data"), 1 TSRMLS_CC);
	zval **tmp;
	long lval;
	double dval;
	if(is_numeric_string(key, len, &lval, &dval, 0) != IS_LONG){
		if(Z_TYPE_P(data) != IS_ARRAY){
			php_error(E_WARNING, "can not get the value of key '%s' from a non-array data", key);
			return ;
		}
		if (zend_hash_find(Z_ARRVAL_P(data), key, len + 1, (void **) &tmp) == FAILURE) {
			RETURN_NULL();
		}
	}else{
		if(Z_TYPE_P(data) != IS_ARRAY){
			php_error(E_WARNING, "can not get the value of key '%lu' from a non-array data", lval);
			return ;
		}
		if (zend_hash_index_find(Z_ARRVAL_P(data), lval, (void **) &tmp) == FAILURE) {
			RETURN_NULL();
		}
	}
	RETURN_ZVAL(*tmp, 1, 0);
}

PHP_METHOD(air_curl, offsetUnset) {
	AIR_INIT_THIS;

	zval *key, *data;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &key) == FAILURE) {
		return;
	}

	if (Z_TYPE_P(key) != IS_STRING || !Z_STRLEN_P(key)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "key must be a string");
		RETURN_FALSE;
	}

	air_curl_execute(self);
	data = zend_read_property(air_curl_ce, self, ZEND_STRL("_data"), 1 TSRMLS_CC);
	if(Z_TYPE_P(data) != IS_ARRAY){
		php_error(E_WARNING, "can not unset a key '%s' from a non-array data", Z_STRVAL_P(key));
		RETURN_FALSE;
	}
	if (zend_hash_del(Z_ARRVAL_P(data), Z_STRVAL_P(key), Z_STRLEN_P(key) + 1) == SUCCESS) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

PHP_METHOD(air_curl, setopt) {
	AIR_INIT_THIS;
	ulong ok;
	zval *ov;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "lz", &ok, &ov) == FAILURE){
	}
	air_curl_set_opt(self, NULL, ok, ov);
	AIR_RET_THIS;
}

PHP_METHOD(air_curl, async) {
	AIR_INIT_THIS;
	zval *waiter = zend_read_static_property(air_curl_ce, ZEND_STRL("_waiter"), 1 TSRMLS_CC);
	zval *_waiter = NULL;
	if(Z_TYPE_P(waiter) == IS_NULL){
		waiter = air_new_object(ZEND_STRL("air\\curl\\waiter"));
		air_call_method(&waiter, air_curl_waiter_ce, NULL, ZEND_STRL("__construct"), NULL, 0, NULL);
		zend_update_static_property(air_curl_ce, ZEND_STRL("_waiter"), waiter);
		_waiter = waiter;
	}
	zval **params[1] = {&self};
	zval *service;
	air_call_method(&waiter, air_curl_waiter_ce, NULL, ZEND_STRL("serve"), &service, 1, params);
	zend_update_property(air_curl_ce, self, ZEND_STRL("_service"), service);
	zval_ptr_dtor(&service);
	if(_waiter){
		zval_ptr_dtor(&_waiter);
	}
	AIR_RET_THIS;
}

PHP_METHOD(air_curl, get) {
	AIR_INIT_THIS;
	zval *url;
	zval *params = NULL;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|a", &url, &params) == FAILURE){
	}
	zval *qs;
	if(params){
		zval *build_params[1] = {params};
		qs = air_call_func("http_build_query", 1, build_params);
		if(qs){
			char *url_str = Z_STRVAL_P(url);
			int url_len = Z_STRLEN_P(url);
			int i = 0;
			while(i < url_len){
				if(url_str[i] == '?'){
					break;
				}
				i++;
			}
			char *tmp_str;
			char tmp_len = spprintf(&tmp_str, 0, "%s%c%s", Z_STRVAL_P(url), (i<url_len?'&': '?'), Z_STRVAL_P(qs));
			ZVAL_STRINGL(url, tmp_str, tmp_len, 0);
			zval_ptr_dtor(&qs);
		}
	}
	zval *opts = zend_read_property(air_curl_ce, self, ZEND_STRL("_opts"), 1 TSRMLS_CC);
	air_curl_set_opt(self, &opts, CURLOPT_URL, url);
	AIR_RET_THIS;
}

PHP_METHOD(air_curl, post) {
	AIR_INIT_THIS;
	zval *url;
	zval *data = NULL;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz", &url, &data) == FAILURE){
		AIR_NEW_EXCEPTION(1, "invalid air\\curl::post params");
	}
	zval *opts = zend_read_property(air_curl_ce, self, ZEND_STRL("_opts"), 1 TSRMLS_CC);
	air_curl_set_opt(self, &opts, CURLOPT_URL, url);
	zval *n1;
	MAKE_STD_ZVAL(n1);
	ZVAL_LONG(n1, 1);
	air_curl_set_opt(self, &opts, CURLOPT_POST, n1);
	zval_ptr_dtor(&n1);
	if(Z_TYPE_P(data) == IS_ARRAY){
		zval *build_params[1] = {data};
		zval *built_data = air_call_func("http_build_query", 1, build_params);
		air_curl_set_opt(self, &opts, CURLOPT_POSTFIELDS, built_data);
		zval_ptr_dtor(&built_data);
	}else{
		air_curl_set_opt(self, &opts, CURLOPT_POSTFIELDS, data);
	}
	AIR_RET_THIS;
}
PHP_METHOD(air_curl, exec) {
	AIR_INIT_THIS;
	air_curl_execute(self);
	zval *data = zend_read_property(air_curl_ce, self, ZEND_STRL("_data"), 1 TSRMLS_CC);
	zval *ret = air_arr_find(data, ZEND_STRS("data"));
	if(ret){
		RETURN_ZVAL(ret, 1, 1);
	}
}

PHP_METHOD(air_curl, data) {
	AIR_INIT_THIS;
	air_curl_execute(self);
	zval *data = zend_read_property(air_curl_ce, self, ZEND_STRL("_data"), 1 TSRMLS_CC);
	RETURN_ZVAL(data, 1, 0);
}

PHP_METHOD(air_curl, reset) {
	AIR_INIT_THIS;
	zval *ch = zend_read_property(air_curl_ce, self, ZEND_STRL("_ch"), 1 TSRMLS_CC);
	zval *params[1] = {ch};
	zval *tmp = air_call_func("curl_reset", 1, params);
	zval_ptr_dtor(&tmp);
	AIR_RET_THIS;
}

PHP_METHOD(air_curl, errno) {
	AIR_INIT_THIS;
	zval *ch = zend_read_property(air_curl_ce, self, ZEND_STRL("_ch"), 1 TSRMLS_CC);
	zval *params[1] = {ch};
	zval *ret = air_call_func("curl_errno", 1, params);
	RETURN_ZVAL(ret, 1, 1);
}

PHP_METHOD(air_curl, error) {
	AIR_INIT_THIS;
	zval *ch = zend_read_property(air_curl_ce, self, ZEND_STRL("_ch"), 1 TSRMLS_CC);
	zval *params[1] = {ch};
	zval *ret = air_call_func("curl_error", 1, params);
	RETURN_ZVAL(ret, 1, 1);
}

PHP_METHOD(air_curl, trigger) {
	AIR_INIT_THIS;
	zval *event, *params;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "za", &event, &params) == FAILURE){
		return ;
	}
	zval *callback = zend_read_property(air_curl_ce, self, ZEND_STRL("_callback"), 1 TSRMLS_CC);
	zval *event_handler = air_arr_find(callback, Z_STRVAL_P(event), Z_STRLEN_P(event)+1);
	zval *_params[2] = {event_handler, params};
	zval *ret = air_call_func("call_user_func_array", 2, _params);
	RETURN_ZVAL(ret, 1, 1);
}

PHP_METHOD(air_curl, on_success) {
	AIR_INIT_THIS;
	zval *handler;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &handler) == FAILURE){
		return ;
	}
	zval *callback = zend_read_property(air_curl_ce, self, ZEND_STRL("_callback"), 1 TSRMLS_CC);
	Z_ADDREF_P(handler);
	add_assoc_zval(callback, "success", handler);
	AIR_RET_THIS;
}

PHP_METHOD(air_curl, on_error) {
	AIR_INIT_THIS;
	zval *handler;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &handler) == FAILURE){
		return ;
	}
	zval *callback = zend_read_property(air_curl_ce, self, ZEND_STRL("_callback"), 1 TSRMLS_CC);
	Z_ADDREF_P(handler);
	add_assoc_zval(callback, "error", handler);
	AIR_RET_THIS;
}

PHP_METHOD(air_curl, on_success_default) {
	zval *ch, *result;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz", &ch, &result) == FAILURE){
		return ;
	}
	RETURN_ZVAL(result, 1, 0);
}

PHP_METHOD(air_curl, on_error_default) {
	zval *ch;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &ch) == FAILURE){
		return ;
	}
}

PHP_METHOD(air_curl, __destruct) {
	AIR_INIT_THIS;
	air_curl_execute(self);
}
/* }}} */

/* {{{ air_curl_methods */
zend_function_entry air_curl_methods[] = {
	PHP_ME(air_curl, __construct, air_curl_construct_arginfo,  ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(air_curl, offsetUnset, air_curl_k_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_curl, offsetGet, air_curl_k_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_curl, offsetExists, air_curl_k_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_curl, offsetSet, air_curl_kv_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_curl, setopt, air_curl_setopt_arginfo,  ZEND_ACC_PUBLIC)
	PHP_ME(air_curl, get, air_curl_url_data_arginfo,  ZEND_ACC_PUBLIC)
	PHP_ME(air_curl, post, air_curl_url_data_arginfo,  ZEND_ACC_PUBLIC)
	PHP_ME(air_curl, async, NULL,  ZEND_ACC_PUBLIC)
	PHP_ME(air_curl, exec, NULL,  ZEND_ACC_PUBLIC)
	PHP_ME(air_curl, data, NULL,  ZEND_ACC_PUBLIC)
	PHP_ME(air_curl, trigger, air_curl_trigger_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_curl, on_success, air_curl_on_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_curl, on_error, air_curl_on_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_curl, on_success_default, air_curl_on_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(air_curl, on_error_default, air_curl_on_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(air_curl, __destruct, NULL,  ZEND_ACC_PUBLIC | ZEND_ACC_DTOR)
	{NULL, NULL, NULL}
};
/* }}} */

/* {{{ AIR_MINIT_FUNCTION */
AIR_MINIT_FUNCTION(air_curl) {
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "air\\curl", air_curl_methods);

	air_curl_ce = zend_register_internal_class_ex(&ce, NULL, NULL TSRMLS_CC);
	zend_class_implements(air_curl_ce TSRMLS_CC, 1, zend_ce_arrayaccess);

	zend_declare_property_null(air_curl_ce, ZEND_STRL("_callback"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(air_curl_ce, ZEND_STRL("_ch"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(air_curl_ce, ZEND_STRL("_data"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_long(air_curl_ce, ZEND_STRL("_status"), 0, ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(air_curl_ce, ZEND_STRL("_service"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(air_curl_ce, ZEND_STRL("_waiter"), ZEND_ACC_PROTECTED | ZEND_ACC_STATIC TSRMLS_CC);

	return SUCCESS;
}
/* }}} */

