INCLUDES = \
	-Wall	\
	 -g	\
	-I$(abs_top_srcdir)/src

MYLDFLAGS = \
	$(abs_top_builddir)/src/libfufs.la	\
	-ljson	\
	-lglib-2.0	\
	-lcurl		\
	-loauth

