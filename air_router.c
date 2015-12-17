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

#include "ext/pcre/php_pcre.h"
#include "ext/standard/php_string.h"
#include "ext/standard/php_smart_str.h"

#include "php_air.h"

#include "air_exception.h"
#include "air_router.h"

zend_class_entry *air_router_ce;

char* air_router_compile(char *key, uint key_len, int *result_len) {
	char *ret;
	zval *replace;
	MAKE_STD_ZVAL(replace);
	ZVAL_STRING(replace, "(?P<$2>$3)", 1);
	ret = php_pcre_replace(ZEND_STRL("#(<([^:]+):([^>]+)>)#"), key, key_len, replace, 0, result_len, -1, NULL TSRMLS_CC);
	zval_ptr_dtor(&replace);

	return ret;
}

ZEND_RESULT_CODE air_router_route_apply_subpats(zval *r, zval *subpats, char *key, int key_len, char *ro, int ro_len) {
	zval *tmp_val = air_arr_find(r, key, key_len);
	if(tmp_val){
		char *tmp = Z_STRVAL_P(tmp_val);
		if(tmp[0] == '$'){
			tmp_val = air_arr_find(subpats, tmp+1, strlen(tmp));
			if(tmp_val){
				add_assoc_stringl_ex(subpats, key, key_len, Z_STRVAL_P(tmp_val), Z_STRLEN_P(tmp_val), 1);
			}else{
				php_error(E_NOTICE, "ref name '%s' not found in route rule '%s', and 'index' will be set instead,", tmp,  ro TSRMLS_CC);
				add_assoc_stringl_ex(subpats, key, key_len, ZEND_STRL("index"), 1);
			}
		}else{
			add_assoc_stringl_ex(subpats, key, key_len, Z_STRVAL_P(tmp_val), Z_STRLEN_P(tmp_val), 1);
		}
		return SUCCESS;
	}
	return FAILURE;
}

zval* air_arr_del_index_el(zval *arr) {
	HashTable *ht = Z_ARRVAL_P(arr);
	zval *tmp;
	MAKE_STD_ZVAL(tmp);
	array_init(tmp);
	for(
		zend_hash_internal_pointer_reset(ht);
		zend_hash_has_more_elements(ht) == SUCCESS;
		zend_hash_move_forward(ht)
	){
		int type;
		ulong idx;
		char *key;
		uint key_len;
		zval **tmp_data;

		type = zend_hash_get_current_key_ex(ht, &key, &key_len, &idx, 0, NULL);
		if(type == HASH_KEY_IS_STRING){
			if(zend_hash_get_current_data(ht, (void**)&tmp_data) != FAILURE) {
				add_assoc_stringl_ex(tmp, key, key_len, Z_STRVAL_PP(tmp_data), Z_STRLEN_PP(tmp_data), 1);
			}
		}
	}
	return tmp;
}

zval *air_router_route(air_router_t *self) {
	zval *return_value;
	MAKE_STD_ZVAL(return_value);
	ZVAL_NULL(return_value);

	pcre_cache_entry *pce;
	HashTable *ht_or, *ht_cr;
	zval *original_rules, *compiled_rules, **entry, *r;
	zval *url;

	url = zend_read_property(air_router_ce, self, ZEND_STRL("_url"), 0 TSRMLS_CC);
	original_rules = zend_read_property(air_router_ce, self, ZEND_STRL("_original_rules"), 0 TSRMLS_CC);
	compiled_rules = zend_read_property(air_router_ce, self, ZEND_STRL("_compiled_rules"), 0 TSRMLS_CC);
	ht_or = Z_ARRVAL_P(original_rules);
	ht_cr = Z_ARRVAL_P(compiled_rules);
	for(
		/*zend_hash_internal_pointer_reset(ht_or)*/;
		zend_hash_has_more_elements(ht_or) == SUCCESS;
		zend_hash_move_forward(ht_or)
	){
		if (zend_hash_get_current_data(ht_or, (void**)&entry) == FAILURE) {
			continue;
		}
		char *key;
		uint key_len = 0;
		ulong idx = 0;

		smart_str ss = {0};
		char *regex = NULL;
		uint regex_len = 0;
		//lazy compiling
		zend_hash_get_current_key_ex(ht_or, &key, &key_len, &idx, 0, NULL);
		r = air_arr_find(compiled_rules, key, key_len);
		if (r == NULL){
			MAKE_STD_ZVAL(r);
			array_init(r);
			regex = air_router_compile(key, key_len, &regex_len);
			char *ca = Z_STRVAL_PP(entry);
			int size = 0;
			while(size<Z_STRLEN_PP(entry)) {
				if(ca[size] == '.'){
					break;
				}
				size++;
			}
			if(size==0){
				add_assoc_stringl_ex(r, ZEND_STRS(air_c_key), ZEND_STRS("index"), 1);
			}else{
				add_assoc_stringl_ex(r, ZEND_STRS(air_c_key), ca, size, 1);
			}
			if(size+1>=Z_STRLEN_PP(entry)){
				add_assoc_stringl_ex(r, ZEND_STRS(air_a_key), ZEND_STRS("index"), 1);
			}else{
				add_assoc_stringl_ex(r, ZEND_STRS(air_a_key), ca+size+1, Z_STRLEN_PP(entry)-size-1, 1);
			}
			add_assoc_zval_ex(compiled_rules, key, key_len, r);
			smart_str_appendc(&ss, '#');
			smart_str_appendl(&ss, regex, regex_len-1);
			smart_str_appendc(&ss, '#');
			smart_str_0(&ss);
			efree(regex);
			add_assoc_stringl_ex(r, ZEND_STRS("regex"), ss.c, ss.len, 1);
		}else{
			zval *r_r = air_arr_find(r, ZEND_STRS("regex"));
			if (r_r == NULL){
				continue;
			}
			smart_str_appendl(&ss, Z_STRVAL_P(r_r), Z_STRLEN_P(r_r));
			smart_str_0(&ss);
		}
		if(ss.len > 0){
			zval *ret;
			pce = pcre_get_compiled_regex_cache(ss.c, ss.len TSRMLS_CC);
			smart_str_free(&ss);
			if(pce){
				zval matches, *subpats;
				MAKE_STD_ZVAL(subpats);
				ZVAL_NULL(subpats);
				php_pcre_match_impl(pce, Z_STRVAL_P(url), Z_STRLEN_P(url), &matches, subpats/* subpats */, 0/* global */, 0/* ZEND_NUM_ARGS() >= 4 */, 0/*flags PREG_OFFSET_CAPTURE*/, 0/* start_offset */ TSRMLS_CC);
				if (zend_hash_num_elements(Z_ARRVAL_P(subpats)) > 0) {
					ret = air_arr_del_index_el(subpats);
					air_router_route_apply_subpats(r, ret, ZEND_STRS(air_c_key), key, key_len);
					air_router_route_apply_subpats(r, ret, ZEND_STRS(air_a_key), key, key_len);
					ZVAL_ZVAL(return_value, ret, 1, 0);
					zval_ptr_dtor(&ret);
					zval_ptr_dtor(&subpats);
					//move forward specially
					zend_hash_move_forward(ht_or);
					break;
				}
				zval_ptr_dtor(&subpats);
			}
		}
	}
	return return_value;
}

