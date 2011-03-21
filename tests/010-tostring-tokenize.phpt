--TEST--
Stringification: basic tokenizing
--SKIPIF--
<?php if (!class_exists('TextTemplate')) print "skip"; ?>
--FILE--
<?php
function testIt($a) {
  $t = new TextTemplate();
  print $t == $t->compile($a) ? "pass\n" : "FAIL\n";
}

testIt(TextTemplate::tokenize('A'));
testIt(TextTemplate::tokenize('A', 'bb'));
testIt('foo'.TextTemplate::tokenize('A'));
testIt('foo'.TextTemplate::tokenize('A', 'bb'));
testIt(TextTemplate::tokenize('A').'bar');
testIt(TextTemplate::tokenize('A', 'bb').'bar');
testIt('foo'.TextTemplate::tokenize('A').'bar');
testIt('foo'.TextTemplate::tokenize('A', 'bb').'bar');
?>
--EXPECT--
pass
pass
pass
pass
pass
pass
pass
pass
