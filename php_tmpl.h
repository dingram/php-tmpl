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

#ifndef PHP_TMPL_H
#define PHP_TMPL_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "ext/standard/php_smart_str.h"
#include "tmpl_parser.h"
#include "zend_hash.h"
#include "zend_interfaces.h"
#if HAVE_SPL
#include "ext/spl/spl_array.h"
#include "ext/spl/spl_engine.h"
#include "ext/spl/spl_exceptions.h"
#include "ext/spl/spl_iterators.h"
#endif

extern zend_module_entry tmpl_module_entry;
#define phpext_tmpl_ptr &tmpl_module_entry

#ifdef PHP_WIN32
#	define PHP_TMPL_API __declspec(dllexport)
#elif defined(__GNUC__) && __GNUC__ >= 4
#	define PHP_TMPL_API __attribute__ ((visibility("default")))
#else
#	define PHP_TMPL_API
#endif

#ifdef ZTS
#include "TSRM.h"
#endif

#ifdef ZEND_ENGINE_2_4
# define TMPL_READ_PROPERTY(_obj, _mem, _type) zend_get_std_object_handlers()->read_property(_obj, _mem, _type, key TSRMLS_CC)
# define TMPL_WRITE_PROPERTY(_obj, _mem, _val) zend_get_std_object_handlers()->write_property(_obj, _mem, _val, key TSRMLS_CC)
#else
# define TMPL_READ_PROPERTY(_obj, _mem, _type) zend_get_std_object_handlers()->read_property(_obj, _mem, _type TSRMLS_CC)
# define TMPL_WRITE_PROPERTY(_obj, _mem, _val) zend_get_std_object_handlers()->write_property(_obj, _mem, _val TSRMLS_CC)
#endif

#define TMPL_TOKENIZE(r,l,t) do { (l) = spprintf(&(r), 0, "%s%s%s", TMPL_T_PRE, (t), TMPL_T_POST); } while (0)

#define TMPL_T_PRE      "<!--{"  // open delimiter
#define TMPL_T_POST     "}-->"   // close delimiter
#define TMPL_T_DEFAULT  "|"      // default value start indicator
#define TMPL_T_COMMENT  "#"      // comment indicator
#define TMPL_T_COND     "?"      // conditional section start indicator
#define TMPL_T_ELSE     ":"      // "else" section start indicator
#define TMPL_T_END      "/"      // section end indicator
#define TMPL_T_LOOP     "@"      // loop section start indicator

PHP_MINIT_FUNCTION(tmpl);
PHP_MSHUTDOWN_FUNCTION(tmpl);
PHP_RINIT_FUNCTION(tmpl);
PHP_RSHUTDOWN_FUNCTION(tmpl);
PHP_MINFO_FUNCTION(tmpl);


typedef struct {
	zend_object zo;
	HashTable *properties;
	void ***thread_ctx;
	zval *this_ptr;
	php_tt_tmpl_el *tmpl;
	HashTable *tmpl_vars;
} php_tt_object;

#define FREE_ARGS_HASH(a)	\
	if (a) { \
		zend_hash_destroy(a);	\
		FREE_HASHTABLE(a); \
	}

PHP_FUNCTION(tmpl_get_first_tag);		/* For testing, remove later. */
PHP_FUNCTION(tmpl_skip_tag);		/* For testing, remove later. */

/*
  	Declare any global variables you may need between the BEGIN
	and END macros here:

ZEND_BEGIN_MODULE_GLOBALS(tmpl)
	long  global_value;
	char *global_string;
ZEND_END_MODULE_GLOBALS(tmpl)
*/

/* In every utility function you add that needs to use variables
   in php_tmpl_globals, call TSRMLS_FETCH(); after declaring other
   variables used by that function, or better yet, pass in TSRMLS_CC
   after the last function argument and declare your utility function
   with TSRMLS_DC after the last declared argument.  Always refer to
   the globals in your function as TMPL_G(variable).  You are
   encouraged to rename these macros something shorter, see
   examples in any other php module directory.
*/

#ifdef ZTS
#define TMPL_G(v) TSRMG(tmpl_globals_id, zend_tmpl_globals *, v)
#else
#define TMPL_G(v) (tmpl_globals.v)
#endif

#endif	/* PHP_TMPL_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
