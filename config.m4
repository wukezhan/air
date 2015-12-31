dnl $Id$
dnl config.m4 for extension air

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(air, for air support,
dnl Make sure that the comment is aligned:
dnl [  --with-air             Include air support])

dnl Otherwise use enable:

PHP_ARG_ENABLE(air, whether to enable air support,
dnl Make sure that the comment is aligned:
[  --enable-air           Enable air support])

if test "$PHP_AIR" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-air -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/air.h"  # you most likely want to change this
  dnl if test -r $PHP_AIR/$SEARCH_FOR; then # path given as parameter
  dnl   AIR_DIR=$PHP_AIR
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for air files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       AIR_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$AIR_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the air distribution])
  dnl fi

  dnl # --with-air -> add include path
  dnl PHP_ADD_INCLUDE($AIR_DIR/include)

  dnl # --with-air -> check for lib and symbol presence
  dnl LIBNAME=air # you may want to change this
  dnl LIBSYMBOL=air # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $AIR_DIR/$PHP_LIBDIR, AIR_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_AIRLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong air lib version or lib not found])
  dnl ],[
  dnl   -L$AIR_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(AIR_SHARED_LIBADD)

  PHP_NEW_EXTENSION(air,
    air.c \
    src/air_app.c \
    src/air_async_service.c \
    src/air_async_waiter.c \
    src/air_config.c \
    src/air_controller.c \
    src/air_curl.c \
    src/air_curl_waiter.c \
    src/air_exception.c \
    src/air_handler.c \
    src/air_loader.c \
    src/air_mysql_builder.c \
    src/air_mysql_keeper.c \
    src/air_mysql_table.c \
    src/air_mysql_waiter.c \
    src/air_router.c \
    src/air_view.c \
    ,
  $ext_shared)
fi
