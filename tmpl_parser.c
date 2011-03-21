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

//#define DEBUG_VAR_DUMP(v, f) do {php_printf(#v " = " f "\n", v);} while (0)

#ifdef _TMPL_PARSER_DEBUG
#define PARSER_DEBUG(m) php_printf(m "\n")
#define PARSER_DEBUGM(m, ...) php_printf(m "\n", __VA_ARGS__)
#else
#define PARSER_DEBUG(m)
#define PARSER_DEBUGM(m, ...)
#endif

#ifdef _TMPL_RENDER_DEBUG
#define RENDER_DEBUG(m) php_printf(m "\n")
#define RENDER_DEBUGM(m, ...) php_printf(m "\n", __VA_ARGS__)
#else
#define RENDER_DEBUG(m)
#define RENDER_DEBUGM(m, ...)
#endif

#ifdef _TMPL_ALLOC_DEBUG
#define ALLOC_DEBUG(m) php_printf(m "\n")
#define ALLOC_DEBUGM(m, ...) php_printf(m "\n", __VA_ARGS__)
#else
#define ALLOC_DEBUG(m)
#define ALLOC_DEBUGM(m, ...)
#endif

#define TMPL_CREATE_EL(x) 							do {                                      \
														(x)=emalloc(sizeof(php_tt_tmpl_el));  \
														memset((x),0,sizeof(php_tt_tmpl_el)); \
														++((x)->refcount);                    \
													} while (0)                               \

#define PARSER_RECURSE() 							do{cur->content_item = _tmpl_parse(&curpos, strend-curpos, cur TSRMLS_CC);} while (0)
#define PARSER_ADVANCE_PAST_TAG() 					do{curpos = tagend + strlen(TMPL_T_POST);} while (0)
#define PARSER_CUR_IS_COND() 						do{                                           \
														enclosure->next_cond = cur;               \
														if (cur==root) {                          \
															TMPL_CREATE_EL(root);                 \
														} else {                                  \
															prev->next = NULL;                    \
														}                                         \
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

static int _tmpl_parse_int(const char **start, long *target) {
	smart_str stmp = {0};
	smart_str_0(&stmp);

	if (**start && **start == '-')
		smart_str_appendc(&stmp, *((*start)++));
	while (**start && !(**start < '0' || **start > '9'))
		smart_str_appendc(&stmp, *((*start)++));

	if (stmp.len && ((*(stmp.c) != '-') || (*(stmp.c) == '-' && stmp.len>1))) {
		*target = atoi(stmp.c);
		smart_str_free(&stmp);
		return 1;
	}
	smart_str_free(&stmp);
	return 0;
}

