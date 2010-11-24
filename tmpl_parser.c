/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2010 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author: Dave Ingram <php@dmi.me.uk>                                  |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#include "php_tmpl.h"
#include "tmpl_parser.h"

static char *tmpl_parse_find_tag_open(const char *tmpl) {
	char *tmp = tmpl;
	if (!tmp) return NULL;
search:
	while (*tmp && *tmp != 0[TMPL_T_PRE]) {
		++tmp;
	}
	// if we don't find the entire string, search again
	if (*tmp && strncmp(tmp, TMPL_T_PRE, sizeof(TMPL_T_PRE)-1)) {
		++tmp;
		goto search;
	}
	return tmp;
}

static char *tmpl_parse_find_tag_close(const char *tmpl) {
	char *tmp = tmpl;
	if (!tmp) return NULL;
search:
	while (*tmp && *tmp != 0[TMPL_T_POST]) {
		++tmp;
	}
	// if we don't find the entire string, search again
	if (*tmp && strncmp(tmp, TMPL_T_POST, sizeof(TMPL_T_POST)-1)) {
		++tmp;
		goto search;
	}
	if (!*tmp) return NULL;
	return tmp;
}

/**
 * Returns the content of the first tag found in the argument (stripped of
 * begin/end delimiters), or NULL if no tags found.
 */
char *tmpl_parse_get_first_tag(const char *tmpl) {
	char *tagstart = tmpl_parse_find_tag_open(tmpl);
	char *tagend   = tmpl_parse_find_tag_close(tagstart);
	if (!tagstart || !tagend) {
		return NULL;
	}
	int tag_len = tagend-tagstart-(sizeof(TMPL_T_PRE)-1);
	char *tag = emalloc(tag_len+1);
	memset(tag, 0, tag_len+1);
	strncpy(tag, tagstart+sizeof(TMPL_T_PRE)-1, tag_len);
	return tag;
}

/**
 * Returns a pointer to the part of the argument after the first tag found, or
 * NULL if there are no more tags.
 */
char *tmpl_parse_skip_tag(const char *tmpl) {
	char *tagstart = tmpl_parse_find_tag_open(tmpl);
	char *tagend   = tmpl_parse_find_tag_close(tagstart);
	if (!tagstart || !tagend) {
		return NULL;
	}
	return tagend+sizeof(TMPL_T_POST)-1;
}


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
