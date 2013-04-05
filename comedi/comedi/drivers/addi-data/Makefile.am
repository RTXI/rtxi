
AM_CFLAGS = $(COMEDI_CFLAGS) $(LINUX_CFLAGS) $(RTAI_CFLAGS) $(RTLINUX_CFLAGS)
LINK = $(top_builddir)/modtool --link -o $@

if CONFIG_KBUILD
module_PROGRAMS =
else
#disabling addi driver until its floating-point house is put in order
#module_PROGRAMS = addi_common.ko
module_PROGRAMS =
endif

#moving addi_common.c into EXTRA_DIST until the driver is fixed.
#addi_common_ko_SOURCES = addi_common.c

EXTRA_DIST = \
	Kbuild \
	APCI1710_82x54.c \
	APCI1710_82x54.h \
	APCI1710_Chrono.c \
	APCI1710_Chrono.h \
	APCI1710_Dig_io.c \
	APCI1710_Dig_io.h \
	APCI1710_INCCPT.c \
	APCI1710_INCCPT.h \
	APCI1710_Inp_cpt.c \
	APCI1710_Inp_cpt.h \
	APCI1710_Pwm.c \
	APCI1710_Pwm.h \
	APCI1710_Ssi.c \
	APCI1710_Ssi.h \
	APCI1710_Tor.c \
	APCI1710_Tor.h \
	APCI1710_Ttl.c \
	APCI1710_Ttl.h \
	addi_amcc_S5920.c \
	addi_amcc_S5920.h \
	addi_amcc_s5933.h \
	addi_common.c \
	addi_common.h \
	addi_eeprom.c \
	amcc_s5933_58.h \
	hwdrv_APCI1710.c \
	hwdrv_APCI1710.h \
	hwdrv_apci035.c \
	hwdrv_apci035.h \
	hwdrv_apci1032.c \
	hwdrv_apci1032.h \
	hwdrv_apci1500.c \
	hwdrv_apci1500.h \
	hwdrv_apci1516.c \
	hwdrv_apci1516.h \
	hwdrv_apci1564.c \
	hwdrv_apci1564.h \
	hwdrv_apci16xx.c \
	hwdrv_apci16xx.h \
	hwdrv_apci2016.c \
	hwdrv_apci2016.h \
	hwdrv_apci2032.c \
	hwdrv_apci2032.h \
	hwdrv_apci2200.c \
	hwdrv_apci2200.h \
	hwdrv_apci3120.c \
	hwdrv_apci3120.h \
	hwdrv_apci3200.c \
	hwdrv_apci3200.h \
	hwdrv_apci3501.c \
	hwdrv_apci3501.h \
	hwdrv_apci3xxx.c \
	hwdrv_apci3xxx.h

clean-modulePROGRAMS:
	-test -z "$(module_PROGRAMS)" || rm -f $(module_PROGRAMS)
	-rm -rf .mods

