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

static inline char *tmpl_parse_find_tag_open(const char *tmpl) {
	return tmpl ? strstr(tmpl, TMPL_T_PRE) : NULL;
}

static inline char *tmpl_parse_find_tag_close(const char *tmpl) {
	return tmpl ? strstr(tmpl, TMPL_T_POST) : NULL;
}

/**
 * Returns a copy of the content of the first tag found in the argument
 * (stripped of begin/end delimiters), or NULL if no tags found.
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
 * Returns a pointer (not a copy!) to the part of the argument after the first
 * tag found, or NULL if there are no more tags.
 */
char *tmpl_parse_skip_tag(const char *tmpl) {
	char *tagstart = tmpl_parse_find_tag_open(tmpl);
	char *tagend   = tmpl_parse_find_tag_close(tagstart);
	if (!tagstart || !tagend) {
		return NULL;
	}
	return tagend+sizeof(TMPL_T_POST)-1;
}

/**
 * Returns a copy of the part of the argument before the first tag found, or
 * NULL if there are no tags
 */
char *tmpl_parse_until_tag(const char *tmpl) {
	char *tagstart = tmpl_parse_find_tag_open(tmpl);
	char *tagend   = tmpl_parse_find_tag_close(tagstart);
	if (!tagstart || !tagend) {
		return NULL;
	}
	int tag_pos = tagstart-tmpl;
	char *frag = emalloc(tag_pos+1);
	memset(frag, 0, tag_pos+1);
	strncpy(frag, tmpl, tag_pos);
	return frag;
}


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
