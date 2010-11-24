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
	//smart_str lastresponse;
	//smart_str headers_in;
	void ***thread_ctx;
	zval *this_ptr;
	//zval *debugArr;
} php_tt_object;

#define TMPL_EXPR_CONST_INT     0x00
#define TMPL_EXPR_CONST_FLOAT   0x01
#define TMPL_EXPR_CONST_STRING  0x02
#define TMPL_EXPR_MASK_CONST    0x0f

#define TMPL_EXPR_VAR           0x10
#define TMPL_EXPR_MASK_VAR      0x1f

#define TMPL_EXPR_OP1_MINUS     0x20
#define TMPL_EXPR_OP1_NOT       0x21
#define TMPL_EXPR_OP1_BIT_NOT   0x22
#define TMPL_EXPR_MASK_OP1      0x2f

#define TMPL_EXPR_OP2_PLUS      0x40
#define TMPL_EXPR_OP2_MINUS     0x41
#define TMPL_EXPR_OP2_TIMES     0x42
#define TMPL_EXPR_OP2_DIVIDE    0x43
#define TMPL_EXPR_OP2_MOD       0x44
#define TMPL_EXPR_OP2_POWER     0x45
#define TMPL_EXPR_OP2_BIT_OR    0x46
#define TMPL_EXPR_OP2_BIT_AND   0x47
#define TMPL_EXPR_OP2_BIT_XOR   0x48
#define TMPL_EXPR_OP2_LOG_OR    0x49
#define TMPL_EXPR_OP2_LOG_AND   0x4A
#define TMPL_EXPR_OP2_EQ        0x4B
#define TMPL_EXPR_OP2_NE        0x4C
#define TMPL_EXPR_OP2_LT        0x4D
#define TMPL_EXPR_OP2_LE        0x4E
#define TMPL_EXPR_OP2_GT        0x4F
#define TMPL_EXPR_OP2_GE        0x50
#define TMPL_EXPR_MASK_OP2      0x4f

#define TMPL_EXPR_OP_GENERAL    0xff
#define TMPL_EXPR_MASK_OP_GEN   0x8f

typedef struct php_tt_tmpl_expr_t {
	int type;
	union {
		long   num;
		double fnum;
		struct {
			char *c;
			int  len;
		} cdata;
		struct {
			struct php_tt_tmpl_expr_t *arg;
		} oper1;
		struct {
			struct php_tt_tmpl_expr_t *arg1;
			struct php_tt_tmpl_expr_t *arg2;
		} oper2;
		struct {
			char  *operation;
			int    o_len;
			int    arg_count;
			struct php_tt_tmpl_expr_t **args; // array
		} func;
	} data;
} php_tt_tmpl_expr;

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

#define FREE_ARGS_HASH(a)	\
	if (a) { \
		zend_hash_destroy(a);	\
		FREE_HASHTABLE(a); \
	}

PHP_FUNCTION(confirm_tmpl_compiled);	/* For testing, remove later. */

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
