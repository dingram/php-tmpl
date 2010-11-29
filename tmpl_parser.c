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

//#define DEBUG_VAR_DUMP(v, f) do {php_printf(#v " = " f "\n", v);} while (0)
//#define PARSER_DEBUG(m) php_printf(m "\n");
//#define PARSER_DEBUGM(m, ...) php_printf(m "\n", __VA_ARGS__);
#define PARSER_DEBUG(m)
#define PARSER_DEBUGM(m, ...)

#define TMPL_CREATE_EL(x) 							do {                                      \
														(x)=emalloc(sizeof(php_tt_tmpl_el));  \
														memset((x),0,sizeof(php_tt_tmpl_el)); \
													} while (0)                               \

#define PARSER_RECURSE() 							do{PARSER_DEBUG("Recursing...\n");cur->content_item = _tmpl_parse(&curpos, cur TSRMLS_CC);PARSER_DEBUG("Recursion done");} while (0)
#define PARSER_ADVANCE_PAST_TAG() 					do{curpos = tagend + strlen(TMPL_T_POST);} while (0)
#define PARSER_CUR_IS_COND() 						do{                                           \
														enclosure->next_cond = cur;               \
														if (cur==root) {                          \
															TMPL_CREATE_EL(root);                 \
														} else {                                  \
															prev->next = NULL;                    \
														}                                         \
													} while (0)
#define PARSER_DROP_LAST_ELEMENT() 					do{                                       \
														memset(cur,0,sizeof(php_tt_tmpl_el)); \
													} while (0)
#define PARSER_CAPTURE_TAG_CONTENT(content, len) 	do{                                      \
														(len) = tagend - tagstart;           \
														(content) = emalloc((len)+1);        \
														memset((content), 0, (len)+1);       \
														strncpy((content), tagstart, (len)); \
													} while (0)
#define PARSER_RETURN() 							do {                                      \
														*tmpl = curpos;                       \
														if (!root->type) {                    \
															efree(root);                      \
															return NULL;                      \
														}                                     \
														if (!cur->type) {                     \
															efree(cur);                       \
															cur = prev;                       \
															if (cur) cur->next = NULL;        \
														}                                     \
														return root;                          \
													} while (0)
