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
#include "Zend/zend_smart_str.h"

#include "ext/pcre/php_pcre.h"
#include "ext/standard/php_smart_string.h"

#include "php_air.h"

#include "air_async_service.h"
#include "air_exception.h"
#include "air_mysqli.h"
#include "air_mysql_waiter.h"

#include "air_mysql_builder.h"

zend_class_entry *air_mysql_builder_ce;

extern PHPAPI zend_class_entry *spl_ce_Countable;

#define AIR_MYSQL_BUILDER_PLACEHOLDER "/:([a-z0-9_:]+?)/isU"

void air_mysql_builder_data_set(zval *data, smart_str *sql, zval *param){
	ulong idx, _idx = 0;
	zend_string *key;
	zval *val;
	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(data), idx, key, val) {
		smart_str skey = {0};
		if(_idx++){
			smart_str_appends(sql, ", ");
		}
		smart_str_appendc(&skey, ':');
		smart_str_appendl(&skey, ZSTR_VAL(key), ZSTR_LEN(key));
		smart_str_0(&skey);
		smart_str_appendl(sql, ZSTR_VAL(key), ZSTR_LEN(key));
		smart_str_appends(sql, "=:");
		smart_str_append_smart_str(sql, &skey);
		Z_TRY_ADDREF_P(val);
		add_assoc_zval_ex(param, ZSTR_VAL(skey.s), ZSTR_LEN(skey.s), val);
		smart_str_free(&skey);
	} ZEND_HASH_FOREACH_END();
}

void air_mysql_builder_data_add(zval *data, smart_str *sql, zval *origin_param){
	smart_str values = {0};
	smart_str_appendc(sql, '(');
	smart_str_appendc(&values, '(');
	ulong idx, _idx = 0;
	zend_string *key;
	zval *val;
	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(data), idx, key, val) {
		smart_str skey = {0};
		if(_idx++){
			smart_str_appends(sql, ", ");
			smart_str_appends(&values, ", ");
		}
		smart_str_appendc(&skey, ':');
		smart_str_append(&skey, key);
		smart_str_0(&skey);
		smart_str_append(sql, key);
		smart_str_appendc(&values, ':');
		smart_str_append_smart_str(&values, &skey);
		Z_TRY_ADDREF_P(val);
		add_assoc_zval_ex(origin_param, ZSTR_VAL(skey.s), ZSTR_LEN(skey.s), val);
		smart_str_free(&skey);
	} ZEND_HASH_FOREACH_END();
	smart_str_appendc(sql, ')');
	smart_str_appendc(&values, ')');
	smart_str_appends(sql, " VALUES");
	smart_str_append_smart_str(sql, &values);
	smart_str_free(&values);
}
void air_mysql_builder_build(zval *self){
	zval *original = zend_read_property(air_mysql_builder_ce, self, ZEND_STRL("_original"), 1, NULL);
	zval *action = zend_hash_str_find(Z_ARRVAL_P(original), ZEND_STRL("action"));
	zend_string *key;
	ulong idx;
	ulong _idx;
	if(action == NULL){
		php_error(E_ERROR, "error action: action must be add, set, get or del");
	}
	zval *table = zend_hash_str_find(Z_ARRVAL_P(original), ZEND_STRL("table"));
	if(table == NULL){
		php_error(E_ERROR, "error table: air\\mysql\\buildr::table() can be ignored only if you use air\\mysql\\builder::query()");
	}
	zval *debug = zend_read_property(air_mysql_builder_ce, self, ZEND_STRL("_debug"), 1, NULL);
	smart_str sql = {0};
	if(Z_TYPE_P(debug) == IS_ARRAY && zend_hash_num_elements(Z_ARRVAL_P(debug))){
		smart_str_appends(&sql, "/*");
		zval *info;
		_idx = 0;
		ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(debug), idx, key, info){
			if(_idx++){
				smart_str_appendc(&sql, ' ');
			}
			smart_str_appends(&sql, Z_STRVAL_P(info));
		}ZEND_HASH_FOREACH_END();
		smart_str_appends(&sql, "*/");
	}
	zval origin_param;
	array_init(&origin_param);
	zval *tmp = NULL;
	zval *val;
	zval *data;
	switch(Z_LVAL_P(action)){
		case AIR_ADD:
			data = zend_hash_str_find(Z_ARRVAL_P(original), ZEND_STRL("data"));
			smart_str_appends(&sql, "INSERT INTO ");
			smart_str_append(&sql, Z_STR_P(table));
			air_mysql_builder_data_add(data, &sql, &origin_param);
			break;
		case AIR_GET:
			tmp = zend_hash_str_find(Z_ARRVAL_P(original), ZEND_STRL("fields"));
			smart_str_appends(&sql, "SELECT ");
			smart_str_append(&sql, Z_STR_P(tmp));
			smart_str_appends(&sql, " FROM ");
			smart_str_append(&sql, Z_STR_P(table));
			break;
		case AIR_SET:
			smart_str_appends(&sql, "UPDATE ");
			smart_str_append(&sql, Z_STR_P(table));
			smart_str_appends(&sql, " SET ");
			data = zend_hash_str_find(Z_ARRVAL_P(original), ZEND_STRL("data"));
			air_mysql_builder_data_set(data, &sql, &origin_param);
			break;
		case AIR_DEL:
			smart_str_appends(&sql, "DELETE FROM ");
			smart_str_append(&sql, Z_STR_P(table));
			break;
		default:
			AIR_NEW_EXCEPTION(1, "error action");
	}
	zval *where = zend_hash_str_find(Z_ARRVAL_P(original), ZEND_STRL("where"));
	if(where){
		smart_str_appends(&sql, " WHERE ");
		zval *w0 = zend_hash_index_find(Z_ARRVAL_P(where), 0);
		zval *w1 = zend_hash_index_find(Z_ARRVAL_P(where), 1);
		if(w0){
			smart_str_append(&sql, Z_STR_P(w0));
		}
		if(w1){
			php_array_merge(Z_ARRVAL(origin_param), Z_ARRVAL_P(w1));
		}
	}
	zval *sort = zend_hash_str_find(Z_ARRVAL_P(original), ZEND_STRL("sort"));
	if(sort){
		smart_str_appends(&sql, " ORDER BY ");
		_idx = 0;
		ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(sort), idx, key, val) {
			if(_idx++){
				smart_str_appends(&sql, ", ");
			}
			smart_str_append(&sql, key);
			if(Z_LVAL_P(val) > 0){
				smart_str_appends(&sql, " ASC");
			}else{
				smart_str_appends(&sql, " DESC");
			}
		} ZEND_HASH_FOREACH_END();
	}
	zval *offset = zend_hash_str_find(Z_ARRVAL_P(original), ZEND_STRL("offset"));
	zval *size = zend_hash_str_find(Z_ARRVAL_P(original), ZEND_STRL("size"));
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
	zval *compiled = zend_read_property(air_mysql_builder_ce, self, ZEND_STRL("_compiled"), 1, NULL);
	add_assoc_str(compiled, "tpl", sql.s);
	add_assoc_zval(compiled, "var", &origin_param);
	zend_update_property(air_mysql_builder_ce, self, ZEND_STRL("_compiled"), compiled);
	//smart_str_free(&sql);
}

