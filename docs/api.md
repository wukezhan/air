## air\app
#### air\app::__construct
---
```php
public air\app::__construct( [ int $type = 1 ] )
```
##### parameters
* **$type**  `int` app type, can be \AIR_SITE or \AIR_EXEC, default is \AIR_SITE

##### return



#### air\app::run
---
 Run the app
```php
public air\app::run( void )
```
##### parameters
* none

##### return
* none


#### air\app::set_router
---
 Set the app router
```php
public air\app::set_router( router $router ) : $this
```
##### parameters
* **$router**  `router`

##### return
* **$this**


#### air\app::dispatch
---
 Dispatch the request
```php
public air\app::dispatch( void ) : $this
```
##### parameters
* none

##### return
* **$this**




## air\controller
#### air\controller::__construct
---
 The construct function
```php
public air\controller::__construct( void )
```
##### parameters
* none

##### return



#### air\controller::init
---
 The init function
```php
public air\controller::init( void ) : $this;
```
##### parameters
* none

##### return
* **$this;**


#### air\controller::before_action
---
 The function execute before the action
```php
public air\controller::before_action( void ) : $this
```
##### parameters
* none

##### return
* **$this**


#### air\controller::assign
---
 Assign the view data

 if $o1 is an array, then the $o2 will be ignored

 if $o1 is a string or int, $o2 will be used as value
```php
public air\controller::assign( string|array $o1 [, mixed $o2 = NULL ] ) : $this
```
##### parameters
* **$o1**  `string|array`
* **$o2**  `mixed`

##### return
* **$this**


#### air\controller::set_view
---
 Set the view path
```php
public air\controller::set_view( $view_path ) : $this
```
##### parameters
* **$view_path**

##### return
* **$this**


#### air\controller::init_view
---
 Initialize the view engine
```php
public air\controller::init_view( void ) : $this
```
##### parameters
* none

##### return
* **$this**


#### air\controller::render_view
---
 Call the view's render_view function
```php
public air\controller::render_view( [ string $view_path = '' [, bool $return = false ] ] ) : string
```
##### parameters
* **$view_path**  `string`
* **$return**  `bool`

##### return
* **string**


#### air\controller::after_action
---
 The function called after action
```php
public air\controller::after_action( void ) : $this
```
##### parameters
* none

##### return
* **$this**


#### air\controller::__destruct
---
```php
public air\controller::__destruct( void )
```
##### parameters
* none

##### return
* none




## air\router
#### air\router::__construct
---
```php
public air\router::__construct( void )
```
##### parameters
* none

##### return



#### air\router::set_url
---
 Set the url waiting for routing
```php
public air\router::set_url( string $url ) : $this
```
##### parameters
* **$url**  `string`

##### return
* **$this**


#### air\router::set_rules
---
 Set the route rules
```php
public air\router::set_rules( array $rules ) : $this
```
##### parameters
* **$rules**  `array` route rules

##### return
* **$this**


#### air\router::route
---
 Do routing

     ''

 ]
```php
public air\router::route( void ) : array
```
##### parameters
* none

##### return
* **array**


#### air\router::reset
---
 Reset the pointer of the route rules
```php
public air\router::reset( void ) : $this
```
##### parameters
* none

##### return
* **$this**




## air\config
#### air\config::get
---
 Retrieves config data by key
```php
public static air\config::get( [ string|int|null $key = NULL [, mixed|null $default = NULL ] ] )
```
##### parameters
* **$key**  `string|int|null`
* **$default**  `mixed|null`

##### return



#### air\config::path_get
---
 Retrieves config data by path
```php
public static air\config::path_get( [ string $path = NULL [, mixed $default = NULL ] ] )
```
##### parameters
* **$path**  `string`
* **$default**  `mixed`

##### return



#### air\config::set
---
 Set the config data, config data can only be set once
```php
public static air\config::set( array $config ) : bool
```
##### parameters
* **$config**  `array` The config data to be set

##### return
* **bool**




## air\curl
#### air\curl::__construct
---
```php
public air\curl::__construct( void )
```
##### parameters
* none

