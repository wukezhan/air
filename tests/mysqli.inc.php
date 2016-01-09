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

air\config::set([
    'mysql' => [
        'config' => [
            'air' => [
                'auth' => [
                    R => [
                        'username' => 'core',
                        'password' => '222222',
                    ],
                    W => [
                        'username' => 'core',
                        'password' => '222222',
                    ],
                ],
                'pool' => [
                    R => [[
                        'host' => 'wdb.io',
                        'port' => 3306,
                    ]],
                    W => [[
                        'host' => 'wdb.io',
                        'port' => 3306,
                    ]],
                ],
            ]
        ]
    ]
]);

function create_database(){
    $db = DB_NAME;
    $mysqli = air\mysql\keeper::factory(DB_CONF, W);
    $sql = "DROP DATABASE IF EXISTS `{$db}`";
    $mysqli->query($sql);
    $sql = "CREATE DATABASE `{$db}` DEFAULT CHARACTER SET `utf8`";
    $mysqli->query($sql);
    if($mysqli->errno){
         exit("create database {$db} error: ".$mysqli->error."\n");
    }
    air\mysql\keeper::release($mysqli, DB_CONF, W);
}

function create_table(){
    $db = DB_NAME;
    $table = TABLE_NAME;
    $mysqli = air\mysql\keeper::factory(DB_CONF, W);
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
    air\mysql\keeper::release($mysqli, DB_CONF, W);
}





