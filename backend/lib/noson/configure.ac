AC_PREREQ([2.60])
AC_INIT([noson], [1.0], [jlbarriere68@gmail.com])

AM_INIT_AUTOMAKE([1.10 foreign subdir-objects])

AC_CONFIG_SRCDIR([noson/src/sonossystem.cpp])

# Configure external packages.

# Checks for programs.
AC_LANG([C++])
AC_PROG_CC
AC_PROG_CXX
AC_PROG_CPP
AC_PROG_LIBTOOL
AM_CONDITIONAL([GCC], [test x$GCC = xyes])

# Checks for pthread.
AC_SEARCH_LIBS([pthread_create],[pthread],, AC_MSG_ERROR([required library 'pthread' is missing]))

# Checks for rt.
AC_SEARCH_LIBS(clock_gettime, [rt])

# Check for inttypes.
AC_CHECK_HEADERS([inttypes.h],, AC_MSG_ERROR([required header 'inttypes.h' is missing]))

# Checks for typedefs, structures, and compiler characteristics.
AC_C_INLINE
case $ac_cv_c_inline in
    yes) CC_INLINE=inline;;
    no) CC_INLINE=;;
    *) CC_INLINE=$ac_cv_c_inline;;
esac
AC_SUBST([CC_INLINE])

# Checks for library functions.
AC_CHECK_FUNCS([timegm localtime_r gmtime_r])

case "$ac_cv_func_timegm" in
     yes) HAVE_TIMEGM=1;;
     *) HAVE_TIMEGM=0;;
esac
AC_SUBST([HAVE_TIMEGM])

case "$ac_cv_func_localtime_r" in
     yes) HAVE_LOCALTIME_R=1;;
     *) HAVE_LOCATIME_R=0;;
esac
AC_SUBST([HAVE_LOCALTIME_R])

case "$ac_cv_func_gmtime_r" in
     yes) HAVE_GMTIME_R=1;;
     *) HAVE_GMTIME_R=0;;
esac
AC_SUBST([HAVE_GMTIME_R])

# Check for intrinsics
AC_MSG_CHECKING([for __sync_add_and_fetch(temp, 1)])
AC_TRY_COMPILE([],[long* temp=0; __sync_add_and_fetch(temp, 1);],
    [have_builtin_sync_add_and_fetch=yes],
    [have_builtin_sync_add_and_fetch=no])
AC_MSG_RESULT($have_builtin_sync_add_and_fetch)
if test "x$have_builtin_sync_add_and_fetch" = "xyes"; then
    AC_DEFINE(HAS_BUILTIN_SYNC_ADD_AND_FETCH, 1,
        [Define to 1 if your compiler supports the __sync_add_and_fetch() intrinsic.])
fi

AC_MSG_CHECKING([for __sync_sub_and_fetch(temp, 1)])
AC_TRY_COMPILE([],[long* temp=0; __sync_sub_and_fetch(temp, 1);],
    [have_builtin_sync_sub_and_fetch=yes],
    [have_builtin_sync_sub_and_fetch=no])
AC_MSG_RESULT($have_builtin_sync_sub_and_fetch)
if test "x$have_builtin_sync_sub_and_fetch" = "xyes"; then
    AC_DEFINE(HAS_BUILTIN_SYNC_SUB_AND_FETCH, 1,
        [Define to 1 if your compiler supports the __sync_sub_and_fetch() intrinsic.])
fi

### Require c++ standard
m4_include([m4/ax_check_compile_flag.m4])

AC_ARG_ENABLE([require-c++98],
  [AS_HELP_STRING([--enable-require-c++98],
  [require standard c++98 [default=no]])],
  [require_cxx_98=$enableval],
  [require_cxx_98=no])

if test "x${require_cxx_98}" = "xyes"; then
  CXX_STANDARD=199711L
else
  AX_CHECK_COMPILE_FLAG([-std=c++11], [compile_flag_cxx11=yes])
  if test "x${compile_flag_cxx11}" = "xyes"; then
    CXXFLAGS="-std=c++11 $CXXFLAGS"
    CXX_STANDARD=201103L
  else
    CXX_STANDARD=199711L
  fi
fi
AC_SUBST([CXX_STANDARD])

AC_CONFIG_FILES([
        Makefile
        noson/noson.pc
        noson/Makefile
        noson/src/local_config.h
        test/Makefile
])

AC_OUTPUT

AC_MSG_CHECKING([
Require c++98 ............... ${require_cxx_98}
Enable c++ features ......... ${CXX_STANDARD}

])