##### return
* none


#### air\curl::offsetExists
---
```php
public air\curl::offsetExists( $key )
```
##### parameters
* none

##### return
* none


#### air\curl::offsetGet
---
 Retrieves the given key
```php
public air\curl::offsetGet( $key ) : bool
```
##### parameters
* **$key**

##### return
* **bool**


#### air\curl::offsetSet
---
```php
public air\curl::offsetSet( $key, $value )
```
##### parameters
* none

##### return
* none


#### air\curl::offsetUnset
---
 Unset an offset by key
```php
public air\curl::offsetUnset( $key )
```
##### parameters
* **$key**

##### return
* none


#### air\curl::setopt
---
 Set an option for a air\curl transfer
```php
public air\curl::setopt( int $ok, mixed $ov ) : $this
```
##### parameters
* **$ok**  `int` The CURLOPT_XXX option to set.
* **$ov**  `mixed` The value to be set on option.

##### return
* **$this**


#### air\curl::async
---
 Enable the async request mode
```php
public air\curl::async( void ) : $this
```
##### parameters
* none

##### return
* **$this**


#### air\curl::get
---
 Call to do a regular HTTP GET request to the $url with param $params
```php
public air\curl::get( string $url [, array $params = array () ] ) : $this
```
##### parameters
* **$url**  `string` The request url
* **$params**  `array` The request params

##### return
* **$this**


#### air\curl::post
---
 Call to do a regular HTTP POST request to the $url with param $params
```php
public air\curl::post( string $url [, array $data = array () ] ) : $this
```
##### parameters
* **$url**  `string` The request url
* **$data**  `array` The request params

##### return
* **$this**


#### air\curl::exec
---
 Explicit call the curl_exec or curl_multi_exec function
```php
public air\curl::exec( void ) : $this
```
##### parameters
* none

##### return
* **$this**


#### air\curl::data
---
 Return the request data
```php
public air\curl::data( void )
```
##### parameters
* none

##### return



#### air\curl::reset
---
 Reset the curl handle, which actually calls the curl_reset function
```php
public air\curl::reset( void ) : $this
```
##### parameters
* none

##### return
* **$this**


#### air\curl::errno
---
 Return the last error number for the current session
```php
public air\curl::errno( void ) : int
```
##### parameters
* none

##### return
* **int**


#### air\curl::error
---
 Return a string containing the last error for the current session
```php
public air\curl::error( void ) : string
```
##### parameters
* none

##### return
* **string**


#### air\curl::trigger
---
```php
public air\curl::trigger( $event, $params )
```
##### parameters
* none

##### return
* none


#### air\curl::on_success
---
```php
public air\curl::on_success( $callback )
```
##### parameters
* none

##### return
* none


#### air\curl::on_error
---
```php
public air\curl::on_error( $callback )
```
##### parameters
* none

##### return
* none


#### air\curl::on_success_default
---
```php
public static air\curl::on_success_default( $ch, $result )
```
##### parameters
* none

##### return
* none


#### air\curl::on_error_default
---
```php
public static air\curl::on_error_default( $ch )
```
##### parameters
* none

##### return
* none


#### air\curl::__destruct
---
```php
public air\curl::__destruct( void )
```
##### parameters
* none

##### return
* none




## air\exception
#### Exception::__clone
---
```php
final private Exception::__clone( void )
```
##### parameters
* none

##### return
* none


#### Exception::__construct
---
```php
public Exception::__construct( [ $message [, $code [, $previous ] ] ] )
```
##### parameters
* none

##### return
* none


#### Exception::getMessage
---
```php
final public Exception::getMessage( void )
```
##### parameters
* none

##### return
* none


#### Exception::getCode
---
```php
final public Exception::getCode( void )
```
##### parameters
* none

##### return
* none


#### Exception::getFile
---
```php
final public Exception::getFile( void )
```
##### parameters
* none

##### return
* none


#### Exception::getLine
---
```php
final public Exception::getLine( void )
```
##### parameters
* none

##### return
* none


#### Exception::getTrace
---
```php
final public Exception::getTrace( void )
```
##### parameters
* none

