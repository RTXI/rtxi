CLEANFILES = *~
DISTCLEANFILES =
MAINTAINERCLEANFILES = Makefile.im

include $(top_srcdir)/Makefile.buildvars

LIBS = -lhdf5 -lhdf5_hl

bin_PROGRAMS =	rtxi_hdf_matlabize \
		rtxi_hdf_reader

rtxi_hdf_matlabize_SOURCES = \
		rtxi_hdf_matlabize.cpp

rtxi_hdf_reader_SOURCES = \
		rtxi_hdf_reader.cpp
