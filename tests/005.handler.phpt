--TEST--
Check for air\handler
--SKIPIF--
<?php if (!extension_loaded("air")) print "skip"; ?>
--FILE--
<?php
air\handler::on_error(function(){
  $args = func_get_args();
  var_dump($args[1]);
});

air\handler::on_exception(function(Exception $e){
  var_dump($e->getMessage());
});

air\handler::on_shutdown(function(){
  var_dump("shutdown");
});

trigger_error('user error', E_USER_ERROR);
throw new Exception("exception", 2);

?>

--EXPECTF--
string(10) "user error"
string(9) "exception"
string(8) "shutdown"