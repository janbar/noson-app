#noinst_LTLIBRARIES = libnoson.la
lib_LTLIBRARIES    = libnoson.la
# LIB_VERSION   : [$1-$3].[$3].[$2]
# LIB_SOVERSION : [$1-$3]
libnoson_la_LDFLAGS = -version-info 1:0:0

AM_CPPFLAGS = -fPIC -Wall -Wextra \
	-I@abs_srcdir@/src

libnoson_la_SOURCES = \
	src/private/os/threads/threadpool.cpp \
	src/private/builtin.c \
	src/private/wsresponse.cpp \
	src/private/uriparser.cpp \
	src/private/didlparser.cpp \
	src/private/wsrequest.cpp \
	src/private/tinyxml2.cpp \
	src/private/debug.cpp \
	src/private/wsrequestbroker.cpp \
	src/private/wscontent.cpp \
	src/private/socket.cpp \
	src/private/eventbroker.cpp \
	src/locked.cpp \
	src/intrinsic.cpp \
	src/avtransport.cpp \
	src/contentdirectory.cpp \
	src/deviceproperties.cpp \
	src/digitalitem.cpp \
	src/eventhandler.cpp \
	src/renderingcontrol.cpp \
	src/service.cpp \
	src/subscription.cpp \
	src/sonosplayer.cpp \
	src/sonossystem.cpp \
	src/sonostypes.cpp \
	src/sonoszone.cpp

nosonheadersdir = $(includedir)/noson
nosonheaders_HEADERS = \
	src/local_config.h \
	src/locked.h \
	src/intrinsic.h \
	src/sharedptr.h \
	src/avtransport.h \
	src/contentdirectory.h \
	src/deviceproperties.h \
	src/digitalitem.h \
	src/element.h \
	src/eventhandler.h \
	src/renderingcontrol.h \
	src/service.h \
	src/subscription.h \
	src/sonosplayer.h \
	src/sonossystem.h \
	src/sonostypes.h \
	src/sonoszone.h