static php_tt_tmpl_el *_tmpl_parse(char const ** tmpl, int len, php_tt_tmpl_el *enclosure TSRMLS_DC) {
	php_tt_tmpl_el *root=NULL, *cur=NULL, *prev=NULL;
	char const *strstart;
	char const *strend;
	char const *tagstart;
	char const *tagend;
	char const *curpos;
	char *tmppos;

	if (!tmpl) {
		return NULL;
	}

	curpos = strstart = *tmpl;
	strend = strstart + len;

	if (curpos > strend) {
		return NULL;
	}

	TMPL_CREATE_EL(root);
	cur = root;

	PARSER_DEBUGM("string start: %p", strstart);
	PARSER_DEBUGM("string end: %p", strstart+strlen(strstart));

	do {
		PARSER_DEBUG("Start of loop");
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
			PARSER_DEBUG("\tPure content, capturing");
			cur->type = TMPL_EL_CONTENT;
			cur->data.content.data = estrdup(curpos);
			cur->data.content.len = strlen(curpos);
			curpos += cur->data.content.len;
			PARSER_DEBUGM("\t\tCaptured content \"%s\"", cur->data.content.data);
			PARSER_RETURN();
		}

		if (tagstart > curpos) {
			// if there is leading content, capture that and restart the loop
			PARSER_DEBUG("\tLeading content, capturing");
			cur->type = TMPL_EL_CONTENT;
			cur->data.content.len = tagstart-curpos;
			cur->data.content.data = emalloc(cur->data.content.len+1);
			memset(cur->data.content.data, 0, cur->data.content.len+1);
			strncpy(cur->data.content.data, curpos, cur->data.content.len);
			curpos += cur->data.content.len;
			PARSER_DEBUGM("\t\tCaptured content \"%s\"", cur->data.content.data);
			continue;
		}

		// skip the opening tag characters
		PARSER_DEBUG("\tSkipping open-tag");
		tagstart += strlen(TMPL_T_PRE);

		if (!strncmp(tagstart, TMPL_T_END, strlen(TMPL_T_END))) {
			PARSER_DEBUG("\tSection end");
			if (!enclosure) {
				PARSER_DEBUG("\t\tNo enclosure");
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Ignoring orphan end-section at top level");
			} else {
				PARSER_DEBUG("\t\tSection end");
				tagstart += strlen(TMPL_T_END);
				PARSER_DEBUG("\t\tSection end");
				if (!strncmp(tagstart, TMPL_T_COND, strlen(TMPL_T_COND))) {
					PARSER_DEBUG("\t\tEnd conditional");
					if (!TMPL_EL_IS_COND_EX(enclosure))
						php_error_docref(NULL TSRMLS_CC, E_WARNING, "Found unexpected end-conditional");
				} else if (!strncmp(tagstart, TMPL_T_LOOP, strlen(TMPL_T_LOOP))) {
					PARSER_DEBUG("\t\tEnd loop");
					if (!TMPL_EL_IS_LOOP_EX(enclosure))
						php_error_docref(NULL TSRMLS_CC, E_WARNING, "Found unexpected end-loop");
				} else {
					PARSER_DEBUG("\t\tUnrecognised end-section");
					int len;
					char *content;
					PARSER_CAPTURE_TAG_CONTENT(content, len);

					if (len)
						php_error_docref(NULL TSRMLS_CC, E_WARNING, "Tried to end unknown section type \"%s\"", content);
					else
						php_error_docref(NULL TSRMLS_CC, E_STRICT, "Generic end-section tag is discouraged; use either end-conditional or end-loop", content);
					efree(content);
				}
				PARSER_DEBUG("\t\tAdvancing past tag");
				PARSER_ADVANCE_PAST_TAG();
				PARSER_DEBUG("\t\tReturning");
				PARSER_RETURN();
			}

		} else if (!strncmp(tagstart, TMPL_T_COMMENT, strlen(TMPL_T_COMMENT))) {
			PARSER_ADVANCE_PAST_TAG();
			continue;

		} else if (!strncmp(tagstart, TMPL_T_COND, strlen(TMPL_T_COND))) {
			PARSER_DEBUG("\tConditional");
			cur->type = TMPL_EL_COND;
			tagstart += strlen(TMPL_T_COND);

		} else if (!strncmp(tagstart, TMPL_T_ELSE, strlen(TMPL_T_ELSE))) {
			PARSER_DEBUG("\tElse");
			tagstart += strlen(TMPL_T_ELSE);
			if (!strncmp(tagstart, TMPL_T_COND, strlen(TMPL_T_COND))) {
				PARSER_DEBUG("\t(if)");
				if (!enclosure) {
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "Ignoring orphan elseif tag at top level");
					PARSER_ADVANCE_PAST_TAG();
					continue;
				}
				cur->type = TMPL_EL_COND;
				tagstart += strlen(TMPL_T_COND);
				PARSER_CAPTURE_TAG_CONTENT(cur->data.var.name, cur->data.var.len);
				PARSER_DEBUGM("\t\tTag content: \"%s\"", cur->data.var.name);
				PARSER_DEBUG("\t\tAdvancing past tag");
				PARSER_ADVANCE_PAST_TAG();
			} else {
				PARSER_DEBUG("\t\tAdvancing past tag");
				PARSER_ADVANCE_PAST_TAG();
				if (!enclosure) {
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "Ignoring orphan else tag at top level");
					continue;
				}
				cur->type = TMPL_EL_ELSE;
			}
			PARSER_DEBUG("\t\tAttaching to next_cond");
			PARSER_CUR_IS_COND();
			if (cur->type == TMPL_EL_ELSE)
				PARSER_DEBUG(">>> Recursing from ELSE");
			else
				PARSER_DEBUGM(">>> Recursing from ELSEIF %s", cur->data.var.name);
			PARSER_RECURSE();
			if (cur->type == TMPL_EL_ELSE)
				PARSER_DEBUG("<<< Recursion from ELSE done");
			else
				PARSER_DEBUGM("<<< Recursion from ELSEIF %s done", cur->data.var.name);
			PARSER_DEBUG("\t\tReturning...");
			PARSER_RETURN();

		} else if (!strncmp(tagstart, TMPL_T_LOOP, strlen(TMPL_T_LOOP))) {
			PARSER_DEBUG("\tLoop");
			tagstart += strlen(TMPL_T_LOOP);
			if (!strncmp(tagstart, TMPL_T_ELSE, strlen(TMPL_T_ELSE))) {
				PARSER_DEBUG("\t\tLoop-else");
				cur->type = TMPL_EL_LOOP_ELSE;
				PARSER_DEBUG("\t\tAttaching to next_cond");
				PARSER_CUR_IS_COND();
				PARSER_DEBUG("\t\tAdvancing past tag");
				PARSER_ADVANCE_PAST_TAG();
				PARSER_DEBUG(">>> Recursing from LOOP-ELSE");
				PARSER_RECURSE();
				PARSER_DEBUG("<<< Recursion from LOOP-ELSE done");
				PARSER_DEBUG("\t\tReturning...");
				PARSER_RETURN();
			} else if (*tagstart == '(') {
				// range, of the format '(' num [ ',' num ] '..' num ')'
				PARSER_DEBUG("\tFor");
				cur->type = TMPL_EL_LOOP_RANGE;

				++tagstart;

				// parse range begin
				if (!_tmpl_parse_int(&tagstart, &(cur->data.range.begin))) {
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to parse loop start value");
					memset(cur, 0, sizeof(php_tt_tmpl_el));
					ADVANCE_PAST_TAG();
					continue;
				}
				PARSER_DEBUGM("\t\tStart: %ld", cur->data.range.begin);
				if (*tagstart == ',') {
					// parse range next-val
					++tagstart;
					if (!_tmpl_parse_int(&tagstart, &(cur->data.range.step))) {
						php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to parse loop next value");
						memset(cur, 0, sizeof(php_tt_tmpl_el));
						ADVANCE_PAST_TAG();
						continue;
					}
					cur->data.range.step -= cur->data.range.begin;
					if (!cur->data.range.step)
						cur->data.range.step = 1;
					PARSER_DEBUGM("\t\tStep: %ld", cur->data.range.step);
				} else if (*tagstart != '.') {
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to parse loop range: next character must either be \".\" or \",\"");
					memset(cur, 0, sizeof(php_tt_tmpl_el));
					ADVANCE_PAST_TAG();
					continue;
				}

				// default step size
				if (!cur->data.range.step)
					cur->data.range.step = 1;

				// parse loop end value
				if (*tagstart == '.' && *(tagstart+1) == '.') {
					tagstart += 2;
					if (!_tmpl_parse_int(&tagstart, &(cur->data.range.end))) {
						php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to parse loop end value");
						memset(cur, 0, sizeof(php_tt_tmpl_el));
						ADVANCE_PAST_TAG();
						continue;
					}
					PARSER_DEBUGM("\t\tEnd: %ld", cur->data.range.end);
				} else {
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to parse loop range: next characters must be \"..\"");
					memset(cur, 0, sizeof(php_tt_tmpl_el));
					ADVANCE_PAST_TAG();
					continue;
				}

				PARSER_DEBUGM("\t\tEnd of spec", cur->data.range.end);
				// make sure the step is sensibly signed
				if ((cur->data.range.begin < cur->data.range.end && cur->data.range.step < 0) || (cur->data.range.begin > cur->data.range.end && cur->data.range.step > 0)) {
					cur->data.range.step *= -1;
				}

				if (*tagstart != ')' || tagstart + 1 > tagend) {
					php_error_docref(NULL TSRMLS_CC, E_WARNING, "Failed to parse loop range: range spec must end with \")\"");
					memset(cur, 0, sizeof(php_tt_tmpl_el));
					ADVANCE_PAST_TAG();
					continue;
				}

			} else {
				// variable
				PARSER_DEBUG("\tForeach");
				cur->type = TMPL_EL_LOOP_VAR;
			}

		} else {
			PARSER_DEBUG("\tSubstitution");
			cur->type = TMPL_EL_SUBST;
		}

		// capture rest of tag
		if (TMPL_EL_HAS_VAR(cur)) {
			PARSER_CAPTURE_TAG_CONTENT(cur->data.var.name, cur->data.var.len);
			if (cur->type == TMPL_EL_SUBST && (tmppos = strchr(cur->data.var.name, '|')) != NULL) {
				*tmppos = '\0';
				++tmppos;
				cur->data.var.dval = estrdup(tmppos);
				cur->data.var.dlen = strlen(cur->data.var.dval);
				tmppos = cur->data.var.name;
				cur->data.var.name = estrdup(tmppos);
				cur->data.var.len = strlen(cur->data.var.name);
				efree(tmppos);
			}
			PARSER_DEBUGM("\t\tTag content: \"%s\"", cur->data.var.name);
		}
		PARSER_ADVANCE_PAST_TAG();
		PARSER_DEBUG("\t\tAdvanced past tag");

		if (TMPL_EL_HAS_CONTENT_ITEM(cur)) {
			PARSER_DEBUGM(">>> Recursing", cur->data.var.name);
			PARSER_RECURSE();
			PARSER_DEBUGM("<<< Recursion done", cur->data.var.name);
		}
	} while (curpos<strend && *curpos);

	PARSER_RETURN();
}

