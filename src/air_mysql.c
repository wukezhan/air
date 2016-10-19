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
#include "Zend/zend_API.h"
#include "Zend/zend_interfaces.h"

#include "ext/pcre/php_pcre.h"
#include "ext/standard/php_smart_str.h"

#include "php_air.h"

#include "src/air_async_service.h"
#include "src/air_exception.h"
#include "src/air_mysqli.h"
#include "src/air_mysql_waiter.h"

#include "src/air_mysql.h"

zend_class_entry *air_mysql_ce;

extern PHPAPI zend_class_entry *spl_ce_Countable;

#define AIR_MYSQL_PLACEHOLDER "/:([a-z0-9_:]+?)/isU"

void air_mysql_build_data_add(zval *data, smart_str *sql, zval **origin_param){
	smart_str_appendc(sql, '(');
	smart_str values = {0};
	smart_str_appendc(&values, '(');
	ulong idx, _idx = 0;
	zval *val;
	char *key;
	int key_len;
	AIR_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(data), idx, key, key_len, val) {
		if(_idx++){
			smart_str_appends(sql, ", ");
			smart_str_appends(&values, ", ");
		}
		smart_str skey = {0};
		//should be specially renamed
		//if it's necessary?
		smart_str_appendc(&skey, ':');
		smart_str_appendl(&skey, key, key_len-1);
		smart_str_0(&skey);
		smart_str_appendl(sql, key, key_len-1);
		smart_str_appendc(&values, ':');
		smart_str_appendl(&values, skey.c, skey.len);
		Z_ADDREF_P(val);
		add_assoc_zval_ex(*origin_param, skey.c, skey.len+1, val);
		smart_str_free(&skey);
	} AIR_HASH_FOREACH_END();
	smart_str_appendc(sql, ')');
	smart_str_appendc(&values, ')');
	smart_str_appends(sql, " VALUES");
	smart_str_appendl(sql, values.c, values.len);
	smart_str_free(&values);
}

void air_mysql_build_data_set(zval *data, smart_str *sql, zval **origin_param){
	smart_str_appends(sql, " SET ");
	ulong idx, _idx = 0;
	zval *val;
	char *key;
	int key_len;
	AIR_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(data), idx, key, key_len, val) {
		if(_idx++){
			smart_str_appends(sql, ", ");
		}
		smart_str skey = {0};
		//should be specially renamed
		//if it's necessary?
		smart_str_appendc(&skey, ':');
		smart_str_appendl(&skey, key, key_len-1);
		smart_str_0(&skey);
		smart_str_appendl(sql, key, key_len-1);
		smart_str_appends(sql, "=:");
		smart_str_appendl(sql, skey.c, skey.len);
		Z_ADDREF_P(val);
		add_assoc_zval_ex(*origin_param, skey.c, skey.len+1, val);
		smart_str_free(&skey);
	} AIR_HASH_FOREACH_END();
}

void air_mysql_build(zval *self){
	smart_str sql = {0};
	zval *original = zend_read_property(air_mysql_ce, self, ZEND_STRL("_original"), 1 TSRMLS_CC);
	zval *action = air_arr_find(original, ZEND_STRS("action"));
	char *key;
	uint key_len;
	ulong idx;
	uint _idx;
	if(action == NULL){
		php_error(E_ERROR, "error action: action must be add, set, get or del");
	}
	zval *table = air_arr_find(original, ZEND_STRS("table"));
	if(table == NULL){
		php_error(E_ERROR, "error table: air\\mysql::table() can be ignored only if you use air\\mysql::query()");
	}
	zval *debug = zend_read_property(air_mysql_ce, self, ZEND_STRL("_debug"), 1 TSRMLS_CC);
	if(Z_TYPE_P(debug) == IS_ARRAY && zend_hash_num_elements(Z_ARRVAL_P(debug))){
		smart_str_appends(&sql, "/*");
		zval *info;
		_idx = 0;
		AIR_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(debug), idx, key, key_len, info){
			if(_idx++){
				smart_str_appendc(&sql, ' ');
			}
			smart_str_appends(&sql, Z_STRVAL_P(info));
		}AIR_HASH_FOREACH_END();
		smart_str_appends(&sql, "*/");
	}
	zval *origin_param;
	MAKE_STD_ZVAL(origin_param);
	array_init(origin_param);
	zval *tmp = NULL;
	zval *val;
	smart_str values = {0};
	smart_str skey = {0};
	zval *data;
	switch(Z_LVAL_P(action)){
		case AIR_ADD:
			smart_str_appends(&sql, "INSERT INTO ");
			smart_str_appendl(&sql, Z_STRVAL_P(table), Z_STRLEN_P(table));
			data = air_arr_find(original, ZEND_STRS("data"));
			air_mysql_build_data_add(data, &sql, &origin_param);
			break;
		case AIR_GET:
			tmp = air_arr_find(original, ZEND_STRS("fields"));
			smart_str_appends(&sql, "SELECT ");
			smart_str_appendl(&sql, Z_STRVAL_P(tmp), Z_STRLEN_P(tmp));
			smart_str_appends(&sql, " FROM ");
			smart_str_appendl(&sql, Z_STRVAL_P(table), Z_STRLEN_P(table));
			break;
		case AIR_SET:
			smart_str_appends(&sql, "UPDATE ");
			smart_str_appendl(&sql, Z_STRVAL_P(table), Z_STRLEN_P(table));
			data = air_arr_find(original, ZEND_STRS("data"));
			air_mysql_build_data_set(data, &sql, &origin_param);
			break;
		case AIR_DEL:
			smart_str_appends(&sql, "DELETE FROM ");
			smart_str_appendl(&sql, Z_STRVAL_P(table), Z_STRLEN_P(table));
			break;
		default:
			AIR_NEW_EXCEPTION(1, "error action");
	}
	zval *where = air_arr_find(original, ZEND_STRS("where"));
	if(where){
		smart_str_appends(&sql, " WHERE ");
		zval *w0 = air_arr_idx_find(where, 0);
		zval *w1 = air_arr_idx_find(where, 1);
		if(w0){
			smart_str_appendl(&sql, Z_STRVAL_P(w0), Z_STRLEN_P(w0));
		}
		if(w1){
			php_array_merge(Z_ARRVAL_P(origin_param), Z_ARRVAL_P(w1), 0 TSRMLS_CC);
		}
	}
	zval *sort = air_arr_find(original, ZEND_STRS("sort"));
	if(sort){
		smart_str_appends(&sql, " ORDER BY ");
		_idx = 0;
		AIR_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(sort), idx, key, key_len, val) {
			if(_idx++){
				smart_str_appends(&sql, ", ");
			}
			smart_str_appendl(&sql, key, key_len-1);
			if(Z_LVAL_P(val) > 0){
				smart_str_appends(&sql, " ASC");
			}else{
				smart_str_appends(&sql, " DESC");
			}
		} AIR_HASH_FOREACH_END();
	}
	zval *offset = air_arr_find(original, ZEND_STRS("offset"));
	zval *size = air_arr_find(original, ZEND_STRS("size"));
	if(offset || size){
		smart_str_appends(&sql, " LIMIT ");
		if(offset){
			smart_str_append_long(&sql, Z_LVAL_P(offset));
			smart_str_appends(&sql, ", ");
		}
		if(size){
			smart_str_append_long(&sql, Z_LVAL_P(size));
		}else{
			//@todo here 10 should be defined as a constant
			smart_str_append_long(&sql, 10);
		}
	}
	smart_str_0(&sql);
	zval *compiled = zend_read_property(air_mysql_ce, self, ZEND_STRL("_compiled"), 1 TSRMLS_CC);
	add_assoc_stringl(compiled, "tpl", sql.c, sql.len, 1);
	add_assoc_zval(compiled, "var", origin_param);
	zend_update_property(air_mysql_ce, self, ZEND_STRL("_compiled"), compiled TSRMLS_CC);
	smart_str_free(&sql);
}

