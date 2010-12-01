<?php

require_once('TextTemplate2.php');

define('TEST_ITERATIONS', 10000);

$t = new TextTemplate();
$t2 = new TextTemplate2();

$totals = array();

function test_head() {
  printf("Number of iterations: %d\n\n", TEST_ITERATIONS);
	printf("+--------------------------------------+----------+----------+---------+\n");
	printf("| Test description                     | PHP-only | C-based  | Speedup |\n");
	printf("+--------------------------------------+----------+----------+---------+\n");
}

function test_foot() {
	global $totals;
	$tot_b=array(0=>0.0, 1=>0.0);
	$tot_f=array(0=>0.0, 1=>0.0);

	foreach ($totals[0] as $t) {
		$tot_b[0] += $t[0];
		$tot_b[1] += $t[1];
	}
	foreach ($totals[1] as $t) {
		$tot_f[0] += $t[0];
		$tot_f[1] += $t[1];
	}

	printf("| Total [Biased]:                      | %8.05f | %8.05f | %6dx | \n", $tot_b[1], $tot_b[0], $tot_b[1]/$tot_b[0]);
	printf("| Total [Fair]:                        | %8.05f | %8.05f | %6dx | \n", $tot_f[1], $tot_f[0], $tot_f[1]/$tot_f[0]);
	printf("+--------------------------------------+----------+----------+---------+\n");
}

function test_perf($desc, $tpl, array $vars) {
	global $totals;

	printf("| %-32.32s [B] | ", $desc);
	$times = test_biased($tpl, $vars);
	$totals[0][]=$times;
	printf("%8.05f | %8.05f | %6dx |\n", $times[1], $times[0], $times[1]/$times[0]);

	printf("| %-32.32s [F] | ", $desc);
	$times = test_fair($tpl, $vars);
	$totals[1][]=$times;
	printf("%8.05f | %8.05f | %6dx |\n", $times[1], $times[0], $times[1]/$times[0]);
	printf("+--------------------------------------+----------+----------+---------+\n");
}

function test_biased($tpl, array $vars) {
	global $t, $t2;
	$times = array();

	$t->set($vars);
	$t2->set($vars);

	$start = microtime(true);
	$t->compile($tpl);
	for ($i=0; $i<TEST_ITERATIONS; ++$i) {
		$t->render();
	}
	$end = microtime(true);
	$times[] = ($end-$start);

	$start = microtime(true);
	for ($i=0; $i<TEST_ITERATIONS; ++$i) {
		$t2->process($tpl);
	}
	$end = microtime(true);
	$times[] = ($end-$start);

	return $times;
}

function test_fair($tpl, array $vars) {
	global $t, $t2;
	$times = array();

	$t->set($vars);
	$t2->set($vars);

	$start = microtime(true);
	for ($i=0; $i<TEST_ITERATIONS; ++$i) {
		$t->compile($tpl);
		$t->render();
	}
	$end = microtime(true);
	$times[] = ($end-$start);

	$start = microtime(true);
	for ($i=0; $i<TEST_ITERATIONS; ++$i) {
		$t2->process($tpl);
	}
	$end = microtime(true);
	$times[] = ($end-$start);

	return $times;
}


test_head();

test_perf('Blank',          '',                                                                          array(            ));
test_perf('Tag',            '<!--{TAG}-->',                                                              array('TAG'=>'foo'));
test_perf('IF',             '<!--{?TAG}--><!--{TAG}--><!--{/}-->',                                       array('TAG'=>'foo'));
test_perf('IF/ELSE',        '<!--{?TAG}--><!--{TAG}--><!--{:}-->fail<!--{/}-->',                         array('TAG'=>'foo'));
test_perf('IF/ELSEIF',      '<!--{?TAG}--><!--{TAG}--><!--{:?TAG2}-->fail<!--{/}-->',                    array('TAG'=>'foo'));
test_perf('IF/ELSEIF/ELSE', '<!--{?TAG}--><!--{TAG}--><!--{:?TAG2}-->fail<!--{:}-->fail more<!--{/}-->', array('TAG'=>'foo'));

test_foot();
