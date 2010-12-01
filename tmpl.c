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

static zend_class_entry *tto_class_entry;
static zend_object_handlers tt_object_handlers;

static zend_object_value tmpl_clone_obj(zval *this_ptr TSRMLS_DC);
static php_tt_object* php_tt_object_new(zend_class_entry *ce TSRMLS_DC);
static zend_object_value php_tt_register_object(php_tt_object *tto TSRMLS_DC);

static zend_object_value tmpl_clone_obj(zval *this_ptr TSRMLS_DC) /* {{{ */
{
	php_tt_object *old_obj = (php_tt_object *)zend_object_store_get_object(this_ptr TSRMLS_CC);
	php_tt_object *new_obj = php_tt_object_new(old_obj->zo.ce TSRMLS_CC);
	zend_object_value new_ov = php_tt_register_object(new_obj TSRMLS_CC);

	zend_objects_clone_members(&new_obj->zo, new_ov, &old_obj->zo, Z_OBJ_HANDLE_P(this_ptr) TSRMLS_CC);

	return new_ov;
}
/* }}} */

static inline php_tt_object *fetch_tt_object(zval *obj TSRMLS_DC) /* {{{ */
{
	php_tt_object *tto = (php_tt_object *)zend_object_store_get_object(obj TSRMLS_CC);

	tto->this_ptr = obj;

	return tto;
}
/* }}} */

static void tt_object_free_storage(void *obj TSRMLS_DC) /* {{{ */
{
	php_tt_object *tto;

	tto = (php_tt_object *) obj;

#if (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION < 3)
	if (tto->zo.guards) {
		zend_hash_destroy(tto->zo.guards);
		FREE_HASHTABLE(tto->zo.guards);
	}
	if (tto->zo.properties) {
		zend_hash_destroy(tto->zo.properties);
		FREE_HASHTABLE(tto->zo.properties);
	}
#else
	zend_object_std_dtor(&tto->zo TSRMLS_CC);
#endif

	if (tto->tmpl) tmpl_free(tto->tmpl);
	if (tto->tmpl_vars) {
		zend_hash_destroy(tto->tmpl_vars);
		FREE_HASHTABLE(tto->tmpl_vars);
	}

	efree(obj);
}
/* }}} */

static zend_object_value php_tt_register_object(php_tt_object *tto TSRMLS_DC) /* {{{ */
{
	zend_object_value rv;

	rv.handle = zend_objects_store_put(tto, (zend_objects_store_dtor_t)zend_objects_destroy_object, tt_object_free_storage, NULL TSRMLS_CC);
	rv.handlers = (zend_object_handlers *) &tt_object_handlers;
	return rv;
}
/* }}} */

static php_tt_object* php_tt_object_new(zend_class_entry *ce TSRMLS_DC) /* {{{ */
{
	php_tt_object *nos;

	nos = ecalloc(1, sizeof(php_tt_object));

#if (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION < 3)
	ALLOC_HASHTABLE(nos->zo.properties);
	zend_hash_init(nos->zo.properties, 0, NULL, ZVAL_PTR_DTOR, 0);

	nos->zo.ce = ce;
	nos->zo.guards = NULL;
#else
	zend_object_std_init(&nos->zo, ce TSRMLS_CC);
#ifdef ZEND_ENGINE_2_4
	object_properties_init(&nos->zo, ce);
#endif
#endif

	ALLOC_HASHTABLE(nos->tmpl_vars);
	zend_hash_init(nos->tmpl_vars, 0, NULL, ZVAL_PTR_DTOR, 0);

	return nos;
}
/* }}} */


static zend_object_value new_tt_object(zend_class_entry *ce TSRMLS_DC) /* {{{ */
{
	php_tt_object *tto;

	tto = php_tt_object_new(ce TSRMLS_CC);
	return php_tt_register_object(tto TSRMLS_CC);
}
/* }}} */

