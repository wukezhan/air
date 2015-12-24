<?php
/**
 *
 * @author wukezhan<wukezhan@gmail.com>
 * 2015-11-08 21:14
 *
 */
namespace app\exec;

use air\controller;
use test\a;
use test\b;

class hello extends controller
{
    public function action_world()
    {
        echo "hello world from console\n";
    }

    public function action_autoload()
    {
        var_dump(new a());
        var_dump(new b());
        var_dump(get_included_files());
    }
}
