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

#ifndef PHP_AIR_CURL_H
#define PHP_AIR_CURL_H

#define CURLOPT_UPLOAD 46
#define CURLOPT_POST 47
#define CURLOPT_PUT 54
#define CURLOPT_CUSTOMREQUEST 10036
#define CURLOPT_TIMEOUT 13
#define CURLOPT_TIMEOUT_MS 155
#define CURLOPT_URL 10002
#define CURLOPT_POSTFIELDS 10015

AIR_MINIT_FUNCTION(air_curl);

extern zend_class_entry *air_curl_ce;

#endif

