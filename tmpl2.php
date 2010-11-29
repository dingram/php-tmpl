<?php

/*
$t = new TextTemplate();

$tpl = 'abcd<!--{TAG}-->efgh';
var_dump($tpl);
$t->compile($tpl);
var_dump($t->render(array('TAG'=>'<TAG_GOES_HERE>')));
var_dump($t->render(array('TAG'=>' ANOTHER_TAG ')));
print "\n\n";

$tpl = 'abcd<!--{?COND}-->efgh<!--{/?}-->ijkl';
var_dump($tpl);
$t->compile($tpl);
var_dump($t->render(array('COND' => true)));
var_dump($t->render(array('COND' => false)));
print "\n\n";

$tpl = 'abcd<!--{?COND}-->efgh<!--{:}-->EFGH<!--{/?}-->ijkl';
var_dump($tpl);
$t->compile($tpl);
var_dump($t->render(array('COND' => true)));
var_dump($t->render(array('COND' => false)));
print "\n\n";

$tpl = 'abcd<!--{?!COND}-->efgh<!--{:}-->EFGH<!--{/?}-->ijkl';
var_dump($tpl);
$t->compile($tpl);
var_dump($t->render(array('COND' => true)));
var_dump($t->render(array('COND' => false)));
print "\n\n";

$tpl = 'abcd <!--{?COND}-->foo<!--{FOO}-->foo<!--{:}-->bar<!--{FOO}-->bar<!--{/?}--> ijkl';
var_dump($tpl);
$t->compile($tpl);
var_dump($t->render(array('COND' => true,  'FOO' => '<bar>')));
var_dump($t->render(array('COND' => false, 'FOO' => '<bar>')));
print "\n\n";

$tpl = 'abcd <!--{?COND}-->foo<!--{FOO}-->foo<!--{:?COND2}-->bar<!--{FOO}-->bar<!--{/?}--> ijkl';
var_dump($tpl);
$t->compile($tpl);
var_dump($t->render(array('COND' => true,  'FOO' => '<bar>')));
var_dump($t->render(array('COND' => false, 'COND2' => true, 'FOO' => '<bar>')));
var_dump($t->render(array('COND' => false, 'COND2' => false, 'FOO' => '<bar>')));
print "\n\n";
*/

/*
function test_parse($tpl) {
	print str_repeat('=', 78)."\n";
	print "$tpl\n";
	print str_repeat('-', 78)."\n";
	tmpl_parse($tpl);
	print str_repeat('=', 78)."\n\n";
}

#test_parse('<!--{?A}--><!--{/#}-->');
#test_parse('<!--{:}-->efgh<!--{TAG}-->ijkl<!--{/?}-->mnop');
test_parse('abcd<!--{?TAG}-->efgh<!--{TAG}-->ijkl<!--{/?}-->mnop');
test_parse('abcd<!--{?TAG}-->efgh<!--{TAG}-->ijkl<!--{:}-->mnop<!--{/?}-->qrst');
test_parse("abcd<!--{?TAG}-->e<!--{?BAR}-->f<!--{:}-->g<!--{/?}-->h<!--{TAG}-->ijkl<!--{:}-->m<!--{?BAR}-->n<!--{:}-->o<!--{/?}-->p<!--{/?}-->qrst");
test_parse('abcd<!--{?TAG}-->efgh<!--{TAG}-->ijkl<!--{:?TAG2}-->mnop<!--{/?}-->qrst');
test_parse('abcd<!--{?TAG}-->efgh<!--{TAG}-->ijkl<!--{:?TAG2}-->mnop<!--{:}-->qsrt<!--{/?}-->uvwx');
 */


function testNested2If($negA, $negB, $order) {

  $t = new TextTemplate();
  $tpl =
    TextTemplate::tokenizeConditional('A', $negA).
      TextTemplate::tokenizeConditional('B', $negB).
        $order[0].
      TextTemplate::tokenizeElse().
        $order[1].
      TextTemplate::tokenizeConditionalEnd().
    TextTemplate::tokenizeElse().
      TextTemplate::tokenizeConditional('B', $negB).
        $order[2].
      TextTemplate::tokenizeElse().
        $order[3].
      TextTemplate::tokenizeConditionalEnd().
    TextTemplate::tokenizeConditionalEnd();

  var_dump($tpl);
  $t->compile($tpl);

  var_dump($t->render(array('A'=> false, 'B' => false)));
  var_dump($t->render(array('A'=> false, 'B' => true)));
  var_dump($t->render(array('A'=> true, 'B' => false)));
  var_dump($t->render(array('A'=> true, 'B' => true)));
}

testNested2If(false, false, '3210');
testNested2If(false, true,  '2301');
testNested2If(true,  false, '1032');
testNested2If(true,  true,  '0123');