/**
 * Go through the tree and post-process to set up ->next for all ->next_cond items
 */
php_tt_tmpl_el *_tmpl_postprocess(php_tt_tmpl_el *tmpl) {
	ALLOC_DEBUGM(">>> POSTPROCESS begin: %p", tmpl);
	php_tt_tmpl_el *root = tmpl;
	if (!tmpl) return;
	while (tmpl) {
		ALLOC_DEBUGM("*** POSTPROCESS loop start: %p", tmpl);
		php_tt_tmpl_el *tmp = tmpl->next_cond;
		ALLOC_DEBUGM("*** POSTPROCESS next_cond of %p is %p", tmpl, tmp);
		while (tmp) {
			if (tmp->content_item) {
				ALLOC_DEBUGM("*** POSTPROCESS content_item of %p is %p", tmp, tmp->content_item);
				tmp->content_item = _tmpl_postprocess(tmp->content_item);
			}
			if (!tmp->next_cond) {
				ALLOC_DEBUGM("*** POSTPROCESS no next_cond of %p", tmp);
				if (tmpl->next) {
					ALLOC_DEBUGM("*** POSTPROCESS tmp[%p]->next := tmpl[%p]->next[%p]", tmp, tmpl, tmpl->next);
					tmp->next = tmpl->next;
					ALLOC_DEBUGM("+++ POSTPROCESS ++(tmp[%p]->next[%p]->refcount[%u])", tmp, tmp->next, tmp->next->refcount);
					++(tmp->next->refcount);
				}
			}
			ALLOC_DEBUGM("*** POSTPROCESS next_cond of %p is %p", tmp, tmp->next_cond);
			tmp = tmp->next_cond;
		}
		if (tmpl->content_item) {
			ALLOC_DEBUGM("*** POSTPROCESS content_item of %p is %p", tmpl, tmpl->content_item);
			tmpl->content_item = _tmpl_postprocess(tmpl->content_item);
		}
		tmpl = tmpl->next;
	}
	ALLOC_DEBUGM("<<< POSTPROCESS end: %p", tmpl);
	return root;
}