zval* air_mysqli_escape(zval *compiled){
	zval *sql = air_arr_find(compiled, ZEND_STRS("sql"));
	if(sql){
		return sql;
	}
	pcre_cache_entry *pce;
	if((pce = pcre_get_compiled_regex_cache(ZEND_STRL(AIR_MYSQL_PLACEHOLDER) TSRMLS_CC)) == NULL){
		//should not happen
		php_error(E_ERROR, "unknown error");
	}
	zval *tpl = air_arr_find(compiled, ZEND_STRS("tpl"));
	zval *var = air_arr_find(compiled, ZEND_STRS("var"));
	zval *escaper = air_arr_find(compiled, ZEND_STRS("escaper"));
	zval matches, *subpats = NULL;
	MAKE_STD_ZVAL(subpats);
	ZVAL_NULL(subpats);
	php_pcre_match_impl(pce, Z_STRVAL_P(tpl), Z_STRLEN_P(tpl), &matches, subpats/* subpats */, 1/* global */, 0/* ZEND_NUM_ARGS() >= 4 */, 0/*flags PREG_OFFSET_CAPTURE*/, 0/* start_offset */ TSRMLS_CC);
	if(Z_LVAL_P(&matches)){
		zval *escaped;
		MAKE_STD_ZVAL(escaped);
		array_init(escaped);

		uint tpl_fmt_len;
		zval *fs;
		MAKE_STD_ZVAL(fs);
		ZVAL_STRING(fs, "'%s'", 1);
		char *tpl_fmt = php_pcre_replace(ZEND_STRL(AIR_MYSQL_PLACEHOLDER), Z_STRVAL_P(tpl), Z_STRLEN_P(tpl), fs, 0, &tpl_fmt_len, -1, NULL TSRMLS_CC);

		zval *tval = NULL;
		zval *tok = air_arr_idx_find(subpats, 1);
		ulong idx;
		uint key_len, _idx = 0;
		char *key;
		zval *val;
		AIR_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(tok), idx, key, key_len, val){
			_idx++;
			tval = air_arr_find(var, Z_STRVAL_P(val), Z_STRLEN_P(val)+1);
			if(!tval){
				php_error(E_ERROR, "unknown placeholder: %s", Z_STRVAL_P(val));
			}
			zval *escaped_val = NULL;
			zend_call_method_with_1_params(&escaper, Z_OBJCE_P(escaper), NULL, "real_escape_string", &escaped_val, tval);
			zend_hash_next_index_insert(Z_ARRVAL_P(escaped), &escaped_val, sizeof(zval *), NULL);
		}AIR_HASH_FOREACH_END();

		zval *z_tpl_fmt;
		MAKE_STD_ZVAL(z_tpl_fmt);
		ZVAL_STRINGL(z_tpl_fmt, tpl_fmt, tpl_fmt_len, 0);
		zval *params[2] = {z_tpl_fmt, escaped};
		sql = air_call_func("vsprintf", 2, params);
		if(!sql){
			php_error(E_ERROR, "escape sql error: %s", Z_STRVAL_P(tpl));
		}
		add_assoc_zval(compiled, "sql", sql);
		zval_ptr_dtor(&fs);
		zval_ptr_dtor(&subpats);
		zval_ptr_dtor(&escaped);
		zval_ptr_dtor(&z_tpl_fmt);
		return sql;
	}
	zval_ptr_dtor(&subpats);
	return tpl;
}

zval *air_mysql_trigger_event(zval *self, zval *mysqli, zval *mysqli_result){
	zval **trigger_params[2];
	zval *event;
	MAKE_STD_ZVAL(event);
	zval *event_params;
	MAKE_STD_ZVAL(event_params);
	array_init(event_params);
	Z_ADDREF_P(mysqli);
	add_next_index_zval(event_params, mysqli);
	if(air_mysqli_get_errno(mysqli)){
		ZVAL_STRING(event, "error", 1);
	}else{
		ZVAL_STRING(event, "success", 1);
		Z_ADDREF_P(mysqli_result);
		add_next_index_zval(event_params, mysqli_result);
	}
	trigger_params[0] = &event;
	trigger_params[1] = &event_params;
	zval *results = NULL;
	air_call_method(&self, air_mysql_ce, NULL, ZEND_STRL("trigger"), &results, 2, trigger_params TSRMLS_CC);
	zval_ptr_dtor(&mysqli_result);
	zval_ptr_dtor(&event);
	zval_ptr_dtor(&event_params);
	return results;
}

