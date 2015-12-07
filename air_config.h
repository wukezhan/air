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

#ifndef PHP_AIR_CONFIG_H
#define PHP_AIR_CONFIG_H

AIR_MINIT_FUNCTION(air_config);

extern zend_class_entry *air_config_ce;


zval *air_config_get_data(TSRMLS_DC);
int air_config_get(zval *data, const char *key, int key_len, zval **val TSRMLS_DC);
int air_config_get_path(zval *data, const char *path, int path_len, zval **val TSRMLS_DC);

#endif

