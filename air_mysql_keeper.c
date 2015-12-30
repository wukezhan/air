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

#include "air_config.h"
#include "air_mysqli.h"
#include "air_mysql_keeper.h"

zend_class_entry *air_mysql_keeper_ce;

int air_mysql_keeper_get_mysqli(zval *mysqli, zval *config, int mode){
	zval params[7];
	zval *host, *user, *pass, *port, *sock, flag, nil;
	ZVAL_NULL(&nil);
	zval *auth = zend_hash_str_find(Z_ARRVAL_P(config), ZEND_STRL("auth"));
	zval *mode_auth = zend_hash_index_find(Z_ARRVAL_P(auth), mode);
	user = zend_hash_str_find(Z_ARRVAL_P(mode_auth), ZEND_STRL("username"));
	pass = zend_hash_str_find(Z_ARRVAL_P(mode_auth), ZEND_STRL("password"));
	params[1] = user? *user: nil;
	params[2] = pass? *pass: nil;
	params[3] = nil;
	ZVAL_LONG(&flag, MYSQLI_FOUND_ROWS);
	params[6] = flag;
	zval *pool = zend_hash_str_find(Z_ARRVAL_P(config), ZEND_STRL("pool"));
	zval *mode_pool = zend_hash_index_find(Z_ARRVAL_P(pool), mode);
	zval rand_arr[1] = {*mode_pool};
	zval *mode_hps;
	while(1){
		zval arr_idx;
		ZVAL_UNDEF(&arr_idx);
		air_call_func("array_rand", 1, rand_arr, &arr_idx);
		if(Z_ISUNDEF(arr_idx)){
			return -3;
		}
		mode_hps = zend_hash_index_find(Z_ARRVAL_P(mode_pool), Z_LVAL(arr_idx));
		host = zend_hash_str_find(Z_ARRVAL_P(mode_hps), ZEND_STRL("host"));
		port = zend_hash_str_find(Z_ARRVAL_P(mode_hps), ZEND_STRL("port"));
		sock = zend_hash_str_find(Z_ARRVAL_P(mode_hps), ZEND_STRL("sock"));
		params[0] = host? *host: nil;
		params[4] = port? *port: nil;
		params[5] = sock? *sock: nil;
		zval ret;
		ZVAL_FALSE(&ret);
		air_call_object_method(mysqli, Z_OBJCE_P(mysqli), "real_connect", &ret, 7, params);
		if(Z_TYPE(ret) == IS_TRUE){
			zval_ptr_dtor(&arr_idx);
			zval_ptr_dtor(&ret);
			break;
		}else{
			zval_ptr_dtor(&ret);
			zend_hash_index_del(Z_ARRVAL_P(mode_pool), Z_LVAL(arr_idx));
			zval_ptr_dtor(&arr_idx);
			if(!zend_hash_num_elements(Z_ARRVAL_P(mode_pool))){
				return -5;
			}
		}
	}
	zval_ptr_dtor(&nil);
	zval_ptr_dtor(&flag);
	return 0;
}

int air_mysql_keeper_close_mysqli(zval *mysqli_array){
	zval *val;
	ulong idx;
	zend_string *key;
	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(mysqli_array), idx, key, val){
		air_call_object_method(val, Z_OBJCE_P(val), "close", NULL, 0, NULL);
	}ZEND_HASH_FOREACH_END();
}

void air_mysql_keeper_make_entry(zval *conf_entry){
	array_init(conf_entry);
	zval free, busy;
	array_init(&free);
	array_init(&busy);
	add_assoc_zval(conf_entry, "free", &free);
	add_assoc_zval(conf_entry, "busy", &busy);
	add_assoc_long(conf_entry, "quota", 5);
}