zval* air_mysqli_escape(zval *compiled){
	zval *sql = zend_hash_str_find(Z_ARRVAL_P(compiled), ZEND_STRL("sql"));
	if(sql){
		return sql;
	}
	zend_string *placeholder = zend_string_init(ZEND_STRL(AIR_MYSQL_BUILDER_PLACEHOLDER), 0);
	pcre_cache_entry *pce;
	if((pce = pcre_get_compiled_regex_cache(placeholder)) == NULL){
		//should not happen
		php_error(E_ERROR, "unknown error");
	}
	zval *tpl = zend_hash_str_find(Z_ARRVAL_P(compiled), ZEND_STRL("tpl"));
	zval *var = zend_hash_str_find(Z_ARRVAL_P(compiled), ZEND_STRL("var"));
	zval *escaper = zend_hash_str_find(Z_ARRVAL_P(compiled), ZEND_STRL("escaper"));
	zval matches, subpats;
	ZVAL_UNDEF(&subpats);
	php_pcre_match_impl(pce, Z_STRVAL_P(tpl), Z_STRLEN_P(tpl), &matches, &subpats/* subpats */, 1/* global */, 0/* ZEND_NUM_ARGS() >= 4 */, 0/*flags PREG_OFFSET_CAPTURE*/, 0/* start_offset */);
	if(Z_LVAL_P(&matches)){
		zval escaped;
		array_init(&escaped);

		zval fs;
		ZVAL_STRING(&fs, "'%s'");
		zend_string *tpl_fmt = php_pcre_replace(placeholder, NULL, Z_STRVAL_P(tpl), Z_STRLEN_P(tpl), &fs, 0, -1, NULL);
		zend_string_release(placeholder);

		zval *tval = NULL;
		zval *tok = zend_hash_index_find(Z_ARRVAL_P(&subpats), 1);
		ulong idx;
		uint _idx = 0;
		zend_string *key;
		zval *val;
		ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(tok), idx, key, val){
			_idx++;
			tval = zend_hash_find(Z_ARRVAL_P(var), Z_STR_P(val));
			if(!tval){
				php_error(E_ERROR, "unknown placeholder: %s", Z_STRVAL_P(val));
			}
			zval escaped_val;
			zend_call_method_with_1_params(escaper, Z_OBJCE_P(escaper), NULL, "real_escape_string", &escaped_val, tval);
			zend_hash_next_index_insert(Z_ARRVAL(escaped), &escaped_val);
		}ZEND_HASH_FOREACH_END();

		zval z_tpl_fmt;
		ZVAL_STR(&z_tpl_fmt, tpl_fmt);
		zval params[2] = {z_tpl_fmt, escaped};
		zval _sql;
		air_call_func("vsprintf", 2, params, &_sql);
		if(Z_ISNULL(_sql)){
			php_error(E_ERROR, "escape sql error: %s", Z_STRVAL_P(tpl));
		}
		add_assoc_zval(compiled, "sql", &_sql);
		zval_ptr_dtor(&fs);
		zval_ptr_dtor(&subpats);
		zval_ptr_dtor(&escaped);
		zend_string_release(tpl_fmt);
		zval_ptr_dtor(&z_tpl_fmt);
		return zend_hash_str_find(Z_ARRVAL_P(compiled), ZEND_STRL("sql"));
	}
	zend_string_release(placeholder);
	zval_ptr_dtor(&subpats);
	return tpl;
}

void air_mysql_builder_trigger_event(zval *self, zval *mysqli, zval *mysqli_result, zval *return_value){
	zval event;
	zval event_params;
	array_init(&event_params);
	Z_TRY_ADDREF_P(mysqli);
	add_next_index_zval(&event_params, mysqli);
	if(air_mysqli_get_errno(mysqli)){
		ZVAL_STRING(&event, "error");
	}else{
		ZVAL_STRING(&event, "success");
		Z_TRY_ADDREF_P(mysqli_result);
		add_next_index_zval(&event_params, mysqli_result);
	}
	zval trigger_params[2] = {event, event_params};
	air_call_object_method(self, air_mysql_builder_ce, "trigger", return_value, 2, trigger_params);
	zval_ptr_dtor(&event);
	zval_ptr_dtor(&event_params);
}

