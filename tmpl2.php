<?php

/*
$t = new TextTemplate();
var_dump($t->compile('abcd<!--{TAG}-->efgh'));
var_dump($t->render(array('TAG'=>'<TAG_GOES_HERE>')));
var_dump($t->render(array('TAG'=>' ANOTHER_TAG ')));
print "\n\n";
*/

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
