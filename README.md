# tmock

## Requirement
- PHP 7.0 +

### Install
```
$/path/to/phpize
$./configure --with-php-config=/path/to/php-config
$make && make install
```

## Methods

### tmock_start
```
   tmock_start(array $array) : bool
```

#### example:
```php
<?php
    tmock_start(['test' => function (){return 2;}]);
    function test(){
        return 3;
    }
    var_dump(test());
```
#### output:
```
int(2)
```

### tmock_get
```
   tmock_get() : array
```

#### example:
```php
<?php
    tmock_start(['test' => function (){return 2;}]);
    var_dump(tmock_get());
```
#### output:
```
array(1) {
  ["test"]=>
  object(Closure)#2 (0) {
  }
}
```

### tmock_end
```
   tmock_end() : bool
```

#### example:
```php
<?php
tmock_start(['test' => function (){return 2;}]);
function test(){
    return 3;
}
var_dump(test());
tmock_end();
var_dump(test());
```
#### output:
```
int(2)
int(3)
```

### tmock_debug_start
```
   tmock_debug_start() : bool
```

#### example:
```php
<?php
tmock_debug_start();
tmock_start(['test' => function (){return 2;}]);
function test(){
    return 3;
}
var_dump(test());
```
#### output:
```
fname test, type 2
fname test match
fname test is_callable
fname test fcall init SUCCESS
fname test callable SUCCESS
fname var_dump, type 1
int(2)
```

### tmock_debug_end
```
   tmock_debug_end() : bool
```

#### example:
```php
<?php
tmock_debug_start();
tmock_start(['test' => function (){return 2;}]);
function test(){
    return 3;
}
var_dump(test());
tmock_debug_end();
var_dump(test());
```
#### output:
```
fname test, type 2
fname test match
fname test is_callable
fname test fcall init SUCCESS
fname test callable SUCCESS
fname var_dump, type 1
int(2)
fname tmock_debug_end, type 1
int(2)
```


## Mock

### function

```php
<?php
var_dump(tmock_start(['MyProject\test1'=>function (){return 1;}]));
function test1(){
    return 2;
}
var_dump(test1());
var_dump(tmock_end());
var_dump(test1());
```
#### output:
```
bool(true)
int(1)
bool(true)
int(2)
```

### system function

```php
<?php
var_dump(tmock_start(['explode'=>function (){return 2;}]));
var_dump(explode(',','1,2'));
var_dump(tmock_end());
var_dump(explode(',','1,2'));
```
#### output:
```
bool(true)
int(2)
bool(true)
array(2) {
  [0]=>
  string(1) "1"
  [1]=>
  string(1) "2"
}
```

### object method

```php
<?php
var_dump(tmock_start(['obj1->test1'=>function (){return 2;}]));
class obj1 {
    function test1() {
        return 3;
    }
}
var_dump((new obj1)->test1());
var_dump(tmock_end());
var_dump((new obj1)->test1());
```
#### output:
```
bool(true)
int(2)
bool(true)
int(3)
```

### object static method

```php
<?php
var_dump(tmock_start(['obj2::test1'=>function (){return 2;}]));
class obj2 {
    static function test1() {
        return 3;
    }
}
var_dump(obj2::test1());
var_dump(tmock_end());
var_dump(obj2::test1());
```
#### output:
```
bool(true)
int(2)
bool(true)
int(3)
```

### namespace

```php
<?php
namespace MyProject;
var_dump(tmock_start(['MyProject\test1'=>function (){return 1;}]));
function test1(){
    return 2;
}
var_dump(test1());
var_dump(tmock_end());
var_dump(test1());
```
#### output:
```
bool(true)
int(1)
bool(true)
int(2)
```














































