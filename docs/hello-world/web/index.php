<?php
/**
 *
 * @author wukezhan<wukezhan@gmail.com>
 * 2015-11-05 23:03
 *
 */
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
