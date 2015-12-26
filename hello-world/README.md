# 快速入门

## 目录结构

一个典型、基本的 `air framework` 应用目录结构如下： 

```sh
. # ROOT_PATH 根目录
├── app # 应用文件目录
│   ├── exec    # 命令行可执行控制器目录
│   │   └── hello.php   # 命令行可执行控制器 `app\exec\hello` 文件路径
│   ├── site    # 网页请求控制器目录
│   │   ├── hello.php   # 网页请求控制器 `app\site\hello` 文件路径 
│   │   └── test.php
│   └── view    # 视图（模板）目录
│       └── hello
│           └── world.php   # 网页请求 控制器行为 `app\site\hello::action_world()` 的默认视图文件路径
├── bin # 命令行可执行文件目录
│   └── index.php # 命令行可执行入口文件
├── conf # 配置目录
│   ├── conf.php # php配置文件
│   └── site.conf # nginx站点配置
├── lib # 类库文件目录
│   └── air # `air framework` 用 PHP 实现的类库 
│       └── view    # 命名空间 `view` 目录 
│           └── smarty.php  # 类 `air\view\smarty` 的文件
└── web # webroot
    ├── index.php   # 网页请求入口文件
    └── static  # 静态资源文件目录
```

## 配置选项

网站的基本配置如下：

>`ROOT_PATH/conf/conf.php` 

```php
<?php
// 定义根目录，必需，否则自动加载应用及类库类报错
define('ROOT_PATH', '/path/to/root');
// 定义类库目录，必需，否则自动加载类库类报错
define('LIB_PATH', ROOT_PATH . '/lib');

air\config::set([
    'app' => [
        'path' => 'app', // app 目录名，同时也作为应用类命名空间前缀，必须为单级目录名
                         // 可以以 air\config::path_get('app.path') 方式进行访问
        'exec' => [
            'path' => 'exec', // 命令行可执行控制器相对路径名
        ],
        'site' => [
            'path' => 'site',// 网页请求控制器相对路径名
        ],
        'view' => [
            'engine' => 'air\view\smarty', // 视图引擎类名，默认为 `air\view`
            'path' => 'view', // 相对路径名
            'type' => '.tpl', // 默认为 `.php`
            'config' => [ // 默认无此配置项，此处为 `air\view\smarty` 的自定义配置项
                'template_dir' => ROOT_PATH."/app/view/",
                'compile_dir' => ROOT_PATH."/tmp/view_c/"
            ],
        ]
    ]
]);

// 开启类的自动加载机制
air\loader::autoload();
```


#### 配置项说明

* `ROOT_PATH` 网站根目录
* `LIB_PATH` 类库根目录
* `app.path` 应用根目录，其值为相对于 `ROOT_PATH` 的 **单级** 目录名，此目录名将会同时被用作应用的命名空间前缀，以配置为例，配置中 `app.path` 为 `app`，那么 `air\loader` 会自动从 `ROOT_PATH`.`'/app'` 目录下寻找所有命名空间以 `app` 开头的类
    * `app.{exec|site}.path` 为 `{exec|site}` `controller` 的目录路径，其值须为与命名空间相对应的相对路径
        * 单级路径，示例中为单级目录 `site`，则控制器 `hello` 的完整类名为 `app\site\hello`；如其值为 `controller`，则控制器的完整类名为 `app\controller\hello`；
        * 多级路径，如可将其值设置为 `sub_app1/{exec|site}`，则控制器 `hello` 的完整类名须为 `app\sub_app1\site\hello`，其完整文件路径为 `ROOT_PATH`.`'/app/sub_app1/site/hello.php'`

## 自动加载

如上述配置所定义：

1. 应用类，以及所有命名空间以 `app.path`（上述示例中为 `'app'` ）开头的类，将会自动从 `ROOT_PATH`.`'/app'` 目录进行自动加载；

2. 类库类，亦即所有应用类除外的类，将会自动从 `LIB_PATH` 目录进行加载；如果加载失败，则将会再次从 `ROOT_PATH` 目录下按照命名空间进行自动加载；
 
## 入口文件

* #### 命令行可执行入口脚本

>`ROOT_PATH/bin/index.php` 

```php
#!/usr/bin/php
<?php
date_default_timezone_set('Asia/Shanghai');

define('ROOT_PATH', dirname(__DIR__));
define('LIB_PATH', ROOT_PATH.'/lib');

air\loader::autoload();

$router = new air\router();
$router->set_rules([
    '^(<c:\w+>/?(<a:\w+>)?)?' => '$c.$a'
])->set_url($argv[1]);

$app = new air\app(AIR_EXEC); // AIR_EXEC 表示是一个命令行应用，app 将从 app.exec.path 下去加载controller 
$app->set_router($router)->run();

```

对于上述路由规则你可以使用如下命令进行访问：

> `ROOT_PATH/bin/index.php hello/world -a1 -b2`

或

> `/path/to/php ROOT_PATH/bin/index.php hello/world -a1 -b2`

* #### 网页请求入口脚本

> `ROOT_PATH/web/index.php` 

```php
<?php
date_default_timezone_set('Asia/Shanghai');
//ini_set('display_errors', 'off');
//error_reporting(0);

define('ROOT_PATH', dirname(__DIR__));
define('LIB_PATH', ROOT_PATH.'/lib');

air\loader::autoload();

if($argv){
    $url = $argv[1]?: '/';
}else{
    $url = $_SERVER['REQUEST_URI']?: '/';
    if($pos = strpos($url, '?')){
        $url = substr($url, 0, $pos);
    }
}

$router = new air\router();
$router->set_rules([
    '^/$' => 'hello.world',
    '^/(<c:\w+>/?(<a:\w+>)?)?' => '$c.$a'
])->set_url($url);

$app = new air\app();
$app->set_router($router)->run();

```

对于上述路由你可以使用如下命令进行访问：

> `http://your.site.name/hello/world`

或

> `/path/to/php ROOT_PATH/web/index.php /hello/world`