static void tmpl_prop_hash_dtor(php_tt_object *tto TSRMLS_DC) /* {{{ */
{
	HashTable *ht;
	ht = tto->properties;
	FREE_ARGS_HASH(ht);
}
/* }}} */


/* {{{ proto void TextTemplate::__construct(void)
   Instantiate a new TextTemplate object */
PHP_METHOD(tt, __construct)
{
	HashTable *hasht;
	zval *obj;
	php_tt_object *tto;

	tto = fetch_tt_object(getThis() TSRMLS_CC);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
		return;
	}

	TSRMLS_SET_CTX(tto->thread_ctx);

	if (tto->properties) {
		zend_hash_clean(tto->properties);
		hasht = tto->properties;
	} else {
		ALLOC_HASHTABLE(hasht);
		zend_hash_init(hasht, 0, NULL, ZVAL_PTR_DTOR, 0);
		tto->properties = hasht;
	}
}
/* }}} */

/* {{{ proto void TextTemplate::__destruct(void)
   Clean up a TextTemplate object */
PHP_METHOD(tt, __destruct)
{
	php_tt_object *tto;

	tto = fetch_tt_object(getThis() TSRMLS_CC);

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
		return;
	}

	if (tto->tmpl) {
		tmpl_free(tto->tmpl);
		tto->tmpl = NULL;
	}

	if (tto->tmpl_vars) {
		zend_hash_destroy(tto->tmpl_vars);
		FREE_HASHTABLE(tto->tmpl_vars);
		tto->tmpl_vars = NULL;
	}

	tmpl_prop_hash_dtor(tto TSRMLS_CC);
}
/* }}} */


/* {{{ proto string TextTemplate::tokenize(string token[, string default])
   Create a template tag with the given content */
PHP_METHOD(tt, tokenize)
{
	char *token, *default_val=NULL, *ret;
	int token_len, default_val_len=0, ret_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|s", &token, &token_len, &default_val, &default_val_len) == FAILURE) {
		return;
	}

	if (token_len < 1) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid token length (0)");
		RETURN_FALSE;
	}

	if (default_val && default_val_len) {
		ret_len = spprintf(&ret, 0, "%s%s%s%s%s", TMPL_T_PRE, token, TMPL_T_DEFAULT, default_val, TMPL_T_POST);
	} else {
		TMPL_TOKENIZE(ret, ret_len, token);
	}

	RETURN_STRINGL(ret, ret_len, 0);
}
/* }}} */


/* {{{ proto string TextTemplate::tokenizeConditional(string condition[, bool invert])
   Create a conditional template tag with the given condition */
PHP_METHOD(tt, tokenizeConditional)
{
	char *cond, *ret;
	int cond_len, invert=0, ret_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|b", &cond, &cond_len, &invert) == FAILURE) {
		return;
	}

	if (cond_len < 1) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid condition length (0)");
		RETURN_FALSE;
	}

	if (invert) {
		ret_len = spprintf(&ret, 0, "%s!%s%s", TMPL_T_PRE TMPL_T_COND, cond, TMPL_T_POST);
	} else {
		ret_len = spprintf(&ret, 0, "%s%s%s", TMPL_T_PRE TMPL_T_COND, cond, TMPL_T_POST);
	}

	RETURN_STRINGL(ret, ret_len, 0);
}
/* }}} */


/* {{{ proto string TextTemplate::tokenizeElseIf(string condition[, bool invert])
   Create a conditional "else if" template tag with the given condition */
PHP_METHOD(tt, tokenizeElseIf)
{
	char *cond, *ret;
	int cond_len, invert=0, ret_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|b", &cond, &cond_len, &invert) == FAILURE) {
		return;
	}

	if (cond_len < 1) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid condition length (0)");
		RETURN_FALSE;
	}

	if (invert) {
		ret_len = spprintf(&ret, 0, "%s!%s%s", TMPL_T_PRE TMPL_T_ELSE TMPL_T_COND, cond, TMPL_T_POST);
	} else {
		ret_len = spprintf(&ret, 0, "%s%s%s", TMPL_T_PRE TMPL_T_ELSE TMPL_T_COND, cond, TMPL_T_POST);
	}

	RETURN_STRINGL(ret, ret_len, 0);
}
/* }}} */


