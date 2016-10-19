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

#ifndef PHP_AIR_MYSQLI_H
#define PHP_AIR_MYSQLI_H

#include "ext/mysqlnd/mysqlnd.h"
#include "ext/mysqli/mysqli_mysqlnd.h"
#include "ext/mysqli/php_mysqli_structs.h"

#define MYSQLI_ASSOC 1
#define MYSQLI_FOUND_ROWS 2
#define MYSQLI_ASYNC 8

#define AIR_ADD 1
#define AIR_DEL 2
#define AIR_SET 3
#define AIR_GET 4


#if PHP_MAJOR_VERSION > 5
#define _AIR_INIT_MYSQL(mysql, mysql_link) \
	zval *return_value;\
	MYSQLI_FETCH_RESOURCE_CONN(mysql, mysql_link, MYSQLI_STATUS_VALID)
#elif PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 4
#define _AIR_INIT_MYSQL(mysql, mysql_link) \
	zval *return_value;\
	MYSQLI_FETCH_RESOURCE_CONN(mysql, &mysql_link, MYSQLI_STATUS_VALID)
#else
#define _AIR_INIT_MYSQL(mysql, mysql_link) \
	zval *return_value;\
	MYSQLI_FETCH_RESOURCE_CONN(mysql, &mysql_link, MYSQLI_STATUS_VALID)
#endif

static inline ulong air_mysqli_get_id(zval *mysql_link TSRMLS_DC){
	MY_MYSQL *mysql;
	_AIR_INIT_MYSQL(mysql, mysql_link);
	return (ulong)mysql_thread_id(mysql->mysql);
}

static inline ulong air_mysqli_get_errno(zval *mysql_link TSRMLS_DC){
	MY_MYSQL *mysql;
	_AIR_INIT_MYSQL(mysql, mysql_link);
	return (ulong)mysql_errno(mysql->mysql);
}

static inline char *air_mysqli_get_error(zval *mysql_link TSRMLS_DC){
	MY_MYSQL *mysql;
	_AIR_INIT_MYSQL(mysql, mysql_link);
	return (char *)mysql_error(mysql->mysql);
}

static inline zval *air_mysqli_get_insert_id(zval *mysql_link TSRMLS_DC){
	MY_MYSQL *mysql;
	_AIR_INIT_MYSQL(mysql, mysql_link);
	my_longlong id = mysql_insert_id(mysql->mysql);
	zval *ret;
	MAKE_STD_ZVAL(ret);
	if (id == (my_longlong) -1) {
		ZVAL_LONG(ret, -1);
	}
	if (id < LONG_MAX) {
		ZVAL_LONG(ret, (long) id);
	} else {
		char *num_str;
		/* always used with my_ulonglong -> %llu */
		int l = spprintf(&num_str, 0, MYSQLI_LLU_SPEC, id);
		ZVAL_STRINGL(ret, num_str, l, 0);
	}
	return ret;
}

static inline zval *air_mysqli_get_total_rows(zval *mysql_link TSRMLS_DC){
	MY_MYSQL *mysql;
	_AIR_INIT_MYSQL(mysql, mysql_link);
	//refer to ext/mysqli/mysqli_api.c: 157
	my_longlong ar = mysql_affected_rows(mysql->mysql);
	zval *ret;
	MAKE_STD_ZVAL(ret);
	if (ar == (my_longlong) -1) {
		ZVAL_LONG(ret, -1);
	}
	if (ar < LONG_MAX) {
		ZVAL_LONG(ret, (long) ar);
	} else {
		char *num_str;
		/* always used with my_ulonglong -> %llu */
		int l = spprintf(&num_str, 0, MYSQLI_LLU_SPEC, ar);
		ZVAL_STRINGL(ret, num_str, l, 0);
	}
	return ret;
}

#endif

