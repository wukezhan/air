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
#include "Zend/zend_smart_str.h"

#include "php_air.h"

#include "src/air_curl_waiter.h"
#include "src/air_curl.h"

zend_class_entry *air_curl_ce;

void air_curl_set_opt(zval *self, zval **opts, ulong ok, zval *ov){
	zval *_opts;
	if(!opts){
		_opts = zend_read_property(air_curl_ce, self, ZEND_STRL("_opts"), 1, NULL);
	}else{
		_opts = *opts;
	}
	Z_TRY_ADDREF_P(ov);
	add_index_zval(_opts, ok, ov);
}

void air_curl_update_result(zval *self, zval *ch, zval *result){
	zend_update_property(air_curl_ce, self, ZEND_STRL("_data"), result);
	zend_update_property_long(air_curl_ce, self, ZEND_STRL("_status"), 1);
}

void air_curl_init(zval *self){
	zval ch;
	air_call_func("curl_init", 0, NULL, &ch);
	zend_update_property(air_curl_ce, self, ZEND_STRL("_ch"), &ch);
	zval_ptr_dtor(&ch);
}

void air_curl_set_opt_array(zval *self){
	zval *ch = zend_read_property(air_curl_ce, self, ZEND_STRL("_ch"), 1, NULL);
	zval *opts = zend_read_property(air_curl_ce, self, ZEND_STRL("_opts"), 1, NULL);
	zval params[2] = {*ch, *opts};
	air_call_func("curl_setopt_array", 2, params, NULL);
}

void air_curl_execute(zval *self){
	zval *status = zend_read_property(air_curl_ce, self, ZEND_STRL("_status"), 1, NULL);
	if(Z_LVAL_P(status)){
		return ;
	}

	zval *data  = NULL;
	zval *service = zend_read_property(air_curl_ce, self, ZEND_STRL("_service"), 1, NULL);
	if(Z_TYPE_P(service) != IS_NULL){
		data = zend_read_property(air_curl_ce, self, ZEND_STRL("_data"), 1, NULL);
		if(Z_TYPE_P(data) != IS_NULL){
			return ;
		}
		zval _data;
		air_call_object_method(service, Z_OBJCE_P(service), "call", &_data, 0, NULL);
		if(Z_TYPE(_data) == IS_NULL){
			ZVAL_BOOL(&_data, 0);
		}
		zval *ch = zend_read_property(air_curl_ce, self, ZEND_STRL("_ch"), 1, NULL);
		air_curl_update_result(self, ch, &_data);
		zval_ptr_dtor(&_data);
	}else{
		air_curl_init(self);
		air_curl_set_opt_array(self);
		zval *ch = zend_read_property(air_curl_ce, self, ZEND_STRL("_ch"), 1, NULL);
		zval params[1] = {*ch};
		zval result;
		air_call_func("curl_exec", 1, params, &result);
		zval trigger_params[2];
		zval event;
		zval event_params;
		array_init(&event_params);
		Z_ADDREF_P(ch);
		add_next_index_zval(&event_params, ch);
		zval curl_errno;
		air_call_func("curl_errno", 1, params, &curl_errno);
		if(Z_LVAL(curl_errno)){
			ZVAL_STRING(&event, "error");
			zval_ptr_dtor(&result);
		}else{
			ZVAL_STRING(&event, "success");
			add_next_index_zval(&event_params, &result);
		}
		zval_ptr_dtor(&curl_errno);
		trigger_params[0] = event;
		trigger_params[1] = event_params;
		zval _data;
		air_call_object_method(self, air_curl_ce, "trigger", &_data, 2, trigger_params);
		air_curl_update_result(self, ch, &_data);
		zval_ptr_dtor(&_data);
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

	zval opts;
	array_init(&opts);
	add_assoc_long(&opts, "19913", 1);
	zend_update_property(air_curl_ce, self, ZEND_STRL("_opts"), &opts);
	zval_ptr_dtor(&opts);

	zval cb_arr;
	array_init(&cb_arr);
	add_assoc_stringl_ex(&cb_arr, ZEND_STRL("success"), ZEND_STRL("air\\curl::on_success_default"));
	add_assoc_stringl_ex(&cb_arr, ZEND_STRL("error"), ZEND_STRL("air\\curl::on_error_default"));
	zend_update_property(air_curl_ce, self, ZEND_STRL("_callback"), &cb_arr);
	zval_ptr_dtor(&cb_arr);
}

PHP_METHOD(air_curl, offsetExists) {
	AIR_INIT_THIS;
	zend_string *key;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &key) == FAILURE) {
		return;
	} else {
		air_curl_execute(self);
		zval *data = zend_read_property(air_curl_ce, self, ZEND_STRL("_data"), 1, NULL);
		RETURN_BOOL(Z_TYPE_P(data) == IS_ARRAY && zend_hash_exists(Z_ARRVAL_P(data), key));
	}
}

PHP_METHOD(air_curl, offsetSet) {
	php_error(E_WARNING, "curl data is not allowed to reset");
}

