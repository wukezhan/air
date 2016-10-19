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

#include "src/air_config.h"
#include "src/air_mysqli.h"
#include "src/air_mysql_keeper.h"

zend_class_entry *air_mysql_keeper_ce;

int air_mysql_keeper_get_mysqli(zval **pp_mysqli, zval *config, int mode TSRMLS_DC){
	zval *mysqli = air_new_object(ZEND_STRL("mysqli") TSRMLS_CC);
	air_call_method(&mysqli, Z_OBJCE_P(mysqli), NULL, ZEND_STRL("init"), NULL, 0, NULL TSRMLS_CC);
	zval **params[7];
	zval *host, *user, *pass, *db, *port, *sock, *flag, *nil;
	MAKE_STD_ZVAL(nil);
	ZVAL_NULL(nil);
	zval *auth = air_arr_find(config, ZEND_STRS("auth"));
	zval *mode_auth = air_arr_idx_find(auth, mode);
	user = air_arr_find(mode_auth, ZEND_STRS("username"));
	pass = air_arr_find(mode_auth, ZEND_STRS("password"));
	params[1] = user? &user: &nil;
	params[2] = pass? &pass: &nil;
	params[3] = &nil;
	MAKE_STD_ZVAL(flag);
	ZVAL_LONG(flag, MYSQLI_FOUND_ROWS);
	params[6] = &flag;
	zval *pool = air_arr_find(config, ZEND_STRS("pool"));
	zval *mode_pool = air_arr_idx_find(pool, mode);
	zval *rand_arr[1] = {mode_pool};
	zval *mode_hps;
	zval *ret;
	while(1){
		zval *arr_idx = air_call_func("array_rand", 1, rand_arr);
		if(!arr_idx){
			return -3;
		}
		mode_hps = air_arr_idx_find(mode_pool, Z_LVAL_P(arr_idx));
		host = air_arr_find(mode_hps, ZEND_STRS("host"));
		port = air_arr_find(mode_hps, ZEND_STRS("port"));
		sock = air_arr_find(mode_hps, ZEND_STRS("sock"));
		params[0] = host? &host: &nil;
		params[4] = port? &port: &nil;
		params[5] = sock? &sock: &nil;
		air_call_method(&mysqli, Z_OBJCE_P(mysqli), NULL, ZEND_STRL("real_connect"), &ret, 7, params TSRMLS_CC);
		if(Z_LVAL_P(ret)){
			zval_ptr_dtor(&arr_idx);
			zval_ptr_dtor(&ret);
			break;
		}else{
			zval_ptr_dtor(&ret);
			zend_hash_index_del(Z_ARRVAL_P(mode_pool), Z_LVAL_P(arr_idx));
			zval_ptr_dtor(&arr_idx);
			if(!zend_hash_num_elements(Z_ARRVAL_P(mode_pool))){
				return -5;
			}
		}
	}
	zval_ptr_dtor(&nil);
	zval_ptr_dtor(&flag);
	*pp_mysqli = mysqli;
	return 0;
}

int air_mysql_keeper_close_mysqli(zval *mysqli_array){
	zval *val;
	ulong idx;
	char *key;
	int key_len;
	AIR_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(mysqli_array), idx, key, key_len, val){
		air_call_method(&val, Z_OBJCE_P(val), NULL, ZEND_STRL("close"), NULL, 0, NULL TSRMLS_CC);
	}AIR_HASH_FOREACH_END();
}

zval *air_mysql_keeper_make_entry(){
	zval *conf_entry, *free, *busy;
	MAKE_STD_ZVAL(conf_entry);
	array_init(conf_entry);
	MAKE_STD_ZVAL(free);
	array_init(free);
	MAKE_STD_ZVAL(busy);
	array_init(busy);
	add_assoc_zval(conf_entry, "free", free);
	add_assoc_zval(conf_entry, "busy", busy);
	add_assoc_long(conf_entry, "quota", 5);
	return conf_entry;
}

/* {{{ ARG_INFO */
ZEND_BEGIN_ARG_INFO_EX(air_mysql_keeper_release_arginfo, 0, 0, 2)
	ZEND_ARG_INFO(0, mysqli)
	ZEND_ARG_INFO(0, config)
	ZEND_ARG_INFO(0, mode)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_mysql_keeper_acquire_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, config)
	ZEND_ARG_INFO(0, mode)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ PHP METHODS */
PHP_METHOD(air_mysql_keeper, __construct) {
}