void air_mysql_update_result(zval *self, zval *result){
	if(Z_TYPE_P(result) != IS_ARRAY){
		php_error(E_ERROR, "type error: air\\mysql event handler must return an array");
	}
	zval *data = air_arr_find(result, ZEND_STRS("data"));
	if(data){
		zend_update_property(air_mysql_ce, self, ZEND_STRL("_data"), data);
		zend_update_property(air_mysql_ce, self, ZEND_STRL("_num_rows"), air_arr_find(result, ZEND_STRS("num_rows")));
		zend_update_property_long(air_mysql_ce, self, ZEND_STRL("_status"), 1);
	}else{
		zval *ar = air_arr_find(result, ZEND_STRS("affected_rows"));
		if(ar){
			zend_update_property(air_mysql_ce, self, ZEND_STRL("_affected_rows"), ar);
			zval *insert_id = air_arr_find(result, ZEND_STRS("insert_id"));
			if(insert_id){
				zend_update_property(air_mysql_ce, self, ZEND_STRL("_insert_id"), insert_id);
			}
			zend_update_property_long(air_mysql_ce, self, ZEND_STRL("_status"), 1);
		}else{
			zval *mysql_errno = air_arr_find(result, ZEND_STRS("errno"));
			if(mysql_errno){
				zend_update_property(air_mysql_ce, self, ZEND_STRL("_errno"), mysql_errno);
				zend_update_property(air_mysql_ce, self, ZEND_STRL("_error"), air_arr_find(result, ZEND_STRS("error")));
			}
			zend_update_property_long(air_mysql_ce, self, ZEND_STRL("_status"), -1);
		}
	}
	zval_ptr_dtor(&result);
}

int air_mysql_auto_mode(zval *self){
	int async_enabled = 0;
	//check if it is a select query
	zval *original = zend_read_property(air_mysql_ce, self, ZEND_STRL("_original"), 1 TSRMLS_CC);
	zval *action = air_arr_find(original, ZEND_STRS("action"));
	if(action == NULL){
		zval *compiled = zend_read_property(air_mysql_ce, self, ZEND_STRL("_compiled"), 1 TSRMLS_CC);
		zval *tpl = air_arr_find(compiled, ZEND_STRS("tpl"));
		if(tpl && Z_STRLEN_P(tpl)> 7){
			char *tpl_str = Z_STRVAL_P(tpl);
			if((tpl_str[0] == 's'||tpl_str[0] == 'S')
					&& (tpl_str[2] == 'l'||tpl_str[2] == 'L')
					&& (tpl_str[4] == 'c'||tpl_str[4] == 'C')
					&& tpl_str[6] == ' '){
				async_enabled = 1;
			}
		}else{
			// $mysql2->async()->query('id>:id', ['id' => $mysql1->data()[0]['id']]);
			// may cause this E_ERROR
			php_error(E_ERROR, "query cannot be empty, you should not use a async data before it been returned");
		}
	}else if(Z_LVAL_P(action) == AIR_GET){
		async_enabled = 1;
	}
	//if it's a modify and mode not been set, then must set mode to 2(write)
	int _mode;
	zval *mode = zend_read_property(air_mysql_ce, self, ZEND_STRL("_mode"), 1 TSRMLS_CC);
	if(Z_TYPE_P(mode) == IS_NULL){
		_mode = async_enabled?AIR_R: AIR_W;
		zend_update_property_long(air_mysql_ce, self, ZEND_STRL("_mode"), _mode);
	}else{
		_mode = Z_LVAL_P(mode);
		if(!async_enabled && _mode == AIR_R){
			//if it might be a modify but mode is set to AIR_R
			php_error(E_NOTICE, "mode is set to AIR_R, but query is not a SELECT, check if it's ok");
		}
	}
	return async_enabled;
}

void air_mysql_execute(zval *self){
	zval *data = NULL;
	zval *status = zend_read_property(air_mysql_ce, self, ZEND_STRL("_status"), 1 TSRMLS_CC);
	if(!Z_LVAL_P(status)){
		zval *service = zend_read_property(air_mysql_ce, self, ZEND_STRL("_service"), 1 TSRMLS_CC);
		int async_enabled = air_mysql_auto_mode(self);
		zval *mode = zend_read_property(air_mysql_ce, self, ZEND_STRL("_mode"), 1 TSRMLS_CC);
		if(Z_TYPE_P(service) != IS_NULL && async_enabled){
			//asynchronous query
			air_call_method(&service, Z_OBJCE_P(service), NULL, ZEND_STRL("call"), &data, 0, NULL);
			if(Z_TYPE_P(data) == IS_NULL){
				array_init(data);
			}
			air_mysql_update_result(self, data);
		}else{
			//synchronous query
			zend_class_entry *mysql_keeper_ce = air_get_ce(ZEND_STRL("air\\mysql\\keeper") TSRMLS_CC);
			zval *mysqli = NULL;
			zval *config = zend_read_property(air_mysql_ce, self, ZEND_STRL("_config"), 0 TSRMLS_CC);
			zval **acquire_params[2] = {&config, &mode};
			air_call_static_method(mysql_keeper_ce, "acquire", &mysqli, 2, acquire_params);
			if(mysqli == NULL || Z_TYPE_P(mysqli) != IS_OBJECT){
				php_error(E_ERROR, "can not get a mysqli instance: config '%s', mode '%lu'", Z_STRVAL_P(config), Z_LVAL_P(mode));
			}
			zval **build_params[1] = {&mysqli};
			zval *sql = NULL;
			air_call_method(&self, air_mysql_ce, NULL, ZEND_STRL("build"), &sql, 1, build_params);
			if(sql){
				zval **query_params[1] = {&sql};
				zval *mysqli_result = NULL;
				air_call_method(&mysqli, Z_OBJCE_P(mysqli), NULL, ZEND_STRL("query"), &mysqli_result, 1, query_params);
				data = air_mysql_trigger_event(self, mysqli, mysqli_result);
				if(Z_TYPE_P(data) == IS_NULL){
					array_init(data);
				}
				air_mysql_update_result(self, data);
				zval_ptr_dtor(&sql);
			}
			zval **release_params[3] = {&mysqli, &config, &mode};
			air_call_static_method(mysql_keeper_ce, "release", NULL, 3, release_params);
			zval_ptr_dtor(&mysqli);
		}
	}
}