/* {{{ proto string TextTemplate::tokenizeElse(void)
   Create an "else" template tag */
PHP_METHOD(tt, tokenizeElse)
{
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
		return;
	}

	RETURN_STRING(TMPL_T_PRE TMPL_T_ELSE TMPL_T_POST, 1);
}
/* }}} */


/* {{{ proto string TextTemplate::tokenizeConditionalEnd(void)
   Create a conditional section end template tag */
PHP_METHOD(tt, tokenizeConditionalEnd)
{
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
		return;
	}

	RETURN_STRING(TMPL_T_PRE TMPL_T_END TMPL_T_COND TMPL_T_POST, 1);
}
/* }}} */


/* {{{ proto string TextTemplate::tokenizeLoop(string variable)
   Create a foreach template tag over the given variable */
PHP_METHOD(tt, tokenizeLoop)
{
	char *var, *ret;
	int var_len, ret_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &var, &var_len) == FAILURE) {
		return;
	}

	if (var_len < 1) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Invalid variable length (0)");
		RETURN_FALSE;
	}

	ret_len = spprintf(&ret, 0, "%s%s%s", TMPL_T_PRE TMPL_T_LOOP, var, TMPL_T_POST);

	RETURN_STRINGL(ret, ret_len, 0);
}
/* }}} */


/* {{{ proto string TextTemplate::tokenizeLoopRange(long start, long end[, long step])
   Create a for loop template tag over the given range and step */
PHP_METHOD(tt, tokenizeLoopRange)
{
	char *ret;
	int ret_len;
	signed long r_start, r_end, r_step=0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ll|l", &r_start, &r_end, &r_step) == FAILURE) {
		return;
	}

	if (!r_step) {
		r_step = (r_end>=r_start) ? 1 : -1;
	}

	if ((r_step>0 && r_end<r_start) || (r_step<0 && r_end>r_start)) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "Step sign does not make sense with range");
		RETURN_FALSE;
	}

	if (r_step == 1 || r_step == -1) {
		ret_len = spprintf(&ret, 0, "%s(%ld..%ld)%s", TMPL_T_PRE TMPL_T_LOOP, r_start, r_end, TMPL_T_POST);
	} else {
		ret_len = spprintf(&ret, 0, "%s(%ld,%ld..%ld)%s", TMPL_T_PRE TMPL_T_LOOP, r_start, r_start+r_step, r_end, TMPL_T_POST);
	}

	RETURN_STRINGL(ret, ret_len, 0);
}
/* }}} */


/* {{{ proto string TextTemplate::tokenizeLoopElse(void)
   Create a loop "else" template tag */
PHP_METHOD(tt, tokenizeLoopElse)
{
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
		return;
	}

	RETURN_STRING(TMPL_T_PRE TMPL_T_LOOP TMPL_T_ELSE TMPL_T_POST, 1);
}
/* }}} */


/* {{{ proto string TextTemplate::tokenizeLoopEnd(void)
   Create a loop section end template tag */
PHP_METHOD(tt, tokenizeLoopEnd)
{
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
		return;
	}

	RETURN_STRING(TMPL_T_PRE TMPL_T_END TMPL_T_LOOP TMPL_T_POST, 1);
}
/* }}} */


/* {{{ proto string TextTemplate::compile(string template)
   Compile the template to an internal representation */
PHP_METHOD(tt, compile)
{
	char *template;
	int template_len;
	zval *obj;
	php_tt_object *tto;

	obj = getThis();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &template, &template_len) == FAILURE) {
		return;
	}

	tto = fetch_tt_object(obj TSRMLS_CC);

	if (tto->tmpl) {
		tmpl_free(tto->tmpl);
		tto->tmpl = NULL;
	}

	tto->tmpl = tmpl_parse(template, template_len TSRMLS_CC);

	if (tto->tmpl) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}
