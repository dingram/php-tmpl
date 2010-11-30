<?php

$t = new TextTemplate();

var_dump(isset($t['tmp']));
var_dump(count($t));
print "\n";

$t['tmp'] = 'TEST';
var_dump(isset($t['tmp']));
var_dump(count($t));
var_dump($t['tmp']);
var_dump($t->get('tmp'));
print "\n";

unset($t['tmp']);
var_dump(isset($t['tmp']));
var_dump(count($t));

print "\n";


$t = new TextTemplate();
print "\n";

var_dump(isset($t['tmp']));
var_dump(count($t));
print "\n";

$t->set('tmp', 'TEST');
var_dump(isset($t['tmp']));
var_dump(count($t));
var_dump($t['tmp']);
var_dump($t->get('tmp'));
print "\n";

unset($t['tmp']);
var_dump(isset($t['tmp']));
var_dump(count($t));

print "\n";

$t = new TextTemplate();
print "\n";

var_dump(isset($t['tmp']));
var_dump(count($t));
print "\n";

$t->set(array('tmp' => 'TEST'));
var_dump(isset($t['tmp']));
var_dump(count($t));
var_dump($t['tmp']);
var_dump($t->get('tmp'));
print "\n";

unset($t['tmp']);
var_dump(isset($t['tmp']));
var_dump(count($t));

