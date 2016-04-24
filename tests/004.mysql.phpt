--TEST--
Check for air\mysql
--SKIPIF--
<?php if (!extension_loaded("air")) print "skip"; ?>
<?php include 'mysql.inc.php'; ?>
--FILE--
<?php

include 'mysql.inc.php';

class db_air extends air\mysql\table
{
  protected $_config = DB_CONF;
  protected $_db = DB_NAME;
}

class air_test extends db_air
{
  protected $_table = TABLE_NAME;

  public function add_rows()
  {
    $n = 10;
    $rows = 0;
    while($n--){
      $b = $this->add([ 'title' => 'hello '.$n ]);
      if($b->affected_rows()){
        $rows += $b->affected_rows();
      }
    }
    echo 'add '.$rows." rows\n";
  }

  public function set_rows()
  {
    $now = time();
    $n = 10;
    $rows = 0;
    while($n--){
      $b = $this->set([ 'create_time' => $now ])->by_key($n+1);
      if($b->affected_rows()){
        $rows += $b->affected_rows();
      }
    }
    echo "set {$rows} rows create_time\n";


    $b2 = $this->async()->query("UPDATE {$this->_db}.{$this->_table} SET update_time='{$now}'");
    echo "set {$b2->affected_rows()} rows update_time\n";
  }

  public function get_rows()
  {
    //mode
    $b1 = $this->async()->get('*')->where('id>:id', ['id'=>0])->offset(0)->size(5);
    $b2 = $this->async()->get('*')->where('id>:id', ['id'=>0])->offset(5)->size(5);
    $b3 = $this->async()->query("SELECT * FROM {$this->_db}.{$this->_table}");
    $b4 = $this->async()->get('*')->where('id>9', []);

    var_dump($b1->data());
    var_dump($b2->data());
    var_dump($b3->data());
    var_dump($b4->data());
  }

  public function del_rows($async=0)
  {
    $b = $this->del()->where('id>0', []);
    if($async){
      $b->async();
    }
    echo "del {$b->affected_rows()} rows\n";
  }
}

create_database();
create_table();

$test = new air_test();
$test->add_rows();
$test->set_rows();
$test->get_rows();

$b = new air\mysql(DB_CONF);
$b->table('air.test')->async()->get('*')->where('id>:id', ['id'=>9]);
var_dump($b->data());

$test->del_rows();
echo "all ok!\n";

?>