/* }}} */

/* {{{ proto string TextTemplate::render([array vars])
   Render a template */
PHP_METHOD(tt, render)
{
	zval *overrides = NULL, *obj = NULL, *tmp = NULL;
	php_tt_object *tto;
	HashTable *all_vars;
	char *rendered;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|a", &overrides) == FAILURE) {
		return;
	}

	obj = getThis();

	tto = fetch_tt_object(obj TSRMLS_CC);

	ALLOC_HASHTABLE(all_vars);
	zend_hash_init(all_vars, 0, NULL, ZVAL_PTR_DTOR, 0);
	zend_hash_copy(all_vars, tto->tmpl_vars, (copy_ctor_func_t) zval_add_ref, (void *) &tmp, sizeof(zval *));

	if (overrides) {
		php_array_merge(all_vars, HASH_OF(overrides), 0 TSRMLS_CC);
	}

	rendered = tmpl_use(tto->tmpl, all_vars TSRMLS_CC);

	zend_hash_destroy(all_vars);
	FREE_HASHTABLE(all_vars);

	RETURN_STRING(rendered, 0);
}
/* }}} */

/* {{{ proto int TextTemplate::offsetExists(string entry)
 */
PHP_METHOD(tt, offsetExists)
{
	char *entry = NULL;
	int elen = 0;
	zval *obj;
	php_tt_object *tto;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &entry, &elen) == FAILURE) {
		return;
	}

	obj = getThis();
	tto = fetch_tt_object(obj TSRMLS_CC);

	if (zend_symtable_exists(tto->tmpl_vars, entry, elen+1)) {
		RETURN_TRUE;
	} else {
		RETURN_FALSE;
	}
}

/* {{{ proto int TextTemplate::getAll(void)
 */
PHP_METHOD(tt, getAll)
{
	char *entry = NULL;
	int elen = 0;
	zval *obj;
	php_tt_object *tto;
	zval *ret, *tmp;
	HashTable *ret_ht;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "", &entry, &elen) == FAILURE) {
		return;
	}

	obj = getThis();
	tto = fetch_tt_object(obj TSRMLS_CC);

	ALLOC_HASHTABLE(ret_ht);
	zend_hash_init(ret_ht, 0, NULL, ZVAL_PTR_DTOR, 0);
	ALLOC_INIT_ZVAL(ret);
	Z_TYPE_P(ret) = IS_ARRAY;
	zend_hash_copy(ret_ht, tto->tmpl_vars, (copy_ctor_func_t) zval_add_ref, (void *) &tmp, sizeof(zval *));
	Z_ARRVAL_P(ret) = ret_ht;

	RETURN_ZVAL(ret, 1, 1);
}

/* {{{ proto int TextTemplate::get(string entry)
 */
PHP_METHOD(tt, get)
{
	char *entry = NULL;
	int elen = 0;
	zval *obj;
	php_tt_object *tto;
	zval **tmp;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &entry, &elen) == FAILURE) {
		return;
	}

	obj = getThis();
	tto = fetch_tt_object(obj TSRMLS_CC);

	if (SUCCESS == zend_symtable_find(tto->tmpl_vars, entry, elen+1, (void **)&tmp)) {
		// TODO: this returns the key rather than the data?!
		RETURN_ZVAL(*tmp, 1, 0);
	} else {
		RETURN_NULL();
	}
}

/* {{{ proto int TextTemplate::set(string entry, string value)
 *     proto int TextTemplate::set(array entries)
 */
PHP_METHOD(tt, set)
{
	char *entry = NULL;
	int elen = 0;
	zval *value = NULL;
	zval *entries = NULL;
	zval *obj;
	php_tt_object *tto;

	if (ZEND_NUM_ARGS() == 1) {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "a", &entries) == FAILURE) {
			return;
		}
	} else {
		if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz", &entry, &elen, &value) == FAILURE) {
			return;
		}
	}

	obj = getThis();
	tto = fetch_tt_object(obj TSRMLS_CC);

	if (entry && elen && value) {
		Z_ADDREF_P(value);
		zend_symtable_update(tto->tmpl_vars, entry, elen+1, (void **)&value, sizeof(zval *), NULL);
	} else if (entries) {
		php_array_merge(tto->tmpl_vars, HASH_OF(entries), 0 TSRMLS_CC);
	}
}

