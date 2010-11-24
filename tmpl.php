<?php

$t = new TextTemplate();
var_dump($t);
$t->foo='bar';
var_dump($t);
var_dump(get_class_methods($t));
var_dump(TextTemplate::tokenize('A'));
var_dump(TextTemplate::tokenize('A', 'bb'));
var_dump(TextTemplate::tokenizeConditional('A'));
var_dump(TextTemplate::tokenizeConditional('A', true));
var_dump(TextTemplate::tokenizeConditional('A', false));
var_dump(TextTemplate::tokenizeElseIf('B'));
var_dump(TextTemplate::tokenizeElseIf('B', true));
var_dump(TextTemplate::tokenizeElseIf('B', false));
var_dump(TextTemplate::tokenizeElse());
var_dump(TextTemplate::tokenizeConditionalEnd());
var_dump(TextTemplate::tokenizeLoop('FOO'));
var_dump(TextTemplate::tokenizeLoopRange(1, 9));
var_dump(TextTemplate::tokenizeLoopRange(1, 9, 2));
var_dump(TextTemplate::tokenizeLoopElse());
var_dump(TextTemplate::tokenizeLoopEnd());
