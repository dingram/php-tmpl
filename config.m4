dnl $Id$
dnl config.m4 for extension tmpl

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(tmpl, for tmpl support,
dnl Make sure that the comment is aligned:
dnl [  --with-tmpl             Include tmpl support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(tmpl, whether to enable TextTemplate support,
dnl Make sure that the comment is aligned:
[  --enable-tmpl           Enable TextTemplate support])

if test "$PHP_TMPL" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-tmpl -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/tmpl.h"  # you most likely want to change this
  dnl if test -r $PHP_TMPL/$SEARCH_FOR; then # path given as parameter
  dnl   TMPL_DIR=$PHP_TMPL
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for tmpl files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       TMPL_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$TMPL_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the tmpl distribution])
  dnl fi

  dnl # --with-tmpl -> add include path
  dnl PHP_ADD_INCLUDE($TMPL_DIR/include)

  dnl # --with-tmpl -> check for lib and symbol presence
  dnl LIBNAME=tmpl # you may want to change this
  dnl LIBSYMBOL=tmpl # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $TMPL_DIR/lib, TMPL_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_TMPLLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong tmpl lib version or lib not found])
  dnl ],[
  dnl   -L$TMPL_DIR/lib -lm
  dnl ])
  dnl
  dnl PHP_SUBST(TMPL_SHARED_LIBADD)

  PHP_NEW_EXTENSION(tmpl, tmpl.c, $ext_shared)
fi
