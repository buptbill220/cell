<?php
require_once('core/autoload.class.php');

define("HOME_DIR", dirname(__FILE__));
date_default_timezone_set('PRC');
	require_once('define.php');
require_once(FM_SIM_CORE . 'autoload.class.php');
$loader = Autoload::init(FM_SIM_PATH);
Autoload::requireAll(FM_SIM_COMMON);
Autoload::requireAll(FM_SIM_EXT);
require_once("core/memcachecache.class.php");
$cache = Cache::getInstance();
$cache->init();
Autoload::requireAll('common');
Autoload::requireAll('ext');
INFO::infoDebug("==============test log================");
test::test_log("ssds");
INFO::infoDebug("=============test cache===============");
test::test_cache();
INFO::infoDebug("============test template=============");
test::test_template();

INFO::infoDebug("============test database=============");
$model = new Model("user");
dump($model->find());
	
