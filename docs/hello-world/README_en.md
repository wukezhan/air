# Quick Start

## App Structure

the basic structure of `air framework` app is as below: 

```sh
. # ROOT_PATH the root dir path
├── app # the app dir path
│   ├── exec    # the console command controller dir path
│   │   └── hello.php   # the console controller `app\exec\hello` file path
│   ├── site    # the web page controller dir path
│   │   ├── hello.php   # the web page controller `app\site\hello` file path 
│   │   └── test.php
│   └── view    # the view(template) dir path
│       └── hello
│           └── world.php   # the default view path of web page controller-action `app\site\hello::action_world()`
├── bin # the console command dir
│   └── index.php # a entry file path of console commands
├── conf # config dir
│   ├── conf.php # php config
│   └── site.conf # nginx server conf
├── lib # 类库文件目录
│   └── air # `air framework` library in PHP 
│       └── view    # dir of namespace `air\view` 
│           └── smarty.php  # `air\view\smarty` class file
└── web # webroot
    ├── index.php   # web page request entry file
    └── static  # static resources dir
```

## Configuration

the default config：

>`ROOT_PATH/conf/conf.php` 

```php
<?php
// required
define('ROOT_PATH', '/path/to/root');
// required
define('LIB_PATH', ROOT_PATH . '/lib');

air\config::set([
    'app' => [
        'path' => 'app', // app dir name, must be used as a part of namespace (no \ or / )
                         // you can get its value by air\config::path_get('app.path')
        'exec' => [
            'path' => 'exec', // console command controller dir
        ],
        'site' => [
            'path' => 'site',// web page controller dir
        ],
        'view' => [
            'engine' => 'air\view', // view engine class name
            'path' => 'view', // the relative path to `app.path`
            'type' => '.php', // 默认为 `.php`
        ]
    ]
]);

// enable the air autoload function
air\loader::autoload();
```


#### Configuration

* `ROOT_PATH` the site root path
* `LIB_PATH` the dir path of all the libraries 
* `app.path` app root path，its value must be a single-level dir path under the `ROOT_PATH`，it must be used as the start of the app namespace. For example, when `app.path` is `app`，then `air\loader` will autoload all the class which started with `app` from ROOT_PATH`.`'/app'`.
    * `app.{exec|site}.path` is the `{exec|site}` `controller` dir path, it's a relative path under ROOT_PATH . `'/app'`
        * eg. when the `app.site.path` value is `site`, then web page controller `hello` name is `app\site\hello`, when the value is `controller`, the complete name is `app\controller\hello`；
        * when the `app.site.path` value is `sub_app1/site`，then the name of web page controller `hello` is `app\sub_app1\site\hello`, its file path is `ROOT_PATH`.`'/app/sub_app1/site/hello.php'`

## Autoload

as defined above:

1. app classes, which started with the `app.path`(`'app'` in this example), will be automatically loaded from `ROOT_PATH`.`'/app'` by namespace；

2. lib classes, all the other classes except app classes ，will be automatically loaded from `LIB_PATH` dir by namespace; if not found, then will be from `ROOT_PATH` once again.
 
## Entry files

* #### console command entry

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

$app = new air\app(AIR_EXEC); // AIR_EXEC indicate that it's a executable app, the controller will be located from app.exec.path 
$app->set_router($router)->run();

```

execute as:

> `ROOT_PATH/bin/index.php hello/world -a1 -b2`

or

> `/path/to/php ROOT_PATH/bin/index.php hello/world -a1 -b2`

* #### web page request entry

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

visit as:

> `http://your.site.name/hello/world`

or

> `/path/to/php ROOT_PATH/web/index.php /hello/world`
