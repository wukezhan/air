# air framework

a high performance, lightweight framework for php written in c

## features

### high performance and lightweight

the QPS, CPU and memory uses performance are all much better than the other php web frameworks.

### simple and extensible

simple to use, and easy to extend it. the **`air framework`** helps building sites from tiny to large and simple to complex more efficiently.

the MVC core can processed in only microseconds.

### Asynchronous and concurrent

helps developer really improve the app performance more efficiently, by using the async and concurrent features in mysql queries and remote http requests.

#### Async and concurrent mysql queries

```php
<?php
/**
 * air\config::path_get('mysql.config.air');
 * @see https://github.com/wukezhan/air/blob/master/tests/mysqli.inc.php
 * @see https://github.com/wukezhan/air/blob/master/tests/004.mysql.phpt
 */
$b1 = new air\mysql\builder('mysql.config.air');
$b2 = new air\mysql\builder('mysql.config.air');
$b3 = new air\mysql\builder('mysql.config.air');
$start = time();
$b1->async()->query('SELECT 1 AS a, sleep(1) AS b');
$b2->async()->query('SELECT 2 AS a, sleep(1) AS b');
$b3->async()->query('SELECT 3 AS a, sleep(1) AS b');
// really start all the async queries
var_dump($b1->data()); // or var_dump($b[0]); // or foreach($b as $k=>$v){...}
var_dump($b2->data());
var_dump($b3->data());
// will totally use 1 second but not 3 seconds
echo time()-$start, "\n";
```

#### Async and concurrent http requests

```php
<?php
/**
 * @see https://github.com/wukezhan/air/blob/master/tests/server.inc.php
 * @see https://github.com/wukezhan/air/blob/master/tests/003.curl.phpt
 */
function data2arr($ch, $data){
    return json_decode($data, 1);
}

$c1 = new air\curl();
$c1->async();
$c1->setopt(CURLOPT_RETURNTRANSFER, 1);
$c1->get(local_server::$host_port . '/get.php?hello=world-1');

$c2 = new air\curl();
$c2->async()
    ->setopt(CURLOPT_RETURNTRANSFER, 1)
    ->get(local_server::$host_port . '/json.php?hello=world-2')
    ->on_success('data2arr')
;

echo $c1->data();
var_dump($c2->data());
```

### Advanced code organization

the default class autoloading is namespace based, which can help developer load almost all the classes without having to write a long list of needed includes. 

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
* PHP 5.4+
    * mysqlnd
    * mysqli
    * curl

```sh
# configure options
./configure --prefix=/path/to/php --enable-mysqlnd --with-mysqli --with-curl
```

### Install

```sh
# 1. download
### php5
https://github.com/wukezhan/air/archive/master.zip
### php7
https://github.com/wukezhan/air/archive/php7.zip
### clone
git clone https://github.com/wukezhan/air

cd air
# for clone
git checkout php7

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

follow me for more information:

* blog: [www.wukezhan.com](http://www.wukezhan.com)
* twitter: [wukezhan](https://twitter.com/wukezhan)
* github: [wukezhan/air](https://github.com/wukezhan/air)

