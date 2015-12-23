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

#include "ext/pcre/php_pcre.h"
#include "ext/standard/php_string.h"

#include "php_air.h"

#include "air_exception.h"
#include "air_router.h"

zend_class_entry *air_router_ce;

zend_string *air_router_compile(zend_string *rule) {
	zend_string *regex = zend_string_init(ZEND_STRL("#(<([^:]+):([^>]+)>)#"), 0);
	zval replace;
	ZVAL_STRING(&replace, "(?P<$2>$3)");
	zend_string *ret = php_pcre_replace(regex, rule, ZSTR_VAL(rule), ZSTR_LEN(rule), &replace, 0, -1, NULL);
	zend_string_release(regex);
	AIR_PZ_DTOR(&replace);

	return ret;
}

int air_router_route_apply_subpats(zval *cr, zval *subpats, char *key, int key_len) {
	zval *tmp_val = zend_hash_str_find(Z_ARRVAL_P(cr), key, key_len);
	if(tmp_val){
		char *tmp = Z_STRVAL_P(tmp_val);
		if(tmp[0] == '$'){
			tmp_val = zend_hash_str_find(Z_ARRVAL_P(subpats), tmp+1, Z_STRLEN_P(tmp_val)-1);
			if(tmp_val){
				add_assoc_stringl_ex(subpats, key, key_len, Z_STRVAL_P(tmp_val), Z_STRLEN_P(tmp_val));
			}else{
				php_error(E_NOTICE, "ref name '%s' not found in route rule, and 'index' will be set instead", tmp);
				add_assoc_stringl_ex(subpats, key, key_len, ZEND_STRL("index"));
			}
		}else{
			add_assoc_stringl_ex(subpats, key, key_len, Z_STRVAL_P(tmp_val), Z_STRLEN_P(tmp_val));
		}
		return SUCCESS;
	}
	return FAILURE;
}

void air_arr_del_index_el(zval *arr, zval *ret) {
	HashTable *ht = Z_ARRVAL_P(arr);
	for(
		zend_hash_internal_pointer_reset(ht);
		zend_hash_has_more_elements(ht) == SUCCESS;
		zend_hash_move_forward(ht)
	){
		int type;
		ulong idx;
		zend_string *key;
		zval *tmp_data;

		type = zend_hash_get_current_key(ht, &key, &idx);
		if(type == HASH_KEY_IS_STRING){
			tmp_data = zend_hash_get_current_data(ht);
			Z_TRY_ADDREF_P(tmp_data);
			add_assoc_zval_ex(ret, ZSTR_VAL(key), ZSTR_LEN(key), tmp_data);
		}
	}
}

int air_router_route(air_router_t *self, zval *return_value) {
	pcre_cache_entry *pce;
	HashTable *ht_or, *ht_cr;
	zval *original_rules, *compiled_rules, *entry, *cr;
	zval *url;
	zend_string *regex_key = zend_string_init(ZEND_STRL("regex"), 0);

	url = zend_read_property(air_router_ce, self, ZEND_STRL("_url"), 0, NULL);
	original_rules = zend_read_property(air_router_ce, self, ZEND_STRL("_original_rules"), 0, NULL);
	compiled_rules = zend_read_property(air_router_ce, self, ZEND_STRL("_compiled_rules"), 0, NULL);
	ht_or = Z_ARRVAL_P(original_rules);
	ht_cr = Z_ARRVAL_P(compiled_rules);
	for(
		/*zend_hash_internal_pointer_reset(ht_or)*/;
		zend_hash_has_more_elements(ht_or) == SUCCESS;
		zend_hash_move_forward(ht_or)
	){
		if ((entry = zend_hash_get_current_data(ht_or)) == NULL) {
			continue;
		}
		zend_string *key = NULL;
		ulong idx = 0;

		smart_str ss = {0};
		zend_string *regex = NULL;
		//lazy compiling
		zend_hash_get_current_key(ht_or, &key, &idx);
		cr = zend_hash_find(Z_ARRVAL_P(compiled_rules), key);
		if (cr == NULL){
			AIR_PZ_INIT(cr);
			array_init(cr);
			regex = air_router_compile(key);
			char *ca = Z_STRVAL_P(entry);
			int size = 0;
			while(size<Z_STRLEN_P(entry)) {
				if(ca[size] == '.'){
					break;
				}
				size++;
			}
			if(size==0){
				add_assoc_stringl_ex(cr, ZEND_STRL(air_c_key), ZEND_STRL("index"));
			}else{
				add_assoc_stringl_ex(cr, ZEND_STRL(air_c_key), ca, size);
			}
			if(size+1>=Z_STRLEN_P(entry)){
				add_assoc_stringl_ex(cr, ZEND_STRL(air_a_key), ZEND_STRL("index"));
			}else{
				add_assoc_stringl_ex(cr, ZEND_STRL(air_a_key), ca+size+1, Z_STRLEN_P(entry)-size-1);
			}
			if(ZSTR_VAL(regex)[0] != '#'){
				smart_str_appendc(&ss, '#');
			}
			smart_str_appendl(&ss, ZSTR_VAL(regex), ZSTR_LEN(regex));
			if(ZSTR_VAL(regex)[ZSTR_LEN(regex)-1] != '#'){
				smart_str_appendc(&ss, '#');
			}
			smart_str_0(&ss);
			zend_string_release(regex);
			add_assoc_str_ex(cr, ZSTR_VAL(regex_key), ZSTR_LEN(regex_key), ss.s);
			add_assoc_zval_ex(compiled_rules, ZSTR_VAL(key), ZSTR_LEN(key), cr);
			cr = zend_hash_find(Z_ARRVAL_P(compiled_rules), key);
		}else{
			zval *r_r = zend_hash_find(Z_ARRVAL_P(cr), regex_key);
			if (r_r == NULL){
				continue;
			}
			smart_str_appendl(&ss, Z_STRVAL_P(r_r), Z_STRLEN_P(r_r));
			smart_str_0(&ss);
		}
		if(ZSTR_LEN(ss.s) > 0){
			zval *ret = NULL;
			pce = pcre_get_compiled_regex_cache(ss.s);
			smart_str_free(&ss);
			if(pce){
				zval matches, subpats;
				ZVAL_UNDEF(&matches);
				ZVAL_UNDEF(&subpats);
				php_pcre_match_impl(pce, Z_STRVAL_P(url), Z_STRLEN_P(url), &matches, &subpats/* subpats */, 0/* global */, 0/* ZEND_NUM_ARGS() >= 4 */, 0/*flags PREG_OFFSET_CAPTURE*/, 0/* start_offset */);
				if (zend_hash_num_elements(Z_ARRVAL(subpats)) > 0) {
					zval ret;
					array_init(&ret);
					air_arr_del_index_el(&subpats, &ret);
					air_router_route_apply_subpats(cr, &ret, ZEND_STRL(air_c_key));
					air_router_route_apply_subpats(cr, &ret, ZEND_STRL(air_a_key));
					ZVAL_ZVAL(return_value, &ret, 1, 0);
					AIR_PZ_DTOR(&ret);
					AIR_PZ_DTOR(&matches);
					AIR_PZ_DTOR(&subpats);
					//move forward specially
					zend_hash_move_forward(ht_or);
					break;
				}
				AIR_PZ_DTOR(&matches);
				AIR_PZ_DTOR(&subpats);
			}
		}
	}
	zend_string_release(regex_key);
	return SUCCESS;
}

