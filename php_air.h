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

#ifndef PHP_AIR_H
#define PHP_AIR_H

extern zend_module_entry air_module_entry;
#define phpext_air_ptr &air_module_entry

#define PHP_AIR_VERSION "0.5.1"

#ifdef PHP_WIN32
#	define PHP_AIR_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_AIR_API __attribute__ ((visibility("default")))
#else
#	define PHP_AIR_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

PHP_MINIT_FUNCTION(air);
PHP_MSHUTDOWN_FUNCTION(air);
PHP_RINIT_FUNCTION(air);
PHP_RSHUTDOWN_FUNCTION(air);
PHP_MINFO_FUNCTION(air);

PHP_FUNCTION(confirm_air_compiled);	/* For testing, remove later. */

ZEND_BEGIN_MODULE_GLOBALS(air)
	long  version;
ZEND_END_MODULE_GLOBALS(air)

/* In every utility function you add that needs to use variables 
   in php_air_globals, call TSRMLS_FETCH(); after declaring other 
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as AIR_G(variable).  You are 
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define AIR_G(v) TSRMG(air_globals_id, zend_air_globals *, v)
#else
#define AIR_G(v) (air_globals.v)
#endif

#include "src/air_common.h"
#include "src/air_exception.h"

#endif	/* PHP_AIR_H */
