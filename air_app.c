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

#include "air_config.h"
#include "air_exception.h"
#include "air_loader.h"
#include "air_router.h"

#include "air_app.h"

zend_class_entry *air_app_ce;

int air_app_try_controller_action(air_app_t *self, smart_str c, smart_str a, zval *route TSRMLS_DC){
	zend_class_entry *ce = air_loader_lookup_class(c.c, c.len TSRMLS_CC);
	if (!ce){
		php_error(E_NOTICE, "controller %s not found", c.c);
		return FAILURE;
	}
	zval *controller = NULL;
	MAKE_STD_ZVAL(controller);
	object_init_ex(controller, ce);
	if(zend_hash_exists(&(ce->function_table), a.c, a.len) == FAILURE){
		zval_ptr_dtor(&controller);
		php_error(E_NOTICE, "%s::%s not found,", c.c, a.c);
		return FAILURE;
	}
	air_controller_set_meta(controller, c, a TSRMLS_CC);
	air_controller_set_route(controller, route TSRMLS_CC);
	zend_call_method_with_0_params(&controller, ce, NULL, "__construct", NULL);
	zend_call_method_with_0_params(&controller, ce, NULL, "init", NULL);
	zend_call_method_with_0_params(&controller, ce, NULL, "before_action", NULL);
	air_call_method(&controller, ce, NULL, a.c, a.len, NULL, 0, NULL TSRMLS_CC);
	zend_call_method_with_0_params(&controller, ce, NULL, "after_action", NULL);
	zend_call_method_with_0_params(&controller, ce, NULL, "__destruct", NULL);
	zval_ptr_dtor(&controller);
	return SUCCESS;
}

int air_app_try_route(air_app_t *self, zval *route TSRMLS_DC){
	smart_str ss_c = {0};
	smart_str ss_a = {0};
	zval **c, **a;
	zend_hash_find(Z_ARRVAL_P(route), ZEND_STRS(air_c_key), (void **)&c);
	zend_hash_find(Z_ARRVAL_P(route), ZEND_STRS(air_a_key), (void **)&a);
	char *classname = Z_STRVAL_PP(c);
	int is_global_ns = 0;
	int idx = 0;
	while(idx < Z_STRLEN_PP(c)){
		if(classname[idx] == '/'){
			classname[idx] = '\\';
		}
		if(idx == 0 && classname[idx] == '\\'){
			is_global_ns = 1;
		}
		idx++;
	}
	if(!is_global_ns){
		zval *app_path = NULL;
		zval *controller_path = NULL;
		zval *app_conf = NULL;
		zval *app_type = zend_read_property(air_app_ce, self, ZEND_STRL("_type"), 1 TSRMLS_CC);
		char *ctrl_path_name = Z_LVAL_P(app_type)==1? "site.path": "exec.path";
		if(air_config_get(NULL, ZEND_STRS("app"), &app_conf TSRMLS_CC) == FAILURE){
			php_error(E_ERROR, "error config: app");
		}
		if(air_config_get(app_conf, ZEND_STRS("path"), &app_path TSRMLS_CC) == FAILURE){
			php_error(E_ERROR, "error config: app.path");
		}
		if(air_config_path_get(app_conf, ctrl_path_name, 10, &controller_path TSRMLS_CC) == FAILURE){
			php_error(E_ERROR, "app.%s not found", ctrl_path_name);
		}
		if(Z_TYPE_P(controller_path) != IS_STRING){
			php_error(E_ERROR, "app.%s must be a string", ctrl_path_name);
		}
		smart_str_appendl(&ss_c, Z_STRVAL_P(app_path), Z_STRLEN_P(app_path));
		smart_str_appendc(&ss_c, '\\');
		smart_str_appendl(&ss_c, Z_STRVAL_P(controller_path), Z_STRLEN_P(controller_path));
		smart_str_appendc(&ss_c, '\\');
		smart_str_appendl(&ss_c, classname, Z_STRLEN_PP(c));
	}else{
		smart_str_appendl(&ss_c, classname+1, Z_STRLEN_PP(c)-1);
	}
	smart_str_0(&ss_c);
	idx = 0;
	while(idx < ss_c.len){
		if(ss_c.c[idx] == '/'){
			ss_c.c[idx] = '\\';
		}
		idx++;
	}
	smart_str_appendl(&ss_a, "action_", 7);
	smart_str_appendl(&ss_a, Z_STRVAL_PP(a), Z_STRLEN_PP(a));
	smart_str_0(&ss_a);
	int status = air_app_try_controller_action(self, ss_c, ss_a, route TSRMLS_CC);
	smart_str_free(&ss_c);
	smart_str_free(&ss_a);
	return status;
}

