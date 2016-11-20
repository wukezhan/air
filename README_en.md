# Air framework

Air framework is a high performance, lightweight framework for php written in c

## Features

### 1. High performance and lightweight

* The QPS, CPU and memory uses performance are all much better than the other php web frameworks

* Specially optimizing for the most time-consuming IO scene of the actual business

### 2. Asynchronous and concurrent

* Based on the RWS asynchronous & concurrency design patterns

* Multiple asynchronous IO of different types can be uniformly scheduled

* Easily write asynchronous & concurrency logic in synchronous & sequential style

* Support for MySQL & CURL requests, and in theory, it can support any type of IO operations

### 3. Simple and extensible

* Well-designed MVC mechanism can easily handle the front-end requests and back-end tasks

* Regular expresions based routing mechanism can easily meet the needs of a variety of tricky routing

* Namespace-based autoloading can maximize the efficiency of code organization & class autoloading 

* Simple to use, and easy to extend

The **`air framework`** helps building websites from tiny to large and simple to complex more efficiently

#### Unified asynchronous and concurrent design patterns (RWS, which short for Request, Waiter and Service)

In the following scenario, each MySQL query and CURL request will take 1 second to process

If the traditional method is used completely, a total of 6 seconds will be required to complete all the requests

But when RWS is used, you can complete all the 6 requests in only 1 second

Let's see the code

```php
<?php
/**
 * air\config::path_get('mysql.config.air');
 * @see https://github.com/wukezhan/air/blob/master/tests/003.curl.phpt
 * @see https://github.com/wukezhan/air/blob/master/tests/mysql.inc.php
 * @see https://github.com/wukezhan/air/blob/master/tests/004.mysql.phpt
 */
define('URL', 'http://localhost/test/sleep');
$time = microtime(1);
$t = 1;
// each request below will use 1 second
$m1 = new mysql(DB_CONF);
$m1->async()->query("select sleep({$t}) as a");
$m2 = new mysql(DB_CONF);
$m2->async()->query("select sleep({$t}) as b");
$m3 = new mysql(DB_CONF);
$m3->async()->query("select sleep({$t}) as b");
$c1 = new curl();
$c1->async()->get(URL , ['sleep' => $t]);
$c2 = new curl();
$c2->async()->get(URL , ['sleep' => $t]);
$c3 = new curl();
$c3->async()->get(URL , ['sleep' => $t]);

var_dump($c3->data());
var_dump($c1->data());
var_dump($c2->data());
var_dump($m1->data());
var_dump($m2->data());
var_dump($m3->data());
echo "time used: ", microtime(1)-$time,"s\n";
// this will totally use 1 second but not 6 seconds
```

## Documents and examples

* **[quick start](docs/hello-world/README_en.md)**
* **examples**
	* [hello-world](docs/hello-world) hello-world app
	* [docker-ui](https://github.com/wukezhan/docker-ui) a simple web interface for docker, powered by air framework
	* [tests](tests) test cases
* **documents**
	* [manual](docs/api.md)
	* [air-book](http://air.wukezhan.com) [coming soon]
	* [code assistant & function prototype](docs/helper/air.php)


## Installation and usage

### Run in docker

```sh
# php5
docker run -it -p 2355:80 --name=airstack5 wukezhan/airstack5
# visit
curl http://localhost:2355/
# or
curl http://localhost:2355/hello/world

#php7
docker run -it -p 2357:80 --name=airstack7 wukezhan/airstack7
# visit
curl http://localhost:2357/
# or
curl http://localhost:2357/hello/world
```

### Requirements

* Linux
* PHP 5.4+ & PHP 7.0.0+
    * mysqlnd
    * mysqli
    * curl

```sh
# configure your php with the following options
./configure --prefix=/path/to/php --enable-mysqlnd --with-mysqli --with-curl
```

### Install

```sh
# 1. download
### php5
https://github.com/wukezhan/air/archive/php5.zip
### php7
https://github.com/wukezhan/air/archive/php7.zip
# or clone
### php5
git clone -b php5 https://github.com/wukezhan/air
### php7
git clone -b php7 https://github.com/wukezhan/air


# 2. configure and make install
/path/to/php/bin/phpize
./configure --with-php-config=/path/to/php/bin/php-config
make && make install
make test

# 3. add config
vim /path/to/php/lib/php.ini
#add
extension = air.so
```

## More

For more information:

* blog: [www.wukezhan.com](http://www.wukezhan.com)
* twitter: [wukezhan](https://twitter.com/wukezhan)
* github: [wukezhan/air](https://github.com/wukezhan/air)

