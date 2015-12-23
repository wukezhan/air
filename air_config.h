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

zval *air_config_get_data();
zval *air_config_get(zval *data, zend_string *key);
zval *air_config_str_get(zval *data, char *key, int key_len);
zval *air_config_path_get(zval *data, zend_string *path);
zval *air_config_str_path_get(zval *data, char *path, int path_len);

#endif

