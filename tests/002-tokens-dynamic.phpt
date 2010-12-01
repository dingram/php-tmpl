--TEST--
Dynamic tokens
--SKIPIF--
<?php if (!class_exists('TextTemplate')) print "skip"; ?>
--FILE--
<?php
echo TextTemplate::tokenize('A'), "\n";
echo TextTemplate::tokenize('A', 'bb'), "\n";
echo TextTemplate::tokenizeConditional('A'), "\n";
echo TextTemplate::tokenizeConditional('A', true), "\n";
echo TextTemplate::tokenizeConditional('A', false), "\n";
echo TextTemplate::tokenizeElseIf('B'), "\n";
echo TextTemplate::tokenizeElseIf('B', true), "\n";
echo TextTemplate::tokenizeElseIf('B', false), "\n";
echo TextTemplate::tokenizeLoop('FOO'), "\n";
echo TextTemplate::tokenizeLoopRange(1, 9), "\n";
echo TextTemplate::tokenizeLoopRange(1, 9, 2), "\n";
?>
--EXPECT--
<!--{A}-->
<!--{A|bb}-->
<!--{?A}-->
<!--{?!A}-->
<!--{?A}-->
<!--{:?B}-->
<!--{:?!B}-->
<!--{:?B}-->
<!--{@FOO}-->
<!--{@(1..9)}-->
<!--{@(1,3..9)}-->