##### return
* none


#### Exception::getPrevious
---
```php
final public Exception::getPrevious( void )
```
##### parameters
* none

##### return
* none


#### Exception::getTraceAsString
---
```php
final public Exception::getTraceAsString( void )
```
##### parameters
* none

##### return
* none


#### Exception::__toString
---
```php
public Exception::__toString( void )
```
##### parameters
* none

##### return
* none




## air\handler
#### air\handler::on_error
---
```php
public static air\handler::on_error( $handler )
```
##### parameters
* none

##### return
* none


#### air\handler::on_exception
---
```php
public static air\handler::on_exception( $handler )
```
##### parameters
* none

##### return
* none


#### air\handler::on_shutdown
---
```php
public static air\handler::on_shutdown( $handler )
```
##### parameters
* none

##### return
* none




## air\loader
#### air\loader::autoload
---
```php
public static air\loader::autoload( void )
```
##### parameters
* none

##### return
* none


#### air\loader::load
---
```php
public static air\loader::load( $class )
```
##### parameters
* none

##### return
* none




## air\view
#### air\view::__construct
---
```php
public air\view::__construct( void )
```
##### parameters
* none

##### return
* none


#### air\view::assign
---
 Assign a key with value to the view data
```php
public air\view::assign( string|int $key, mixed $value ) : $this
```
##### parameters
* **$key**  `string|int`
* **$value**  `mixed`

##### return
* **$this**


#### air\view::render
---
 Render a view by the view path
```php
public air\view::render( $view_path [, bool $return = false ] ) : string
```
##### parameters
* **$view_path**
* **$return**  `bool`

##### return
* **string**


#### air\view::set_config
---
 Set the view config, it's not used at present
```php
public air\view::set_config( $config )
```
##### parameters
* **$config**

##### return
* none




## air\async\service
#### air\async\service::__construct
---
```php
public air\async\service::__construct( waiter $waiter, $request )
```
##### parameters
* **$waiter**  `waiter`
* **$request**

##### return
* none


#### air\async\service::call
---
 Retrieves the service data
```php
public air\async\service::call( void ) : array
```
##### parameters
* none

##### return
* **array**




## air\async\waiter
#### air\async\waiter::__construct
---
 The __construct method

 all the services of the same waiter will wait for the same polling procedure
```php
public air\async\waiter::__construct( void )
```
##### parameters
* none

##### return
* none


#### air\async\waiter::serve
---
 Serve a async request, and return an async service
```php
public air\async\waiter::serve( $request )
```
##### parameters
* **$request**

##### return



#### air\async\waiter::_response
---
 The real response function which is not visible to the service
```php
protected air\async\waiter::_response( void )
```
##### parameters
* none

##### return
* none


#### air\async\waiter::response
---
 Retrieves the request data of an async service by a service id
```php
public air\async\waiter::response( $service_id )
```
##### parameters
* **$service_id**

##### return





## air\curl\waiter
#### air\curl\waiter::_response
---
 The real response function which is not visible to the service
```php
protected air\curl\waiter::_response( void )
```
##### parameters
* none

##### return
* none


#### air\curl\waiter::__destruct
---
```php
public air\curl\waiter::__destruct( void )
```
##### parameters
* none

##### return
* none


#### air\async\waiter::__construct
---
 The __construct method

 all the services of the same waiter will wait for the same polling procedure
```php
public air\async\waiter::__construct( void )
```
##### parameters
* none

##### return
* none


#### air\async\waiter::serve
---
 Serve a async request, and return an async service
```php
public air\async\waiter::serve( $request )
```
##### parameters
* **$request**

##### return



#### air\async\waiter::response
---
 Retrieves the request data of an async service by a service id
```php
public air\async\waiter::response( $service_id )
```
##### parameters
* **$service_id**

##### return





## air\mysql\keeper
#### air\mysql\keeper::__construct
---
```php
protected air\mysql\keeper::__construct( void )
```
##### parameters
* none

##### return
* none