php_tt_tmpl_el *tmpl_parse(char const * const tmpl, int len TSRMLS_DC) {
	char const * tmp = tmpl;
	PARSER_DEBUG("Starting parse");
	php_tt_tmpl_el *ret = _tmpl_parse(&tmp, len, NULL TSRMLS_CC);
	PARSER_DEBUG("Parse complete");
	//PARSER_DUMP(ret);
	ret = _tmpl_postprocess(ret);
	PARSER_DEBUG("Postprocess complete");
	//PARSER_DUMP(ret);
	return ret;
}

int _tmpl_truthy_str(char *str) {
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
				retval = Z_LVAL_PP(dest_entry) ? 1 : 0;
				break;
			case IS_DOUBLE:
				retval = Z_DVAL_PP(dest_entry) ? 1 : 0;
				break;
			case IS_STRING:
				retval = _tmpl_truthy_str(Z_STRVAL_PP(dest_entry));
				break;
			case IS_ARRAY:
				retval = zend_hash_num_elements(Z_ARRVAL_PP(dest_entry)) ? 1 : 0;
				break;
			case IS_OBJECT:
				php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Replacement for \"%s\" is an object, which is always true", var);
				retval = 1;
				break;
			default:
				php_error_docref(NULL TSRMLS_CC, E_WARNING, "Type of replacement for \"%s\" is unsupported, assuming false", var);
				retval = 0;
				break;
		}
	}
	return invert ? !retval : retval;
}

#define TMPL_INIT_ITER_VAR()	do { \
									ALLOC_HASHTABLE(tmp_vars); \
									zend_hash_init(tmp_vars, 0, NULL, ZVAL_PTR_DTOR, 0); \
									zend_hash_copy(tmp_vars, vars, (copy_ctor_func_t) zval_add_ref, (void *) &tmp_item, sizeof(zval *)); \
									tmp_item = NULL; \
									smart_str_free(&iter_var_name); \
									 \
									smart_str_appendc(&iter_var_name, '$'); \
									smart_str_0(&iter_var_name); \
									 \
									while (zend_hash_exists(vars, iter_var_name.c, iter_var_name.len+1)) { \
										smart_str_appendc(&iter_var_name, '$'); \
										smart_str_0(&iter_var_name); \
									} \
								} while (0)

#define TMPL_DESTROY_ITER_VAR()	do { \
									smart_str_free(&iter_var_name); \
									zend_hash_destroy(tmp_vars); \
									FREE_HASHTABLE(tmp_vars); \
								} while (0)

