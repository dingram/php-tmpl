<?php

/*
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
 */

function test_fn($n, $d) {
	echo "$n(\"$d\") \t= ";
	var_dump($n($d));
}

print "\nget_first_tag:\n";
test_fn('tmpl_get_first_tag', 'no tag here!');
test_fn('tmpl_get_first_tag', '<!--{TAG}-->');
test_fn('tmpl_get_first_tag', 'asdf<!--{TAG}-->');
test_fn('tmpl_get_first_tag', '<!--{TAG}-->asdf');
test_fn('tmpl_get_first_tag', 'asdf<!--{TAG}-->asdf');
test_fn('tmpl_get_first_tag', 'asdf<!--{TAG}-->asdf<!--{TAG2}-->');
test_fn('tmpl_get_first_tag', 'asdf<!--{TAG}--><!--{TAG2}-->');
print "Pathological:\n";
test_fn('tmpl_get_first_tag', 'asdf<!--TAG}->NO_TAG<!-{TAG2}-->');
test_fn('tmpl_get_first_tag', 'asdf<!--{TAG}->STILL_TAG<!--{TAG2}-->');
test_fn('tmpl_get_first_tag', '}-->');
test_fn('tmpl_get_first_tag', '<!--{');
test_fn('tmpl_get_first_tag', '');

print "\nskip_tag:\n";
test_fn('tmpl_skip_tag', 'no tag here!');
test_fn('tmpl_skip_tag', '<!--{TAG}-->');
test_fn('tmpl_skip_tag', 'asdf<!--{TAG}-->');
test_fn('tmpl_skip_tag', '<!--{TAG}-->asdf');
test_fn('tmpl_skip_tag', 'asdf<!--{TAG}-->asdf');
test_fn('tmpl_skip_tag', 'asdf<!--{TAG}-->asdf<!--{TAG2}-->');
test_fn('tmpl_skip_tag', 'asdf<!--{TAG}--><!--{TAG2}-->');
print "Pathological:\n";
test_fn('tmpl_skip_tag', 'asdf<!--TAG}->NO_TAG<!-{TAG2}-->');
test_fn('tmpl_skip_tag', 'asdf<!--{TAG}->STILL_TAG<!--{TAG2}-->');
test_fn('tmpl_skip_tag', '}-->');
test_fn('tmpl_skip_tag', '<!--{');
test_fn('tmpl_skip_tag', '');
