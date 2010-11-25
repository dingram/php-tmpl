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

/* NOTE: in SUBST, content_item is the default value */

/* Plain content: data.content */
#define TMPL_EL_CONTENT     0
/* Simple substitution: data.var, content_item */
#define TMPL_EL_SUBST       1
/* Expression substitution: data.expr, content_item */
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

/* Error message in data.content */
#define TMPL_EL_ERROR		255

#define TMPL_EL_HAS_CONTENT(x) ((x)->type == TMPL_EL_CONTENT || (x)->type == TMPL_EL_ERROR)
#define TMPL_EL_HAS_VAR(x)     ((x)->type == TMPL_EL_SUBST || (x)->type == TMPL_EL_COND || (x)->type == TMPL_EL_LOOP_VAR)
#define TMPL_EL_HAS_EXPR(x)    ((x)->type == TMPL_EL_SUBST_EXPR || (x)->type == TMPL_EL_COND_EXPR)
#define TMPL_EL_HAS_RANGE(x)   ((x)->type == TMPL_EL_LOOP_RANGE)

#define TMPL_EL_HAS_CONTENT_ITEM(x) (                                                                       \
			(x)->type == TMPL_EL_COND || (x)->type == TMPL_EL_COND_EXPR || (x)->type == TMPL_EL_ELSE ||     \
			(x)->type == TMPL_EL_LOOP_RANGE || (x)->type == TMPL_EL_LOOP_VAR || (x)->type == TMPL_LOOP_ELSE \
		)

#define TMPL_EL_HAS_NEXT_COND(x) (                                              \
			(x)->type == TMPL_EL_COND || (x)->type == TMPL_EL_COND_EXPR ||      \
			(x)->type == TMPL_EL_LOOP_RANGE || (x)->type == TMPL_EL_LOOP_VAR || \
		)

typedef struct php_tt_tmpl_el_t {
	int type;
	union {
		struct {
			char *data;
			long len;
		} content;
		struct {
			char *name;
			long len;
		} var;
		php_tt_tmpl_expr *expr;
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


char *tmpl_parse_get_first_tag(const char *tmpl);
char *tmpl_parse_skip_tag(const char *tmpl);
char *tmpl_parse_until_tag(const char *tmpl);
php_tt_tmpl_el *tmpl_parse(const char *tmpl);
void tmpl_dump(php_tt_tmpl_el *tmpl);
void tmpl_free(php_tt_tmpl_el *tmpl);


#endif	/* PHP_TMPL_PARSER_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