/* {{{ ARG_INFO */
ZEND_BEGIN_ARG_INFO_EX(air_mysql_construct_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, config)
	ZEND_ARG_INFO(0, table)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_mysql_void_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_mysql_k_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_mysql_kv_arginfo, 0, 0, 2)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_mysql_table_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, table)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_mysql_mode_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, mode)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_mysql_get_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, fields)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_mysql_data_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_mysql_where_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, conds)
	ZEND_ARG_INFO(0, values)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_mysql_by_key_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, value)
	ZEND_ARG_INFO(0, pk_name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_mysql_sort_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, sort)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_mysql_offset_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, offset)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_mysql_size_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, size)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_mysql_query_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, sql)
	ZEND_ARG_INFO(0, params)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_mysql_build_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, escaper)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_mysql_trigger_arginfo, 0, 0, 2)
	ZEND_ARG_INFO(0, event)
	ZEND_ARG_INFO(0, param_array)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_mysql_on_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, handler)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ PHP METHODS */
PHP_METHOD(air_mysql, __construct) {
	AIR_INIT_THIS;

	zval *config;
	zval *table = NULL;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|z", &config, &table) == FAILURE){
		return ;
	}
	zend_update_property(air_mysql_ce, self, ZEND_STRL("_config"), config);

	zval *debug_info;

	MAKE_STD_ZVAL(debug_info);
	array_init(debug_info);
	zend_update_property(air_mysql_ce, self, ZEND_STRL("_debug"), debug_info);
	zval_ptr_dtor(&debug_info);

	zval *orig;
	MAKE_STD_ZVAL(orig);
	array_init(orig);
	if(table){
		Z_ADDREF_P(table);
		add_assoc_zval(orig, "table", table);
	}
	zend_update_property(air_mysql_ce, self, ZEND_STRL("_original"), orig);
	zval_ptr_dtor(&orig);

	zval *arr;
	MAKE_STD_ZVAL(arr);
	array_init(arr);
	zend_update_property(air_mysql_ce, self, ZEND_STRL("_compiled"), arr);
	zval_ptr_dtor(&arr);

	zval *cb_arr;
	MAKE_STD_ZVAL(cb_arr);
	array_init(cb_arr);
	zval *cb_succ;
	MAKE_STD_ZVAL(cb_succ);
	array_init(cb_succ);
	add_next_index_string(cb_succ, "air\\mysql", 1);
	add_next_index_string(cb_succ, "on_success_default", 1);
	zval *cb_err;
	MAKE_STD_ZVAL(cb_err);
	array_init(cb_err);
	add_next_index_string(cb_err, "air\\mysql", 1);
	add_next_index_string(cb_err, "on_error_default", 1);
	add_assoc_zval(cb_arr, "success", cb_succ);
	add_assoc_zval(cb_arr, "error", cb_err);
	zend_update_property(air_mysql_ce, self, ZEND_STRL("_callback"), cb_arr);
	zval_ptr_dtor(&cb_arr);
	int is_debug = 0;
	zval _debug;
	if(zend_get_constant(ZEND_STRL("DEBUG"), &_debug TSRMLS_CC)){
		is_debug = Z_LVAL(_debug);
		zval_dtor(&_debug);
	}
	zend_execute_data *ced = EG(current_execute_data);
	while(ced && is_debug){
		if(ced->op_array && ced->opline){
			const char *c = ced->op_array->filename;
			int len = strlen(c);
			int idx = len - 1;
			int ds_cnt = 0;
			while(c[idx]){
				if(c[idx]=='/'){
					ds_cnt++;
					if(ds_cnt>2){
						idx++;
						break;
					}
				}
				idx--;
			}
			char *tmp;
			len = spprintf(&tmp, 0, "@%s#%d", ced->op_array->filename+idx, ced->opline->lineno);
			zval *debug = zend_read_property(air_mysql_ce, self, ZEND_STRL("_debug"), 1 TSRMLS_CC);
			add_next_index_stringl(debug, tmp, len, 0);
			break;
		}else{
			ced = ced->prev_execute_data;
		}
	}
}

PHP_METHOD(air_mysql, config) {
	AIR_INIT_THIS;
	zval *config = zend_read_property(air_mysql_ce, self, ZEND_STRL("_config"), 1 TSRMLS_CC);
	RETURN_ZVAL(config, 1, 0);
}

PHP_METHOD(air_mysql, offsetExists) {
	AIR_INIT_THIS;
	char *key;
	uint len;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &key, &len) == FAILURE) {
		return;
	} else {
		air_mysql_execute(self);
		zval *data = zend_read_property(air_mysql_ce, self, ZEND_STRL("_data"), 1 TSRMLS_CC);
		RETURN_BOOL(zend_hash_exists(Z_ARRVAL_P(data), key, len + 1));
	}
}

PHP_METHOD(air_mysql, offsetSet) {
	php_error(E_WARNING, "mysql data is not allowed to reset");
}

