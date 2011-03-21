--TEST--
Stringification: simple conditionals
--SKIPIF--
<?php if (!class_exists('TextTemplate')) print "skip"; ?>
--FILE--
<?php
function testIt($a) {
  $t = new TextTemplate();
  print $t == $t->compile($a) ? "pass\n" : "FAIL\n";
}

testIt(
  TextTemplate::tokenizeConditional('A').
  TextTemplate::tokenizeConditionalEnd()
);
testIt(
  'foo'.
  TextTemplate::tokenizeConditional('A').
  TextTemplate::tokenizeConditionalEnd()
);
testIt(
  TextTemplate::tokenizeConditional('A').
  'foo'.
  TextTemplate::tokenizeConditionalEnd()
);
testIt(
  'foo'.
  TextTemplate::tokenizeConditional('A').
  'foo'.
  TextTemplate::tokenizeConditionalEnd()
);
testIt(
  TextTemplate::tokenizeConditional('A').
  TextTemplate::tokenizeConditionalEnd().
  'foo'
);
testIt(
  'foo'.
  TextTemplate::tokenizeConditional('A').
  TextTemplate::tokenizeConditionalEnd().
  'foo'
);
testIt(
  TextTemplate::tokenizeConditional('A').
  'foo'.
  TextTemplate::tokenizeConditionalEnd().
  'foo'
);
testIt(
  'foo'.
  TextTemplate::tokenizeConditional('A').
  'foo'.
  TextTemplate::tokenizeConditionalEnd().
  'foo'
);
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
