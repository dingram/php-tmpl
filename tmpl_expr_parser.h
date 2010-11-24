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

#ifndef PHP_TMPL_EXPR_PARSER_H
#define PHP_TMPL_EXPR_PARSER_H

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


void tmpl_expr_free(php_tt_tmpl_expr *expr);


#endif	/* PHP_TMPL_EXPR_PARSER_H */


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
