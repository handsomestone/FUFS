## Process this file with automake to produce Makefile.in
include $(abs_top_builddir)/src/rules.mk



DIST_SUBDIRS = $(SUBDIRS)

lib_LTLIBRARIES = libfufs.la

libfufs_la_SOURCES = \	
	fufs_curl.c	\
	sha2.c	\
	hmac_sha2.c\
	fufs_conf.c\
	fufs_api.c\
	fufs_util.c\
	fufs_node.c
	
AM_CPPFLAGS = -DFUSE_USE_VERSION=26

bin_PROGRAMS = fufs

fufs_SOURCES = \
	fufs.h		\
	fufs.c	

fufs_LDADD = $(MYLDFLAGS)

EXTRA_DIST = \
	rules.mk	\
	fufs_api.h		\
	fufs_curl.h	\
	sha2.h	\
	hmac_sha2.h\
	fufs_node.h\
	fufs_util.h\
	fufs_conf.h\
	fufs.h

fufsdir=$(includedir)/fufs
fufs_HEADERS = \
	fufs_api.h		\
	fufs_curl.h	\
	sha2.h	\
	hmac_sha2.h\
	fufs.h\
	fufs_util.h\
	fufs_conf.h\
	fufs.h\
	fufs_node.h
