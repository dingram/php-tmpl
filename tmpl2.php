<?php

$t = new TextTemplate();
var_dump($t->compile('abcd<!--{TAG}-->efgh'));
var_dump($t->render(array('TAG'=>'<TAG_GOES_HERE>')));
var_dump($t->render(array('TAG'=>' ANOTHER_TAG ')));
