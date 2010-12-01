<?php

$t = new TextTemplate();
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

print "\n\nParsing:\n";

function test_parse($tpl) {
	print str_repeat('=', 78)."\n";
	print "$tpl\n";
	print str_repeat('-', 78)."\n";
	tmpl_parse($tpl);
	print str_repeat('=', 78)."\n\n";
}

test_parse('');
test_parse('asdf');
test_parse('<!--{TAG}-->');
test_parse('asdf<!--{TAG}-->');
test_parse('<!--{TAG}-->asdf');
test_parse('asdf<!--{TAG}-->wtf');

test_parse('<!--{?FOO}-->A<!--{/?}-->');
test_parse('<!--{?FOO}-->A<!--{:}-->z<!--{/?}-->');
test_parse('<!--{?FOO}-->A<!--{:?BAR}-->B<!--{:}-->z<!--{/?}-->');

test_parse('<!--{@FOO}-->A<!--{/@}-->');
test_parse('<!--{@FOO}-->A<!--{@:}-->B<!--{/@}-->');

test_parse('<!--{@(1..9)}-->A<!--{/@}-->');
test_parse('<!--{@(1..9)}-->A<!--{@:}-->B<!--{/@}-->');
test_parse('<!--{@(0,4..16)}-->A<!--{/@}-->');
test_parse('<!--{@(1,3..9)}-->A<!--{/@}-->');
test_parse('<!--{@(9..1)}-->A<!--{/@}-->');
test_parse('<!--{@(9,7..1)}-->A<!--{/@}-->');