int air_app_dispatch(air_app_t *self TSRMLS_DC) {
	air_router_t *router = zend_read_property(air_app_ce, self, ZEND_STRL("_router"), 1 TSRMLS_CC);
	if (router == NULL || Z_TYPE_P(router) != IS_OBJECT || !instanceof_function(Z_OBJCE_P(router), air_router_ce TSRMLS_CC)) {
		php_error(E_ERROR, "error router type");
	}
	zval *route = NULL;
	air_call_object_method(&router, Z_OBJCE_P(router), NULL, "route", &route, 0, NULL TSRMLS_CC);
	int status = FAILURE;
	do{
		if(Z_TYPE_P(route) == IS_NULL){
			zval_ptr_dtor(&route);
			break;
		}else{
			status = air_app_try_route(self, route TSRMLS_CC);
			zval_ptr_dtor(&route);
			if(status == FAILURE){
				air_call_object_method(&router, Z_OBJCE_P(router), NULL, "route", &route, 0, NULL TSRMLS_CC);
			}else{
				break;
			}
		}
	}while(status == FAILURE);
	return status;
}

/* {{{ ARG_INFO */
ZEND_BEGIN_ARG_INFO_EX(air_app_construct_arginfo, 0, 0, 0)
	ZEND_ARG_INFO(0, type)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_app_set_router_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, router)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_app_run_arginfo, 0, 0, 0)
	ZEND_ARG_INFO(0, url)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(air_app_try_route_arginfo, 0, 0, 1)
	ZEND_ARG_INFO(0, route)
ZEND_END_ARG_INFO()
/* }}} */

/* {{{ PHP METHODS */
PHP_METHOD(air_app, __construct) {
	AIR_INIT_THIS;
	long type = AIR_SITE;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|l", &type) == FAILURE){
		//use default
		return ;
	}
	zend_update_property_long(air_app_ce, self, ZEND_STRL("_type"), type TSRMLS_CC);
}

PHP_METHOD(air_app, instance) {
	air_app_t *instance = zend_read_static_property(air_app_ce, ZEND_STRL("_instance"), 1 TSRMLS_CC);
	if (ZVAL_IS_NULL(instance)) {
		zval_ptr_dtor(&instance);
		MAKE_STD_ZVAL(instance);
		object_init_ex(instance, air_app_ce);
		zend_update_static_property(air_app_ce, ZEND_STRL("_instance"), instance TSRMLS_CC);
	}
	RETVAL_ZVAL(instance, 1, 0);
	zval_ptr_dtor(&instance);
}

PHP_METHOD(air_app, set_router) {
	AIR_INIT_THIS;
	air_router_t *router;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &router) == FAILURE)
	{
		AIR_NEW_EXCEPTION(1, "error router");
	}
	if(Z_TYPE_P(router) != IS_OBJECT || !instanceof_function(Z_OBJCE_P(router), air_router_ce TSRMLS_CC)) {
		AIR_NEW_EXCEPTION(1, "error router");
	}
	zend_update_property(air_app_ce, self, ZEND_STRL("_router"), router TSRMLS_CC);
	AIR_RET_THIS;
}

PHP_METHOD(air_app, run) {
	AIR_INIT_THIS;
	zend_call_method_with_0_params(&self, air_app_ce, NULL, "dispatch", NULL);
	AIR_RET_THIS;
}

PHP_METHOD(air_app, dispatch) {
	AIR_INIT_THIS;

	if(air_app_dispatch(self TSRMLS_CC) == FAILURE){
		AIR_NEW_EXCEPTION(1, "404 not found");
	}
	AIR_RET_THIS;
}

/* }}} */

/* {{{ air_app_methods */
zend_function_entry air_app_methods[] = {
	PHP_ME(air_app, __construct, air_app_construct_arginfo,  ZEND_ACC_PUBLIC | ZEND_ACC_CTOR)
	PHP_ME(air_app, run,	air_app_run_arginfo,  ZEND_ACC_PUBLIC)
	PHP_ME(air_app, set_router,	air_app_set_router_arginfo,  ZEND_ACC_PUBLIC)
	PHP_ME(air_app, dispatch,	NULL,  ZEND_ACC_PUBLIC)
	PHP_ME(air_app, instance,	NULL,  ZEND_ACC_PUBLIC | ZEND_ACC_STATIC)
	{NULL, NULL, NULL}
};
/* }}} */

/* {{{ AIR_MINIT_FUNCTION */
AIR_MINIT_FUNCTION(air_app) {
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "air\\app", air_app_methods);

	air_app_ce = zend_register_internal_class_ex(&ce, NULL, NULL TSRMLS_CC);

	zend_declare_property_null(air_app_ce, ZEND_STRL("_instance"), ZEND_ACC_STATIC | ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_null(air_app_ce, ZEND_STRL("_router"), ZEND_ACC_PROTECTED TSRMLS_CC);
	zend_declare_property_long(air_app_ce, ZEND_STRL("_type"), AIR_SITE, ZEND_ACC_PROTECTED TSRMLS_CC);
	return SUCCESS;
}
/* }}} */