void air_mysql_builder_update_result(zval *self, zval *result){
	if(Z_TYPE_P(result) != IS_ARRAY){
		php_error(E_ERROR, "type error: air\\mysql\\builder event handler must return an array");
	}
	zval *data = zend_hash_str_find(Z_ARRVAL_P(result), ZEND_STRL("data"));
	if(data){
		zend_update_property(air_mysql_builder_ce, self, ZEND_STRL("_data"), data);
		zend_update_property(air_mysql_builder_ce, self, ZEND_STRL("_num_rows"), zend_hash_str_find(Z_ARRVAL_P(result), ZEND_STRL("num_rows")));
		zend_update_property_long(air_mysql_builder_ce, self, ZEND_STRL("_status"), 1);
	}else{
		zval *ar = zend_hash_str_find(Z_ARRVAL_P(result), ZEND_STRL("affected_rows"));
		if(ar){
			zend_update_property(air_mysql_builder_ce, self, ZEND_STRL("_affected_rows"), ar);
			zval *insert_id = zend_hash_str_find(Z_ARRVAL_P(result), ZEND_STRL("insert_id"));
			if(insert_id){
				zend_update_property(air_mysql_builder_ce, self, ZEND_STRL("_insert_id"), insert_id);
			}
			zend_update_property_long(air_mysql_builder_ce, self, ZEND_STRL("_status"), 1);
		}else{
			zval *mysql_errno = zend_hash_str_find(Z_ARRVAL_P(result), ZEND_STRL("errno"));
			if(mysql_errno){
				zend_update_property(air_mysql_builder_ce, self, ZEND_STRL("_errno"), mysql_errno);
				zend_update_property(air_mysql_builder_ce, self, ZEND_STRL("_error"), zend_hash_str_find(Z_ARRVAL_P(result), ZEND_STRL("error")));
			}
			zend_update_property_long(air_mysql_builder_ce, self, ZEND_STRL("_status"), -1);
		}
	}
}

int air_mysql_builder_auto_mode(zval *self){
	int async_enabled = 0;
	//check if it is a select query
	zval *original = zend_read_property(air_mysql_builder_ce, self, ZEND_STRL("_original"), 1, NULL);
	zval *action = zend_hash_str_find(Z_ARRVAL_P(original), ZEND_STRL("action"));
	if(action == NULL){
		zval *compiled = zend_read_property(air_mysql_builder_ce, self, ZEND_STRL("_compiled"), 1, NULL);
		zval *tpl = zend_hash_str_find(Z_ARRVAL_P(compiled), ZEND_STRL("tpl"));
		if(tpl && Z_STRLEN_P(tpl)> 7){
			char *tpl_str = Z_STRVAL_P(tpl);
			if((tpl_str[0] == 's'||tpl_str[0] == 'S')
					&& (tpl_str[2] == 'l'||tpl_str[2] == 'L')
					&& (tpl_str[4] == 'C'||tpl_str[4] == 'C')
					&& tpl_str[6] == ' '){
				async_enabled = 1;
			}
		}else{
			php_error(E_ERROR, "query cannot be empty");
		}
	}else if(Z_LVAL_P(action) == AIR_GET){
		async_enabled = 1;
	}
	//if it's a modify and mode not been set, then must set mode to 2(write)
	int _mode;
	zval *mode = zend_read_property(air_mysql_builder_ce, self, ZEND_STRL("_mode"), 1, NULL);
	if(Z_TYPE_P(mode) == IS_NULL){
		_mode = async_enabled?AIR_R: AIR_W;
		zend_update_property_long(air_mysql_builder_ce, self, ZEND_STRL("_mode"), _mode);
	}else{
		_mode = Z_LVAL_P(mode);
		if(!async_enabled && _mode == AIR_R){
			//if it might be a modify but mode is set to AIR_R
			php_error(E_NOTICE, "mode is set to AIR_R, but query is not a SELECT, check if it's ok");
		}
	}
	return async_enabled;
}

void air_mysql_builder_execute(zval *self){
	zval data;
	ZVAL_UNDEF(&data);
	zval *status = zend_read_property(air_mysql_builder_ce, self, ZEND_STRL("_status"), 1, NULL);
	if(!Z_LVAL_P(status)){
		zval *service = zend_read_property(air_mysql_builder_ce, self, ZEND_STRL("_service"), 1, NULL);
		int async_enabled = air_mysql_builder_auto_mode(self);
		zval *mode = zend_read_property(air_mysql_builder_ce, self, ZEND_STRL("_mode"), 1, NULL);
		if(Z_TYPE_P(service) != IS_NULL && async_enabled){
			//asynchronous query
			air_call_object_method(service, Z_OBJCE_P(service), "call", &data, 0, NULL);
			if(Z_ISUNDEF(data) || Z_TYPE(data) == IS_NULL){
				array_init(&data);
			}
			air_mysql_builder_update_result(self, &data);
			zval_ptr_dtor(&data);
		}else{
			//synchronous query
			zend_class_entry *mysql_keeper_ce = air_get_ce(ZEND_STRL("air\\mysql\\keeper"));
			zval mysqli;
			zval *config = zend_read_property(air_mysql_builder_ce, self, ZEND_STRL("_config"), 0, NULL);
			zval factory_params[2] = {*config, *mode};
			air_call_static_method(mysql_keeper_ce, "factory", &mysqli, 2, factory_params);
			if(Z_TYPE(mysqli) != IS_OBJECT){
				php_error(E_ERROR, "can not get a mysqli instance: config '%s', mode '%lu'", Z_STRVAL_P(config), Z_LVAL_P(mode));
			}
			zval build_params[1] = {mysqli};
			zval sql;
			air_call_object_method(self, air_mysql_builder_ce, "build", &sql, 1, build_params);
			if(!Z_ISUNDEF(sql)){
				zval query_params[1] = {sql};
				zval mysqli_result;
				air_call_object_method(&mysqli, Z_OBJCE(mysqli), "query", &mysqli_result, 1, query_params);
				air_mysql_builder_trigger_event(self, &mysqli, &mysqli_result, &data);
				zval_ptr_dtor(&mysqli_result);
				if(Z_ISUNDEF(data) || Z_TYPE(data) == IS_NULL){
					array_init(&data);
				}
				air_mysql_builder_update_result(self, &data);
				zval_ptr_dtor(&data);
			}
			zval_ptr_dtor(&sql);
			zval release_params[3] = {mysqli, *config, *mode};
			air_call_static_method(mysql_keeper_ce, "release", NULL, 3, release_params);
			zval_ptr_dtor(&mysqli);
		}
	}
}