/* {{{ proto int TextTemplate::offsetSet(string entry, string value)
 */
PHP_METHOD(tt, offsetSet)
{
	char *entry = NULL;
	int elen = 0;
	zval *value = NULL;
	zval *obj;
	php_tt_object *tto;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "sz", &entry, &elen, &value) == FAILURE) {
		return;
	}

	obj = getThis();
	tto = fetch_tt_object(obj TSRMLS_CC);

	Z_ADDREF_P(value);
	zend_symtable_update(tto->tmpl_vars, entry, elen+1, (void **)&value, sizeof(zval *), NULL);
}

/* {{{ proto int TextTemplate::offsetUnset(string entry)
 */
PHP_METHOD(tt, offsetUnset)
{
	char *entry = NULL;
	int elen = 0;
	zval *obj;
	zval *tmp;
	php_tt_object *tto;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &entry, &elen) == FAILURE) {
		return;
	}

	obj = getThis();
	tto = fetch_tt_object(obj TSRMLS_CC);

	if (zend_symtable_exists(tto->tmpl_vars, entry, elen+1)) {
		if (SUCCESS == zend_symtable_find(tto->tmpl_vars, entry, elen+1, (void **)&tmp)) {
			zend_symtable_del(tto->tmpl_vars, entry, elen+1);
			Z_DELREF_P(tmp);
		}
	}
}

/* {{{ proto int TextTemplate::count(void)
 */
PHP_METHOD(tt, count)
{
	zval *obj;
	php_tt_object *tto;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
		return;
	}

	obj = getThis();
	tto = fetch_tt_object(obj TSRMLS_CC);

	RETURN_LONG(zend_hash_num_elements(tto->tmpl_vars));
}

/* {{{ proto string TextTemplate::__toString(void)
   Return a string representation of the template */
PHP_METHOD(tt, __toString)
{
	zval *obj;
	php_tt_object *tto;
	char *tmpl;

	obj = getThis();

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "") == FAILURE) {
		return;
	}

	tto = fetch_tt_object(obj TSRMLS_CC);

	tmpl = tmpl_to_string(tto->tmpl);
	RETURN_STRING(tmpl, 0);
}


ZEND_BEGIN_ARG_INFO_EX(arginfo_tmpl_noparams, 0, 0, 0)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_tmpl_tokenize, 0, 0, 1)
	ZEND_ARG_INFO(0, token)
	ZEND_ARG_INFO(0, default)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_tmpl_tokenize_cond, 0, 0, 1)
	ZEND_ARG_INFO(0, token)
	ZEND_ARG_INFO(0, invert)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_tmpl_tokenize_loop, 0, 0, 1)
	ZEND_ARG_INFO(0, token)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_tmpl_tokenize_loop_range, 0, 0, 2)
	ZEND_ARG_INFO(0, start)
	ZEND_ARG_INFO(0, end)
	ZEND_ARG_INFO(0, step)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_tmpl_get, 0, 0, 1)
	ZEND_ARG_INFO(0, token)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_tmpl_set, 0, 0, 1)
	ZEND_ARG_INFO(0, token)
	ZEND_ARG_INFO(0, replacement)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_tmpl_compile, 0, 0, 1)
	ZEND_ARG_INFO(0, template)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_tmpl_render, 0, 0, 0)
	ZEND_ARG_INFO(0, overrides)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_tmpl_offsetExists, 0, 0, 1)
	ZEND_ARG_INFO(0, entry)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_tmpl_offsetSet, 0, 0, 2)
	ZEND_ARG_INFO(0, entry)
	ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()


