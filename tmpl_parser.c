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

static inline char *tmpl_parse_find_tag_open(char const * const tmpl) {
	return tmpl ? strstr(tmpl, TMPL_T_PRE) : NULL;
}

static inline char *tmpl_parse_find_tag_close(char const * const tmpl) {
	return tmpl ? strstr(tmpl, TMPL_T_POST) : NULL;
}

/**
 * Returns a copy of the content of the first tag found in the argument
 * (stripped of begin/end delimiters), or NULL if no tags found.
 */
char *tmpl_parse_get_first_tag(char const * const tmpl) {
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
char *tmpl_parse_skip_tag(char const * const tmpl) {
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
char *tmpl_parse_until_tag(char const * const tmpl) {
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

#define TMPL_CREATE_EL(x) do {(x)=emalloc(sizeof(php_tt_tmpl_el));memset((x),0,sizeof(php_tt_tmpl_el));} while (0)

static php_tt_tmpl_el *_tmpl_parse(char const * const tmpl, int depth) {
	php_tt_tmpl_el *root, *cur;
	char const *tagstart;
	char const *tagend;
	char const *curpos;

	if (!tmpl) {
		return NULL;
	}

	TMPL_CREATE_EL(root);
	cur=root;
	curpos = tmpl;

	do {
		if (curpos > tmpl) {
			TMPL_CREATE_EL(cur->next);
			cur = cur->next;
		}
		tagstart = tmpl_parse_find_tag_open(curpos);
		tagend   = tmpl_parse_find_tag_close(tagstart);
		if (!tagstart || !tagend) {
			cur->type = TMPL_EL_CONTENT;
			cur->data.content.data = estrdup(curpos);
			cur->data.content.len = strlen(curpos);
			return root;
		}

		if (tagstart > curpos) {
			cur->type = TMPL_EL_CONTENT;
			cur->data.content.len = tagstart-curpos;
			cur->data.content.data = emalloc(cur->data.content.len+1);
			memset(cur->data.content.data, 0, cur->data.content.len+1);
			strncpy(cur->data.content.data, curpos, cur->data.content.len);
			curpos += cur->data.content.len;
		} else {
			tagstart += strlen(TMPL_T_PRE);

			if (!strncmp(tagstart, TMPL_T_COND, strlen(TMPL_T_COND))) {
				cur->type = TMPL_EL_COND;
				tagstart += strlen(TMPL_T_COND);
			} else if (!strncmp(tagstart, TMPL_T_ELSE, strlen(TMPL_T_ELSE))) {
				tagstart += strlen(TMPL_T_ELSE);
				if (!strncmp(tagstart, TMPL_T_COND, strlen(TMPL_T_COND))) {
					cur->type = TMPL_EL_COND;
					tagstart += strlen(TMPL_T_COND);
				} else {
					cur->type = TMPL_EL_ELSE;
				}
			} else {
				cur->type = TMPL_EL_SUBST;
			}
			cur->data.var.len = tagend-tagstart;
			cur->data.var.name = emalloc(cur->data.var.len+1);
			memset(cur->data.var.name, 0, cur->data.var.len+1);
			strncpy(cur->data.var.name, tagstart, cur->data.var.len);
			curpos = tagend + strlen(TMPL_T_POST);
		}
	} while (*curpos);

	return root;
}

php_tt_tmpl_el *tmpl_parse(char const * const tmpl) {
	return _tmpl_parse(tmpl, 0);
}

char *tmpl_use(php_tt_tmpl_el *tmpl, HashTable *vars) {
	php_tt_tmpl_el *cur;
	smart_str *out = NULL;
	char *final_out;
	out = emalloc(sizeof(smart_str));
	memset(out, 0, sizeof(smart_str));
	smart_str_0(out);

	for (cur = tmpl; cur; cur = cur->next) {
		switch (cur->type) {
			case TMPL_EL_CONTENT:
				if (*(cur->data.content.data))
					smart_str_appends(out, cur->data.content.data);
				break;
		}
	}

	if (out->c) {
		final_out = estrndup(out->c, out->len);
	} else {
		final_out = estrdup("");
	}
	smart_str_free(out);
	return final_out;
}

static void _tmpl_dump(php_tt_tmpl_el *tmpl, int ind_lvl) {
	if (!tmpl) return;
	char *ind = emalloc(ind_lvl+1);
	if (ind_lvl) memset(ind, '\t', ind_lvl);
	ind[ind_lvl]='\0';
	switch (tmpl->type) {
		case TMPL_EL_CONTENT:
			/* Plain content: data.content */
			php_printf("%sSTRING: (%ld) \"%s\"\n", ind, tmpl->data.content.len, tmpl->data.content.data);
			break;
		case TMPL_EL_SUBST:
			/* Simple substitution: data.var */
			php_printf("%sVALUE-OF: \"%s\"\n", ind, tmpl->data.var.name);
			break;
		case TMPL_EL_SUBST_EXPR:
			/* Expression substitution: data.expr */
			php_printf("Unknown type code %d\n", tmpl->type);
			break;
		case TMPL_EL_COND:
			/* Simple conditional: data.var, next_cond, content_item */
			php_printf("%sIF: \"%s\"\n", ind, tmpl->data.var.name);
			_tmpl_dump(tmpl->content_item, ind_lvl+1);
			if (tmpl->next_cond) {
				// if we have a next condition, then the final next_condition's
				// next will carry us on properly
				php_printf("%sELSE...\n", ind);
				_tmpl_dump(tmpl->next_cond, ind_lvl);
				return;
			}
			break;
		case TMPL_EL_COND_EXPR:
			/* Expression conditional: data.expr, next_cond, content_item */
			php_printf("Unknown type code %d\n", tmpl->type);
			break;
		case TMPL_EL_ELSE:
			/* Else block: content_item */
			php_printf("%sELSE:\n", ind);
			_tmpl_dump(tmpl->content_item, ind_lvl+1);
			break;
		case TMPL_EL_LOOP_RANGE:
			/* Loop over range: data.range, content_item, next_cond */
			php_printf("Unknown type code %d\n", tmpl->type);
			break;
		case TMPL_EL_LOOP_VAR:
			/* Loop over array var: data.var, content_item, next_cond */
			php_printf("Unknown type code %d\n", tmpl->type);
			break;
		case TMPL_EL_LOOP_ELSE:
			/* Loop "else": content_item */
			php_printf("Unknown type code %d\n", tmpl->type);
			break;
		case TMPL_EL_ERROR:
			/* Error message: data.content */
			php_printf("%sERROR: %s\n", ind, tmpl->data.content.data);
			break;
		default:
			php_printf("Unknown type code %d\n", tmpl->type);
			break;
	}
	if(tmpl->next) tmpl_dump(tmpl->next);
}

void tmpl_dump(php_tt_tmpl_el *tmpl) {
	_tmpl_dump(tmpl, 0);
}

void tmpl_free(php_tt_tmpl_el *tmpl) {
	php_tt_tmpl_el *next = NULL;
	while (tmpl) {
		if (tmpl->content_item)
			tmpl_free(tmpl->content_item);
		if (tmpl->next_cond)
			tmpl_free(tmpl->next_cond);
		if (TMPL_EL_HAS_CONTENT(tmpl) && tmpl->data.content.data)
			efree(tmpl->data.content.data);
		if (TMPL_EL_HAS_VAR(tmpl) && tmpl->data.var.name)
			efree(tmpl->data.var.name);
		if (TMPL_EL_HAS_EXPR(tmpl) )
			tmpl_expr_free(tmpl->data.expr);

		next = tmpl->next;
		efree(tmpl);
		tmpl = next;
	}
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