zval *air_mysql_keeper_find_entry(char *conf_name, int conf_len, int mode){
	zval *instance = zend_read_static_property(air_mysql_keeper_ce, ZEND_STRL("_instance"), 0 TSRMLS_CC);
	if(Z_TYPE_P(instance) == IS_NULL){
		object_init_ex(instance, air_mysql_keeper_ce);
		zend_update_static_property(air_mysql_keeper_ce, ZEND_STRL("_instance"), instance TSRMLS_CC);
	}
	zval *keeper_pool = zend_read_property(air_mysql_keeper_ce, instance, ZEND_STRL("_pool"), 0 TSRMLS_CC);
	if(Z_TYPE_P(keeper_pool) == IS_NULL){
		zval *arr;
		MAKE_STD_ZVAL(arr);
		array_init(arr);
		zend_update_property(air_mysql_keeper_ce, instance, ZEND_STRL("_pool"), arr);
		keeper_pool = zend_read_property(air_mysql_keeper_ce, instance, ZEND_STRL("_pool"), 0 TSRMLS_CC);
		zval_ptr_dtor(&arr);
	}
	zval *conf_pool = air_arr_find(keeper_pool, conf_name, conf_len+1);
	zval *conf_entry;

	if(!conf_pool){
		MAKE_STD_ZVAL(conf_pool);
		array_init(conf_pool);

		conf_entry = air_mysql_keeper_make_entry();
		zval *conf_entry2 = air_mysql_keeper_make_entry();
		add_index_zval(conf_pool, AIR_R, conf_entry);
		add_index_zval(conf_pool, AIR_W, conf_entry2);
		add_assoc_zval_ex(keeper_pool, conf_name, conf_len+1, conf_pool);
	}
	conf_pool = air_arr_find(keeper_pool, conf_name, conf_len+1);
	return air_arr_idx_find(conf_pool, mode);
}

PHP_METHOD(air_mysql_keeper, acquire) {
	char *conf_name = NULL;
	int len = 0;
	long mode = AIR_R;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|l", &conf_name, &len, &mode) == FAILURE){
		php_error(E_WARNING, "air\\mysql\\keeper::acquire($config, $mode) params error");
		return;
	}
	zval *conf_entry = air_mysql_keeper_find_entry(conf_name, len, mode);
	zval *free = air_arr_find(conf_entry, ZEND_STRS("free"));
	zval *busy = air_arr_find(conf_entry, ZEND_STRS("busy"));
	zval *mysqli = NULL;
	int status = 0;
	if(!zend_hash_num_elements(Z_ARRVAL_P(free))){
		zval *config;
		int found_conf_path = air_config_path_get(NULL, conf_name, len, &config TSRMLS_CC);
		if(found_conf_path == FAILURE){
			air_throw_exception_ex(1, "mysql config %s not found", conf_name);
			return ;
		}
		//todo add quota
		status = air_mysql_keeper_get_mysqli(&mysqli, config, mode TSRMLS_CC);
	}else{
		//pop
		char *key;
		int key_len;
		ulong index;
		zend_hash_internal_pointer_end(Z_ARRVAL_P(free));
		zend_hash_get_current_key_ex(Z_ARRVAL_P(free), &key, &key_len, &index, 0, NULL);
		mysqli = air_arr_idx_find(free, index);
		Z_ADDREF_P(mysqli);
		zend_hash_index_del(Z_ARRVAL_P(free), index);
	}
	if(status == 0){
		ulong mysqli_id = air_mysqli_get_id(mysqli);
		add_index_zval(busy, mysqli_id, mysqli);
		RETURN_ZVAL(mysqli, 1, 0);
	}else{
		air_throw_exception_ex(1, "could not connect to %s", conf_name);
		return ;
	}
	RETURN_LONG(status);
}