static zend_function_entry tt_functions[] = { /* {{{ */
	PHP_ME(tt, __construct,				arginfo_tmpl_noparams,				ZEND_ACC_PUBLIC|ZEND_ACC_FINAL|ZEND_ACC_CTOR)
	PHP_ME(tt, __toString,				arginfo_tmpl_noparams,				ZEND_ACC_PUBLIC)
	PHP_ME(tt, __destruct,				arginfo_tmpl_noparams,				ZEND_ACC_PUBLIC|ZEND_ACC_FINAL|ZEND_ACC_DTOR)

	PHP_ME(tt, tokenize,				arginfo_tmpl_tokenize,				ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	PHP_ME(tt, tokenizeConditional,		arginfo_tmpl_tokenize_cond,			ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	PHP_ME(tt, tokenizeElseIf,			arginfo_tmpl_tokenize_cond,			ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	PHP_ME(tt, tokenizeElse,			arginfo_tmpl_noparams,				ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	PHP_ME(tt, tokenizeConditionalEnd,	arginfo_tmpl_noparams,				ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	PHP_ME(tt, tokenizeLoop,			arginfo_tmpl_tokenize_loop,			ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	PHP_ME(tt, tokenizeLoopRange,		arginfo_tmpl_tokenize_loop_range,	ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	PHP_ME(tt, tokenizeLoopElse,		arginfo_tmpl_noparams,				ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	PHP_ME(tt, tokenizeLoopEnd,			arginfo_tmpl_noparams,				ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)

	PHP_ME(tt, get,						arginfo_tmpl_get,					ZEND_ACC_PUBLIC)
	PHP_ME(tt, getAll,					arginfo_tmpl_noparams,				ZEND_ACC_PUBLIC)
	PHP_ME(tt, set,						arginfo_tmpl_set,					ZEND_ACC_PUBLIC)
#if HAVE_SPL
	PHP_ME(    tt, offsetExists,			arginfo_tmpl_offsetExists,			ZEND_ACC_PUBLIC)
	PHP_MALIAS(tt, offsetGet,		get,	arginfo_tmpl_offsetExists,			ZEND_ACC_PUBLIC)
	PHP_ME(    tt, offsetSet,				arginfo_tmpl_offsetSet,				ZEND_ACC_PUBLIC)
	PHP_ME(    tt, offsetUnset,				arginfo_tmpl_offsetExists,			ZEND_ACC_PUBLIC)
	PHP_ME(    tt, count,					arginfo_tmpl_noparams,				ZEND_ACC_PUBLIC)
#endif
	PHP_ME(tt, compile,					arginfo_tmpl_compile,				ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)
	PHP_ME(tt, render,					arginfo_tmpl_render,				ZEND_ACC_PUBLIC|ZEND_ACC_FINAL)

	PHP_MALIAS(tt, tokenizeIf,		tokenizeConditional,		arginfo_tmpl_offsetExists,		ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	PHP_MALIAS(tt, tokenizeEndIf,	tokenizeConditionalEnd,		arginfo_tmpl_offsetExists,		ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)

	{NULL, NULL, NULL}
};
/* }}} */

#ifdef ZEND_ENGINE_2_4
static zval *tmpl_read_member(zval *obj, zval *mem, int type, const zend_literal *key TSRMLS_DC) /* {{{ */
#else
static zval *tmpl_read_member(zval *obj, zval *mem, int type TSRMLS_DC) /* {{{ */
#endif
{
	zval *return_value = NULL;
	php_tt_object *tto;

	tto = fetch_tt_object(obj TSRMLS_CC);

	return TMPL_READ_PROPERTY(obj, mem, type);
} /* }}} */

#ifdef ZEND_ENGINE_2_4
static void tmpl_write_member(zval *obj, zval *mem, zval *value, const zend_literal *key TSRMLS_DC) /* {{{ */
#else
static void tmpl_write_member(zval *obj, zval *mem, zval *value TSRMLS_DC) /* {{{ */
#endif
{
	char *property;
	php_tt_object *tto;

	property = Z_STRVAL_P(mem);
	tto = fetch_tt_object(obj TSRMLS_CC);

	TMPL_WRITE_PROPERTY(obj, mem, value);
} /* }}} */


/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(tmpl)
{
	zend_class_entry ttce;
	INIT_CLASS_ENTRY(ttce, "TextTemplate", tt_functions);
	ttce.create_object = new_tt_object;

	tto_class_entry = zend_register_internal_class(&ttce TSRMLS_CC);
	memcpy(&tt_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

#if HAVE_SPL
	zend_class_implements(tto_class_entry TSRMLS_CC, 2, spl_ce_Countable, zend_ce_arrayaccess);
#endif

	tt_object_handlers.read_property = tmpl_read_member;
	tt_object_handlers.write_property = tmpl_write_member;
	tt_object_handlers.clone_obj = tmpl_clone_obj;

	REGISTER_STRING_CONSTANT("TMPL_T_PRE",     TMPL_T_PRE,     CONST_CS | CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("TMPL_T_POST",    TMPL_T_POST,    CONST_CS | CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("TMPL_T_DEFAULT", TMPL_T_DEFAULT, CONST_CS | CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("TMPL_T_COMMENT", TMPL_T_COMMENT, CONST_CS | CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("TMPL_T_COND",    TMPL_T_COND,    CONST_CS | CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("TMPL_T_ELSE",    TMPL_T_ELSE,    CONST_CS | CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("TMPL_T_END",     TMPL_T_END,     CONST_CS | CONST_PERSISTENT);
	REGISTER_STRING_CONSTANT("TMPL_T_LOOP",    TMPL_T_LOOP,    CONST_CS | CONST_PERSISTENT);

	/* If you have INI entries, uncomment these lines
	REGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(tmpl)
{
	tto_class_entry = NULL;
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(tmpl)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "TextTemplate support", "enabled");
	php_info_print_table_row(2, "Open tag",            TMPL_T_PRE    );
	php_info_print_table_row(2, "Close tag",           TMPL_T_POST   );
	php_info_print_table_colspan_header(2, "Indicators");
	php_info_print_table_row(2, "Default value",       TMPL_T_DEFAULT);
	php_info_print_table_row(2, "Comment",             TMPL_T_COMMENT);
	php_info_print_table_row(2, "Conditional section", TMPL_T_COND   );
	php_info_print_table_row(2, "Else section",        TMPL_T_ELSE   );
	php_info_print_table_row(2, "Loop section",        TMPL_T_LOOP   );
	php_info_print_table_row(2, "Section end",         TMPL_T_END    );
	php_info_print_table_end();
}
/* }}} */


/* {{{ proto string tmpl_parse(string template)
   */
PHP_FUNCTION(tmpl_parse)
{
	char *template = NULL;
	int template_len;
	php_tt_tmpl_el *tmpl = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &template, &template_len) == FAILURE) {
		return;
	}

	tmpl = tmpl_parse(template, template_len TSRMLS_CC);
	tmpl_dump(tmpl);
	char *tmp = tmpl_use(tmpl, NULL TSRMLS_CC);
	php_printf("Result: \"%s\"\n", tmp);
	efree(tmp);
	tmpl_free(tmpl);

	RETURN_NULL();
}
/* }}} */


/* {{{ tmpl_functions[]
 */
const zend_function_entry tmpl_functions[] = {
	PHP_FE(tmpl_parse,	NULL)		/* For testing, remove later. */
	{NULL, NULL, NULL}	/* Must be the last line in tmpl_functions[] */
};
/* }}} */

/* {{{ tmpl_module_entry
 */
zend_module_entry tmpl_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"TextTemplate",
	tmpl_functions,
	PHP_MINIT(tmpl),
	PHP_MSHUTDOWN(tmpl),
	NULL,
	NULL,
	PHP_MINFO(tmpl),
#if ZEND_MODULE_API_NO >= 20010901
	"0.1", /* Replace with version number for your extension */
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_TMPL
ZEND_GET_MODULE(tmpl)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