PHP_METHOD(air_mysql, offsetGet) {
	AIR_INIT_THIS;
	char *key;
	int len;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &key, &len) == FAILURE) {
		return ;
	}
	air_mysql_execute(self);
	zval *data = zend_read_property(air_mysql_ce, self, ZEND_STRL("_data"), 1 TSRMLS_CC);
	zval **tmp;
	long lval;
	double dval;
	if(is_numeric_string(key, len, &lval, &dval, 0) != IS_LONG){
		if (zend_hash_find(Z_ARRVAL_P(data), key, len + 1, (void **) &tmp) == FAILURE) {
			RETURN_NULL();
		}
	}else{
		if (zend_hash_index_find(Z_ARRVAL_P(data), lval, (void **) &tmp) == FAILURE) {
			RETURN_NULL();
		}
	}
	RETURN_ZVAL(*tmp, 1, 0);
}

PHP_METHOD(air_mysql, offsetUnset) {
	AIR_INIT_THIS;

	zval *key, *data;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &key) == FAILURE) {
		return;
	}

	if (Z_TYPE_P(key) != IS_STRING || !Z_STRLEN_P(key)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Expect a string key name");
		RETURN_FALSE;
	}

	air_mysql_execute(self);
	data = zend_read_property(air_mysql_ce, self, ZEND_STRL("_data"), 1 TSRMLS_CC);
	if (zend_hash_del(Z_ARRVAL_P(data), Z_STRVAL_P(key), Z_STRLEN_P(key) + 1) == SUCCESS) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

PHP_METHOD(air_mysql, count) {
	AIR_INIT_THIS;
	air_mysql_execute(self);
	zval *data = zend_read_property(air_mysql_ce, self, ZEND_STRL("_data"), 1 TSRMLS_CC);
	RETURN_LONG(zend_hash_num_elements(Z_ARRVAL_P(data)));
}

PHP_METHOD(air_mysql, rewind) {
	AIR_INIT_THIS;
	air_mysql_execute(self);
	zval *data = zend_read_property(air_mysql_ce, self, ZEND_STRL("_data"), 1 TSRMLS_CC);
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(data));
}

PHP_METHOD(air_mysql, current) {
	AIR_INIT_THIS;
	zval *data, **ppzval, *ret;

	air_mysql_execute(self);
	data = zend_read_property(air_mysql_ce, self, ZEND_STRL("_data"), 1 TSRMLS_CC);
	if (zend_hash_get_current_data(Z_ARRVAL_P(data), (void **)&ppzval) == FAILURE) {
		RETURN_FALSE;
	}

	RETURN_ZVAL(*ppzval, 1, 0);
}

PHP_METHOD(air_mysql, key) {
	AIR_INIT_THIS;
	char *key;
	ulong index;

	air_mysql_execute(self);
	zval *data = zend_read_property(air_mysql_ce, self, ZEND_STRL("_data"), 1 TSRMLS_CC);
	zend_hash_get_current_key(Z_ARRVAL_P(data), &key, &index, 0);
	switch(zend_hash_get_current_key_type(Z_ARRVAL_P(data))) {
		case HASH_KEY_IS_LONG:
			RETURN_LONG(index);
			break;
		case HASH_KEY_IS_STRING:
			RETURN_STRING(key, 1);
			break;
		default:
			RETURN_FALSE;
	}
}

PHP_METHOD(air_mysql, next) {
	AIR_INIT_THIS;
	air_mysql_execute(self);
	zval *data = zend_read_property(air_mysql_ce, self, ZEND_STRL("_data"), 1 TSRMLS_CC);
	zend_hash_move_forward(Z_ARRVAL_P(data));
	RETURN_TRUE;
}

PHP_METHOD(air_mysql, valid) {
	AIR_INIT_THIS;
	air_mysql_execute(self);
	zval *data = zend_read_property(air_mysql_ce, self, ZEND_STRL("_data"), 1 TSRMLS_CC);
	RETURN_BOOL(zend_hash_has_more_elements(Z_ARRVAL_P(data)) == SUCCESS);
}

PHP_METHOD(air_mysql, serialize) {
	AIR_INIT_THIS;
	air_mysql_execute(self);
	zval *data = zend_read_property(air_mysql_ce, self, ZEND_STRL("_data"), 1 TSRMLS_CC);
	zval *serialize;
	MAKE_STD_ZVAL(serialize);
	ZVAL_STRING(serialize, "serialize", 1);
	zval *params[1] = {data};
	if(call_user_function(EG(function_table), NULL, serialize, return_value, 1, params TSRMLS_CC) == FAILURE){
	}
	zval_ptr_dtor(&serialize);
}

PHP_METHOD(air_mysql, unserialize) {
}

PHP_METHOD(air_mysql, data) {
	AIR_INIT_THIS;
	air_mysql_execute(self);
	zval *data = zend_read_property(air_mysql_ce, self, ZEND_STRL("_data"), 1 TSRMLS_CC);
	RETURN_ZVAL(data, 1, 0);
}

PHP_METHOD(air_mysql, affected_rows) {
	AIR_INIT_THIS;
	air_mysql_execute(self);
	zval *rows = zend_read_property(air_mysql_ce, self, ZEND_STRL("_affected_rows"), 1 TSRMLS_CC);
	RETURN_ZVAL(rows, 1, 0);
}

PHP_METHOD(air_mysql, num_rows) {
	AIR_INIT_THIS;
	air_mysql_execute(self);
	zval *rows = zend_read_property(air_mysql_ce, self, ZEND_STRL("_num_rows"), 1 TSRMLS_CC);
	RETURN_ZVAL(rows, 1, 0);
}

PHP_METHOD(air_mysql, insert_id) {
	AIR_INIT_THIS;
	air_mysql_execute(self);
	zval *insert_id = zend_read_property(air_mysql_ce, self, ZEND_STRL("_insert_id"), 1 TSRMLS_CC);
	RETURN_ZVAL(insert_id, 1, 0);
}

PHP_METHOD(air_mysql, get_errno) {
	AIR_INIT_THIS;
	air_mysql_execute(self);
	zval *prop = zend_read_property(air_mysql_ce, self, ZEND_STRL("_errno"), 1 TSRMLS_CC);
	RETURN_ZVAL(prop, 1, 0);
}

