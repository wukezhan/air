<?php
/**
 *
 * @author wukezhan<wukezhan@gmail.com>
 * 2015-10-25 19:51
 *
 */

namespace {
    define('AIR_SITE', 1);
    define('AIR_EXEC', 2);
    define('AIR_R', 1);
    define('AIR_W', 2);
}

namespace air {
    /**
     * Class app
     * @package air\site
     */
    class app
    {
        protected static $_instance;

        protected $_router;

        /**
         * @param int $type app type, can be \AIR_SITE or \AIR_EXEC, default is \AIR_SITE
         * @return app
         */
        public function __construct($type=\AIR_SITE)
        {
            return $this;
        }

        /**
         * Run the app
         */
        public function run()
        {
            //
        }

        /**
         * Set the app router
         * @param router $router
         * @return $this
         */
        public function set_router($router)
        {
            return $this;
        }

        /**
         * Dispatch the request
         * @return $this
         */
        public function dispatch()
        {
            return $this;
        }
    }

    /**
     * Class controller
     * @package air\site
     */
    class controller
    {
        protected $_view_path;
        protected $_data = [];
        protected $_c;
        protected $_a;
        protected $_route;

        /**
         * The construct function
         * @return controller
         */
        public function __construct()
        {
            //
        }

        /**
         * The init function
         * @return $this;
         */
        public function init()
        {
            return $this;
        }

        /**
         * The function execute before the action
         * @return $this
         */
        public function before_action()
        {
            return $this;
        }

        /**
         * Assign the view data
         * if $o1 is an array, then the $o2 will be ignored
         * if $o1 is a string or int, $o2 will be used as value
         * @param string|array $o1
         * @param mixed $o2
         * @return $this
         */
        public function assign($o1, $o2=null)
        {
        }

        /**
         * Set the view path
         * @param $view_path
         * @return $this
         */
        public function set_view($view_path)
        {
            //
        }

        /**
         * Initialize the view engine
         * @return $this
         */
        public function init_view()
        {
            //
        }

        /**
         * Call the view's render_view function
         * @param string $view_path
         * @param bool $return
         * @return string|bool
         */
        public function render_view($view_path='', $return=false)
        {
            //
        }

        /**
         * The function called after action
         * @return $this
         */
        public function after_action()
        {
            return $this;
        }

        public function __destruct()
        {
            //
        }
    }

    /**
     * Class router
     * @package air\site
     */
    class router
    {
        protected $_url;
        protected $_original_rules = [];
        protected $_compiled_rules = [];
        protected $_route;

        /**
         * @return router
         */
        public function __construct()
        {
            //
        }

        /**
         * Set the url waiting for routing
         * @param string $url
         * @return $this
         */
        public function set_url($url)
        {
            return $this;
        }

        /**
         * Set the route rules
         * @param array $rules route rules
         * @return $this
         */
        public function set_rules($rules)
        {
            return $this;
        }

        /**
         * Do routing
         * @return array [
         *     ''
         * ]
         */
        public function route()
        {
            return [];
        }

        /**
         * Reset the pointer of the route rules
         * @return $this
         */
        public function reset()
        {
            return $this;
        }
    }

    /**
     * Class config
     * @package air
     */
    class config
    {
        /**
         * Store the config data
         * @var array
         */
        protected static $_data = [
            'app' => [
                'path' => 'app',
                'exec' => [
                    'path' => 'exec',
                ],
                'site' => [
                    'path' => 'site',
                ],
                'view' => [
                    'path' => 'view',
                    'type' => '.php',
                    'engine' => 'air\view',
                ]
            ]
        ];

        /**
         * Retrieves config data by key
         * @param string|int|null $key
         * @param mixed|null $default
         * @return mixed|$default
         */
        public static function get($key=null, $default=null)
        {
            if(0){
                return $default;
            }else{
                return 1?[]: '';
            }
        }

        /**
         * Retrieves config data by path
         * @param string $path
         * @param mixed $default
         * @return mixed|$default
         */
        public static function path_get($path=null, $default=null)
        {
            if(0){
                return $default;
            }else{
                return 1?[]: '';
            }
        }

        /**
         * Set the config data, config data can only be set once
         * @param array $config The config data to be set
         * @return bool Returns true on success, and false on failure
         */
        public static function set($config)
        {
            return true;
        }
    }

    /**
     * Class curl
     * @package air
     */
    class curl
    {
        /**
         * @var \air\curl\waiter
         */
        protected static $_waiter;

        /**
         * @var \air\async\service
         */
        protected $_service;

        /**
         * @var curl A cURL handle returned by curl_init().
         */
        protected $_ch;
        protected $_data;
        protected $_status = 0;

        protected $_callback = [
            'success' => '\air\curl::on_success_default',
            'error' => '\air\curl::on_error_default',
        ];

        public function __construct()
        {
            //
        }

        public function offsetExists($key)
        {
            //
        }

        /**
         * Retrieves the given key
         * @param $key
         * @return bool
         */
        public function offsetGet($key)
        {
            //
        }

