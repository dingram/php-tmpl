<?php

$t = new TextTemplate();
$tpl =
  TextTemplate::tokenizeLoopRange(1, 9).
    'Test! '.
  TextTemplate::tokenizeLoopEnd();

var_dump($tpl);
$t->compile($tpl);
var_dump($t->render());

print "\n\n";

$t = new TextTemplate();
$tpl =
  TextTemplate::tokenizeLoopRange(1, 9).
    TextTemplate::tokenize('$').' '.
  TextTemplate::tokenizeLoopEnd();

var_dump($tpl);
$t->compile($tpl);
var_dump($t->render());

print "\n\n";

$t = new TextTemplate();
$tpl =
  TextTemplate::tokenizeLoopRange(1, 9, 2).
    TextTemplate::tokenize('$').' '.
  TextTemplate::tokenizeLoopEnd();

var_dump($tpl);
$t->compile($tpl);
var_dump($t->render());

print "\n\n";

$t = new TextTemplate();
$tpl =
  TextTemplate::tokenizeLoopRange(1, 9, 3).
    TextTemplate::tokenize('$').' '.
  TextTemplate::tokenizeLoopEnd();

var_dump($tpl);
$t->compile($tpl);
var_dump($t->render());

print "\n\n";

$t = new TextTemplate();
$tpl =
  TextTemplate::tokenizeLoopRange(9, 1).
    TextTemplate::tokenize('$').' '.
  TextTemplate::tokenizeLoopEnd();

var_dump($tpl);
$t->compile($tpl);
var_dump($t->render());

print "\n\n";

$t = new TextTemplate();
$tpl =
  TextTemplate::tokenizeLoopRange(9, 1, -2).
    TextTemplate::tokenize('$').' '.
  TextTemplate::tokenizeLoopEnd();

var_dump($tpl);
$t->compile($tpl);
var_dump($t->render());

print "\n\n";

$t = new TextTemplate();
$tpl = "\n".
  TextTemplate::tokenizeLoopRange(0, 2).
    '$['.TextTemplate::tokenize('$').']: '.
    TextTemplate::tokenizeLoopRange(0, 3).
      TextTemplate::tokenize('$').
      '['.TextTemplate::tokenize('$$').'] '.
    TextTemplate::tokenizeLoopEnd().
    "\n".
  TextTemplate::tokenizeLoopEnd();

var_dump($tpl);
$t->compile($tpl);
var_dump($t->render());

print "\n\n";

$t = new TextTemplate();
$tpl = "\n".
  TextTemplate::tokenizeLoopRange(0, 2).
    '$['.TextTemplate::tokenize('$').']: '.
    TextTemplate::tokenizeLoopRange(0, 3).
      TextTemplate::tokenizeConditional('$').
        TextTemplate::tokenize('$').
      TextTemplate::tokenizeElse().
        'zero'.
      TextTemplate::tokenizeConditionalEnd().
      '['.
        TextTemplate::tokenizeConditional('$$', true).
          TextTemplate::tokenize('$$').
        TextTemplate::tokenizeElse().
          'nonzero'.
        TextTemplate::tokenizeConditionalEnd().
      '] '.
    TextTemplate::tokenizeLoopEnd().
    "\n".
  TextTemplate::tokenizeLoopEnd();

var_dump($tpl);
$t->compile($tpl);
var_dump($t->render());

print "\n\n";

$t = new TextTemplate();
$tpl =
  TextTemplate::tokenizeLoop('ARRY').
    TextTemplate::tokenize('$').' '.
  TextTemplate::tokenizeLoopEnd();

var_dump($tpl);
$t->compile($tpl);
var_dump($t->render(array('ARRY'=>array('foo', 'bar', 'baz', 'quux'))));

print "\n\n";

$t = new TextTemplate();
$tpl =
  TextTemplate::tokenizeLoop('ARRY').
    TextTemplate::tokenize('$@').' '.
  TextTemplate::tokenizeLoopEnd();

var_dump($tpl);
$t->compile($tpl);
var_dump($t->render(array('ARRY'=>array('foo'=>0, 'bar'=>1, 'baz'=>2, 'quux'=>3))));

print "\n\n";

$t = new TextTemplate();
$tpl = "\n".
  TextTemplate::tokenizeLoop('ARRY').
    TextTemplate::tokenize('$@').': '.TextTemplate::tokenize('$')."\n".
  TextTemplate::tokenizeLoopEnd();

var_dump($tpl);
$t->compile($tpl);
$result = $t->render(array('ARRY'=>array('foo', 'bar', 'baz', 'quux')));
var_dump($result);
if (strpos($result, '9') !== false) {
  var_dump('FAILURE');
  exit;
}

print "\n\n";

$t = new TextTemplate();
$tpl = "\n".
  TextTemplate::tokenizeLoop('ARRY').
    TextTemplate::tokenize('$@').': '.TextTemplate::tokenize('$')."\n".
  TextTemplate::tokenizeLoopEnd();

var_dump($tpl);
$t->compile($tpl);
$result = $t->render(array('ARRY'=>array('foo'=>0, 'bar'=>1, 'baz'=>2, 'quux'=>3)));
var_dump($result);
if (strpos($result, '`') !== false) {
  var_dump('FAILURE');
  exit;
}

print "\n\n";

$t = new TextTemplate();
$tpl = "\n".
  TextTemplate::tokenizeLoop('ARRY').
    TextTemplate::tokenize('$@').': '.TextTemplate::tokenize('$')."\n".
  TextTemplate::tokenizeLoopEnd();

var_dump($tpl);
$t->compile($tpl);
var_dump($t->render(array('ARRY'=>array('foo'=>0, 'bar'=>1, 'baz'=>2, 'quux'=>3, 'ble'=>4, 'wtf'=>6, 'idunno'=>8))));

print "\n\n";

$t = new TextTemplate();
$tpl = "\n".
  TextTemplate::tokenizeLoop('ARRY').
    TextTemplate::tokenize('$@').': '.TextTemplate::tokenize('$')."\n".
  TextTemplate::tokenizeLoopEnd();

var_dump($tpl);
$t->compile($tpl);
var_dump($t->render(array('ARRY'=>array('foo'=>0, 'bar'=>1, 'baz'=>2))));

print "\n\n";
