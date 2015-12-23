--TEST--
Check for air\curl
--SKIPIF--
<?php if (!extension_loaded("air")) print "skip"; ?>
--FILE--
<?php
include 'server.inc.php';

function header_func($ch, $header_line){
    echo $header_line;
    return strlen($header_line);
}

function data2array($ch, $data){
    return json_decode($data, 1);
}

function sync(){
    $c1 = new air\curl();
    $c1->async();
    $c1->setopt(CURLOPT_HEADERFUNCTION, 'header_func');
    $c1->setopt(CURLOPT_RETURNTRANSFER, 1);
    $c1->get(local_server::$host_port . '/get.php?hello=world-1');

    $c2 = new air\curl();
    $c2->setopt(CURLOPT_HEADERFUNCTION, 'header_func')
        ->setopt(CURLOPT_RETURNTRANSFER, 1)
        ->get(local_server::$host_port . '/json.php?hello=world-2')->on_success('data2array');
    echo $c1->data();
    var_dump($c2->data());

    $c3 = new air\curl();
    $c3->setopt(CURLOPT_HEADERFUNCTION, 'header_func')
        ->setopt(CURLOPT_RETURNTRANSFER, 1)
        ->post(local_server::$host_port . '/post.php', ['hello' => 'world-3']);
    echo $c3->data();

    $c4 = new air\curl();
    $c4->setopt(CURLOPT_HEADERFUNCTION, 'header_func')
        ->setopt(CURLOPT_RETURNTRANSFER, 1)
        ->post(local_server::$host_port . '/json.php', ['hello' => 'world-4'])
        ->on_success('data2array');
    var_dump($c4->data());
}

function async(){
    $c1 = new air\curl();
    $c1->async();
    $c1->setopt(CURLOPT_HEADERFUNCTION, 'header_func');
    $c1->setopt(CURLOPT_RETURNTRANSFER, 1);
    $c1->get(local_server::$host_port . '/get.php?hello=world-1');

    $c2 = new air\curl();
    $c2->async()
        ->setopt(CURLOPT_HEADERFUNCTION, 'header_func')
        ->setopt(CURLOPT_RETURNTRANSFER, 1)
        ->get(local_server::$host_port . '/json.php?hello=world-2')->on_success('data2array')
    ;
    echo $c1->data();
    var_dump($c2->data());

    $c3 = new air\curl();
    $c3->async()
        ->setopt(CURLOPT_HEADERFUNCTION, 'header_func')
        ->setopt(CURLOPT_RETURNTRANSFER, 1)
        ->post(local_server::$host_port . '/post.php', ['hello' => 'world-3'])
    ;
    echo $c3->data();

    $c4 = new air\curl();
    $c4->async()
        ->setopt(CURLOPT_HEADERFUNCTION, 'header_func')
        ->setopt(CURLOPT_RETURNTRANSFER, 1)
        ->post(local_server::$host_port . '/json.php', ['hello' => 'world-4'])
        ->on_success('data2array')
    ;
    var_dump($c4->data());
}

sync();
async();

?>

--EXPECTF--
start local server
local server started
HTTP/1.1 200 OK
Host: 127.0.0.1:12345
Connection: close
X-Powered-By: PHP/%s
Content-type: %s

array(1) {
  ["hello"]=>
  string(7) "world-1"
}
HTTP/1.1 200 OK
Host: 127.0.0.1:12345
Connection: close
X-Powered-By: PHP/%s
Content-type: %s

array(1) {
  ["hello"]=>
  string(7) "world-2"
}
HTTP/1.1 200 OK
Host: 127.0.0.1:12345
Connection: close
X-Powered-By: PHP/%s
Content-type: %s

array(1) {
  ["hello"]=>
  string(7) "world-3"
}
HTTP/1.1 200 OK
Host: 127.0.0.1:12345
Connection: close
X-Powered-By: PHP/%s
Content-type: %s

array(1) {
  ["hello"]=>
  string(7) "world-4"
}
HTTP/1.1 200 OK
Host: 127.0.0.1:12345
Connection: close
X-Powered-By: PHP/%s
Content-type: %s

HTTP/1.1 200 OK
Host: 127.0.0.1:12345
Connection: close
X-Powered-By: PHP/%s
Content-type: %s

array(1) {
  ["hello"]=>
  string(7) "world-1"
}
array(1) {
  ["hello"]=>
  string(7) "world-2"
}
HTTP/1.1 200 OK
Host: 127.0.0.1:12345
Connection: close
X-Powered-By: PHP/%s
Content-type: %s

array(1) {
  ["hello"]=>
  string(7) "world-3"
}
HTTP/1.1 200 OK
Host: 127.0.0.1:12345
Connection: close
X-Powered-By: PHP/%s
Content-type: %s

array(1) {
  ["hello"]=>
  string(7) "world-4"
}

local server stopped