        public function offsetSet($key, $value)
        {
            //
        }

        /**
         * Unset an offset by key
         * @param $key
         */
        public function offsetUnset($key)
        {
            //
        }

        /**
         * Set an option for a air\curl transfer
         * @param int $ok The CURLOPT_XXX option to set.
         * @param mixed $ov The value to be set on option.
         * @return $this
         */
        public function setopt($ok, $ov)
        {
            return $this;
        }

        /**
         * Enable the async request mode
         * @return $this
         */
        public function async()
        {
            return $this;
        }

        /**
         * Call to do a regular HTTP GET request to the $url with param $params
         * @param string $url The request url
         * @param array $params The request params
         * @return $this
         */
        public function get($url, $params=[])
        {
            return $this;
        }

        /**
         * Call to do a regular HTTP POST request to the $url with param $params
         * @param string $url The request url
         * @param array $data The request params
         * @return $this
         */
        public function post($url, $data=[])
        {
            return $this;
        }

        /**
         * Explicit call the curl_exec or curl_multi_exec function
         * @return $this
         */
        public function exec()
        {
            return $this;
        }

        /**
         * Return the request data
         * @return mixed
         */
        public function data()
        {
            return [];
        }

        /**
         * Reset the curl handle, which actually calls the curl_reset function
         * @return $this
         */
        public function reset()
        {
            return $this;
        }

        /**
         * Return the last error number for the current session
         * @return int
         */
        public function get_errno()
        {
            return 0;
        }

        /**
         * Return a string containing the last error for the current session
         * @return string Returns the error message or '' (the empty string) if no error occurred
         */
        public function get_error()
        {
            return '';
        }

        public function trigger($event, $params)
        {
            //
        }

        public function on_success(callable $callback)
        {
            //
        }

        public function on_error(callable $callback)
        {
            //
        }

        public static function on_success_default($ch, $result)
        {
            //
        }

        public static function on_error_default($ch)
        {
            //
        }

        public function __destruct()
        {
            //
        }
    }

    /**
     * Class exception
     * @package air
     */
    class exception extends \Exception
    {
        //
    }

    /**
     * Class handler
     * @package air
     */
    class handler
    {
        public static function on_error(callable $handler)
        {
            //
        }

        public static function on_exception(callable $handler)
        {
            //
        }

        public static function on_shutdown(callable $handler)
        {
            //
        }
    }

    /**
     * Class loader
     * @package air
     */
    class loader
    {
        public static function autoload()
        {
            //
        }

        public static function load($class)
        {
            //
        }
    }

    /**
     * Class view
     * @package air
     */
    class view
    {
        protected $_config;
        protected $_data;

        public function __construct()
        {
            //
        }

        /**
         * Assign a key with value to the view data
         * @param string|int $key
         * @param mixed $value
         * @return $this
         */
        public function assign($key, $value)
        {
            return $this;
        }

        /**
         * Render a view by the view path
         * @param $view_path
         * @param bool $return
         * @return string|bool Returns string if $return is true, bool if $return is false
         */
        public function render($view_path, $return=false)
        {
            return '';
        }

        /**
         * Set the view config, it's not used at present
         * @param $config
         */
        public function set_config($config)
        {
            //
        }
    }
}

namespace air\async
{
    /**
     * Class service
     * @package air\async
     */
    class service
    {
        protected static $__id = 0;
        protected $_id;
        protected $_waiter;
        protected $_request;

        /**
         * @param waiter $waiter
         * @param $request
         */
        public function __construct(waiter $waiter, $request)
        {
            //
        }

        /**
         * Retrieves the service data
         * @return array
         */
        public function call()
        {
        }
    }

    /**
     * Class waiter
     * @package air\async
     */
    class waiter
    {
        protected $_services = [];
        protected $_responses = [];

        /**
         * The __construct method
         * all the services of the same waiter will wait for the same polling procedure
         */
        public function __construct()
        {
            //
        }

        /**
         * Serve a async request, and return an async service
         * @param $request
         * @return service
         */
        public function serve($request)
        {
        }

        /**
         * The real response function which is not visible to the service
         */
        protected function _response()
        {
            //
        }

        /**
         * Retrieves the request data of an async service by a service id
         * @param $service_id
         * @return mixed
         */
        public function response($service_id)
        {
            //
        }
    }
}

namespace air\curl
{
    /**
     * Class waiter
     * @package air\curl
     */
    class waiter extends \air\async\waiter
    {
        /**
         * The real response function which is not visible to the service
         */
        protected function _response()
        {
            //
        }

        public function __destruct()
        {
            //
        }
    }
}

namespace air\mysql
{
    /**
     * Class builder
     * @package air\mysql
     */
    class builder implements \ArrayAccess, \Iterator, \Serializable
    {
        protected static $_waiter;
        protected $_service;
        protected $_config;
        protected $_mode;

        protected $_status = 0;
        protected $_data;
        protected $_affected_rows;
        protected $_num_rows;

        protected $_callback;

        protected $_original = [];
        protected $_compiled = [];