/** {{{ ARG_INFO */
ZEND_BEGIN_ARG_INFO_EX(air_mysql_builder_construct_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, config)
	ZEND_ARG_INFO(0, table)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_mysql_builder_void_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_mysql_builder_k_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, key)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_mysql_builder_kv_arginfo, 0, 0, 2)
	ZEND_ARG_INFO(0, key)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_mysql_builder_table_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, table)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_mysql_builder_mode_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, mode)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_mysql_builder_get_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, fields)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_mysql_builder_data_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, data)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_mysql_builder_where_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, conds)
	ZEND_ARG_INFO(0, values)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_mysql_builder_by_key_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, value)
	ZEND_ARG_INFO(0, pk_name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_mysql_builder_sort_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, sort)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_mysql_builder_offset_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, offset)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_mysql_builder_size_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, size)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_mysql_builder_query_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, sql)
	ZEND_ARG_INFO(0, params)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_mysql_builder_build_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, escaper)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_mysql_builder_trigger_arginfo, 0, 0, 2)
	ZEND_ARG_INFO(0, event)
	ZEND_ARG_INFO(0, param_array)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_mysql_builder_on_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, handler)
ZEND_END_ARG_INFO()
/* }}} */

/** {{{ PHP METHODS */
PHP_METHOD(air_mysql_builder, __construct) {
	AIR_INIT_THIS;

	zval *config = NULL;
	zval *table = NULL;
	if(zend_parse_parameters(ZEND_NUM_ARGS(), "z|z", &config, &table) == FAILURE){
		return ;
	}
	zend_update_property(air_mysql_builder_ce, self, ZEND_STRL("_config"), config);

	zval arr;
	array_init(&arr);
	zend_update_property(air_mysql_builder_ce, self, ZEND_STRL("_debug"), &arr);
	zval_ptr_dtor(&arr);

	zval orig;
	array_init(&orig);
	if(table){
		Z_TRY_ADDREF_P(table);
		add_assoc_zval(&orig, "table", table);
	}
	zend_update_property(air_mysql_builder_ce, self, ZEND_STRL("_original"), &orig);
	zval_ptr_dtor(&orig);

	zval carr;
	array_init(&carr);
	zend_update_property(air_mysql_builder_ce, self, ZEND_STRL("_compiled"), &carr);
	zval_ptr_dtor(&carr);

	zval cb_arr;
	array_init(&cb_arr);
	add_assoc_string(&cb_arr, "success", "air\\mysql\\builder::on_success_default");
	add_assoc_string(&cb_arr, "error", "air\\mysql\\builder::on_error_default");
	zend_update_property(air_mysql_builder_ce, self, ZEND_STRL("_callback"), &cb_arr);
	zval_ptr_dtor(&cb_arr);
	int is_debug = 0;
	zval *_debug;
	if((_debug = zend_get_constant_str(ZEND_STRL("DEBUG"))) != NULL){
		is_debug = Z_LVAL_P(_debug);
	}
	zend_execute_data *ced = EG(current_execute_data);
	while(ced && is_debug){
		zend_string *s = (ced->func->op_array).filename;
		if(s && ced->func && ced->opline){
			char *c = ZSTR_VAL(s);
			int idx = ZSTR_LEN(s) - 1;
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
			if(idx>-1){
				char *tmp;
				int len = spprintf(&tmp, 0, "@%s#%d", c+idx, ced->opline->lineno);
				zval *debug = zend_read_property(air_mysql_builder_ce, self, ZEND_STRL("_debug"), 1, NULL);
				add_next_index_stringl(debug, tmp, len);
				efree(tmp);
				break;
			}
		}
		ced = ced->prev_execute_data;
	}
}

PHP_METHOD(air_mysql_builder, config) {
	AIR_INIT_THIS;
	zval *config = zend_read_property(air_mysql_builder_ce, self, ZEND_STRL("_config"), 1, NULL);
	RETURN_ZVAL(config, 1, 0);
}

PHP_METHOD(air_mysql_builder, offsetExists) {
	AIR_INIT_THIS;
	zend_string *key;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &key) == FAILURE) {
		return;
	} else {
		air_mysql_builder_execute(self);
		zval *data = zend_read_property(air_mysql_builder_ce, self, ZEND_STRL("_data"), 1, NULL);
		RETURN_BOOL(zend_hash_exists(Z_ARRVAL_P(data), key));
	}
}

PHP_METHOD(air_mysql_builder, offsetSet) {
	php_error(E_WARNING, "builder data is not allowed to reset");
}

PHP_METHOD(air_mysql_builder, offsetGet) {
	AIR_INIT_THIS;
	zend_string *key;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S", &key) == FAILURE) {
		return ;
	}
	air_mysql_builder_execute(self);
	zval *data = zend_read_property(air_mysql_builder_ce, self, ZEND_STRL("_data"), 1, NULL);
	zval *tmp;
	long lval;
	double dval;
	if(is_numeric_string(ZSTR_VAL(key), ZSTR_LEN(key), &lval, &dval, 0) != IS_LONG){
		if ((tmp = zend_hash_find(Z_ARRVAL_P(data), key)) == NULL) {
			RETURN_NULL();
		}
	}else{
		if ((tmp = zend_hash_index_find(Z_ARRVAL_P(data), lval)) == NULL) {
			RETURN_NULL();
		}
	}
	RETURN_ZVAL(tmp, 1, 0);
}

PHP_METHOD(air_mysql_builder, offsetUnset) {
	AIR_INIT_THIS;

	zval *key = NULL, *data;
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &key) == FAILURE) {
		return;
	}

	if (Z_TYPE_P(key) != IS_STRING || !Z_STRLEN_P(key)) {
		php_error_docref(NULL, E_WARNING, "Expect a string key name");
		RETURN_FALSE;
	}

	air_mysql_builder_execute(self);
	data = zend_read_property(air_mysql_builder_ce, self, ZEND_STRL("_data"), 1, NULL);
	if (zend_hash_del(Z_ARRVAL_P(data), Z_STR_P(key)) == SUCCESS) {
		RETURN_TRUE;
	}

	RETURN_FALSE;
}

