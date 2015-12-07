<?php
/**
 *
 * @author wukezhan<wukezhan@gmail.com>
 * 2015-11-08 21:14
 *
 */
namespace app\site;

use air\controller;

class hello extends controller
{
    public function action_world()
    {
        $this->assign('hello', htmlspecialchars($this->_route['a']));
        $this->render_view();
    }
}
