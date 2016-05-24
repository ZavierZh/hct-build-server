<?php

$tab = new swoole_table(1024*1024);

$tab = $tab->create();

foreach ($tab as $v){
	print_r($v);

}
/*
$fp = fopen("php://stdin", "r"); 

while ($str = fgets($fp)){
	if ($str == "exit\n"){
		fclose($fp);
		die;
	}
	echo ">>:".$str;
}
 */
