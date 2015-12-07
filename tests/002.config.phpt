--TEST--
Check for air\config
--SKIPIF--
<?php if (!extension_loaded("air")) print "skip"; ?>
--FILE--
<?php
air\config::set([
    'hello' => 'world',
]);
var_dump(air\config::get());
var_dump(air\config::get('hello'));

var_dump(air\config::get('ufo'));
var_dump(air\config::get('ufo-with-default-value', 'ufo-with-default-value'));

var_dump(air\config::get('ufo.path'));
var_dump(air\config::get('ufo.path.with.default.value', 'ufo.path.with.default.value'));

?>

--EXPECTF--
array(2) {
  ["app"]=>
  array(4) {
    ["path"]=>
    string(3) "app"
    ["exec"]=>
    array(1) {
      ["path"]=>
      string(4) "exec"
    }
    ["site"]=>
    array(1) {
      ["path"]=>
      string(4) "site"
    }
    ["view"]=>
    array(3) {
      ["engine"]=>
      string(8) "air\view"
      ["path"]=>
      string(4) "view"
      ["type"]=>
      string(4) ".php"
    }
  }
  ["hello"]=>
  string(5) "world"
}
string(5) "world"
NULL
string(22) "ufo-with-default-value"
NULL
string(27) "ufo.path.with.default.value"

