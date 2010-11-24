--TEST--
get_first_tag (pathological)
--SKIPIF--
<?php if (!function_exists("tmpl_get_first_tag")) print "skip"; ?>
--FILE--
<?php
echo tmpl_get_first_tag('asdf<!--TAG}->NO_TAG<!-{TAG2}-->'), "\n";
echo tmpl_get_first_tag('asdf<!--{TAG}->STILL_TAG<!--{TAG2}-->'), "\n";
echo tmpl_get_first_tag('}-->'), "\n";
echo tmpl_get_first_tag('<!--{'), "\n";
echo tmpl_get_first_tag(''), "\n";
?>
--EXPECT--

TAG}->STILL_TAG<!--{TAG2



