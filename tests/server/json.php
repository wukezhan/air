<?php
/**
 *
 * @author wukezhan<wukezhan@gmail.com>
 * 2015-10-28 21:00
 *
 */
include __DIR__.'/inc.php';

echo json_encode($_POST?:$_GET);