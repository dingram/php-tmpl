<?php

$t = new TextTemplate();
var_dump(isset($t['tmp']));
$t['tmp'] = 'TEST';
var_dump(isset($t['tmp']));
var_dump($t['tmp']);
unset($t['tmp']);
var_dump(isset($t['tmp']));
