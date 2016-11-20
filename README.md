# air framework

一款使用 `C` 语言写成的、现代的高性能轻量级`PHP` `web`框架。

> ####see [README_en.md](README_en.md) for english introduction.

## 特性

### 1. 高性能、轻量级
  - 在各PHP框架的基准压力测试中，air framework 在并发请求数、CPU及内存资源占用等各方面指标上均以显著优势大幅领先于其他框架
  - 更针对实际业务中最耗时的IO场景做特别的异步并发优化

### 2. 独家首创的全局异步并发模式
  - 基于独创的 RWS 异步并发模式
  - 多种IO同时并发，全局异步统一调度
  - 支持以顺序、自然的同步风格书写异步、并发的代码逻辑
  - 已支持MySQL、curl两大主要IO场景，理论上可支持任意类型IO的并发执行

### 3. 极简高效的代码运行组织机制
  - 设计良好的 MVC 机制，轻松处理前端请求与后端任务
  - 基于正则的路由机制，高效满足各种刁钻路由需求
  - 基于命名空间的自动加载，最大化提升类库加载效率
  - 天然无限制扩展特性，air framework 始终以轻量高效为基本原则，拒绝冗长庞杂和面面俱到，只提供最基本、最核心底层机制，不会预置任何诸如表单验证之类的具体业务功能实现，而是强烈建议、鼓励开发者基于自身特点选用最适合自己的解决方案

### 全局的异步并发模式

在以下的场景中，每个数据库请求和curl请求都将需要1秒的等待时间，如果完全使用传统同步阻塞方案，完成全部将总共需要6秒的时间。

使用普通的PHP原生异步解决方案时，MySQL和curl将各需要1秒等待时间，此时完成全部请求将总共需要2秒时间，可3倍的提升响应速度。

但使用 air framework 的全局异步并发模式，完成全部的6个请求将总共只需要1秒等待时间，轻而易举即可取得6倍的响应性能提升。

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

## 文档及实例

* **[快速开始](docs/hello-world/README.md)**
* **实例**
	* [hello-world](docs/hello-world) 入门演示实例
	* [docker-ui](https://github.com/wukezhan/docker-ui) 一个简单的docker管理界面
	* [tests](tests) 测试实例
* **文档**
	* [接口文档](docs/api.md)
	* [air-book](http://air.wukezhan.com) [预备中]
	* [代码补全 & 函数原型](docs/helper/air.php)


## 安装及使用

### 环境要求

* Linux
* PHP 5.4+
    * mysqlnd
    * mysqli
    * curl
* PHP 7.0.0+

```sh
# configure options
./configure --prefix=/path/to/php --enable-mysqlnd --with-mysqli --with-curl
```

### 安装使用

- 一键体验

对于 docker 用户，可以通过以下命令一键体验 **`air framework`**

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

- 从源码安装

```sh
# 1. 准备源码
## 下载并解压缩源码包
### php5
https://github.com/wukezhan/air/archive/master.zip
### php7
https://github.com/wukezhan/air/archive/php7.zip
### 或从源码克隆
git clone https://github.com/wukezhan/air

cd air
# 对于克隆源码，可执行以下命令切换到 php7 分支
git checkout php7

# 2. 编译安装
/path/to/php/bin/phpize
./configure --with-php-config=/path/to/php/bin/php-config
make && make install
make test

# 3. 修改配置
vim /path/to/php/lib/php.ini
增加以下配置：
extension = air.so
```

## 更多

对本项目感兴趣的朋友，可以关注微博 [weibo.com/wukezhan](http://weibo.com/wukezhan) 或通过以下方式进行交流：

* QQ群: 141971669
* twitter: [wukezhan](https://twitter.com/wukezhan)
* github: [wukezhan/air](https://github.com/wukezhan/air)