zval *air_mysql_keeper_find_entry(zend_string *conf_name, int mode){
	zval *instance = zend_read_static_property(air_mysql_keeper_ce, ZEND_STRL("_instance"), 0);
	if(Z_TYPE_P(instance) == IS_NULL){
		zval _instance;
		object_init_ex(&_instance, air_mysql_keeper_ce);
		zend_update_static_property(air_mysql_keeper_ce, ZEND_STRL("_instance"), &_instance);
		zval_ptr_dtor(&_instance);
		instance = zend_read_static_property(air_mysql_keeper_ce, ZEND_STRL("_instance"), 0);
	}
	zval *keeper_pool = zend_read_property(air_mysql_keeper_ce, instance, ZEND_STRL("_pool"), 0, NULL);
	if(Z_TYPE_P(keeper_pool) == IS_NULL){
		zval arr;
		array_init(&arr);
		zend_update_property(air_mysql_keeper_ce, instance, ZEND_STRL("_pool"), &arr);
		zval_ptr_dtor(&arr);
		keeper_pool = zend_read_property(air_mysql_keeper_ce, instance, ZEND_STRL("_pool"), 0, NULL);
	}
	zval *conf_pool = zend_hash_find(Z_ARRVAL_P(keeper_pool), conf_name);
	if(!conf_pool){
		zval _conf_pool;
		array_init(&_conf_pool);

		zval _conf_entry;
		air_mysql_keeper_make_entry(&_conf_entry);
		zval _conf_entry2;
		air_mysql_keeper_make_entry(&_conf_entry2);
		add_index_zval(&_conf_pool, AIR_R, &_conf_entry);
		add_index_zval(&_conf_pool, AIR_W, &_conf_entry2);
		add_assoc_zval_ex(keeper_pool, ZSTR_VAL(conf_name), ZSTR_LEN(conf_name), &_conf_pool);
		conf_pool = zend_hash_find(Z_ARRVAL_P(keeper_pool), conf_name);
	}
	return zend_hash_index_find(Z_ARRVAL_P(conf_pool), mode);
}

/* {{{ ARG_INFO */
ZEND_BEGIN_ARG_INFO_EX(air_mysql_keeper_factory_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, config)
	ZEND_ARG_INFO(0, mode)
ZEND_END_ARG_INFO()

/* }}} */

/* {{{ PHP METHODS */
PHP_METHOD(air_mysql_keeper, __construct) {
}

PHP_METHOD(air_mysql_keeper, factory) {
	zend_string *conf_name = NULL;
	int mode = AIR_R;
	if(zend_parse_parameters(ZEND_NUM_ARGS(), "S|l", &conf_name, &mode) == FAILURE){
		php_error(E_WARNING, "air\\mysql\\keeper::factory($config, $mode) params error");
		return;
	}
	zval *conf_entry = air_mysql_keeper_find_entry(conf_name, mode);
	zval *free = zend_hash_str_find(Z_ARRVAL_P(conf_entry), ZEND_STRL("free"));
	zval *busy = zend_hash_str_find(Z_ARRVAL_P(conf_entry), ZEND_STRL("busy"));
	zval *mysqli = NULL;
	int status = 0;
	ulong mysqli_id = 0;
	if(!zend_hash_num_elements(Z_ARRVAL_P(free))){
		zval *config = air_config_path_get(NULL, conf_name);
		if(!config){
			air_throw_exception_ex(1, "mysql config %s not found", conf_name);
			return ;
		}
		zval _mysqli;
		AIR_OBJ_INIT(&_mysqli, "mysqli");
		//todo add quota
		if(Z_ISUNDEF(_mysqli)){
			php_error(E_ERROR, "air\\mysql\\keeper can not find mysqli");
		}
		air_call_object_method(&_mysqli, Z_OBJCE_P(&_mysqli), "init", NULL, 0, NULL);
		status = air_mysql_keeper_get_mysqli(&_mysqli, config, mode);
		if(status == SUCCESS){
			mysqli_id = air_mysqli_get_id(&_mysqli);
			Z_TRY_ADDREF(_mysqli);
			add_index_zval(busy, mysqli_id, &_mysqli);
		}
		zval_ptr_dtor(&_mysqli);
	}else{
		//pop
		zend_string *key;
		ulong index;
		zend_hash_internal_pointer_end(Z_ARRVAL_P(free));
		zend_hash_get_current_key(Z_ARRVAL_P(free), &key, &index);
		mysqli = zend_hash_index_find(Z_ARRVAL_P(free), index);
		ZVAL_DEREF(mysqli);
		zval _mysqli;
		ZVAL_COPY(&_mysqli, mysqli);
		mysqli_id = air_mysqli_get_id(mysqli);
		add_index_zval(busy, mysqli_id, &_mysqli);
		zend_hash_index_del(Z_ARRVAL_P(free), index);
	}
	if(mysqli_id){
		RETURN_ZVAL(zend_hash_index_find(Z_ARRVAL_P(busy), mysqli_id), 1, 0);
	}else{
		air_throw_exception_ex(1, "could not connect to %s", ZSTR_VAL(conf_name));
		return ;
	}
	RETURN_LONG(status);
}