PHP_METHOD(air_mysql, get_error) {
	AIR_INIT_THIS;
	air_mysql_execute(self);
	zval *prop = zend_read_property(air_mysql_ce, self, ZEND_STRL("_error"), 1 TSRMLS_CC);
	RETURN_ZVAL(prop, 1, 0);
}

PHP_METHOD(air_mysql, jsonSerialize) {
	AIR_INIT_THIS;
	zval *t;
	MAKE_STD_ZVAL(t);
	array_init(t);
	RETURN_ZVAL(t, 1, 0);
}

PHP_METHOD(air_mysql, async) {
	AIR_INIT_THIS;
	zval *waiter;
	air_call_static_method(air_mysql_waiter_ce, "acquire", &waiter, 0, NULL);
	zval **params[1] = {&self};
	zval *service;
	air_call_method(&waiter, air_mysql_waiter_ce, NULL, ZEND_STRL("serve"), &service, 1, params);
	zend_update_property(air_mysql_ce, self, ZEND_STRL("_service"), service);
	zval_ptr_dtor(&service);
	zval_ptr_dtor(&waiter);
	zval *debug = zend_read_property(air_mysql_ce, self, ZEND_STRL("_debug"), 1 TSRMLS_CC);
	add_next_index_string(debug, "async", 1);
	AIR_RET_THIS;
}

PHP_METHOD(air_mysql, mode) {
	AIR_INIT_THIS;
	long mode = AIR_R;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &mode) == FAILURE){
		AIR_NEW_EXCEPTION(1, "error mode");
	}
	if(mode != AIR_R && mode != AIR_W){
		AIR_NEW_EXCEPTION(1, "error mode, it must be AIR_R or AIR_W");
	}
	zend_update_property_long(air_mysql_ce, self, ZEND_STRL("_mode"), mode TSRMLS_CC);
	AIR_RET_THIS;
}

PHP_METHOD(air_mysql, table) {
	AIR_INIT_THIS;
	char *table;
	int len = 0;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &table, &len) == FAILURE){
		AIR_NEW_EXCEPTION(1, "error table");
	}
	zval *original = zend_read_property(air_mysql_ce, self, ZEND_STRL("_original"), 1 TSRMLS_CC);
	add_assoc_stringl(original, "table", table, len, 1);
	AIR_RET_THIS;
}

PHP_METHOD(air_mysql, add) {
	AIR_INIT_THIS;
	zval *data;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &data) == FAILURE){
		return ;
	}
	zval *original = zend_read_property(air_mysql_ce, self, ZEND_STRL("_original"), 1 TSRMLS_CC);
	add_assoc_long_ex(original, ZEND_STRS("action"), AIR_ADD);
	Z_ADDREF_P(data);
	add_assoc_zval(original, "data", data);

	AIR_RET_THIS;
}

PHP_METHOD(air_mysql, get) {
	AIR_INIT_THIS;
	char *fields;
	int len;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &fields, &len) == FAILURE){
		return ;
	}
	zval *original = zend_read_property(air_mysql_ce, self, ZEND_STRL("_original"), 1 TSRMLS_CC);
	add_assoc_long_ex(original, ZEND_STRS("action"), AIR_GET);
	add_assoc_stringl(original, "fields", fields, len, 1);

	AIR_RET_THIS;
}

PHP_METHOD(air_mysql, set) {
	AIR_INIT_THIS;
	zval *data;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &data) == FAILURE){
		return ;
	}
	zval *original = zend_read_property(air_mysql_ce, self, ZEND_STRL("_original"), 1 TSRMLS_CC);
	add_assoc_long_ex(original, ZEND_STRS("action"), AIR_SET);
	Z_ADDREF_P(data);
	add_assoc_zval(original, "data", data);

	AIR_RET_THIS;
}

PHP_METHOD(air_mysql, del) {
	AIR_INIT_THIS;
	zval *original = zend_read_property(air_mysql_ce, self, ZEND_STRL("_original"), 1 TSRMLS_CC);
	add_assoc_long_ex(original, ZEND_STRS("action"), AIR_DEL);

	AIR_RET_THIS;
}
PHP_METHOD(air_mysql, where) {
	AIR_INIT_THIS;
	zval *conds;
	zval *values = NULL;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|a", &conds, &values) == FAILURE){
		return ;
	}
	if(Z_TYPE_P(conds) != IS_STRING){
		AIR_NEW_EXCEPTION(1, "invalid air\\mysql::where conds value");
	}
	zval *original = zend_read_property(air_mysql_ce, self, ZEND_STRL("_original"), 1 TSRMLS_CC);
	zval *where;
	MAKE_STD_ZVAL(where);
	array_init(where);
	Z_ADDREF_P(conds);
	zend_hash_next_index_insert(Z_ARRVAL_P(where), &conds, sizeof(zval *), NULL);
	if(values){
		Z_ADDREF_P(values);
		zend_hash_next_index_insert(Z_ARRVAL_P(where), &values, sizeof(zval *), NULL);
	}
	add_assoc_zval(original, "where", where);

	AIR_RET_THIS;
}

PHP_METHOD(air_mysql, by_key) {
	AIR_INIT_THIS;
	zval *value;
	zval *pk_name = NULL;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|z", &value, &pk_name) == FAILURE){
		return ;
	}
	zval *free_pk_name = NULL;
	if(!pk_name){
		MAKE_STD_ZVAL(free_pk_name);
		ZVAL_STRING(free_pk_name, "id", 1);
		pk_name = free_pk_name;
	}
	zval *original = zend_read_property(air_mysql_ce, self, ZEND_STRL("_original"), 1 TSRMLS_CC);
	zval *where;
	MAKE_STD_ZVAL(where);
	array_init(where);
	char *pk_placeholder = NULL;
	int len = spprintf(&pk_placeholder, 0, ":%s", Z_STRVAL_P(pk_name));
	char *conds;
	len = spprintf(&conds, 0, "`%s`=%s", Z_STRVAL_P(pk_name), pk_placeholder);
	zval *zconds;
	MAKE_STD_ZVAL(zconds);
	ZVAL_STRINGL(zconds, conds, len, 0);
	zend_hash_next_index_insert(Z_ARRVAL_P(where), &zconds, sizeof(zval *), NULL);
	zval *values;
	MAKE_STD_ZVAL(values);
	array_init(values);
	Z_ADDREF_P(value);
	add_assoc_zval(values, Z_STRVAL_P(pk_name), value);
	zend_hash_next_index_insert(Z_ARRVAL_P(where), &values, sizeof(zval *), NULL);
	add_assoc_zval(original, "where", where);

	if(free_pk_name){
		zval_ptr_dtor(&free_pk_name);
	}
	efree(pk_placeholder);

	AIR_RET_THIS;
}