PHP_METHOD(air_mysql_builder, count) {
	AIR_INIT_THIS;
	air_mysql_builder_execute(self);
	zval *data = zend_read_property(air_mysql_builder_ce, self, ZEND_STRL("_data"), 1, NULL);
	RETURN_LONG(zend_hash_num_elements(Z_ARRVAL_P(data)));
}

PHP_METHOD(air_mysql_builder, rewind) {
	AIR_INIT_THIS;
	air_mysql_builder_execute(self);
	zval *data = zend_read_property(air_mysql_builder_ce, self, ZEND_STRL("_data"), 1, NULL);
	zend_hash_internal_pointer_reset(Z_ARRVAL_P(data));
}

PHP_METHOD(air_mysql_builder, current) {
	AIR_INIT_THIS;
	zval *data, *pzval, *ret;

	air_mysql_builder_execute(self);
	data = zend_read_property(air_mysql_builder_ce, self, ZEND_STRL("_data"), 1, NULL);
	if (( pzval = zend_hash_get_current_data(Z_ARRVAL_P(data))) == NULL) {
		RETURN_FALSE;
	}

	RETURN_ZVAL(pzval, 1, 0);
}

PHP_METHOD(air_mysql_builder, key) {
	AIR_INIT_THIS;
	zend_string *key;
	zend_ulong index;
	air_mysql_builder_execute(self);
	zval *data = zend_read_property(air_mysql_builder_ce, self, ZEND_STRL("_data"), 1, NULL);
	zend_hash_get_current_key(Z_ARRVAL_P(data), &key, &index);
	switch(zend_hash_get_current_key_type(Z_ARRVAL_P(data))) {
		case HASH_KEY_IS_LONG:
			RETURN_LONG(index);
			break;
		case HASH_KEY_IS_STRING:
			RETURN_STR(key);
			break;
		default:
			RETURN_FALSE;
	}
}

PHP_METHOD(air_mysql_builder, next) {
	AIR_INIT_THIS;
	air_mysql_builder_execute(self);
	zval *data = zend_read_property(air_mysql_builder_ce, self, ZEND_STRL("_data"), 1, NULL);
	zend_hash_move_forward(Z_ARRVAL_P(data));
	RETURN_TRUE;
}

PHP_METHOD(air_mysql_builder, valid) {
	AIR_INIT_THIS;
	air_mysql_builder_execute(self);
	zval *data = zend_read_property(air_mysql_builder_ce, self, ZEND_STRL("_data"), 1, NULL);
	RETURN_BOOL(zend_hash_has_more_elements(Z_ARRVAL_P(data)) == SUCCESS);
}

PHP_METHOD(air_mysql_builder, serialize) {
	AIR_INIT_THIS;
	air_mysql_builder_execute(self);
	zval *data = zend_read_property(air_mysql_builder_ce, self, ZEND_STRL("_data"), 1, NULL);
	zval params[1] = {*data};
	air_call_func("serialize", 1, params, return_value);
}

PHP_METHOD(air_mysql_builder, unserialize) {
}

PHP_METHOD(air_mysql_builder, data) {
	AIR_INIT_THIS;
	air_mysql_builder_execute(self);
	zval *data = zend_read_property(air_mysql_builder_ce, self, ZEND_STRL("_data"), 1, NULL);
	RETURN_ZVAL(data, 1, 0);
}

PHP_METHOD(air_mysql_builder, affected_rows) {
	AIR_INIT_THIS;
	air_mysql_builder_execute(self);
	zval *rows = zend_read_property(air_mysql_builder_ce, self, ZEND_STRL("_affected_rows"), 1, NULL);
	RETURN_ZVAL(rows, 1, 0);
}

PHP_METHOD(air_mysql_builder, num_rows) {
	AIR_INIT_THIS;
	air_mysql_builder_execute(self);
	zval *rows = zend_read_property(air_mysql_builder_ce, self, ZEND_STRL("_num_rows"), 1, NULL);
	RETURN_ZVAL(rows, 1, 0);
}

PHP_METHOD(air_mysql_builder, insert_id) {
	AIR_INIT_THIS;
	air_mysql_builder_execute(self);
	zval *insert_id = zend_read_property(air_mysql_builder_ce, self, ZEND_STRL("_insert_id"), 1, NULL);
	RETURN_ZVAL(insert_id, 1, 0);
}

PHP_METHOD(air_mysql_builder, errno) {
	AIR_INIT_THIS;
	air_mysql_builder_execute(self);
	zval *prop = zend_read_property(air_mysql_builder_ce, self, ZEND_STRL("_errno"), 1, NULL);
	RETURN_ZVAL(prop, 1, 0);
}

PHP_METHOD(air_mysql_builder, error) {
	AIR_INIT_THIS;
	air_mysql_builder_execute(self);
	zval *prop = zend_read_property(air_mysql_builder_ce, self, ZEND_STRL("_error"), 1, NULL);
	RETURN_ZVAL(prop, 1, 0);
}

PHP_METHOD(air_mysql_builder, jsonSerialize) {
	AIR_INIT_THIS;
	zval *t;
	MAKE_STD_ZVAL(t);
	array_init(t);
	RETURN_ZVAL(t, 1, 0);
}

PHP_METHOD(air_mysql_builder, async) {
	AIR_INIT_THIS;
	zval *waiter = zend_read_static_property(air_mysql_builder_ce, ZEND_STRL("_waiter"), 1);
	if(Z_TYPE_P(waiter) == IS_NULL){
		zval _waiter;
		object_init_ex(&_waiter, air_mysql_waiter_ce);
		air_call_method(&_waiter, air_mysql_waiter_ce, NULL, ZEND_STRL("__construct"), NULL, 0, NULL);
		zend_update_static_property(air_mysql_builder_ce, ZEND_STRL("_waiter"), &_waiter);
		zval_ptr_dtor(&_waiter);
		waiter = zend_read_static_property(air_mysql_builder_ce, ZEND_STRL("_waiter"), 1);
	}
	zval params[1] = {*self};
	zval service;
	air_call_object_method(waiter, air_mysql_waiter_ce, "serve", &service, 1, params);
	zend_update_property(air_mysql_builder_ce, self, ZEND_STRL("_service"), &service);
	zval_ptr_dtor(&service);
	zval *debug = zend_read_property(air_mysql_builder_ce, self, ZEND_STRL("_debug"), 1, NULL);
	add_next_index_string(debug, "async");
	AIR_RET_THIS;
}

