noinst_PROGRAMS = nosontest

nosontest_SOURCES = src/test.cpp

AM_CPPFLAGS = -I@abs_top_srcdir@/noson/src

nosontest_LDFLAGS = -static

nosontest_LDADD   = \
	@abs_top_builddir@/noson/libnoson.la
