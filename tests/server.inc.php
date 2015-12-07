<?php
/**
 *
 * @author wukezhan<wukezhan@gmail.com>
 * 2015-10-28 12:30
 *
 */

class local_server
{
    public static $host_port;
    protected $_server;
    public function listen($host, $port)
    {
        self::$host_port = $host.':'.$port;
        // partly borrowed from https://github.com/php/php-src/blob/master/ext/curl/tests/server.inc
        // but make it more simple
        $php_executable = getenv('TEST_PHP_EXECUTABLE');
        $php_executable = $php_executable ?: getenv('_');
        $doc_root = __DIR__ . DIRECTORY_SEPARATOR . 'server';
        $descriptorspec = array(
            0 => STDIN,
            1 => STDOUT,
            2 => STDERR,
        );

        echo "start local server\n";
        if (substr(PHP_OS, 0, 3) == 'WIN') {
            $cmd = "{$php_executable} -t {$doc_root} -n -S ".self::$host_port;
            $this->_server = proc_open(addslashes($cmd), $descriptorspec, $pipes, $doc_root, NULL, array("bypass_shell" => true, "suppress_errors" => true));
        } else {
            $cmd = "exec {$php_executable} -t {$doc_root} -n -S ".self::$host_port;
            $cmd .= " 2>/dev/null";
            $this->_server = proc_open($cmd, $descriptorspec, $pipes, $doc_root);
        }
        if(is_resource($this->_server)){
            $fp = 0;
            $i = 0;
            while (($i++ < 30) && !($fp = @fsockopen($host, $port))) {
                usleep(10000);
            }
            if ($fp) {
                fclose($fp);
            }
            echo "local server started\n";
        }else{
            echo "start local server failed\n";
            exit;
        }
    }

    public function __destruct()
    {
        $status = proc_terminate($this->_server);
        if($status) {
            echo "local server stopped\n";
        }else{
            echo "stop local server failed";
        }
    }
}

$server = new local_server();
$server->listen('127.0.0.1', 12345);

