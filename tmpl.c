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
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id: header 297205 2010-03-30 21:09:07Z johannes $ */

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

	tmpl_prop_hash_dtor(tto TSRMLS_CC);
}
/* }}} */


ZEND_BEGIN_ARG_INFO_EX(arginfo_tmpl_noparams, 0, 0, 0)
ZEND_END_ARG_INFO()

static zend_function_entry tt_functions[] = { /* {{{ */
	PHP_ME(tt, __construct,				arginfo_tmpl_noparams,			ZEND_ACC_PUBLIC|ZEND_ACC_FINAL|ZEND_ACC_CTOR)
	/*
	PHP_ME(tt, tokenize,				arginfo_tmpl_noparams,			ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	PHP_ME(tt, tokenizeConditional,		arginfo_tmpl_noparams,			ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	PHP_ME(tt, tokenizeElseIf,			arginfo_tmpl_noparams,			ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	PHP_ME(tt, tokenizeElse,			arginfo_tmpl_noparams,			ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	PHP_ME(tt, tokenizeConditionalEnd,	arginfo_tmpl_noparams,			ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	PHP_ME(tt, tokenizeLoop,			arginfo_tmpl_noparams,			ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	PHP_ME(tt, tokenizeLoopElse,		arginfo_tmpl_noparams, 			ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	PHP_ME(tt, tokenizeLoopEnd,			arginfo_tmpl_noparams,			ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_FINAL)
	PHP_ME(tt, get,						arginfo_tmpl_noparams,			ZEND_ACC_PUBLIC)
	PHP_ME(tt, set,						arginfo_tmpl_noparams,			ZEND_ACC_PUBLIC)
	PHP_ME(tt, compile,					arginfo_tmpl_noparams,			ZEND_ACC_PUBLIC)
	PHP_ME(tt, process,					arginfo_tmpl_noparams,			ZEND_ACC_PUBLIC)
	*/
	PHP_ME(tt, __destruct,				arginfo_tmpl_noparams,			ZEND_ACC_PUBLIC)
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


/* Remove the following function when you have succesfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_tmpl_compiled(string arg)
   Return a string to confirm that the module is compiled in */
PHP_FUNCTION(confirm_tmpl_compiled)
{
	char *arg = NULL;
	int arg_len, len;
	char *strg;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &arg, &arg_len) == FAILURE) {
		return;
	}

	len = spprintf(&strg, 0, "Congratulations! Module %.78s is now compiled into PHP.", "tmpl", arg);
	RETURN_STRINGL(strg, len, 0);
}
/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and
   unfold functions in source code. See the corresponding marks just before
   function definition, where the functions purpose is also documented. Please
   follow this convention for the convenience of others editing your code.
*/


/* {{{ tmpl_functions[]
 *
 * Every user visible function must have an entry in tmpl_functions[].
 */
const zend_function_entry tmpl_functions[] = {
	PHP_FE(confirm_tmpl_compiled,	NULL)		/* For testing, remove later. */
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