PHP_METHOD(air_mysql_builder, mode) {
	AIR_INIT_THIS;
	int mode = AIR_R;
	if(zend_parse_parameters(ZEND_NUM_ARGS(), "l", &mode) == FAILURE){
		AIR_NEW_EXCEPTION(1, "error mode");
	}
	if(mode != AIR_R && mode != AIR_W){
		AIR_NEW_EXCEPTION(1, "error mode, it must be AIR_R or AIR_W");
	}
	zend_update_property_long(air_mysql_builder_ce, self, ZEND_STRL("_mode"), mode);
	AIR_RET_THIS;
}

PHP_METHOD(air_mysql_builder, table) {
	AIR_INIT_THIS;
	zend_string *table;
	if(zend_parse_parameters(ZEND_NUM_ARGS(), "S", &table) == FAILURE){
		AIR_NEW_EXCEPTION(1, "error table");
	}
	zval *original = zend_read_property(air_mysql_builder_ce, self, ZEND_STRL("_original"), 1, NULL);
	add_assoc_str(original, "table", table);
	AIR_RET_THIS;
}

PHP_METHOD(air_mysql_builder, add) {
	AIR_INIT_THIS;
	zval *data = NULL;
	if(zend_parse_parameters(ZEND_NUM_ARGS(), "a", &data) == FAILURE){
		return ;
	}
	zval *original = zend_read_property(air_mysql_builder_ce, self, ZEND_STRL("_original"), 1, NULL);
	add_assoc_long_ex(original, ZEND_STRL("action"), AIR_ADD);
	Z_TRY_ADDREF_P(data);
	add_assoc_zval(original, "data", data);

	AIR_RET_THIS;
}

PHP_METHOD(air_mysql_builder, get) {
	AIR_INIT_THIS;
	zend_string *fields;
	if(zend_parse_parameters(ZEND_NUM_ARGS(), "S", &fields) == FAILURE){
		return ;
	}
	zval *original = zend_read_property(air_mysql_builder_ce, self, ZEND_STRL("_original"), 1, NULL);
	add_assoc_long_ex(original, ZEND_STRL("action"), AIR_GET);
	add_assoc_str(original, "fields", fields);

	AIR_RET_THIS;
}

PHP_METHOD(air_mysql_builder, set) {
	AIR_INIT_THIS;
	zval *data;
	if(zend_parse_parameters(ZEND_NUM_ARGS(), "a", &data) == FAILURE){
		return ;
	}
	zval *original = zend_read_property(air_mysql_builder_ce, self, ZEND_STRL("_original"), 1, NULL);
	add_assoc_long_ex(original, ZEND_STRL("action"), AIR_SET);
	Z_TRY_ADDREF_P(data);
	add_assoc_zval(original, "data", data);

	AIR_RET_THIS;
}

PHP_METHOD(air_mysql_builder, del) {
	AIR_INIT_THIS;
	zval *original = zend_read_property(air_mysql_builder_ce, self, ZEND_STRL("_original"), 1, NULL);
	add_assoc_long_ex(original, ZEND_STRL("action"), AIR_DEL);

	AIR_RET_THIS;
}
PHP_METHOD(air_mysql_builder, where) {
	AIR_INIT_THIS;
	zval *conds;
	zval *values = NULL;
	if(zend_parse_parameters(ZEND_NUM_ARGS(), "z|a", &conds, &values) == FAILURE){
		return ;
	}
	if(Z_TYPE_P(conds) != IS_STRING){
		AIR_NEW_EXCEPTION(1, "invalid air\\mysql\\builder::where conds value");
	}
	zval *original = zend_read_property(air_mysql_builder_ce, self, ZEND_STRL("_original"), 1, NULL);
	zval where;
	array_init(&where);
	Z_TRY_ADDREF_P(conds);
	zend_hash_next_index_insert(Z_ARRVAL(where), conds);
	if(values){
		Z_TRY_ADDREF_P(values);
		zend_hash_next_index_insert(Z_ARRVAL(where), values);
	}
	add_assoc_zval(original, "where", &where);

	AIR_RET_THIS;
}

PHP_METHOD(air_mysql_builder, by_key) {
	AIR_INIT_THIS;
	zval *value;
	zend_string *pk_name = NULL;
	if(zend_parse_parameters(ZEND_NUM_ARGS(), "z|S", &value, &pk_name) == FAILURE){
		return ;
	}
	zval *original = zend_read_property(air_mysql_builder_ce, self, ZEND_STRL("_original"), 1, NULL);
	char *pk_place_holder = NULL;
	int len = spprintf(&pk_place_holder, 0, ":%s", pk_name?ZSTR_VAL(pk_name): "id");
	char *conds;
	len = spprintf(&conds, 0, "`%s`=%s", (pk_name?ZSTR_VAL(pk_name): "id"), pk_place_holder);
	efree(pk_place_holder);
	zval zconds;
	ZVAL_STRINGL(&zconds, conds, len);
	zval where;
	array_init(&where);
	zend_hash_next_index_insert(Z_ARRVAL(where), &zconds);
	efree(conds);
	zval values;
	array_init(&values);
	Z_TRY_ADDREF_P(value);
	add_assoc_zval(&values, (pk_name?ZSTR_VAL(pk_name):"id"), value);
	zend_hash_next_index_insert(Z_ARRVAL(where), &values);
	add_assoc_zval(original, "where", &where);

	AIR_RET_THIS;
}

PHP_METHOD(air_mysql_builder, sort) {
	AIR_INIT_THIS;
	zval *sort;
	if(zend_parse_parameters(ZEND_NUM_ARGS(), "z", &sort) == FAILURE){
		return ;
	}
	zval *original = zend_read_property(air_mysql_builder_ce, self, ZEND_STRL("_original"), 1, NULL);
	Z_TRY_ADDREF_P(sort);
	add_assoc_zval(original, "sort", sort);
	AIR_RET_THIS;
}

