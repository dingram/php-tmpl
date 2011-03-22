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
print $t->render(array('ARRY'=>array('foo', 'bar')));
print $t->render(array('ARRY'=>array('foo'=>0, 'bar'=>1)));
print $t->render(array('ARRY'=>array('foo', 'bar', 'baz')));
print $t->render(array('ARRY'=>array('foo'=>0, 'bar'=>1, 'baz'=>2)));
print $t->render(array('ARRY'=>array('foo', 'bar', 'b', 'qux')));
print $t->render(array('ARRY'=>array('foo'=>0, 'bar'=>1, 'b'=>2, 'qux'=>3)));
print $t->render(array('ARRY'=>array('foo', 'bar', 'ba', 'qux')));
print $t->render(array('ARRY'=>array('foo'=>0, 'bar'=>1, 'ba'=>2, 'qux'=>3)));
print $t->render(array('ARRY'=>array('foo', 'bar', 'b', 'quux')));
print $t->render(array('ARRY'=>array('foo'=>0, 'bar'=>1, 'b'=>2, 'quux'=>3)));
print $t->render(array('ARRY'=>array('foo', 'bar', 'ba', 'quux')));
print $t->render(array('ARRY'=>array('foo'=>0, 'bar'=>1, 'ba'=>2, 'quux'=>3)));
print $t->render(array('ARRY'=>array('foo', 'bar', 'baz', 'quux')));
print $t->render(array('ARRY'=>array('foo'=>0, 'bar'=>1, 'baz'=>2, 'quux'=>3)));
print $t->render(array('ARRY'=>array('foo', 'bar', 'bazz', 'quux')));
print $t->render(array('ARRY'=>array('foo'=>0, 'bar'=>1, 'bazz'=>2, 'quux'=>3)));
print $t->render(array('ARRY'=>array('foo', 'bar', 'bazzz', 'quux')));
print $t->render(array('ARRY'=>array('foo'=>0, 'bar'=>1, 'bazzz'=>2, 'quux'=>3)));
print $t->render(array('ARRY'=>array('foo', 'bar', 'bazzzz', 'quux')));
print $t->render(array('ARRY'=>array('foo'=>0, 'bar'=>1, 'bazzzz'=>2, 'quux'=>3)));
print $t->render(array('ARRY'=>array('foo', 'bar', 'bazzzzz', 'quux')));
print $t->render(array('ARRY'=>array('foo'=>0, 'bar'=>1, 'bazzzzz'=>2, 'quux'=>3)));
print $t->render(array('ARRY'=>array('foo', 'bar', 'bazzzzzz', 'quux')));
print $t->render(array('ARRY'=>array('foo'=>0, 'bar'=>1, 'bazzzzzz'=>2, 'quux'=>3)));
print $t->render(array('ARRY'=>array('foo', 'bar', 'bazzzzzzz', 'quux')));
print $t->render(array('ARRY'=>array('foo'=>0, 'bar'=>1, 'bazzzzzzz'=>2, 'quux'=>3)));
print $t->render(array('ARRY'=>array('foo', 'bar', 'baz', 'quux', 'wibble')));
print $t->render(array('ARRY'=>array('foo'=>0, 'bar'=>1, 'baz'=>2, 'quux'=>3, 'wibble'=>4)));
?>
--EXPECT--
0: foo
1: bar

foo: 0
bar: 1

0: foo
1: bar
2: baz

foo: 0
bar: 1
baz: 2

0: foo
1: bar
2: b
3: qux

foo: 0
bar: 1
b: 2
qux: 3

0: foo
1: bar
2: ba
3: qux

foo: 0
bar: 1
ba: 2
qux: 3

0: foo
1: bar
2: b
3: quux

foo: 0
bar: 1
b: 2
quux: 3

0: foo
1: bar
2: ba
3: quux

foo: 0
bar: 1
ba: 2
quux: 3

0: foo
1: bar
2: baz
3: quux

foo: 0
bar: 1
baz: 2
quux: 3

0: foo
1: bar
2: bazz
3: quux

foo: 0
bar: 1
bazz: 2
quux: 3

0: foo
1: bar
2: bazzz
3: quux

foo: 0
bar: 1
bazzz: 2
quux: 3

0: foo
1: bar
2: bazzzz
3: quux

foo: 0
bar: 1
bazzzz: 2
quux: 3

0: foo
1: bar
2: bazzzzz
3: quux

foo: 0
bar: 1
bazzzzz: 2
quux: 3

0: foo
1: bar
2: bazzzzzz
3: quux

foo: 0
bar: 1
bazzzzzz: 2
quux: 3

0: foo
1: bar
2: bazzzzzzz
3: quux

foo: 0
bar: 1
bazzzzzzz: 2
quux: 3

0: foo
1: bar
2: baz
3: quux
4: wibble

foo: 0
bar: 1
baz: 2
quux: 3
wibble: 4