PHP_METHOD(air_mysql, sort) {
	AIR_INIT_THIS;
	zval *sort;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &sort) == FAILURE){
		return ;
	}
	zval *original = zend_read_property(air_mysql_ce, self, ZEND_STRL("_original"), 1 TSRMLS_CC);
	Z_ADDREF_P(sort);
	add_assoc_zval(original, "sort", sort);
	AIR_RET_THIS;
}

PHP_METHOD(air_mysql, offset) {
	AIR_INIT_THIS;
	ulong offset = 0;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &offset) == FAILURE){
		return ;
	}
	zval *original = zend_read_property(air_mysql_ce, self, ZEND_STRL("_original"), 1 TSRMLS_CC);
	add_assoc_long(original, "offset", offset);
	AIR_RET_THIS;
}

PHP_METHOD(air_mysql, size) {
	AIR_INIT_THIS;
	long size = 10;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "l", &size) == FAILURE){
		return ;
	}
	zval *original = zend_read_property(air_mysql_ce, self, ZEND_STRL("_original"), 1 TSRMLS_CC);
	add_assoc_long(original, "size", size);
	AIR_RET_THIS;
}

PHP_METHOD(air_mysql, query) {
	AIR_INIT_THIS;
	zval *sql;
	zval *param = NULL;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z|a", &sql, &param) == FAILURE || Z_TYPE_P(sql) != IS_STRING){
		AIR_NEW_EXCEPTION(1, "error params");
		return ;
	}
	zval *param_tmp = NULL;
	if(!param){
		MAKE_STD_ZVAL(param_tmp);
		array_init(param_tmp);
		param = param_tmp;
	}
	zval *compiled = zend_read_property(air_mysql_ce, self, ZEND_STRL("_compiled"), 1 TSRMLS_CC);
	if(!compiled){
		MAKE_STD_ZVAL(compiled);
		array_init(compiled);
		zend_update_property(air_mysql_ce, self, ZEND_STRL("_compiled"), compiled TSRMLS_CC);
		zval_ptr_dtor(&compiled);
		compiled = zend_read_property(air_mysql_ce, self, ZEND_STRL("_compiled"), 1 TSRMLS_CC);
	}
	Z_ADDREF_P(sql);
	Z_ADDREF_P(param);
	add_assoc_zval(compiled, "tpl", sql);
	add_assoc_zval(compiled, "var", param);
	zval *empty_array;
	MAKE_STD_ZVAL(empty_array);
	array_init(empty_array);
	zend_update_property(air_mysql_ce, self, ZEND_STRL("_original"), empty_array TSRMLS_CC);
	zval_ptr_dtor(&empty_array);
	if(param_tmp){
		zval_ptr_dtor(&param_tmp);
	}
	AIR_RET_THIS;
}

PHP_METHOD(air_mysql, build) {
	AIR_INIT_THIS;
	zval *escaper;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &escaper) == FAILURE){
		AIR_NEW_EXCEPTION(1, "error params");
	}
	zval *compiled = zend_read_property(air_mysql_ce, self, ZEND_STRL("_compiled"), 1 TSRMLS_CC);
	Z_ADDREF_P(escaper);
	add_assoc_zval(compiled, "escaper", escaper);
	zval *tpl = air_arr_find(compiled, ZEND_STRS("tpl"));
	if(!tpl){
		air_mysql_build(self);
		compiled = zend_read_property(air_mysql_ce, self, ZEND_STRL("_compiled"), 1 TSRMLS_CC);
	}
	zval *sql = air_mysqli_escape(compiled);
	if(sql){
		RETURN_ZVAL(sql, 1, 0);
	}else{
		RETURN_NULL();
	}
}

PHP_METHOD(air_mysql, trigger) {
	AIR_INIT_THIS;
	zval *event, *params;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "za", &event, &params) == FAILURE){
		return ;
	}
	zval *callback = zend_read_property(air_mysql_ce, self, ZEND_STRL("_callback"), 1 TSRMLS_CC);
	zval *event_handler = air_arr_find(callback, Z_STRVAL_P(event), Z_STRLEN_P(event)+1);
	zval *_params[2] = {event_handler, params};
	zval *ret = air_call_func("call_user_func_array", 2, _params);
	RETURN_ZVAL(ret, 1, 1);
}

PHP_METHOD(air_mysql, on_success) {
	AIR_INIT_THIS;
	zval *handler;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &handler) == FAILURE){
		return ;
	}
	zval *callback = zend_read_property(air_mysql_ce, self, ZEND_STRL("_callback"), 1 TSRMLS_CC);
	Z_ADDREF_P(handler);
	add_assoc_zval(callback, "success", handler);
	AIR_RET_THIS;
}

PHP_METHOD(air_mysql, on_error) {
	AIR_INIT_THIS;
	zval *handler;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &handler) == FAILURE){
		return ;
	}
	zval *callback = zend_read_property(air_mysql_ce, self, ZEND_STRL("_callback"), 1 TSRMLS_CC);
	Z_ADDREF_P(handler);
	add_assoc_zval(callback, "error", handler);
	AIR_RET_THIS;
}