#### air\mysql\keeper::__destruct
---
```php
public air\mysql\keeper::__destruct( void )
```
##### parameters
* none

##### return
* none


#### air\mysql\keeper::factory
---
```php
public static air\mysql\keeper::factory( string $config, int $mode )
```
##### parameters
* **$config**  `string`
* **$mode**  `int`

##### return



#### air\mysql\keeper::release
---
```php
public static air\mysql\keeper::release( \mysqli $mysql, string $config, int $mode )
```
##### parameters
* **$mysql**  `\mysqli`
* **$config**  `string`
* **$mode**  `int`

##### return
* none




## air\mysql\table
#### air\mysql\table::__construct
---
```php
public air\mysql\table::__construct( void )
```
##### parameters
* none

##### return
* none


#### air\mysql\table::async
---
```php
public air\mysql\table::async( void )
```
##### parameters
* none

##### return
* none


#### air\mysql\table::add
---
```php
public air\mysql\table::add( $data )
```
##### parameters
* none

##### return
* none


#### air\mysql\table::set
---
```php
public air\mysql\table::set( $data )
```
##### parameters
* none

##### return
* none


#### air\mysql\table::get
---
```php
public air\mysql\table::get( $fields )
```
##### parameters
* none

##### return
* none


#### air\mysql\table::del
---
```php
public air\mysql\table::del( void )
```
##### parameters
* none

##### return
* none




## air\mysql\waiter
#### air\mysql\waiter::_response
---
```php
protected air\mysql\waiter::_response( void )
```
##### parameters
* none

##### return
* none


#### air\async\waiter::__construct
---
 The __construct method

 all the services of the same waiter will wait for the same polling procedure
```php
public air\async\waiter::__construct( void )
```
##### parameters
* none

##### return
* none


#### air\async\waiter::serve
---
 Serve a async request, and return an async service
```php
public air\async\waiter::serve( $request )
```
##### parameters
* **$request**

##### return



#### air\async\waiter::response
---
 Retrieves the request data of an async service by a service id
```php
public air\async\waiter::response( $service_id )
```
##### parameters
* **$service_id**

##### return





## air\mysql
#### air\mysql::__construct
---
```php
public air\mysql::__construct( $config [, $table = NULL ] )
```
##### parameters
* none

##### return
* none


#### air\mysql::config
---
```php
public air\mysql::config( $config )
```
##### parameters
* none

##### return
* none


#### air\mysql::offsetExists
---
```php
public air\mysql::offsetExists( $key )
```
##### parameters
* none

##### return
* none


#### air\mysql::offsetGet
---
```php
public air\mysql::offsetGet( $key )
```
##### parameters
* none

##### return
* none


#### air\mysql::offsetSet
---
```php
public air\mysql::offsetSet( $key, $value )
```
##### parameters
* none

##### return
* none


#### air\mysql::offsetUnset
---
```php
public air\mysql::offsetUnset( $key )
```
##### parameters
* none

##### return
* none


#### air\mysql::count
---
```php
public air\mysql::count( void )
```
##### parameters
* none

##### return
* none


#### air\mysql::rewind
---
```php
public air\mysql::rewind( void )
```
##### parameters
* none

##### return
* none


#### air\mysql::current
---
```php
public air\mysql::current( void )
```
##### parameters
* none

##### return
* none


#### air\mysql::key
---
```php
public air\mysql::key( void )
```
##### parameters
* none

##### return
* none


#### air\mysql::next
---
```php
public air\mysql::next( void )
```
##### parameters
* none

##### return
* none


#### air\mysql::valid
---
```php
public air\mysql::valid( void )
```
##### parameters
* none

##### return
* none


#### air\mysql::serialize
---
```php
public air\mysql::serialize( void )
```
##### parameters
* none

##### return
* none


#### air\mysql::unserialize
---
```php
public air\mysql::unserialize( $data )
```
##### parameters
* none

##### return
* none


#### air\mysql::data
---
```php
public air\mysql::data( void ) : array
```
##### parameters
* none

##### return
* **array**


#### air\mysql::affected_rows
---
```php
public air\mysql::affected_rows( void ) : int
```
##### parameters
* none

