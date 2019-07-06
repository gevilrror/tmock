--TEST--
Check for 01
--SKIPIF--
--FILE--
<?php

namespace MyProject;




echo "---test3.function------------------------\n";
var_dump(tmock_start(['MyProject\test1'=>function (){return 1;}]));
function test1(){
	return 2;
}
var_dump(test1());
var_dump(tmock_end());
var_dump(test1());

echo "---test4.system_function------------------------\n";
var_dump(tmock_start(['explode'=>function (){return 2;}]));
var_dump(explode(',','1,2'));
var_dump(tmock_end());
var_dump(explode(',','1,2'));

echo "---test5.member_function------------------------\n";
var_dump(tmock_start(['MyProject\obj1->test1'=>function (){return 2;}]));
class obj1 {
	function test1() {
		return 3;
	}
}
var_dump((new obj1)->test1());
var_dump(tmock_end());
var_dump((new obj1)->test1());

echo "---test6.member_static_function------------------------\n";
var_dump(tmock_start(['MyProject\obj2::test1'=>function (){return 2;}]));
class obj2 {
	static function test1() {
		return 3;
	}
}
var_dump(obj2::test1());
var_dump(tmock_end());
var_dump(obj2::test1());

echo "---test7.member_static_args_function------------------------\n";
var_dump(tmock_start(['MyProject\obj3::test1'=>function ($funcName, $args, $obj){
	var_dump($funcName, $args, $obj);
	$args[0] = 3;
	return 2;
}]));
class obj3 {
	static function test1(&$a, $b, $c = 2) {
		return 3;
	}
}
var_dump(obj3::test1($c, 2));
var_dump($c);
var_dump(obj3::test1($d, 2));
var_dump($d);
var_dump(tmock_end());
var_dump(obj3::test1($d, 2));
var_dump($d);

echo "---test8.member_args_function------------------------\n";
// tmock_debug_start();
var_dump(tmock_start(['MyProject\obj5->test1'=>function ($funcName, $args, $obj){
	var_dump($funcName, $args, $obj);
	return 2;
}]));
class obj5 {
	var $a = 3;
	function test1() {
		return 3;
	}
}
var_dump((new obj5)->test1());
var_dump(tmock_end());
var_dump((new obj5)->test1());






?>
--EXPECT--
---test3.function------------------------
bool(true)
int(1)
bool(true)
int(2)
---test4.system_function------------------------
bool(true)
int(2)
bool(true)
array(2) {
  [0]=>
  string(1) "1"
  [1]=>
  string(1) "2"
}
---test5.member_function------------------------
bool(true)
int(2)
bool(true)
int(3)
---test6.member_static_function------------------------
bool(true)
int(2)
bool(true)
int(3)
---test7.member_static_args_function------------------------
bool(true)
string(21) "MyProject\obj3::test1"
array(2) {
  [0]=>
  &NULL
  [1]=>
  int(2)
}
NULL
int(2)
int(3)
string(21) "MyProject\obj3::test1"
array(2) {
  [0]=>
  &NULL
  [1]=>
  int(2)
}
NULL
int(2)
int(3)
bool(true)
int(3)
int(3)
---test8.member_args_function------------------------
bool(true)
string(21) "MyProject\obj5->test1"
array(0) {
}
object(MyProject\obj5)#3 (1) {
  ["a"]=>
  int(3)
}
int(2)
bool(true)
int(3)