PHP_METHOD(air_curl, offsetGet) {
	AIR_INIT_THIS;
	zend_string *key;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &key) == FAILURE) {
		return ;
	}
	air_curl_execute(self);
	zval *data = zend_read_property(air_curl_ce, self, ZEND_STRL("_data"), 1, NULL);
	zval *tmp = NULL;
	long lval;
	double dval;
	if(is_numeric_string(ZSTR_VAL(key), ZSTR_LEN(key), &lval, &dval, 0) != IS_LONG){
		if(Z_TYPE_P(data) != IS_ARRAY){
			php_error(E_WARNING, "can not get the value of key '%s' from a non-array data", key);
			return ;
		}
		if ((tmp = zend_hash_find(Z_ARRVAL_P(data), key)) == NULL) {
			RETURN_NULL();
		}
	}else{
		if(Z_TYPE_P(data) != IS_ARRAY){
			php_error(E_WARNING, "can not get the value of key '%lu' from a non-array data", lval);
			return ;
		}
		if ((tmp = zend_hash_index_find(Z_ARRVAL_P(data), lval)) == NULL) {
			RETURN_NULL();
		}
	}
	RETURN_ZVAL(tmp, 1, 0);
}

PHP_METHOD(air_curl, offsetUnset) {
	AIR_INIT_THIS;

	zend_string *key;
	zval *data;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &key) == FAILURE) {
		return;
	}

	air_curl_execute(self);
	data = zend_read_property(air_curl_ce, self, ZEND_STRL("_data"), 1, NULL);
	if(Z_TYPE_P(data) != IS_ARRAY){
		php_error(E_WARNING, "can not unset a key '%s' from a non-array data", ZSTR_VAL(key));
		RETURN_FALSE;
	}
	if (zend_hash_del(Z_ARRVAL_P(data), key) == SUCCESS) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

PHP_METHOD(air_curl, setopt) {
	AIR_INIT_THIS;
	ulong ok;
	zval *ov;
	if(zend_parse_parameters(ZEND_NUM_ARGS(), "lz", &ok, &ov) == FAILURE){
		php_error(E_WARNING, "invalid air\\curl::setopt($ok, $ov) params");
	}
	air_curl_set_opt(self, NULL, ok, ov);
	AIR_RET_THIS;
}

PHP_METHOD(air_curl, async) {
	AIR_INIT_THIS;
	zval waiter;
	air_call_static_method(air_curl_waiter_ce, "acquire", &waiter, 0, NULL);
	zval params[1] = { *self };
	zval service;
	air_call_object_method(&waiter, air_curl_waiter_ce, "serve", &service, 1, params);
	zend_update_property(air_curl_ce, self, ZEND_STRL("_service"), &service);
	zval_ptr_dtor(&service);
	zval_ptr_dtor(&waiter);
	AIR_RET_THIS;
}

PHP_METHOD(air_curl, get) {
	AIR_INIT_THIS;
	zval *url = NULL;
	zval *params = NULL;
	if(zend_parse_parameters(ZEND_NUM_ARGS(), "z|a", &url, &params) == FAILURE){
	}
	if(params){
		zval qs;
		zval build_params[1] = { *params };
		air_call_func("http_build_query", 1, build_params, &qs);
		if(!Z_ISUNDEF(qs)){
			char *url_str = Z_STRVAL_P(url);
			int url_len = Z_STRLEN_P(url);
			int i = 0;
			while(i < url_len){
				if(url_str[i] == '?'){
					break;
				}
				i++;
			}
			smart_str s = {0};
			smart_str_appends(&s, Z_STRVAL_P(url));
			smart_str_appendc(&s, (i<url_len?'&': '?'));
			smart_str_appends(&s, Z_STRVAL_P(&qs));
			smart_str_0(&s);
			ZVAL_STRINGL(url, ZSTR_VAL(s.s), ZSTR_LEN(s.s));
			smart_str_free(&s);
			zval_ptr_dtor(&qs);
		}
	}
	air_curl_set_opt(self, NULL, CURLOPT_URL, url);
	AIR_RET_THIS;
}

PHP_METHOD(air_curl, post) {
	AIR_INIT_THIS;
	zval *url;
	zval *data = NULL;
	if(zend_parse_parameters(ZEND_NUM_ARGS(), "zz", &url, &data) == FAILURE){
		AIR_NEW_EXCEPTION(1, "invalid air\\curl::post params");
	}
	zval *opts = zend_read_property(air_curl_ce, self, ZEND_STRL("_opts"), 1, NULL);
	air_curl_set_opt(self, &opts, CURLOPT_URL, url);
	zval n1;
	ZVAL_LONG(&n1, 1);
	air_curl_set_opt(self, &opts, CURLOPT_POST, &n1);
	//ptr_dtor is not necessary for scalar types
	zval_ptr_dtor(&n1);
	if(Z_TYPE_P(data) == IS_ARRAY){
		zval build_params[1] = { *data };
		zval built_data;
		air_call_func("http_build_query", 1, build_params, &built_data);
		air_curl_set_opt(self, &opts, CURLOPT_POSTFIELDS, &built_data);
		zval_ptr_dtor(&built_data);
	}else{
		air_curl_set_opt(self, &opts, CURLOPT_POSTFIELDS, data);
	}
	AIR_RET_THIS;
}