PHP_METHOD(air_mysql_builder, offset) {
	AIR_INIT_THIS;
	ulong offset = 0;
	if(zend_parse_parameters(ZEND_NUM_ARGS(), "l", &offset) == FAILURE){
		return ;
	}
	zval *original = zend_read_property(air_mysql_builder_ce, self, ZEND_STRL("_original"), 1, NULL);
	add_assoc_long(original, "offset", offset);
	AIR_RET_THIS;
}

PHP_METHOD(air_mysql_builder, size) {
	AIR_INIT_THIS;
	ulong size = 10;
	if(zend_parse_parameters(ZEND_NUM_ARGS(), "l", &size) == FAILURE){
		return ;
	}
	zval *original = zend_read_property(air_mysql_builder_ce, self, ZEND_STRL("_original"), 1, NULL);
	add_assoc_long(original, "size", size);
	AIR_RET_THIS;
}

PHP_METHOD(air_mysql_builder, query) {
	AIR_INIT_THIS;
	zval *sql;
	zval *param = NULL;
	if(zend_parse_parameters(ZEND_NUM_ARGS(), "z|a", &sql, &param) == FAILURE || Z_TYPE_P(sql) != IS_STRING){
		AIR_NEW_EXCEPTION(1, "error params");
		return ;
	}
	zval *compiled = zend_read_property(air_mysql_builder_ce, self, ZEND_STRL("_compiled"), 1, NULL);
	if(!compiled){
		zval _compiled;
		array_init(&_compiled);
		zend_update_property(air_mysql_builder_ce, self, ZEND_STRL("_compiled"), &_compiled);
		zval_ptr_dtor(&_compiled);
		compiled = zend_read_property(air_mysql_builder_ce, self, ZEND_STRL("_compiled"), 1, NULL);
	}
	Z_TRY_ADDREF_P(sql);
	add_assoc_zval(compiled, "tpl", sql);
	if(param){
		Z_TRY_ADDREF_P(param);
		add_assoc_zval(compiled, "var", param);
	}else{
		zval _param;
		array_init(&_param);
		add_assoc_zval(compiled, "var", &_param);
	}
	zval empty_array;
	array_init(&empty_array);
	zend_update_property(air_mysql_builder_ce, self, ZEND_STRL("_original"), &empty_array);
	zval_ptr_dtor(&empty_array);
	AIR_RET_THIS;
}

PHP_METHOD(air_mysql_builder, build) {
	AIR_INIT_THIS;
	zval *escaper;
	if(zend_parse_parameters(ZEND_NUM_ARGS(), "z", &escaper) == FAILURE){
		AIR_NEW_EXCEPTION(1, "error params");
	}
	zval *compiled = zend_read_property(air_mysql_builder_ce, self, ZEND_STRL("_compiled"), 1, NULL);
	Z_TRY_ADDREF_P(escaper);
	add_assoc_zval(compiled, "escaper", escaper);
	zval *tpl = zend_hash_str_find(Z_ARRVAL_P(compiled), ZEND_STRL("tpl"));
	if(!tpl){
		air_mysql_builder_build(self);
		compiled = zend_read_property(air_mysql_builder_ce, self, ZEND_STRL("_compiled"), 1, NULL);
	}
	zval *sql = air_mysqli_escape(compiled);
	if(sql){
		RETURN_ZVAL(sql, 1, 0);
	}else{
		RETURN_NULL();
	}
}

PHP_METHOD(air_mysql_builder, trigger) {
	AIR_INIT_THIS;
	zval *event, *params;
	if(zend_parse_parameters(ZEND_NUM_ARGS(), "za", &event, &params) == FAILURE){
		return ;
	}
	zval *callback = zend_read_property(air_mysql_builder_ce, self, ZEND_STRL("_callback"), 1, NULL);
	zval *event_handler = zend_hash_find(Z_ARRVAL_P(callback), Z_STR_P(event));
	zval _params[2] = {*event_handler, *params};
	air_call_func("call_user_func_array", 2, _params, return_value);
}

PHP_METHOD(air_mysql_builder, on_success) {
	AIR_INIT_THIS;
	zval *handler;
	if(zend_parse_parameters(ZEND_NUM_ARGS(), "z", &handler) == FAILURE){
		return ;
	}
	zval *callback = zend_read_property(air_mysql_builder_ce, self, ZEND_STRL("_callback"), 1, NULL);
	Z_TRY_ADDREF_P(handler);
	add_assoc_zval(callback, "success", handler);
	AIR_RET_THIS;
}

PHP_METHOD(air_mysql_builder, on_error) {
	AIR_INIT_THIS;
	zval *handler;
	if(zend_parse_parameters(ZEND_NUM_ARGS(), "z", &handler) == FAILURE){
		return ;
	}
	zval *callback = zend_read_property(air_mysql_builder_ce, self, ZEND_STRL("_callback"), 1, NULL);
	Z_TRY_ADDREF_P(handler);
	add_assoc_zval(callback, "error", handler);
	AIR_RET_THIS;
}

PHP_METHOD(air_mysql_builder, on_success_default) {
	zval *mysqli, *mysqli_result;
	if(zend_parse_parameters(ZEND_NUM_ARGS(), "zz", &mysqli, &mysqli_result) == FAILURE){
		return ;
	}
	zval assoc;
	ZVAL_LONG(&assoc, MYSQLI_ASSOC);
	zval fetch_params[1] = {assoc};
	zval ret;
	array_init(&ret);
	zval rows;
	air_mysqli_get_total_rows(mysqli, &rows);
	if(Z_TYPE_P(mysqli_result) != IS_OBJECT){
		add_assoc_zval(&ret, "affected_rows", &rows);
		zval insert_id;
		air_mysqli_get_insert_id(mysqli, &insert_id);
		if(Z_LVAL(insert_id)>0){
			add_assoc_zval(&ret, "insert_id", &insert_id);
		}else{
			zval_ptr_dtor(&insert_id);
		}
	}else{
		zval results;
		air_call_object_method(mysqli_result, Z_OBJCE_P(mysqli_result), "fetch_all", &results, 1, fetch_params);
		add_assoc_zval(&ret, "data", &results);
		add_assoc_zval(&ret, "num_rows", &rows);
		air_call_object_method(mysqli_result, Z_OBJCE_P(mysqli_result), "free", NULL, 0, NULL);
	}
	zval_ptr_dtor(&assoc);
	RETURN_ZVAL(&ret, 1, 1);
}