#define PARSER_DUMP(x) 								do {                   \
														PARSER_DEBUG("\033[1;32m>>>\033[m "#x); \
														tmpl_dump(x);                           \
														PARSER_DEBUG("\033[1;32m<<<\033[m "#x); \
													} while (0)

static php_tt_tmpl_el *_tmpl_parse(char const ** tmpl, php_tt_tmpl_el *enclosure TSRMLS_DC) {
	php_tt_tmpl_el *root=NULL, *cur=NULL, *prev=NULL;
	char const *strstart;
	char const *tagstart;
	char const *tagend;
	char const *curpos;

	if (!tmpl) {
		return NULL;
	}

	TMPL_CREATE_EL(root);
	cur = root;
	curpos = strstart = *tmpl;

	do {
		if (cur->type) {
			php_tt_tmpl_el *tmp = NULL;

			// create next element in the chain
			prev = cur;
			TMPL_CREATE_EL(prev->next);
			cur = prev->next;
		}
		// find the next tag
		PARSER_DEBUG("Finding the next tag...");
		tagstart = tmpl_parse_find_tag_open(curpos);
		tagend   = tmpl_parse_find_tag_close(tagstart);
		if (!tagstart || !tagend) {
			// just content remains, so capture that and return
			PARSER_DEBUG("Pure content, capturing");
			cur->type = TMPL_EL_CONTENT;
			cur->data.content.data = estrdup(curpos);
			cur->data.content.len = strlen(curpos);
			curpos += cur->data.content.len;
			PARSER_DEBUGM("Captured content \"%s\"", cur->data.content.data);
			PARSER_RETURN();
		}

		if (tagstart > curpos) {
			// if there is leading content, capture that and restart the loop
			PARSER_DEBUG("Leading content, capturing");
			cur->type = TMPL_EL_CONTENT;
			cur->data.content.len = tagstart-curpos;
			cur->data.content.data = emalloc(cur->data.content.len+1);
			memset(cur->data.content.data, 0, cur->data.content.len+1);
			strncpy(cur->data.content.data, curpos, cur->data.content.len);
			curpos += cur->data.content.len;
			PARSER_DEBUGM("Captured content \"%s\"", cur->data.content.data);
			continue;
		}

		// skip the opening tag characters
		PARSER_DEBUG("Skipping open-tag");
		tagstart += strlen(TMPL_T_PRE);

		if (!strncmp(tagstart, TMPL_T_END, strlen(TMPL_T_END))) {
			PARSER_DEBUG("Section end");
			if (!enclosure) {
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Ignoring orphan end-section at top level");
			} else {
				tagstart += strlen(TMPL_T_END);
				if (!strncmp(tagstart, TMPL_T_COND, strlen(TMPL_T_COND))) {
					if (!TMPL_EL_IS_COND_EX(enclosure))
						php_error_docref(NULL TSRMLS_CC, E_WARNING, "Found end-conditional when expecting to end a loop");
				} else if (!strncmp(tagstart, TMPL_T_LOOP, strlen(TMPL_T_LOOP))) {
					if (!TMPL_EL_IS_LOOP_EX(enclosure))
						php_error_docref(NULL TSRMLS_CC, E_WARNING, "Found end-loop when expecting to end a conditional");
				} else {
					int len;
					char *content;
					PARSER_CAPTURE_TAG_CONTENT(content, len);

					if (len)
						php_error_docref(NULL TSRMLS_CC, E_WARNING, "Tried to end unknown section type \"%s\"", content);
					else
						php_error_docref(NULL TSRMLS_CC, E_STRICT, "Generic end-section tag is discouraged; use either end-conditional or end-loop", content);
					efree(content);
				}
				PARSER_DROP_LAST_ELEMENT();
				PARSER_ADVANCE_PAST_TAG();
				PARSER_RETURN();
			}

		} else if (!strncmp(tagstart, TMPL_T_COND, strlen(TMPL_T_COND))) {
			PARSER_DEBUG("Conditional");
			cur->type = TMPL_EL_COND;
			tagstart += strlen(TMPL_T_COND);

		} else if (!strncmp(tagstart, TMPL_T_ELSE, strlen(TMPL_T_ELSE))) {
			PARSER_DEBUG("Else");
			tagstart += strlen(TMPL_T_ELSE);
			if (!strncmp(tagstart, TMPL_T_COND, strlen(TMPL_T_COND))) {
				PARSER_DEBUG("(if)");
				if (!enclosure) {
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "Ignoring orphan elseif tag at top level");
					PARSER_ADVANCE_PAST_TAG();
					continue;
				}
				cur->type = TMPL_EL_COND;
				tagstart += strlen(TMPL_T_COND);
				PARSER_CAPTURE_TAG_CONTENT(cur->data.var.name, cur->data.var.len);
				PARSER_DEBUGM("Tag content: \"%s\"", cur->data.var.name);
				PARSER_ADVANCE_PAST_TAG();
			} else {
				PARSER_ADVANCE_PAST_TAG();
				if (!enclosure) {
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "Ignoring orphan else tag at top level");
					continue;
				}
				cur->type = TMPL_EL_ELSE;
			}
			PARSER_CUR_IS_COND();
			PARSER_RECURSE();
			PARSER_RETURN();

		} else {
			PARSER_DEBUG("Substitution");
			cur->type = TMPL_EL_SUBST;
		}
		PARSER_CAPTURE_TAG_CONTENT(cur->data.var.name, cur->data.var.len);
		PARSER_DEBUGM("Tag content: \"%s\"", cur->data.var.name);
		PARSER_ADVANCE_PAST_TAG();

		if (TMPL_EL_HAS_CONTENT_ITEM(cur)) {
			PARSER_RECURSE();
		}
	} while (*curpos);

	PARSER_RETURN();
}

/**
 * Go through the tree and post-process to set up ->next for all ->next_cond items
 */
php_tt_tmpl_el *_tmpl_postprocess(php_tt_tmpl_el *tmpl) {
	php_tt_tmpl_el *root = tmpl;
	if (!tmpl) return;
	while (tmpl) {
		php_tt_tmpl_el *tmp = tmpl->next_cond;
		while (tmp) {
			if (tmp->content_item) tmp->content_item = _tmpl_postprocess(tmp->content_item);
			if (!tmp->next_cond) tmp->next = tmpl->next;
			tmp = tmp->next_cond;
		}
		if (tmpl->content_item) tmpl->content_item = _tmpl_postprocess(tmpl->content_item);
		tmpl = tmpl->next;
	}
	return root;
}