PHP_METHOD(air_mysql, on_success_default) {
	zval *mysqli, *mysqli_result;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zz", &mysqli, &mysqli_result) == FAILURE){
		return ;
	}
	zval *assoc;
	MAKE_STD_ZVAL(assoc);
	ZVAL_LONG(assoc, MYSQLI_ASSOC);
	zval **fetch_params[1] = {&assoc};
	zval *results = NULL;
	zval *ret;
	MAKE_STD_ZVAL(ret);
	array_init(ret);
	zval *rows = air_mysqli_get_total_rows(mysqli);
	if(Z_TYPE_P(mysqli_result) == IS_BOOL){
		add_assoc_zval(ret, "affected_rows", rows);
		zval *insert_id = air_mysqli_get_insert_id(mysqli);
		if(Z_LVAL_P(insert_id)>0){
			add_assoc_zval(ret, "insert_id", insert_id);
		}else{
			zval_ptr_dtor(&insert_id);
		}
	}else{
		air_call_method(&mysqli_result, Z_OBJCE_P(mysqli_result), NULL, ZEND_STRL("fetch_all"), &results, 1, fetch_params TSRMLS_CC);
		add_assoc_zval(ret, "data", results);
		add_assoc_zval(ret, "num_rows", rows);
		air_call_method(&mysqli_result, Z_OBJCE_P(mysqli_result), NULL, ZEND_STRL("free"), NULL, 0, NULL TSRMLS_CC);
	}
	zval_ptr_dtor(&assoc);
	RETURN_ZVAL(ret, 1, 1);
}

PHP_METHOD(air_mysql, on_error_default) {
	zval *mysqli;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &mysqli) == FAILURE){
		return ;
	}
	ulong mysqli_errno = air_mysqli_get_errno(mysqli);
	char *mysqli_error = air_mysqli_get_error(mysqli);
	zval *ret;
	MAKE_STD_ZVAL(ret);
	array_init(ret);
	add_assoc_long(ret, "errno", mysqli_errno);
	add_assoc_string(ret, "error", mysqli_error, 1);
	RETURN_ZVAL(ret, 1, 1);
}

PHP_METHOD(air_mysql, __destruct) {
	AIR_INIT_THIS;
	zval *status = zend_read_property(air_mysql_ce, self, ZEND_STRL("_status"), 1 TSRMLS_CC);
	if(!Z_LVAL_P(status)){
		//todo remove
		air_mysql_execute(self);
	}
}

/* {{{ air_mysql_methods */
zend_function_entry air_mysql_methods[] = {
	PHP_ME(air_mysql, __construct, air_mysql_construct_arginfo,  ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(air_mysql, __destruct, air_mysql_void_arginfo,  ZEND_ACC_PUBLIC | ZEND_ACC_DTOR)
	PHP_ME(air_mysql, count, air_mysql_void_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql, offsetUnset, air_mysql_k_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql, offsetGet, air_mysql_k_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql, offsetExists, air_mysql_k_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql, offsetSet, air_mysql_kv_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql, rewind, air_mysql_void_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql, current, air_mysql_void_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql, next,	air_mysql_void_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql, valid, air_mysql_void_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql, key, air_mysql_void_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql, serialize, air_mysql_void_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql, unserialize, air_mysql_data_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql, data, air_mysql_void_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql, affected_rows, air_mysql_void_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql, num_rows, air_mysql_void_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql, insert_id, air_mysql_void_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql, get_error, air_mysql_void_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql, get_errno, air_mysql_void_arginfo, ZEND_ACC_PUBLIC)
	//methods
	PHP_ME(air_mysql, table, air_mysql_table_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql, mode, air_mysql_mode_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql, async, air_mysql_void_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql, add, air_mysql_data_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql, get, air_mysql_get_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql, set, air_mysql_data_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql, del, air_mysql_void_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql, where, air_mysql_where_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql, by_key, air_mysql_by_key_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql, sort, air_mysql_sort_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql, offset, air_mysql_offset_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql, size, air_mysql_size_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql, query, air_mysql_query_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql, build, air_mysql_build_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql, trigger, air_mysql_trigger_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql, on_success, air_mysql_on_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql, on_error, air_mysql_on_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql, on_success_default, air_mysql_on_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(air_mysql, on_error_default, air_mysql_on_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};
/* }}} */

/* {{{ AIR_MINIT_FUNCTION */
AIR_MINIT_FUNCTION(air_mysql) {
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "air\\mysql", air_mysql_methods);

	air_mysql_ce = zend_register_internal_class_ex(&ce, NULL, NULL TSRMLS_CC);
	zend_class_implements(air_mysql_ce TSRMLS_CC, 4, zend_ce_iterator, zend_ce_arrayaccess, spl_ce_Countable, zend_ce_serializable);

	zend_declare_property_null(air_mysql_ce, ZEND_STRL("_waiter"), ZEND_ACC_PROTECTED | ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_null(air_mysql_ce, ZEND_STRL("_service"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(air_mysql_ce, ZEND_STRL("_config"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(air_mysql_ce, ZEND_STRL("_mode"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_long(air_mysql_ce, ZEND_STRL("_status"), 0, ZEND_ACC_PROTECTED TSRMLS_CC);

	//keep results
	zend_declare_property_null(air_mysql_ce, ZEND_STRL("_data"), ZEND_ACC_PROTECTED TSRMLS_CC);
	//can be affected rows, or the count
	zend_declare_property_null(air_mysql_ce, ZEND_STRL("_affected_rows"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(air_mysql_ce, ZEND_STRL("_num_rows"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(air_mysql_ce, ZEND_STRL("_insert_id"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_long(air_mysql_ce, ZEND_STRL("_errno"), 0, ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_string(air_mysql_ce, ZEND_STRL("_error"), "", ZEND_ACC_PROTECTED TSRMLS_CC);
	//keep params
	zend_declare_property_null(air_mysql_ce, ZEND_STRL("_debug"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(air_mysql_ce, ZEND_STRL("_original"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(air_mysql_ce, ZEND_STRL("_compiled"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(air_mysql_ce, ZEND_STRL("_callback"), ZEND_ACC_PROTECTED TSRMLS_CC);
	return SUCCESS;
}
/* }}} */

