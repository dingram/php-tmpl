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
static zend_object_handlers tto_object_handlers;

/*
static zend_object_value tmpl_clone_obj(zval *this_ptr TSRMLS_DC);
static php_tt_object* php_tt_object_new(zend_class_entry *ce TSRMLS_DC);
static zend_object_value php_tt_register_object(php_tt_object *tto TSRMLS_DC);

static zend_object_value tmpl_clone_obj(zval *this_ptr TSRMLS_DC) // {{{ //
{
	php_tt_object *old_obj = (php_tt_object *)zend_object_store_get_object(this_ptr TSRMLS_CC);
	php_tt_object *new_obj = php_tt_object_new(old_obj->zo.ce TSRMLS_CC);
	zend_object_value new_ov = php_tt_register_object(new_obj TSRMLS_CC);

	zend_objects_clone_members(&new_obj->zo, new_ov, &old_obj->zo, Z_OBJ_HANDLE_P(this_ptr) TSRMLS_CC);

	return new_ov;
}
// }}} */

/* If you declare any globals in php_tmpl.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(tmpl)
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
	"tmpl",
	tmpl_functions,
	PHP_MINIT(tmpl),
	PHP_MSHUTDOWN(tmpl),
	PHP_RINIT(tmpl),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(tmpl),	/* Replace with NULL if there's nothing to do at request end */
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

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("tmpl.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_tmpl_globals, tmpl_globals)
    STD_PHP_INI_ENTRY("tmpl.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_tmpl_globals, tmpl_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_tmpl_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_tmpl_init_globals(zend_tmpl_globals *tmpl_globals)
{
	tmpl_globals->global_value = 0;
	tmpl_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(tmpl)
{
	/*
	zend_class_entry ttce;
	INIT_CLASS_ENTRY(soce, "TextTemplate", so_functions);
	soce.create_object = new_so_object;

	soo_class_entry = zend_register_internal_class(&soce TSRMLS_CC);
	memcpy(&so_object_handlers, zend_get_std_object_handlers(), sizeof(zend_object_handlers));

	so_object_handlers.read_property = oauth_read_member;
	so_object_handlers.write_property = oauth_write_member;
	so_object_handlers.clone_obj = oauth_clone_obj;
	*/

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
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(tmpl)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(tmpl)
{
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

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
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


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
