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
#include "ext/standard/info.h"
#include "php_air.h"

#include "air_async_service.h"
#include "air_async_waiter.h"
#include "air_config.h"
#include "air_curl.h"
#include "air_curl_waiter.h"
#include "air_exception.h"
#include "air_handler.h"
#include "air_loader.h"
#include "air_mysql_builder.h"
#include "air_mysql_keeper.h"
#include "air_mysql_table.h"
#include "air_mysql_waiter.h"
#include "air_app.h"
#include "air_router.h"
#include "air_controller.h"
#include "air_view.h"

ZEND_DECLARE_MODULE_GLOBALS(air)

/* True global resources - no need for thread safety here */
static int le_air;

/* {{{ air_functions[]
 *
 * Every user visible function must have an entry in air_functions[].
 */
const zend_function_entry air_functions[] = {
	//PHP_FE(confirm_air_compiled,	NULL)		/* For testing, remove later. */
	PHP_FE_END	/* Must be the last line in air_functions[] */
};
/* }}} */

/* {{{ air_module_entry
 */
zend_module_entry air_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"air",
	air_functions,
	PHP_MINIT(air),
	PHP_MSHUTDOWN(air),
	PHP_RINIT(air),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(air),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(air),
#if ZEND_MODULE_API_NO >= 20010901
	PHP_AIR_VERSION,
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_AIR
ZEND_GET_MODULE(air)
#endif

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("air.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_air_globals, air_globals)
    STD_PHP_INI_ENTRY("air.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_air_globals, air_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_air_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_air_init_globals(zend_air_globals *air_globals)
{
	air_globals->global_value = 0;
	air_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(air)
{
	/* If you have INI entries, uncomment these lines
	REGISTER_INI_ENTRIES();
	*/
	AIR_MODULE_STARTUP(air_app);
	AIR_MODULE_STARTUP(air_async_service);
	AIR_MODULE_STARTUP(air_async_waiter);
	AIR_MODULE_STARTUP(air_config);
	AIR_MODULE_STARTUP(air_controller);
	AIR_MODULE_STARTUP(air_curl);
	AIR_MODULE_STARTUP(air_curl_waiter);
	AIR_MODULE_STARTUP(air_exception);
	AIR_MODULE_STARTUP(air_handler);
	AIR_MODULE_STARTUP(air_loader);
	AIR_MODULE_STARTUP(air_mysql_builder);
	AIR_MODULE_STARTUP(air_mysql_keeper);
	AIR_MODULE_STARTUP(air_mysql_table);
	AIR_MODULE_STARTUP(air_mysql_waiter);
	AIR_MODULE_STARTUP(air_router);
	AIR_MODULE_STARTUP(air_view);

	//if it's necessary to make it case-insensitive?
	REGISTER_LONG_CONSTANT("AIR_SITE", AIR_SITE, CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("AIR_EXEC", AIR_EXEC, CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("AIR_R", AIR_R, CONST_CS|CONST_PERSISTENT);
	REGISTER_LONG_CONSTANT("AIR_W", AIR_W, CONST_CS|CONST_PERSISTENT);
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(air)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(air)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(air)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(air)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "air support", "enabled");
	php_info_print_table_row(2, "version", PHP_AIR_VERSION);
	php_info_print_table_row(2, "support", "http://air.wukezhan.com/");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */


/* Remove the following function when you have successfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */

/* The previous line is meant for vim and emacs, so it can correctly fold and 
   unfold functions in source code. See the corresponding marks just before 
   function definition, where the functions purpose is also documented. Please 
   follow this convention for the convenience of others editing your code.
*/


