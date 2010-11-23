<?php
$br = (php_sapi_name() == "cli")? "":"<br>";

if(!extension_loaded('tmpl')) {
	dl('tmpl.' . PHP_SHLIB_SUFFIX);
}
$module = 'tmpl';
$consts = get_defined_constants();
echo "Constants available in the test extension:$br\n";
foreach($consts as $const => $val) {
	if (substr($const, 0, 5) !=='TMPL_') continue;
    echo $const.': '.$val."$br\n";
}
echo "$br\n";
$functions = get_extension_funcs($module);
echo "Functions available in the test extension:$br\n";
foreach($functions as $func) {
    echo $func."$br\n";
}
echo "$br\n";
$function = 'confirm_' . $module . '_compiled';
if (extension_loaded($module)) {
	$str = $function($module);
} else {
	$str = "Module $module is not compiled into PHP";
}
echo "$str\n";

