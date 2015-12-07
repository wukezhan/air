<?php
/**
 *
 * @author wukezhan<wukezhan@gmail.com>
 * 2015-11-05 23:03
 *
 */
define('ROOT_PATH', __DIR__);
define('LIB_PATH', ROOT_PATH.'/lib');

ini_set('display_errors', 'off');
error_reporting(0);

air\loader::autoload();

air\handler::on_error(function(){
    $args = func_get_args();
    var_dump('error', $args);
}, E_ALL & ~E_NOTICE & ~E_STRICT & ~E_DEPRECATED);

air\handler::on_exception(function(Exception $e){
    echo '<pre>';
    echo "<h2>", $e->getCode().': '.$e->getMessage(), "</h2>";
    echo $e->getTraceAsString();
    echo '</pre>';
});

air\handler::on_shutdown(function(){
    $error = error_get_last();
    if(isset($error) && ($error['type'] & (E_ALL & ~E_NOTICE & ~E_STRICT & ~E_DEPRECATED))){
        echo "<pre>";
        echo "<h2>";
        echo $error['type'].': '.$error['message'];
        echo "</h2>";
        echo "<p>";
        echo "in {$error['file']} on line {$error['line']}";
        echo "</p>";
        echo "</pre>";
    }else{
        echo "\n<!--[status ok]-->\n";
    }
});

if($argv){
    $url = $argv[1]?: '/';
}else{
    $url = isset($_GET['_'])?$_GET['_']: ($_SERVER['REQUEST_URI']?:'/');
}

$router = new air\router();
$router->set_rules([
    '^/$' => 'hello.world',
    '^/hello/?(<a:\w+>)?' => 'hello.world',
    '^/(<c:\w+>/?(<a:\w+>)?)?' => '$c.$a'
])->set_url($url);

$app = new air\app();
$app->set_router($router)->run();
