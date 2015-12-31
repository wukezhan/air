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

#ifndef PHP_AIR_EXCEPTION_H
#define PHP_AIR_EXCEPTION_H

AIR_MINIT_FUNCTION(air_exception);

extern zend_class_entry *air_exception_ce;

zend_object *air_throw_exception(long code, const char* message);
zend_object *air_throw_exception_ex(long code, const char *format, ...);

#define AIR_NEW_EXCEPTION(code, message) air_throw_exception(code, message);return

#endif

