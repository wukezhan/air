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
#include "ext/standard/php_smart_str.h"

#include "php_air.h"
#include "src/air_async_waiter.h"
#include "src/air_loader.h"

#include "src/air_async_scheduler.h"

zend_class_entry *air_async_scheduler_ce;

int air_async_scheduler_loop(zval *self){
	if(ZVAL_IS_NULL(self)){
		php_error(E_NOTICE, "no active scheduler found in air\\async\\scheduler::loop()");
		return 0;
	}
	zval *waiting_waiters = zend_read_property(air_async_scheduler_ce, self, ZEND_STRL("_working_waiters"), 0 TSRMLS_CC);
	if(ZVAL_IS_NULL(waiting_waiters)){
		waiting_waiters = zend_read_property(air_async_scheduler_ce, self, ZEND_STRL("_waiters"), 0 TSRMLS_CC);
		zval *utime = zend_read_static_property(air_async_scheduler_ce, ZEND_STRL("utime"), 0);
		unsigned int ut = Z_LVAL_P(utime)>0?Z_LVAL_P(utime): 0;
		zend_update_property(air_async_scheduler_ce, self, ZEND_STRL("_working_waiters"), waiting_waiters TSRMLS_CC);
		smart_str func_name[6];
		int i = 0;
		for(; i<6; i++){
			smart_str s = {0};
			smart_str_appends(&s, "step_");
			smart_str_append_long(&s, i);
			smart_str_0(&s);
			func_name[i] = s;
		}
		while(zend_hash_num_elements(Z_ARRVAL_P(waiting_waiters))){
			ulong idx;
			char *key;
			int key_len;
			zval *waiter;
			AIR_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(waiting_waiters), idx, key, key_len, waiter){
				zval *context = zend_read_property(air_async_waiter_ce, waiter, ZEND_STRL("_context"), 0 TSRMLS_CC);
				zval *step = air_arr_find(context, ZEND_STRS("step"));
				int is = step?Z_LVAL_P(step):-1;
				if(is >= 0){
					air_call_method(&waiter, Z_OBJCE_P(waiter), NULL, func_name[is].c, func_name[is].len, NULL, 0, NULL);
				}else{
					zend_hash_del(Z_ARRVAL_P(waiting_waiters), key, key_len);
				}
			}AIR_HASH_FOREACH_END();
			usleep(ut);
		}
		zend_update_property_null(air_async_scheduler_ce, self, ZEND_STRL("_working_waiters"));
		i = 0;
		for(; i<6; i++){
			smart_str_free(&func_name[i]);
		}
	}else{
		php_error(E_ERROR, "air\\async\\scheduler::loop() is not reentrancy");
	}
	return 1;
}

/* {{{ ARG_INFO */
ZEND_BEGIN_ARG_INFO_EX(air_async_scheduler_construct_arginfo, 0, 0, 0)
	ZEND_ARG_INFO(0, config)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_async_scheduler_acquire_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, class_name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_async_scheduler_none_arginfo, 0, 0, 0)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ PHP METHODS */
PHP_METHOD(air_async_scheduler, __construct) {
	AIR_INIT_THIS;
	zval *waiters;
	MAKE_STD_ZVAL(waiters);
	array_init(waiters);
	zend_update_property(air_async_scheduler_ce, self, ZEND_STRL("_waiters"), waiters TSRMLS_CC);
	zval_ptr_dtor(&waiters);
	AIR_RET_THIS;
}

PHP_METHOD(air_async_scheduler, acquire) {
	zval *class_name;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &class_name) == FAILURE){
		return ;
	}

	zval *worker = zend_read_static_property(air_async_scheduler_ce, ZEND_STRL("_worker"), 1);
	if(ZVAL_IS_NULL(worker)){
		zval *_worker;
		MAKE_STD_ZVAL(_worker);
		object_init_ex(_worker, air_async_scheduler_ce);
		air_call_object_method(&_worker, air_async_scheduler_ce, "__construct", NULL, 0, NULL);
		zend_update_static_property(air_async_scheduler_ce, ZEND_STRL("_worker"), _worker);
		zval_ptr_dtor(&_worker);
		worker = zend_read_static_property(air_async_scheduler_ce, ZEND_STRL("_worker"), 1);
	}
	zval *waiters = zend_read_property(air_async_scheduler_ce, worker, ZEND_STRL("_waiters"), 0 TSRMLS_CC);
	zval *obj = air_arr_find(waiters, Z_STRVAL_P(class_name), Z_STRLEN_P(class_name));
	if(!obj){
		zend_class_entry *ce = air_loader_lookup_class(Z_STRVAL_P(class_name), Z_STRLEN_P(class_name) TSRMLS_CC);
		if (!ce){
			php_error(E_ERROR, "class `%s` not found", Z_STRVAL_P(class_name));
			return ;
		}
		if(!instanceof_function(ce, air_async_waiter_ce)){
			php_error(E_ERROR, "class `%s` is not a subclass of air\\async\\waiter", Z_STRVAL_P(class_name));
		}
		zval *_obj;
		MAKE_STD_ZVAL(_obj);
		object_init_ex(_obj, ce);
		air_call_object_method(&_obj, ce, "__construct", NULL, 0, NULL);
		add_assoc_zval_ex(waiters, Z_STRVAL_P(class_name), Z_STRLEN_P(class_name)+1, _obj);
		obj = air_arr_find(waiters, Z_STRVAL_P(class_name), Z_STRLEN_P(class_name)+1);
	}
	RETURN_ZVAL(obj, 1, 0);
}

PHP_METHOD(air_async_scheduler, loop) {
	AIR_INIT_THIS;
	zval *worker = zend_read_static_property(air_async_scheduler_ce, ZEND_STRL("_worker"), 0 TSRMLS_CC);
	air_async_scheduler_loop(worker);
}
/* }}} */

/* {{{ air_async_scheduler_methods */
zend_function_entry air_async_scheduler_methods[] = {
	PHP_ME(air_async_scheduler, __construct, air_async_scheduler_construct_arginfo,  ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(air_async_scheduler, acquire, air_async_scheduler_acquire_arginfo,  ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	PHP_ME(air_async_scheduler, loop, air_async_scheduler_none_arginfo,  ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};
/* }}} */

/* {{{ AIR_MINIT_FUNCTION */
AIR_MINIT_FUNCTION(air_async_scheduler) {
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "air\\async\\scheduler", air_async_scheduler_methods);

	air_async_scheduler_ce = zend_register_internal_class_ex(&ce, NULL, NULL TSRMLS_CC);

	zend_declare_property_null(air_async_scheduler_ce, ZEND_STRL("_worker"), ZEND_ACC_PROTECTED | ZEND_ACC_STATIC);
	zend_declare_property_long(air_async_scheduler_ce, ZEND_STRL("utime"), 200, ZEND_ACC_PUBLIC | ZEND_ACC_STATIC);
	zend_declare_property_null(air_async_scheduler_ce, ZEND_STRL("_waiters"), ZEND_ACC_PROTECTED);
	zend_declare_property_null(air_async_scheduler_ce, ZEND_STRL("_working_waiters"), ZEND_ACC_PROTECTED);
	return SUCCESS;
}
/* }}} */