php_tt_tmpl_el *tmpl_parse(char const * const tmpl TSRMLS_DC) {
	char const * tmp = tmpl;
	return _tmpl_postprocess(_tmpl_parse(&tmp, NULL TSRMLS_CC));
}

int _tmpl_truthy(char *str) {
	return !(
			!strncasecmp(str, "",      0) ||
			!strncasecmp(str, "0",     0) ||
			!strncasecmp(str, "no",    0) ||
			!strncasecmp(str, "off",   0) ||
			!strncasecmp(str, "false", 0)
		   );
}

int _tmpl_eval_cond(php_tt_tmpl_el *tmpl, HashTable *vars TSRMLS_DC) {
	zval **dest_entry = NULL;
	char *var = tmpl->data.var.name;
	int varlen = tmpl->data.var.len;
	int invert = 0;
	int retval = 0;
	if (tmpl->type == TMPL_EL_ELSE) {
		// always counts as "true"
		return 1;
	}
	if (!var) {
		return 0;
	}
	if (*var == '!') {
		invert = 1;
		++var;
		--varlen;
	}
	if (vars && zend_hash_find(vars, var, varlen+1, (void**)&dest_entry)==SUCCESS) {
		switch (Z_TYPE_PP(dest_entry))
		{
			case IS_NULL:
				retval = 0;
				break;
			case IS_BOOL:
				retval = Z_BVAL_PP(dest_entry);
				break;
			case IS_LONG:
				retval = Z_LVAL_PP(dest_entry) != 0;
				break;
			case IS_DOUBLE:
				retval = Z_DVAL_PP(dest_entry) != 0;
				break;
			case IS_STRING:
				retval = _tmpl_truthy_str(Z_STRVAL_PP(dest_entry));
				break;
			default:
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Type of replacement \"%s\" is unsupported, assuming false", var);
				retval = 0;
				break;
		}
	}
	return invert ? !retval : retval;
}

char *tmpl_use(php_tt_tmpl_el *tmpl, HashTable *vars TSRMLS_DC) {
	php_tt_tmpl_el *cur = NULL;
	php_tt_tmpl_el *cond = NULL;
	smart_str *out = NULL;
	char *final_out = NULL;
	zval **dest_entry = NULL;

	out = emalloc(sizeof(smart_str));
	memset(out, 0, sizeof(smart_str));
	smart_str_0(out);

	for (cur = tmpl; cur; cur = cur->next) {
		switch (cur->type) {
			case TMPL_EL_CONTENT:
				if (*(cur->data.content.data))
					smart_str_appendl(out, cur->data.content.data, cur->data.content.len);
				break;

			case TMPL_EL_SUBST:
				if (!cur->data.var.len) // error?
					break;
				if (vars && zend_hash_find(vars, cur->data.var.name, cur->data.var.len+1, (void**)&dest_entry)==SUCCESS) {
					smart_str_appends(out, Z_STRVAL_PP(dest_entry));
				} else {
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "Could not find a replacement for tag \"%s\"", cur->data.var.name);

					smart_str_appends(out, TMPL_T_PRE);
					smart_str_appendl(out, cur->data.var.name, cur->data.var.len);
					smart_str_appends(out, TMPL_T_POST);
				}
				break;

			case TMPL_EL_COND:
				cond = cur;

				while (cond) {
					if (_tmpl_eval_cond(cond, vars TSRMLS_CC)) {
						if (cond->content_item)
							smart_str_appends(out, tmpl_use(cond->content_item, vars TSRMLS_CC));
						break;
					}
					if (!cond->next_cond) break;
					cond = cond->next_cond;
				}
				while (cond->next_cond) {
					cond = cond->next_cond;
				}
				cur = cond;
				break;
		}
	}

	if (out->c) {
		final_out = estrndup(out->c, out->len);
	} else {
		final_out = estrdup("");
	}
	smart_str_free(out);
	efree(out);
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
				if (tmpl->next_cond->type != TMPL_EL_ELSE) php_printf("%sELSE...\n", ind);
				_tmpl_dump(tmpl->next_cond, ind_lvl);
				efree(ind);
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
	if(tmpl->next) _tmpl_dump(tmpl->next, ind_lvl);
	efree(ind);
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

		if (tmpl->next_cond) {
			// the last next_cond's "next" is equal to the first, so need to
			// make sure we don't try to free twice!
			efree(tmpl);
			tmpl = NULL;
		} else {
			next = tmpl->next;
			efree(tmpl);
			tmpl = next;
		}
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
