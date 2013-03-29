
bin_PROGRAMS = comedi_soft_calibrate

comedi_soft_calibrate_SOURCES = \
	comedi_soft_calibrate.cpp calibrator.hpp ni_m_series_calibrator.cpp ni_m_series_calibrator.hpp \
	calibrator_misc.hpp calibrator_misc.cpp calibration_set.cpp calibration_set.hpp
#comedi_soft_calibrate_CXXFLAGS =
comedi_soft_calibrate_LDADD = ../libcomedi_calibrate/libcomedi_calibrate.a \
	-lcomedi -lboost_program_options -lgsl -lgslcblas -lm

install-data-local:
	$(INSTALL) -d $(localstatedir)/lib/comedi/calibrations

EXTRA_DIST = results