##### return
* **int**


#### air\mysql::num_rows
---
```php
public air\mysql::num_rows( void ) : int
```
##### parameters
* none

##### return
* **int**


#### air\mysql::insert_id
---
```php
public air\mysql::insert_id( void ) : int
```
##### parameters
* none

##### return
* **int**


#### air\mysql::errno
---
```php
public air\mysql::errno( void ) : int
```
##### parameters
* none

##### return
* **int**


#### air\mysql::error
---
```php
public air\mysql::error( void ) : string
```
##### parameters
* none

##### return
* **string**


#### air\mysql::table
---
```php
public air\mysql::table( string $table ) : $this
```
##### parameters
* **$table**  `string` the table name

##### return
* **$this**


#### air\mysql::mode
---
```php
public air\mysql::mode( int $mode ) : $this
```
##### parameters
* **$mode**  `int` connect mode, can be \AIR_R or \AIR_W, if not set, it'll be set to \AIR_R for GET and \AIR_W for ADD|SET|DEL automatically

##### return
* **$this**


#### air\mysql::async
---
 Set query to the async mode, but it will be force reset to sync mode when update, insert and delete at present
```php
public air\mysql::async( void ) : $this
```
##### parameters
* none

##### return
* **$this**


#### air\mysql::add
---
```php
public air\mysql::add( array $data ) : $this
```
##### parameters
* **$data**  `array`

##### return
* **$this**


#### air\mysql::set
---
```php
public air\mysql::set( array $data ) : $this
```
##### parameters
* **$data**  `array`

##### return
* **$this**


#### air\mysql::get
---
```php
public air\mysql::get( string $fields ) : $this
```
##### parameters
* **$fields**  `string`

##### return
* **$this**


#### air\mysql::del
---
```php
public air\mysql::del( void ) : $this
```
##### parameters
* none

##### return
* **$this**


#### air\mysql::where
---
```php
public air\mysql::where( string $where [, array $params = array () ] ) : $this
```
##### parameters
* **$where**  `string`
* **$params**  `array`

##### return
* **$this**


#### air\mysql::by_key
---
```php
public air\mysql::by_key( string|int $value [, string $key_name = 'id' ] ) : $this
```
##### parameters
* **$value**  `string|int`
* **$key_name**  `string`

##### return
* **$this**


#### air\mysql::sort
---
```php
public air\mysql::sort( array $sort ) : $this
```
##### parameters
* **$sort**  `array`

##### return
* **$this**


#### air\mysql::offset
---
```php
public air\mysql::offset( int $offset ) : $this
```
##### parameters
* **$offset**  `int`

##### return
* **$this**


#### air\mysql::size
---
```php
public air\mysql::size( int $size ) : $this
```
##### parameters
* **$size**  `int`

##### return
* **$this**


#### air\mysql::query
---
```php
public air\mysql::query( string $sql [, array $params = array () ] ) : $this
```
##### parameters
* **$sql**  `string`
* **$params**  `array`

##### return
* **$this**


#### air\mysql::build
---
```php
public air\mysql::build( \mysqli $escaper ) : $this
```
##### parameters
* **$escaper**  `\mysqli`

##### return
* **$this**


#### air\mysql::trigger
---
```php
public air\mysql::trigger( string $event, array $params ) : array
```
##### parameters
* **$event**  `string`
* **$params**  `array`

##### return
* **array**


#### air\mysql::on_success
---
```php
public air\mysql::on_success( $callback )
```
##### parameters
* none

##### return
* none


#### air\mysql::on_error
---
```php
public air\mysql::on_error( $callback )
```
##### parameters
* none

##### return
* none


#### air\mysql::on_success_default
---
```php
public static air\mysql::on_success_default( $ch, $result )
```
##### parameters
* none

##### return
* none


#### air\mysql::on_error_default
---
```php
public static air\mysql::on_error_default( $ch )
```
##### parameters
* none

##### return
* none


#### air\mysql::__destruct
---
```php
public air\mysql::__destruct( void )
```
##### parameters
* none

##### return
* none
