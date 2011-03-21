--TEST--
Loops: keys/values bug
--SKIPIF--
<?php if (!class_exists('TextTemplate')) print "skip"; ?>
--FILE--
<?php
$t = new TextTemplate();
$tpl = "\n".
  TextTemplate::tokenizeLoop('ARRY').
    TextTemplate::tokenize('$@').': '.TextTemplate::tokenize('$')."\n".
  TextTemplate::tokenizeLoopEnd();
$t->compile($tpl);
print $t->render(array('ARRY'=>array('foo', 'bar', 'baz', 'quux')));
print $t->render(array('ARRY'=>array('foo'=>0, 'bar'=>1, 'baz'=>2, 'quux'=>3)));
?>
--EXPECT--
0: foo
1: bar
2: baz
3: quux

foo: 0
bar: 1
baz: 2
quux: 3