PHP_METHOD(air_mysql_keeper, release) {
	zend_string *conf_name = NULL;
	zval *mysqli;
	ulong mode = AIR_R;
	if(zend_parse_parameters(ZEND_NUM_ARGS(), "zS|l", &mysqli, &conf_name, &mode) == FAILURE){
		return ;
	}
	zval *conf_entry = air_mysql_keeper_find_entry(conf_name, mode);
	zval *free = zend_hash_str_find(Z_ARRVAL_P(conf_entry), ZEND_STRL("free"));
	zval *busy = zend_hash_str_find(Z_ARRVAL_P(conf_entry), ZEND_STRL("busy"));

	ulong mysqli_id = air_mysqli_get_id(mysqli);
	ZVAL_DEREF(mysqli);
	zval _mysqli;
	ZVAL_COPY(&_mysqli, mysqli);
	zend_hash_index_del(Z_ARRVAL_P(busy), mysqli_id);
	//array push
	zend_hash_next_index_insert(Z_ARRVAL_P(free), &_mysqli);
}

PHP_METHOD(air_mysql_keeper, __destruct) {
	zval *instance = zend_read_static_property(air_mysql_keeper_ce, ZEND_STRL("_instance"), 0);
	if(Z_ISNULL_P(instance)){
		return ;
	}
	zval *pool = zend_read_property(air_mysql_keeper_ce, instance, ZEND_STRL("_pool"), 0, NULL);
	zval *val;
	ulong idx;
	zend_string *key;
	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(pool), idx, key, val){
		zval *r = zend_hash_index_find(Z_ARRVAL_P(val), AIR_R);
		air_mysql_keeper_close_mysqli(zend_hash_str_find(Z_ARRVAL_P(r), ZEND_STRL("free")));
		air_mysql_keeper_close_mysqli(zend_hash_str_find(Z_ARRVAL_P(r), ZEND_STRL("busy")));
		zval *w = zend_hash_index_find(Z_ARRVAL_P(val), AIR_W);
		air_mysql_keeper_close_mysqli(zend_hash_str_find(Z_ARRVAL_P(w), ZEND_STRL("free")));
		air_mysql_keeper_close_mysqli(zend_hash_str_find(Z_ARRVAL_P(w), ZEND_STRL("busy")));
	}ZEND_HASH_FOREACH_END();
}

/* }}} */

/* {{{ air_mysql_keeper_methods */
zend_function_entry air_mysql_keeper_methods[] = {
	PHP_ME(air_mysql_keeper, __construct, NULL,  ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(air_mysql_keeper, __destruct, NULL,  ZEND_ACC_PUBLIC | ZEND_ACC_DTOR)
	PHP_ME(air_mysql_keeper, release, air_mysql_keeper_factory_arginfo,  ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(air_mysql_keeper, factory, air_mysql_keeper_factory_arginfo,  ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};
/* }}} */

/* {{{ AIR_MINIT_FUNCTION */
AIR_MINIT_FUNCTION(air_mysql_keeper) {
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "air\\mysql\\keeper", air_mysql_keeper_methods);

	air_mysql_keeper_ce = zend_register_internal_class_ex(&ce, NULL);
	zend_declare_property_null(air_mysql_keeper_ce, ZEND_STRL("_instance"), ZEND_ACC_PROTECTED | ZEND_ACC_STATIC);
	zend_declare_property_null(air_mysql_keeper_ce, ZEND_STRL("_pool"), ZEND_ACC_PROTECTED);
	return SUCCESS;
}
/* }}} */

