dnl $Id$
dnl config.m4 for extension tmock

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(tmock, for tmock support,
dnl Make sure that the comment is aligned:
dnl [  --with-tmock             Include tmock support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(tmock, whether to enable tmock support,
Make sure that the comment is aligned:
[  --enable-tmock           Enable tmock support])

if test "$PHP_TMOCK" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-tmock -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/tmock.h"  # you most likely want to change this
  dnl if test -r $PHP_TMOCK/$SEARCH_FOR; then # path given as parameter
  dnl   TMOCK_DIR=$PHP_TMOCK
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for tmock files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       TMOCK_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$TMOCK_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the tmock distribution])
  dnl fi

  dnl # --with-tmock -> add include path
  dnl PHP_ADD_INCLUDE($TMOCK_DIR/include)

  dnl # --with-tmock -> check for lib and symbol presence
  dnl LIBNAME=tmock # you may want to change this
  dnl LIBSYMBOL=tmock # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $TMOCK_DIR/$PHP_LIBDIR, TMOCK_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_TMOCKLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong tmock lib version or lib not found])
  dnl ],[
  dnl   -L$TMOCK_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(TMOCK_SHARED_LIBADD)

  PHP_NEW_EXTENSION(tmock, tmock.c, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi
