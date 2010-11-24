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

#ifndef PHP_TMPL_PARSER_H
#define PHP_TMPL_PARSER_H

#include "tmpl_expr_parser.h"
#include "ext/standard/php_string.h"

char *tmpl_parse_get_first_tag(const char *tmpl);
char *tmpl_parse_skip_tag(const char *tmpl);
char *tmpl_parse_until_tag(const char *tmpl);

/* Plain content: data.content */
#define TMPL_EL_CONTENT     0
/* Simple substitution: data.var */
#define TMPL_EL_SUBST       1
/* Expression substitution: data.expr */
#define TMPL_EL_SUBST_EXPR  2
/* Simple conditional: data.var, next_cond, content_item */
#define TMPL_EL_COND        3
/* Expression conditional: data.expr, next_cond, content_item */
#define TMPL_EL_COND_EXPR   4
/* Else block: content_item */
#define TMPL_EL_ELSE        5
/* Loop over range: data.range, content_item, next_cond */
#define TMPL_EL_LOOP_RANGE  6
/* Loop over array var: data.var, content_item, next_cond */
#define TMPL_EL_LOOP_VAR    7
/* Loop "else": content_item */
#define TMPL_EL_LOOP_ELSE   8

typedef struct php_tt_tmpl_el_t {
	int type;
	union {
		struct {
			char *name;
			long cname;
		} var;
		struct {
			char *content;
			long content_len;
		} content;
		struct {
			php_tt_tmpl_expr *expr;
		} expr;
		struct {
			long begin;
			long end;
			long step;
		} range;
	} data;
	// XXX: if condition is false (or loop would have zero iterations), move to
	// next_cond and evaluate it, otherwise process content_item and go to next
	struct php_tt_tmpl_el_t *next;
	struct php_tt_tmpl_el_t *next_cond;
	// XXX: NULL if no content or this is a text-type node rather than an action
	struct php_tt_tmpl_el_t *content_item;
} php_tt_tmpl_el;



#endif	/* PHP_TMPL_PARSER_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