PHP_METHOD(air_mysql_keeper, simplex) {
	char *conf_name = NULL;
	int len = 0;
	long mode = AIR_R;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|l", &conf_name, &len, &mode) == FAILURE){
		php_error(E_WARNING, "air\\mysql\\keeper::acquire($config, $mode) params error");
		return;
	}
	zval *conf_entry = air_mysql_keeper_find_entry(conf_name, len, mode);
	zval *simplex = air_arr_find(conf_entry, ZEND_STRS("simplex"));
	zval *mysqli = NULL;
	int status = 0;
	if(!simplex){
		zval *config;
		int found_conf_path = air_config_path_get(NULL, conf_name, len, &config TSRMLS_CC);
		if(found_conf_path == FAILURE){
			air_throw_exception_ex(1, "mysql config %s not found", conf_name);
			return ;
		}
		//todo add quota
		status = air_mysql_keeper_get_mysqli(&mysqli, config, mode TSRMLS_CC);
		if(status == SUCCESS){
			add_assoc_zval_ex(conf_entry, ZEND_STRS("simplex"), mysqli);
			simplex = air_arr_find(conf_entry, ZEND_STRS("simplex"));
		}
	}
	if(status == SUCCESS){
		RETURN_ZVAL(simplex, 1, 0);
	}else{
		air_throw_exception_ex(1, "could not connect to %s", conf_name);
		return ;
	}
	RETURN_LONG(status);
}

PHP_METHOD(air_mysql_keeper, release) {
	char *conf_name;
	int len;
	zval *mysqli;
	long mode = AIR_R;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "zs|l", &mysqli, &conf_name, &len, &mode) == FAILURE){
		return ;
	}
	zval *conf_entry = air_mysql_keeper_find_entry(conf_name, len, mode);
	zval *free = air_arr_find(conf_entry, ZEND_STRS("free"));
	zval *busy = air_arr_find(conf_entry, ZEND_STRS("busy"));

	ulong mysqli_id = air_mysqli_get_id(mysqli);
	if(!air_arr_idx_find(busy, mysqli_id)){
		zval *simplex = air_arr_find(conf_entry, ZEND_STRS("simplex"));
		if(simplex && air_mysqli_get_id(simplex) == mysqli_id){
			php_error(E_ERROR, "mysqli not found in busy pool, please check your code");
		}
		return ;
	}
	Z_ADDREF_P(mysqli);
	zend_hash_index_del(Z_ARRVAL_P(busy), mysqli_id);
	//array push
	zend_hash_next_index_insert(Z_ARRVAL_P(free), (void **)&mysqli, sizeof(zval *), NULL);
}

PHP_METHOD(air_mysql_keeper, __destruct) {
	zval *instance = zend_read_static_property(air_mysql_keeper_ce, ZEND_STRL("_instance"), 0 TSRMLS_CC);
	zval *pool = zend_read_property(air_mysql_keeper_ce, instance, ZEND_STRL("_pool"), 0 TSRMLS_CC);
	zval *val;
	ulong idx;
	char *key;
	int key_len;
	AIR_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(pool), idx, key, key_len, val){
		zval *r = air_arr_idx_find(val, AIR_R);
		air_mysql_keeper_close_mysqli(air_arr_find(r, ZEND_STRS("free")));
		air_mysql_keeper_close_mysqli(air_arr_find(r, ZEND_STRS("busy")));
		zval *w = air_arr_idx_find(val, AIR_W);
		air_mysql_keeper_close_mysqli(air_arr_find(w, ZEND_STRS("free")));
		air_mysql_keeper_close_mysqli(air_arr_find(w, ZEND_STRS("busy")));
	}AIR_HASH_FOREACH_END();
}

/* }}} */

/* {{{ air_mysql_keeper_methods */
zend_function_entry air_mysql_keeper_methods[] = {
	PHP_ME(air_mysql_keeper, __construct, NULL,  ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(air_mysql_keeper, __destruct, NULL,  ZEND_ACC_PUBLIC | ZEND_ACC_DTOR)
	PHP_ME(air_mysql_keeper, release, air_mysql_keeper_acquire_arginfo,  ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(air_mysql_keeper, acquire, air_mysql_keeper_acquire_arginfo,  ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(air_mysql_keeper, simplex, air_mysql_keeper_acquire_arginfo,  ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};
/* }}} */

/* {{{ AIR_MINIT_FUNCTION */
AIR_MINIT_FUNCTION(air_mysql_keeper) {
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "air\\mysql\\keeper", air_mysql_keeper_methods);

	air_mysql_keeper_ce = zend_register_internal_class_ex(&ce, NULL, NULL TSRMLS_CC);
	zend_declare_property_null(air_mysql_keeper_ce, ZEND_STRL("_instance"), ZEND_ACC_PROTECTED | ZEND_ACC_STATIC TSRMLS_CC);
	zend_declare_property_null(air_mysql_keeper_ce, ZEND_STRL("_pool"), ZEND_ACC_PROTECTED TSRMLS_CC);
	return SUCCESS;
}
/* }}} */

