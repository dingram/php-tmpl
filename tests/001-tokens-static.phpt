--TEST--
Static tokens
--SKIPIF--
<?php if (!class_exists('TextTemplate')) print "skip"; ?>
--FILE--
<?php
echo TextTemplate::tokenizeElse(), "\n";
echo TextTemplate::tokenizeConditionalEnd(), "\n";
echo TextTemplate::tokenizeLoopElse(), "\n";
echo TextTemplate::tokenizeLoopEnd(), "\n";
?>
--EXPECT--
<!--{:}-->
<!--{/?}-->
<!--{@:}-->
<!--{/@}-->
