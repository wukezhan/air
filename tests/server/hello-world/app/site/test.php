<?php
/**
 *
 * @author wukezhan<wukezhan@gmail.com>
 * 2015-11-08 21:15
 *
 */
namespace app\site;

use air\controller;

class test extends controller
{
    public function action_index()
    {
        $this->assign('hello', 'test');
        $this->render_view('hello/world.php');
    }
    public function action_a()
    {
        $this->assign('hello', 'test/a');
        $this->render_view('hello/world.php');
    }
    public function action_error()
    {
        new bbb();
    }

    public function action_assign_error()
    {
        $this->assign([], []);
    }
}