char *tmpl_use(php_tt_tmpl_el *tmpl, HashTable *vars TSRMLS_DC) {
	php_tt_tmpl_el *cur = NULL;
	php_tt_tmpl_el *cond = NULL;
	smart_str out = {0};
	char *final_out = NULL;
	zval **dest_entry = NULL;
	zval *tmp_item = NULL;
	int iterations = 0;
	long i=0;
	HashTable *tmp_vars;
	smart_str iter_var_name = {0};

	RENDER_DEBUG("Starting render");

	smart_str_0(&out);

	for (cur = tmpl; cur; cur = cur->next) {
#ifdef _TMPL_RENDER_DEBUG
		smart_str_0(&out);
#endif
		RENDER_DEBUGM("Start of iteration: (%lu) \"%s\"", out.len, out.c);
		switch (cur->type) {
			case TMPL_EL_CONTENT:
				if (*(cur->data.content.data)) {
					RENDER_DEBUGM("LITERAL: \"%s\"", cur->data.content.data);
					smart_str_appendl(&out, cur->data.content.data, cur->data.content.len);
				}
				break;

			case TMPL_EL_SUBST:
				if (!cur->data.var.len) // error?
					break;
				if (vars && zend_hash_find(vars, cur->data.var.name, cur->data.var.len+1, (void**)&dest_entry)==SUCCESS) {
					RENDER_DEBUGM("VAR: \"%s\"", cur->data.var.name);
					char *str = NULL;
					int len = 0, free_str = 1, free_zval = 0;
					zval str_tmp;
					switch (Z_TYPE_PP(dest_entry)) {
						case IS_NULL:
							RENDER_DEBUGM("  value: NULL", len, str);
							break;
						case IS_BOOL:
							len = spprintf(&str, 0, "%s", Z_LVAL_PP(dest_entry) ? "true" : "false");
							RENDER_DEBUGM("  value: (%d) \"%s\"", len, str);
							break;
						case IS_LONG:
							len = spprintf(&str, 0, "%ld", Z_LVAL_PP(dest_entry));
							RENDER_DEBUGM("  value: (%d) \"%s\"", len, str);
							break;
						case IS_STRING:
							free_str = 0;
							str = Z_STRVAL_PP(dest_entry);
							len = Z_STRLEN_PP(dest_entry);
							RENDER_DEBUGM("  value: (%d) \"%s\"", len, str);
							break;
						case IS_DOUBLE:
						case IS_ARRAY:
						case IS_OBJECT:
							zend_make_printable_zval(*dest_entry, &str_tmp, &free_zval);
							free_str = 0;
							str = Z_STRVAL(str_tmp);
							len = Z_STRLEN(str_tmp);
							RENDER_DEBUGM("  value: (%d) \"%s\"", len, str);
							break;
						case IS_RESOURCE:
							php_error_docref(NULL TSRMLS_CC, E_NOTICE, "Type of replacement for \"%s\" is resource; you probably don't want this", cur->data.var.name);
							str = estrdup("[Resource]");
							RENDER_DEBUGM("  value: (%d) \"%s\"", len, str);
							break;
						default:
							php_error_docref(NULL TSRMLS_CC, E_WARNING, "Type of replacement for \"%s\" is unsupported; ignoring", cur->data.var.name);
							break;
					}
					if (str) {
						if (len) {
							smart_str_appendl(&out, str, len);
						} else {
							smart_str_appends(&out, str);
						}
						if (free_str) {
							efree(str);
						}
						if (free_zval) {
							zval_dtor(&str_tmp);
						}
					}
				} else {
					if (!cur->data.var.dval) {
						php_error_docref(NULL TSRMLS_CC, E_WARNING, "Could not find a replacement for \"%s\"", cur->data.var.name);

						smart_str_appends(&out, TMPL_T_PRE);
						smart_str_appendl(&out, cur->data.var.name, cur->data.var.len);
						smart_str_appends(&out, TMPL_T_POST);
					} else {
						smart_str_appendl(&out, cur->data.var.dval, cur->data.var.dlen);
					}
				}
				break;

			case TMPL_EL_COND:
				cond = cur;

				while (cond) {
					if (_tmpl_eval_cond(cond, vars TSRMLS_CC)) {
						if (cond->content_item) {
							char *tmp;
							RENDER_DEBUG(">>> Recursing...");
							tmp = tmpl_use(cond->content_item, vars TSRMLS_CC);
							RENDER_DEBUG("<<< Unrecursing...");
							smart_str_appends(&out, tmp);
							efree(tmp);
						}
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

			case TMPL_EL_LOOP_VAR:

				TMPL_INIT_ITER_VAR();

				iterations = 0;

				if (vars && zend_hash_find(vars, cur->data.var.name, cur->data.var.len+1, (void**)&dest_entry)==SUCCESS && Z_TYPE_PP(dest_entry) == IS_ARRAY) {
					zval **cur_val;
					char *key = NULL;
					char *cur_key;
					uint cur_key_len;
					int hash_key_type;
					ulong num_index;
					HashPosition pos;
					char *tmp;

					smart_str skey={0};

					smart_str_appendl(&skey, iter_var_name.c, iter_var_name.len);
					smart_str_appendc(&skey, '@');
					smart_str_0(&skey);

					for (zend_hash_internal_pointer_reset_ex(HASH_OF(*dest_entry), &pos);
							HASH_KEY_NON_EXISTANT != (hash_key_type = zend_hash_get_current_key_ex(HASH_OF(*dest_entry), &cur_key, &cur_key_len, &num_index, 0, &pos));
							zend_hash_move_forward_ex(HASH_OF(*dest_entry), &pos)) {
						zend_hash_get_current_data_ex(HASH_OF(*dest_entry), (void *)&cur_val, &pos);
						smart_str s = {0};

						switch (hash_key_type) {
							case HASH_KEY_IS_STRING:
								key = cur_key;
								break;
							case HASH_KEY_IS_LONG:
								smart_str_append_unsigned(&s, num_index);
								smart_str_0(&s);
								key = s.c;
								cur_key_len = s.len;
								break;
							default:
								continue;
						}

						++iterations;

						// set key-var
						MAKE_STD_ZVAL(tmp_item);
						ZVAL_STRINGL(tmp_item, key, cur_key_len-1, 0);
						Z_ADDREF_P(tmp_item);
						RENDER_DEBUGM("Key: (%d) \"%s\"", Z_STRLEN_P(tmp_item), Z_STRVAL_P(tmp_item));
						zend_hash_update(tmp_vars, skey.c, skey.len+1, (void **)&tmp_item, sizeof(zval *), NULL);

						// set val-var
						Z_ADDREF_PP(cur_val);
						zend_hash_update(tmp_vars, iter_var_name.c, iter_var_name.len+1, (void **)cur_val, sizeof(zval *), NULL);

						RENDER_DEBUG(">>> Recursing...");
						tmp = tmpl_use(cur->content_item, tmp_vars TSRMLS_CC);
						RENDER_DEBUG("<<< Unrecursing...");

						zend_hash_del(tmp_vars, skey.c, skey.len+1);

						Z_DELREF_PP(cur_val);
						FREE_ZVAL(tmp_item);

						smart_str_appends(&out, tmp);
						efree(tmp);
						smart_str_free(&s);
					}

					smart_str_free(&skey);
				}

				TMPL_DESTROY_ITER_VAR();

				if (!iterations && cur->next_cond) {
					char *tmp;
					cur = cur->next_cond;
					RENDER_DEBUG(">>> Recursing...");
					tmp = tmpl_use(cur->content_item, vars TSRMLS_CC);
					RENDER_DEBUG("<<< Unrecursing...");
					smart_str_appends(&out, tmp);
					efree(tmp);
				}

				while (cur->next_cond) {
					cur = cur->next_cond;
				}
				break;

			case TMPL_EL_LOOP_RANGE:

				TMPL_INIT_ITER_VAR();

				iterations = 0;

				MAKE_STD_ZVAL(tmp_item);
				Z_TYPE_P(tmp_item) = IS_LONG;
				Z_LVAL_P(tmp_item) = 0;

				zend_hash_update(tmp_vars, iter_var_name.c, iter_var_name.len+1, (void **)&tmp_item, sizeof(zval *), NULL);

				if (cur->data.range.begin < cur->data.range.end) {
					char *tmp;
					for (i = cur->data.range.begin; i <= cur->data.range.end; i += cur->data.range.step) {
						++iterations;
						Z_LVAL_P(tmp_item) = i;
						RENDER_DEBUG(">>> Recursing...");
						tmp = tmpl_use(cur->content_item, tmp_vars TSRMLS_CC);
						RENDER_DEBUG("<<< Unrecursing...");
						smart_str_appends(&out, tmp);
						efree(tmp);
					}
				} else {
					char *tmp;
					// NOTE: step is negative in this case
					for (i = cur->data.range.begin; i >= cur->data.range.end; i += cur->data.range.step) {
						++iterations;
						Z_LVAL_P(tmp_item) = i;
						RENDER_DEBUG(">>> Recursing...");
						tmp = tmpl_use(cur->content_item, tmp_vars TSRMLS_CC);
						RENDER_DEBUG("<<< Unrecursing...");
						smart_str_appends(&out, tmp);
						efree(tmp);
					}
				}

				TMPL_DESTROY_ITER_VAR();

				if (!iterations && cur->next_cond) {
					char *tmp;
					cur = cur->next_cond;
					RENDER_DEBUG(">>> Recursing...");
					tmp = tmpl_use(cur->content_item, vars TSRMLS_CC);
					RENDER_DEBUG("<<< Unrecursing...");
					smart_str_appends(&out, tmp);
					efree(tmp);
				}

				while (cur->next_cond) {
					cur = cur->next_cond;
				}
				break;
		}
	}

	if (out.c) {
		// character 17 is being modified?!
		smart_str_0(&out);
		RENDER_DEBUGM("Result: (%lu) \"%.30s\"", out.len, out.c);
		final_out = estrdup(out.c);
	} else {
		RENDER_DEBUGM("Result: (0) \"\"", out.len, out.c);
		final_out = estrdup("");
	}
	smart_str_free(&out);
	RENDER_DEBUG("Render complete");
	return final_out;
}

void _tmpl_to_string(php_tt_tmpl_el *tmpl, smart_str *out) {
	php_tt_tmpl_el *cur = NULL;
	php_tt_tmpl_el *cond = NULL;

	for (cur = tmpl; cur; cur = cur->next) {
		if (cur->type == TMPL_EL_CONTENT) {
			// optimise for the simple case
			if (*(cur->data.content.data))
				smart_str_appendl(out, cur->data.content.data, cur->data.content.len);
			continue;
		}

		smart_str_appends(out, TMPL_T_PRE);

		switch (cur->type) {
			case TMPL_EL_SUBST:
				smart_str_appendl(out, cur->data.var.name, cur->data.var.len);
				if (cur->data.var.dval) {
					smart_str_appends(out, TMPL_T_DEFAULT);
					smart_str_appendl(out, cur->data.var.dval, cur->data.var.dlen);
				}
				break;
			case TMPL_EL_ELSE:
				smart_str_appends(out, TMPL_T_ELSE);
				smart_str_appends(out, TMPL_T_POST);
				_tmpl_to_string(cur->content_item, out);
				smart_str_appends(out, TMPL_T_PRE);
				smart_str_appends(out, TMPL_T_END);
				smart_str_appends(out, TMPL_T_COND);
				break;
			case TMPL_EL_COND:
				smart_str_appends(out, TMPL_T_COND);
				smart_str_appendl(out, cur->data.var.name, cur->data.var.len);
				smart_str_appends(out, TMPL_T_POST);
				_tmpl_to_string(cur->content_item, out);
				while (cur->next_cond) {
					cur = cur->next_cond;
					if (cur->type == TMPL_EL_COND) {
						smart_str_appends(out, TMPL_T_PRE);
						smart_str_appends(out, TMPL_T_ELSE);
						smart_str_appends(out, TMPL_T_COND);
						smart_str_appendl(out, cur->data.var.name, cur->data.var.len);
						smart_str_appends(out, TMPL_T_POST);
						_tmpl_to_string(cur->content_item, out);
					} else if (cur->type == TMPL_EL_ELSE) {
						smart_str_appends(out, TMPL_T_PRE);
						smart_str_appends(out, TMPL_T_ELSE);
						smart_str_appends(out, TMPL_T_POST);
						_tmpl_to_string(cur->content_item, out);
					} else {
						php_printf("WTF, srsly\n");
					}
				}
				smart_str_appends(out, TMPL_T_PRE);
				smart_str_appends(out, TMPL_T_END);
				smart_str_appends(out, TMPL_T_COND);
				break;
			case TMPL_EL_LOOP_VAR:
				smart_str_appends(out, TMPL_T_LOOP);
				smart_str_appendl(out, cur->data.var.name, cur->data.var.len);
				smart_str_appends(out, TMPL_T_POST);
				_tmpl_to_string(cur->content_item, out);
				while (cur->next_cond) {
					cur = cur->next_cond;
					if (cur->type == TMPL_EL_LOOP_ELSE) {
						smart_str_appends(out, TMPL_T_PRE);
						smart_str_appends(out, TMPL_T_LOOP);
						smart_str_appends(out, TMPL_T_ELSE);
						smart_str_appends(out, TMPL_T_POST);
						_tmpl_to_string(cur->content_item, out);
					} else {
						php_printf("WTF, srsly\n");
					}
				}
				smart_str_appends(out, TMPL_T_PRE);
				smart_str_appends(out, TMPL_T_END);
				smart_str_appends(out, TMPL_T_LOOP);
				break;
			case TMPL_EL_LOOP_RANGE:
				smart_str_appends(out, TMPL_T_LOOP);
				smart_str_appendc(out, '(');
				smart_str_append_long(out, cur->data.range.begin);
				if (cur->data.range.step != 1 && cur->data.range.step != -1) {
					smart_str_appendc(out, ',');
					smart_str_append_long(out, cur->data.range.begin + cur->data.range.step);
				}
				smart_str_appendl(out, "..", sizeof("..")-1);
				smart_str_append_long(out, cur->data.range.end);
				smart_str_appendc(out, ')');
				smart_str_appends(out, TMPL_T_POST);
				_tmpl_to_string(cur->content_item, out);
				while (cur->next_cond) {
					cur = cur->next_cond;
					if (cur->type == TMPL_EL_LOOP_ELSE) {
						smart_str_appends(out, TMPL_T_PRE);
						smart_str_appends(out, TMPL_T_LOOP);
						smart_str_appends(out, TMPL_T_ELSE);
						smart_str_appends(out, TMPL_T_POST);
						_tmpl_to_string(cur->content_item, out);
					} else {
						php_printf("WTF, srsly\n");
					}
				}
				smart_str_appends(out, TMPL_T_PRE);
				smart_str_appends(out, TMPL_T_END);
				smart_str_appends(out, TMPL_T_LOOP);
				break;
			case TMPL_EL_LOOP_ELSE:
				smart_str_appends(out, TMPL_T_LOOP);
				smart_str_appends(out, TMPL_T_ELSE);
				smart_str_appends(out, TMPL_T_POST);
				_tmpl_to_string(cur->content_item, out);
				smart_str_appends(out, TMPL_T_PRE);
				smart_str_appends(out, TMPL_T_END);
				smart_str_appends(out, TMPL_T_LOOP);
				break;
		}
		smart_str_appends(out, TMPL_T_POST);
	}
	smart_str_0(out);
}

char *tmpl_to_string(php_tt_tmpl_el *tmpl) {
	smart_str out = {0};
	char *final_out;

	smart_str_0(&out);

	_tmpl_to_string(tmpl, &out);

	smart_str_0(&out);

	if (out.c) {
		final_out = estrndup(out.c, out.len);
	} else {
		final_out = estrdup("");
	}
	smart_str_free(&out);
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
#ifdef _TMPL_ALLOC_DEBUG
			php_printf("%s[rc=%u][%p]STRING: (%ld) \"%s\"\n", ind, tmpl->refcount, tmpl, tmpl->data.content.len, tmpl->data.content.data);
#else
			php_printf("%s[rc=%u]STRING: (%ld) \"%s\"\n", ind, tmpl->refcount, tmpl->data.content.len, tmpl->data.content.data);
#endif
			break;
		case TMPL_EL_SUBST:
			/* Simple substitution: data.var */
			if (tmpl->data.var.dval) {
#ifdef _TMPL_ALLOC_DEBUG
				php_printf("%s[rc=%u][%p]VALUE-OF: \"%s\", default \"%s\"\n", ind, tmpl->refcount, tmpl, tmpl->data.var.name, tmpl->data.var.dval);
#else
				php_printf("%s[rc=%u]VALUE-OF: \"%s\", default \"%s\"\n", ind, tmpl->refcount, tmpl->data.var.name, tmpl->data.var.dval);
#endif
			} else {
#ifdef _TMPL_ALLOC_DEBUG
				php_printf("%s[rc=%u][%p]VALUE-OF: \"%s\"\n", ind, tmpl->refcount, tmpl, tmpl->data.var.name);
#else
				php_printf("%s[rc=%u]VALUE-OF: \"%s\"\n", ind, tmpl->refcount, tmpl->data.var.name);
#endif
			}
			break;
		case TMPL_EL_SUBST_EXPR:
			/* Expression substitution: data.expr */
#ifdef _TMPL_ALLOC_DEBUG
			php_printf("%sUnsupported type code %d [rc=%u][%p]\n", ind, tmpl->type, tmpl->refcount, tmpl);
#else
			php_printf("%sUnsupported type code %d [rc=%u]\n", ind, tmpl->type, tmpl->refcount);
#endif
			break;
		case TMPL_EL_COND:
			/* Simple conditional: data.var, next_cond, content_item */
#ifdef _TMPL_ALLOC_DEBUG
			php_printf("%s[rc=%u][%p]IF: \"%s\"\n", ind, tmpl->refcount, tmpl, tmpl->data.var.name);
#else
			php_printf("%s[rc=%u]IF: \"%s\"\n", ind, tmpl->refcount, tmpl->data.var.name);
#endif
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
#ifdef _TMPL_ALLOC_DEBUG
			php_printf("%sUnsupported type code %d [rc=%u][%p]\n", ind, tmpl->type, tmpl->refcount, tmpl);
#else
			php_printf("%sUnsupported type code %d [rc=%u]\n", ind, tmpl->type, tmpl->refcount);
#endif
			break;
		case TMPL_EL_ELSE:
			/* Else block: content_item */
#ifdef _TMPL_ALLOC_DEBUG
			php_printf("%s[rc=%u][%p]ELSE:\n", ind, tmpl->refcount, tmpl);
#else
			php_printf("%s[rc=%u]ELSE:\n", ind, tmpl->refcount);
#endif
			_tmpl_dump(tmpl->content_item, ind_lvl+1);
			break;
		case TMPL_EL_LOOP_RANGE:
			/* Loop over range: data.range, content_item, next_cond */
#ifdef _TMPL_ALLOC_DEBUG
			php_printf("%s[rc=%u][%p]FOR item = %ld TO %ld STEP %ld:\n", ind, tmpl->refcount, tmpl, tmpl->data.range.begin, tmpl->data.range.end, tmpl->data.range.step);
#else
			php_printf("%s[rc=%u]FOR item = %ld TO %ld STEP %ld:\n", ind, tmpl->refcount, tmpl->data.range.begin, tmpl->data.range.end, tmpl->data.range.step);
#endif
			_tmpl_dump(tmpl->content_item, ind_lvl+1);
			if (tmpl->next_cond) {
				// if we have a next condition, then the final next_condition's
				// next will carry us on properly
				_tmpl_dump(tmpl->next_cond, ind_lvl);
				efree(ind);
				return;
			}
			break;
		case TMPL_EL_LOOP_VAR:
			/* Loop over array var: data.var, content_item, next_cond */
#ifdef _TMPL_ALLOC_DEBUG
			php_printf("%s[rc=%u][%p]FOREACH: \"%s\"\n", ind, tmpl->refcount, tmpl, tmpl->data.var.name);
#else
			php_printf("%s[rc=%u]FOREACH: \"%s\"\n", ind, tmpl->refcount, tmpl->data.var.name);
#endif
			_tmpl_dump(tmpl->content_item, ind_lvl+1);
			if (tmpl->next_cond) {
				// if we have a next condition, then the final next_condition's
				// next will carry us on properly
				_tmpl_dump(tmpl->next_cond, ind_lvl);
				efree(ind);
				return;
			}
			break;
		case TMPL_EL_LOOP_ELSE:
			/* Loop "else": content_item */
#ifdef _TMPL_ALLOC_DEBUG
			php_printf("%s[rc=%u][%p]ELSEIF no iterations:\n", ind, tmpl->refcount, tmpl);
#else
			php_printf("%s[rc=%u]ELSEIF no iterations:\n", ind, tmpl->refcount);
#endif
			_tmpl_dump(tmpl->content_item, ind_lvl+1);
			break;
		case TMPL_EL_ERROR:
			/* Error message: data.content */
#ifdef _TMPL_ALLOC_DEBUG
			php_printf("%s[rc=%u][%p]ERROR: %s\n", ind, tmpl->refcount, tmpl, tmpl->data.content.data);
#else
			php_printf("%s[rc=%u]ERROR: %s\n", ind, tmpl->refcount, tmpl->data.content.data);
#endif
			break;
		default:
#ifdef _TMPL_ALLOC_DEBUG
			php_printf("%sUnknown type code %d [rc=%u][%p]\n", ind, tmpl->type, tmpl->refcount, tmpl);
#else
			php_printf("%sUnknown type code %d [rc=%u]\n", ind, tmpl->type, tmpl->refcount);
#endif
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
		ALLOC_DEBUGM("Starting to free tmpl at %p[rc=%u]", tmpl, tmpl->refcount);
		if (--(tmpl->refcount) > 0) {
			ALLOC_DEBUGM("Dec refcount (now %u) > 0", tmpl->refcount);
			tmpl = tmpl->next;
			continue;
		}
		if (tmpl->content_item) {
			ALLOC_DEBUGM("Freeing content_item at %p", tmpl->content_item);
			tmpl_free(tmpl->content_item);
		}
		if (tmpl->next_cond) {
			ALLOC_DEBUGM("Freeing next_cond at %p", tmpl->next_cond);
			tmpl_free(tmpl->next_cond);
		}
		if (TMPL_EL_HAS_CONTENT(tmpl) && tmpl->data.content.data) {
			ALLOC_DEBUGM("Freeing data.content.data at %p", tmpl->data.content.data);
			efree(tmpl->data.content.data);
		}
		if (TMPL_EL_HAS_VAR(tmpl) && tmpl->data.var.name) {
			ALLOC_DEBUGM("Freeing data.var.name at %p", tmpl->data.var.name);
			efree(tmpl->data.var.name);
		}
		if (TMPL_EL_HAS_VAR(tmpl) && tmpl->data.var.dval) {
			ALLOC_DEBUGM("Freeing data.var.dval at %p", tmpl->data.var.dval);
			efree(tmpl->data.var.dval);
		}
		if (TMPL_EL_HAS_EXPR(tmpl) ) {
			ALLOC_DEBUGM("Freeing expr at %p", tmpl->data.expr);
			tmpl_expr_free(tmpl->data.expr);
		}
		tmpl->type = 0;

		if (tmpl->next_cond) {
			// the last next_cond's "next" is equal to the first, so need to
			// make sure we don't try to free twice!
			ALLOC_DEBUGM("Has next_cond; freeing tmpl at %p and exiting loop", tmpl);
			efree(tmpl);
			tmpl = NULL;
		} else {
			ALLOC_DEBUGM("No next_cond; freeing tmpl at %p and continuing to %p", tmpl, tmpl->next);
			next = tmpl->next;
			efree(tmpl);
			tmpl = next;
		}
	}
	ALLOC_DEBUG("Unrecursing...");
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