PHP_METHOD(air_mysql_builder, on_error_default) {
	zval *mysqli;
	if(zend_parse_parameters(ZEND_NUM_ARGS(), "z", &mysqli) == FAILURE){
		return ;
	}
	ulong mysqli_errno = air_mysqli_get_errno(mysqli);
	char *mysqli_error = air_mysqli_get_error(mysqli);
	zval ret;
	array_init(&ret);
	add_assoc_long(&ret, "errno", mysqli_errno);
	add_assoc_string(&ret, "error", mysqli_error);
	RETURN_ZVAL(&ret, 1, 1);
}

PHP_METHOD(air_mysql_builder, __destruct) {
	AIR_INIT_THIS;
	zval *status = zend_read_property(air_mysql_builder_ce, self, ZEND_STRL("_status"), 1, NULL);
	if(!Z_LVAL_P(status)){
		//todo remove
		air_mysql_builder_execute(self);
	}
}

/** {{{ air_mysql_builder_methods */
zend_function_entry air_mysql_builder_methods[] = {
	PHP_ME(air_mysql_builder, __construct, air_mysql_builder_construct_arginfo,  ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(air_mysql_builder, __destruct, air_mysql_builder_void_arginfo,  ZEND_ACC_PUBLIC | ZEND_ACC_DTOR)
	PHP_ME(air_mysql_builder, count, air_mysql_builder_void_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql_builder, offsetUnset, air_mysql_builder_k_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql_builder, offsetGet, air_mysql_builder_k_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql_builder, offsetExists, air_mysql_builder_k_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql_builder, offsetSet, air_mysql_builder_kv_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql_builder, rewind, air_mysql_builder_void_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql_builder, current, air_mysql_builder_void_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql_builder, next,	air_mysql_builder_void_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql_builder, valid, air_mysql_builder_void_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql_builder, key, air_mysql_builder_void_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql_builder, serialize, air_mysql_builder_void_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql_builder, unserialize, air_mysql_builder_data_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql_builder, data, air_mysql_builder_void_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql_builder, affected_rows, air_mysql_builder_void_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql_builder, num_rows, air_mysql_builder_void_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql_builder, insert_id, air_mysql_builder_void_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql_builder, error, air_mysql_builder_void_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql_builder, errno, air_mysql_builder_void_arginfo, ZEND_ACC_PUBLIC)
	//methods
	PHP_ME(air_mysql_builder, table, air_mysql_builder_table_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql_builder, mode, air_mysql_builder_mode_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql_builder, async, air_mysql_builder_void_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql_builder, add, air_mysql_builder_data_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql_builder, get, air_mysql_builder_get_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql_builder, set, air_mysql_builder_data_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql_builder, del, air_mysql_builder_void_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql_builder, where, air_mysql_builder_where_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql_builder, by_key, air_mysql_builder_by_key_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql_builder, sort, air_mysql_builder_sort_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql_builder, offset, air_mysql_builder_offset_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql_builder, size, air_mysql_builder_size_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql_builder, query, air_mysql_builder_query_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql_builder, build, air_mysql_builder_build_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql_builder, trigger, air_mysql_builder_trigger_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql_builder, on_success, air_mysql_builder_on_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql_builder, on_error, air_mysql_builder_on_arginfo, ZEND_ACC_PUBLIC)
	PHP_ME(air_mysql_builder, on_success_default, air_mysql_builder_on_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(air_mysql_builder, on_error_default, air_mysql_builder_on_arginfo, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};
/* }}} */

/** {{{ AIR_MINIT_FUNCTION */
AIR_MINIT_FUNCTION(air_mysql_builder) {
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "air\\mysql\\builder", air_mysql_builder_methods);

	air_mysql_builder_ce = zend_register_internal_class_ex(&ce, NULL);
	zend_class_implements(air_mysql_builder_ce, 4, zend_ce_iterator, zend_ce_arrayaccess, spl_ce_Countable, zend_ce_serializable);

	zend_declare_property_null(air_mysql_builder_ce, ZEND_STRL("_waiter"), ZEND_ACC_PROTECTED | ZEND_ACC_STATIC);
	zend_declare_property_null(air_mysql_builder_ce, ZEND_STRL("_service"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(air_mysql_builder_ce, ZEND_STRL("_config"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(air_mysql_builder_ce, ZEND_STRL("_mode"), ZEND_ACC_PROTECTED);
	zend_declare_property_long(air_mysql_builder_ce, ZEND_STRL("_status"), 0, ZEND_ACC_PROTECTED);

	//keep results
	zend_declare_property_null(air_mysql_builder_ce, ZEND_STRL("_data"), ZEND_ACC_PROTECTED);
	//can be affected rows, or the count
	zend_declare_property_null(air_mysql_builder_ce, ZEND_STRL("_affected_rows"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(air_mysql_builder_ce, ZEND_STRL("_num_rows"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(air_mysql_builder_ce, ZEND_STRL("_insert_id"), ZEND_ACC_PROTECTED);
	zend_declare_property_long(air_mysql_builder_ce, ZEND_STRL("_errno"), 0, ZEND_ACC_PROTECTED);
	zend_declare_property_string(air_mysql_builder_ce, ZEND_STRL("_error"), "", ZEND_ACC_PROTECTED);
	//keep params
	zend_declare_property_null(air_mysql_builder_ce, ZEND_STRL("_debug"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(air_mysql_builder_ce, ZEND_STRL("_original"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(air_mysql_builder_ce, ZEND_STRL("_compiled"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(air_mysql_builder_ce, ZEND_STRL("_callback"), ZEND_ACC_PROTECTED);
	return SUCCESS;
}
/* }}} */

