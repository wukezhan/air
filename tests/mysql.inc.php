<?php
/**
 *
 * @author wukezhan<wukezhan@gmail.com>
 * 2015-10-28 12:30
 *
 */

define('DB_NAME', 'air');
define('TABLE_NAME', 'test');
define('DB_CONF', 'mysql.config.'.DB_NAME);
define('R', AIR_R);
define('W', AIR_W);
define('MYSQL_HOST', 'localhost');
define('MYSQL_PORT', '3306');
define('MYSQL_USER', 'root');
define('MYSQL_PASS', '');

air\config::set([
    'mysql' => [
        'config' => [
            'air' => [
                'auth' => [
                    R => [
                        'username' => MYSQL_USER,
                        'password' => MYSQL_PASS,
                    ],
                    W => [
                        'username' => MYSQL_USER,
                        'password' => MYSQL_PASS,
                    ],
                ],
                'pool' => [
                    R => [[
                        'host' => MYSQL_HOST,
                        'port' => MYSQL_PORT,
                    ]],
                    W => [[
                        'host' => MYSQL_HOST,
                        'port' => MYSQL_PORT,
                    ]],
                ],
            ]
        ]
    ]
]);

function create_database(){
    $db = DB_NAME;
    $mysqli = air\mysql\keeper::simplex(DB_CONF, W);
    $sql = "DROP DATABASE IF EXISTS `{$db}`";
    $mysqli->query($sql);
    $sql = "CREATE DATABASE `{$db}` DEFAULT CHARACTER SET `utf8`";
    $mysqli->query($sql);
    if($mysqli->errno){
         exit("create database {$db} error: ".$mysqli->error."\n");
    }
}

function create_table(){
    $db = DB_NAME;
    $table = TABLE_NAME;
    $mysqli = air\mysql\keeper::simplex(DB_CONF, W);
    $sql = "DROP TABLE IF EXISTS `{$table}`";
    $mysqli->query($sql);
    $sql = <<<EOT
CREATE TABLE `{$db}`.`{$table}` (
    `id` int(11) unsigned NOT NULL AUTO_INCREMENT COMMENT 'id',
    `title` varchar(32) NOT NULL COMMENT 'title',
    `create_time` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'create time',
    `update_time` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'update time',
    PRIMARY KEY (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=1 DEFAULT CHARSET=utf8 COMMENT='test table'
EOT;

    $mysqli->query($sql);
    if($mysqli->errno){
        exit("create table {$table} error: ".$mysqli->error."\n");
    }
}

try{
    $mysqli = air\mysql\keeper::simplex(DB_CONF, R);
}catch(Exception $e){
    echo "skip";
    exit;
}


