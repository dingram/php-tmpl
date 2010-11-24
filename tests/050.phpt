--TEST--
get_first_tag
--SKIPIF--
<?php if (!function_exists("tmpl_get_first_tag")) print "skip"; ?>
--FILE--
<?php
echo tmpl_get_first_tag('no tag here!'), "\n";
echo tmpl_get_first_tag('<!--{TAG}-->'), "\n";
echo tmpl_get_first_tag('asdf<!--{TAG}-->'), "\n";
echo tmpl_get_first_tag('<!--{TAG}-->asdf'), "\n";
echo tmpl_get_first_tag('asdf<!--{TAG}-->asdf'), "\n";
echo tmpl_get_first_tag('asdf<!--{TAG}-->asdf<!--{TAG2}-->'), "\n";
echo tmpl_get_first_tag('asdf<!--{TAG}--><!--{TAG2}-->'), "\n";
?>
--EXPECT--

TAG
TAG
TAG
TAG
TAG
TAG