--EXPECTF--
add 10 rows
set 10 rows create_time
set 10 rows update_time
array(5) {
  [0]=>
  array(4) {
    ["id"]=>
    string(1) "1"
    ["title"]=>
    string(7) "hello 9"
    ["create_time"]=>
    string(10) "%d"
    ["update_time"]=>
    string(10) "%d"
  }
  [1]=>
  array(4) {
    ["id"]=>
    string(1) "2"
    ["title"]=>
    string(7) "hello 8"
    ["create_time"]=>
    string(10) "%d"
    ["update_time"]=>
    string(10) "%d"
  }
  [2]=>
  array(4) {
    ["id"]=>
    string(1) "3"
    ["title"]=>
    string(7) "hello 7"
    ["create_time"]=>
    string(10) "%d"
    ["update_time"]=>
    string(10) "%d"
  }
  [3]=>
  array(4) {
    ["id"]=>
    string(1) "4"
    ["title"]=>
    string(7) "hello 6"
    ["create_time"]=>
    string(10) "%d"
    ["update_time"]=>
    string(10) "%d"
  }
  [4]=>
  array(4) {
    ["id"]=>
    string(1) "5"
    ["title"]=>
    string(7) "hello 5"
    ["create_time"]=>
    string(10) "%d"
    ["update_time"]=>
    string(10) "%d"
  }
}
array(5) {
  [0]=>
  array(4) {
    ["id"]=>
    string(1) "6"
    ["title"]=>
    string(7) "hello 4"
    ["create_time"]=>
    string(10) "%d"
    ["update_time"]=>
    string(10) "%d"
  }
  [1]=>
  array(4) {
    ["id"]=>
    string(1) "7"
    ["title"]=>
    string(7) "hello 3"
    ["create_time"]=>
    string(10) "%d"
    ["update_time"]=>
    string(10) "%d"
  }
  [2]=>
  array(4) {
    ["id"]=>
    string(1) "8"
    ["title"]=>
    string(7) "hello 2"
    ["create_time"]=>
    string(10) "%d"
    ["update_time"]=>
    string(10) "%d"
  }
  [3]=>
  array(4) {
    ["id"]=>
    string(1) "9"
    ["title"]=>
    string(7) "hello 1"
    ["create_time"]=>
    string(10) "%d"
    ["update_time"]=>
    string(10) "%d"
  }
  [4]=>
  array(4) {
    ["id"]=>
    string(2) "10"
    ["title"]=>
    string(7) "hello 0"
    ["create_time"]=>
    string(10) "%d"
    ["update_time"]=>
    string(10) "%d"
  }
}
array(10) {
  [0]=>
  array(4) {
    ["id"]=>
    string(1) "1"
    ["title"]=>
    string(7) "hello 9"
    ["create_time"]=>
    string(10) "%d"
    ["update_time"]=>
    string(10) "%d"
  }
  [1]=>
  array(4) {
    ["id"]=>
    string(1) "2"
    ["title"]=>
    string(7) "hello 8"
    ["create_time"]=>
    string(10) "%d"
    ["update_time"]=>
    string(10) "%d"
  }
  [2]=>
  array(4) {
    ["id"]=>
    string(1) "3"
    ["title"]=>
    string(7) "hello 7"
    ["create_time"]=>
    string(10) "%d"
    ["update_time"]=>
    string(10) "%d"
  }
  [3]=>
  array(4) {
    ["id"]=>
    string(1) "4"
    ["title"]=>
    string(7) "hello 6"
    ["create_time"]=>
    string(10) "%d"
    ["update_time"]=>
    string(10) "%d"
  }
  [4]=>
  array(4) {
    ["id"]=>
    string(1) "5"
    ["title"]=>
    string(7) "hello 5"
    ["create_time"]=>
    string(10) "%d"
    ["update_time"]=>
    string(10) "%d"
  }
  [5]=>
  array(4) {
    ["id"]=>
    string(1) "6"
    ["title"]=>
    string(7) "hello 4"
    ["create_time"]=>
    string(10) "%d"
    ["update_time"]=>
    string(10) "%d"
  }
  [6]=>
  array(4) {
    ["id"]=>
    string(1) "7"
    ["title"]=>
    string(7) "hello 3"
    ["create_time"]=>
    string(10) "%d"
    ["update_time"]=>
    string(10) "%d"
  }
  [7]=>
  array(4) {
    ["id"]=>
    string(1) "8"
    ["title"]=>
    string(7) "hello 2"
    ["create_time"]=>
    string(10) "%d"
    ["update_time"]=>
    string(10) "%d"
  }
  [8]=>
  array(4) {
    ["id"]=>
    string(1) "9"
    ["title"]=>
    string(7) "hello 1"
    ["create_time"]=>
    string(10) "%d"
    ["update_time"]=>
    string(10) "%d"
  }
  [9]=>
  array(4) {
    ["id"]=>
    string(2) "10"
    ["title"]=>
    string(7) "hello 0"
    ["create_time"]=>
    string(10) "%d"
    ["update_time"]=>
    string(10) "%d"
  }
}
array(1) {
  [0]=>
  array(4) {
    ["id"]=>
    string(2) "10"
    ["title"]=>
    string(7) "hello 0"
    ["create_time"]=>
    string(10) "%d"
    ["update_time"]=>
    string(10) "%d"
  }
}
array(1) {
  [0]=>
  array(4) {
    ["id"]=>
    string(2) "10"
    ["title"]=>
    string(7) "hello 0"
    ["create_time"]=>
    string(10) "%d"
    ["update_time"]=>
    string(10) "%d"
  }
}
del 10 rows
all ok!