/* {{{ ARG_INFO */
ZEND_BEGIN_ARG_INFO_EX(air_router_construct_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, config)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_router_set_url_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, url)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_router_set_rules_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, rules)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_router_0_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ PHP METHODS */
PHP_METHOD(air_router, __construct) {
	AIR_INIT_THIS;

	zval *or_arr;
	MAKE_STD_ZVAL(or_arr);
	array_init(or_arr);
	zval *cr_arr;
	MAKE_STD_ZVAL(cr_arr);
	array_init(cr_arr);

	zend_update_property(air_router_ce, self, ZEND_STRL("_original_rules"), or_arr TSRMLS_CC);
	zend_update_property(air_router_ce, self, ZEND_STRL("_compiled_rules"), cr_arr TSRMLS_CC);
	zval_ptr_dtor(&or_arr);
	zval_ptr_dtor(&cr_arr);
}

PHP_METHOD(air_router, set_url) {
	AIR_INIT_THIS;

	char *url;
	uint len;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &url, &len) == FAILURE) {
		AIR_NEW_EXCEPTION(1, "invalid set_url param");
	}else{
		zend_update_property_stringl(air_router_ce, self, ZEND_STRL("_url"), url, len TSRMLS_CC);
	}
	AIR_RET_THIS;
}

PHP_METHOD(air_router, set_rules) {
	AIR_INIT_THIS;

	zval *rules;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &rules) == FAILURE) {
	}else{
		zend_update_property(air_router_ce, self, ZEND_STRL("_original_rules"), rules TSRMLS_CC);
		zval *original_rules = zend_read_property(air_router_ce, self, ZEND_STRL("_original_rules"), 1 TSRMLS_CC);
		zend_hash_internal_pointer_reset(Z_ARRVAL_P(original_rules));
	}

	AIR_RET_THIS;
}

PHP_METHOD(air_router, reset) {
	AIR_INIT_THIS;

	zval *original_rules = zend_read_property(air_router_ce, self, ZEND_STRL("_original_rules"), 1 TSRMLS_CC);
	HashTable *ro = Z_ARRVAL_P(original_rules);
	zend_hash_internal_pointer_reset(ro);
	php_error(E_NOTICE, "router cursor has been reset");
	AIR_RET_THIS;
}

PHP_METHOD(air_router, route) {
	AIR_INIT_THIS;
	zval *ret = air_router_route(self);
	RETURN_ZVAL(ret, 1, 1);
}
/* }}} */

/* {{{ air_router_methods */
zend_function_entry air_router_methods[] = {
	PHP_ME(air_router, __construct, air_router_construct_arginfo,  ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(air_router, set_url, air_router_set_url_arginfo,  ZEND_ACC_PUBLIC)
	PHP_ME(air_router, set_rules, air_router_set_rules_arginfo,  ZEND_ACC_PUBLIC)
	PHP_ME(air_router, route, air_router_0_arginfo,  ZEND_ACC_PUBLIC)
	PHP_ME(air_router, reset, air_router_0_arginfo,  ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};
/* }}} */

/* {{{ AIR_MINIT_FUNCTION */
AIR_MINIT_FUNCTION(air_router) {
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "air\\router", air_router_methods);

	air_router_ce = zend_register_internal_class_ex(&ce, NULL, NULL TSRMLS_CC);

	zend_declare_property_null(air_router_ce, ZEND_STRL("_url"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(air_router_ce, ZEND_STRL("_original_rules"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(air_router_ce, ZEND_STRL("_compiled_rules"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(air_router_ce, ZEND_STRL("_route"), ZEND_ACC_PROTECTED TSRMLS_CC);

	return SUCCESS;
}
/* }}} */