        public function __construct($config, $table=null)
        {
            //
        }

        public function config($config)
        {
        }

        public function offsetExists($key)
        {
            //
        }

        public function offsetGet($key)
        {
            //
        }

        public function offsetSet($key, $value)
        {
            //
        }

        public function offsetUnset($key)
        {
            //
        }

        public function count()
        {
            //
        }

        public function rewind()
        {
            //
        }

        public function current()
        {
            //
        }

        public function key()
        {
            //
        }

        public function next()
        {
            //
        }

        public function valid()
        {
            //
        }

        public function serialize()
        {
            //
        }

        public function unserialize($data)
        {
            //
        }

        /**
         * @return array|null
         */
        public function data()
        {
            //
        }

        /**
         * @return int
         */
        public function affected_rows()
        {
            //
        }

        /**
         * @return int
         */
        public function num_rows()
        {
            //
        }

        /**
         * @return int
         */
        public function insert_id()
        {
            //
        }

        /**
         * @return int
         */
        public function get_errno()
        {
            return 0;
        }

        /**
         * @return string
         */
        public function get_error()
        {
            return '';
        }

        //methods
        /**
         * @param string $table the table name
         * @return $this
         */
        public function table($table)
        {
            return $this;
        }

        /**
         * @param int $mode connect mode, can be \AIR_R or \AIR_W, if not set, it'll be set to \AIR_R for GET and \AIR_W for ADD|SET|DEL automatically
         * @return $this
         */
        public function mode($mode)
        {
            return $this;
        }

        /**
         * Set query to the async mode, but it will be force reset to sync mode when update, insert and delete at present
         * @return $this
         */
        public function async()
        {
            return $this;
        }

        /**
         * @param array $data
         * @return $this
         */
        public function add(array $data)
        {
            return $this;
        }

        /**
         * @param array $data
         * @return $this
         */
        public function set(array $data)
        {
            return $this;
        }

        /**
         * @param string $fields
         * @return $this
         */
        public function get($fields)
        {
            return $this;
        }

        /**
         * @return $this
         */
        public function del()
        {
            return $this;
        }

        /**
         * @param string $where
         * @param array $params
         * @return $this
         */
        public function where($where, $params=[])
        {
            return $this;
        }

        /**
         * @param string|int $value
         * @param string $key_name
         * @return $this
         */
        public function by_key($value, $key_name='id')
        {
            return $this;
        }

        /**
         * @param array $sort
         * @return $this
         */
        public function sort($sort)
        {
            return $this;
        }

        /**
         * @param int $offset
         * @return $this
         */
        public function offset($offset)
        {
            return $this;
        }

        /**
         * @param int $size
         * @return $this
         */
        public function size($size)
        {
            return $this;
        }

        /**
         * @param string $sql
         * @param array $params
         * @return $this
         */
        public function query($sql, $params=[])
        {
            return $this;
        }

        /**
         * @param \mysqli $escaper
         * @return $this
         */
        public function build($escaper)
        {
            return $this;
        }

        /**
         * @param string $event
         * @param array $params
         * @return array
         */
        public function trigger($event, $params)
        {
            return [];
        }

        public function on_success(callable $callback)
        {
            return $this;
        }

        public function on_error(callable $callback)
        {
            return $this;
        }

        public static function on_success_default($ch, $result)
        {
            //
        }

        public static function on_error_default($ch)
        {
            //
        }

        public function __destruct()
        {
            //
        }
    }

    /**
     * Class keeper
     * @package air\mysql
     */
    class keeper
    {
        protected static $_instance;
        protected $_pool = [];
        protected function __construct()
        {
            //
        }

        public function __destruct()
        {
            //
        }

        /**
         * @param string $config
         * @param int $mode
         * @return \mysqli
         */
        public static function factory($config, $mode)
        {
            return new \mysqli();
        }

        /**
         * @param \mysqli $mysql
         * @param string $config
         * @param int $mode
         */
        public static function release(\mysqli $mysql, $config, $mode)
        {
            //
        }
    }

    /**
     * Class table
     * @package air\mysql
     */
    class table
    {
        protected $_config;
        protected $_db;
        protected $_table;

        public function __construct()
        {
            //
        }

        /**
         * @return builder
         */
        public function async()
        {
            return new builder('mysql.config.default', 'test.test');
        }

        /**
         * @param $data
         * @return builder
         */
        public function add($data)
        {
            return new builder('mysql.config.default', 'test.test');
        }

        /**
         * @param $data
         * @return builder
         */
        public function set($data)
        {
            return new builder('mysql.config.default', 'test.test');
        }

        /**
         * @param $fields
         * @return builder
         */
        public function get($fields)
        {
            return new builder('mysql.config.default', 'test.test');
        }

        /**
         * @return builder
         */
        public function del()
        {
            return new builder('mysql.config.default', 'test.test');
        }

    }

    /**
     * Class waiter
     * @package air\mysql
     */
    class waiter extends \air\async\waiter
    {
        protected function _response()
        {
            //
        }
    }
}