/** {{{ ARG_INFO */
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

/** {{{ PHP METHODS */
PHP_METHOD(air_router, __construct) {
	AIR_INIT_THIS;

	zval or_arr;
	array_init(&or_arr);
	zval cr_arr;
	array_init(&cr_arr);

	zend_update_property(air_router_ce, self, ZEND_STRL("_original_rules"), &or_arr);
	zend_update_property(air_router_ce, self, ZEND_STRL("_compiled_rules"), &cr_arr);
	zval_ptr_dtor(&or_arr);
	zval_ptr_dtor(&cr_arr);
}

PHP_METHOD(air_router, set_url) {
	AIR_INIT_THIS;

	zend_string *url = NULL;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &url) == FAILURE) {
		AIR_NEW_EXCEPTION(1, "invalid set_url param");
	}else{
		zend_update_property_str(air_router_ce, self, ZEND_STRL("_url"), url);
	}
	AIR_RET_THIS;
}

PHP_METHOD(air_router, set_rules) {
	AIR_INIT_THIS;

	zval *rules;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &rules) == FAILURE) {
		php_error(E_ERROR, "invalid air\\router::set_rules($rules) param, it must be an array");
	}else{
		zend_update_property(air_router_ce, self, ZEND_STRL("_original_rules"), rules);
	}

	AIR_RET_THIS;
}

PHP_METHOD(air_router, reset) {
	AIR_INIT_THIS;

	zval *original_rules = zend_read_property(air_router_ce, self, ZEND_STRL("_original_rules"), 1, NULL);
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(original_rules));
	php_error(E_NOTICE, "router cursor has been reset");
	AIR_RET_THIS;
}

PHP_METHOD(air_router, route) {
	AIR_INIT_THIS;
	air_router_route(self, return_value);
}
/* }}} */

/** {{{ air_router_methods */
zend_function_entry air_router_methods[] = {
	PHP_ME(air_router, __construct, air_router_construct_arginfo,  ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(air_router, set_url, air_router_set_url_arginfo,  ZEND_ACC_PUBLIC)
	PHP_ME(air_router, set_rules, air_router_set_rules_arginfo,  ZEND_ACC_PUBLIC)
	PHP_ME(air_router, route, air_router_0_arginfo,  ZEND_ACC_PUBLIC)
	PHP_ME(air_router, reset, air_router_0_arginfo,  ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};
/* }}} */

/** {{{ AIR_MINIT_FUNCTION */
AIR_MINIT_FUNCTION(air_router) {
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "air\\router", air_router_methods);

	air_router_ce = zend_register_internal_class_ex(&ce, NULL);

	zend_declare_property_null(air_router_ce, ZEND_STRL("_url"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(air_router_ce, ZEND_STRL("_original_rules"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(air_router_ce, ZEND_STRL("_compiled_rules"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(air_router_ce, ZEND_STRL("_route"), ZEND_ACC_PROTECTED);

	return SUCCESS;
}
/* }}} */