PHP_METHOD(air_curl, init) {
	AIR_INIT_THIS;
	air_curl_init(self);
	air_curl_set_opt_array(self);
}

PHP_METHOD(air_curl, exec) {
	AIR_INIT_THIS;
	air_curl_execute(self);
	zval *data = zend_read_property(air_curl_ce, self, ZEND_STRL("_data"), 1, NULL);
	RETURN_ZVAL(data, 1, 0);
}

PHP_METHOD(air_curl, data) {
	AIR_INIT_THIS;
	air_curl_execute(self);
	zval *data = zend_read_property(air_curl_ce, self, ZEND_STRL("_data"), 1, NULL);
	RETURN_ZVAL(data, 1, 0);
}

PHP_METHOD(air_curl, reset) {
	AIR_INIT_THIS;
	zval *ch = zend_read_property(air_curl_ce, self, ZEND_STRL("_ch"), 1, NULL);
	zval params[1] = { *ch };
	air_call_func("curl_reset", 1, params, NULL);
	AIR_RET_THIS;
}

PHP_METHOD(air_curl, get_errno) {
	AIR_INIT_THIS;
	air_curl_execute(self);
	zval *ch = zend_read_property(air_curl_ce, self, ZEND_STRL("_ch"), 1, NULL);
	zval params[1] = { *ch };
	air_call_func("curl_error", 1, params, return_value);
}

PHP_METHOD(air_curl, get_error) {
	AIR_INIT_THIS;
	air_curl_execute(self);
	zval *ch = zend_read_property(air_curl_ce, self, ZEND_STRL("_ch"), 1, NULL);
	zval params[1] = { *ch };
	air_call_func("curl_error", 1, params, return_value);
}

PHP_METHOD(air_curl, trigger) {
	AIR_INIT_THIS;
	zval *event = NULL, *params = NULL;
	if(zend_parse_parameters(ZEND_NUM_ARGS(), "za", &event, &params) == FAILURE){
		return ;
	}
	zval *callback = zend_read_property(air_curl_ce, self, ZEND_STRL("_callback"), 1, NULL);
	zval *event_handler = zend_hash_find(Z_ARRVAL_P(callback), Z_STR_P(event));
	zval _params[2] = { *event_handler, *params};
	air_call_func("call_user_func_array", 2, _params, return_value);
}

PHP_METHOD(air_curl, on_success) {
	AIR_INIT_THIS;
	zval *handler = NULL;
	if(zend_parse_parameters(ZEND_NUM_ARGS(), "z", &handler) == FAILURE){
		return ;
	}
	zval *callback = zend_read_property(air_curl_ce, self, ZEND_STRL("_callback"), 1, NULL);
	Z_TRY_ADDREF_P(handler);
	add_assoc_zval(callback, "success", handler);
	AIR_RET_THIS;
}

PHP_METHOD(air_curl, on_error) {
	AIR_INIT_THIS;
	zval *handler = NULL;
	if(zend_parse_parameters(ZEND_NUM_ARGS(), "z", &handler) == FAILURE){
		return ;
	}
	zval *callback = zend_read_property(air_curl_ce, self, ZEND_STRL("_callback"), 1, NULL);
	Z_TRY_ADDREF_P(handler);
	add_assoc_zval(callback, "error", handler);
	AIR_RET_THIS;
}

PHP_METHOD(air_curl, on_success_default) {
	zval *ch = NULL, *result = NULL;
	if(zend_parse_parameters(ZEND_NUM_ARGS(), "zz", &ch, &result) == FAILURE){
		return ;
	}
	RETURN_ZVAL(result, 1, 0);
}

PHP_METHOD(air_curl, on_error_default) {
	zval *ch;
	if(zend_parse_parameters(ZEND_NUM_ARGS(), "z", &ch) == FAILURE){
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
	PHP_ME(air_curl, init, NULL,  ZEND_ACC_PUBLIC)
	PHP_ME(air_curl, exec, NULL,  ZEND_ACC_PUBLIC)
	PHP_ME(air_curl, data, NULL,  ZEND_ACC_PUBLIC)
	PHP_ME(air_curl, get_errno, NULL,  ZEND_ACC_PUBLIC)
	PHP_ME(air_curl, get_error, NULL,  ZEND_ACC_PUBLIC)
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

	air_curl_ce = zend_register_internal_class_ex(&ce, NULL);
	zend_class_implements(air_curl_ce, 1, zend_ce_arrayaccess);

	zend_declare_property_null(air_curl_ce, ZEND_STRL("_callback"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(air_curl_ce, ZEND_STRL("_ch"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(air_curl_ce, ZEND_STRL("_data"), ZEND_ACC_PROTECTED);
	zend_declare_property_long(air_curl_ce, ZEND_STRL("_status"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_null(air_curl_ce, ZEND_STRL("_service"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(air_curl_ce, ZEND_STRL("_waiter"), ZEND_ACC_PROTECTED | ZEND_ACC_STATIC);

	return SUCCESS;
}
/* }}} */

