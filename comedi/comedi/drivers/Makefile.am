
SUBDIRS = addi-data

AM_CFLAGS = $(COMEDI_CFLAGS) $(LINUX_CFLAGS) $(RTAI_CFLAGS) $(RTLINUX_CFLAGS)
LINK = $(top_builddir)/modtool --link -o $@ \
	-i ../.mods/comedi.o.symvers \
	-i ../kcomedilib/.mods/kcomedilib.o.symvers \
	-i .mods/8255.o.symvers \
	-i .mods/comedi_fc.o.symvers \
	-i .mods/ni_tio.o.symvers \
	-i .mods/das08.o.symvers \
	-i .mods/ni_labpc.o.symvers
if CONFIG_COMEDI_PCI
LINK += \
	-i .mods/mite.o.symvers \
	-i .mods/ni_tiocmd.o.symvers
endif

8255_ko_LINK = $(top_builddir)/modtool --link -o $@ \
	-i ../.mods/comedi.o.symvers
comedi_fc_ko_LINK = $(top_builddir)/modtool --link -o $@ \
	-i ../.mods/comedi.o.symvers
das08_ko_LINK = $(top_builddir)/modtool --link -o $@ \
	-i ../.mods/comedi.o.symvers \
	-i .mods/8255.o.symvers
mite_ko_LINK = $(top_builddir)/modtool --link -o $@ \
	-i ../.mods/comedi.o.symvers
ni_labpc_ko_LINK = $(top_builddir)/modtool --link -o $@ \
	-i ../.mods/comedi.o.symvers \
	-i .mods/8255.o.symvers \
	-i .mods/comedi_fc.o.symvers
if CONFIG_COMEDI_PCI
ni_labpc_ko_LINK += \
	-i .mods/mite.o.symvers
endif
ni_tio_ko_LINK = $(top_builddir)/modtool --link -o $@ \
	-i ../.mods/comedi.o.symvers
ni_tiocmd_ko_LINK = $(top_builddir)/modtool --link -o $@ \
	-i ../.mods/comedi.o.symvers \
	-i .mods/ni_tio.o.symvers \
	-i .mods/mite.o.symvers

8255_ko_CFLAGS = $(AM_CFLAGS) -DEXPORT_SYMTAB
comedi_fc_ko_CFLAGS = $(AM_CFLAGS) -DEXPORT_SYMTAB
das08_ko_CFLAGS = $(AM_CFLAGS) -DEXPORT_SYMTAB
mite_ko_CFLAGS = $(AM_CFLAGS) -DEXPORT_SYMTAB
ni_labpc_ko_CFLAGS = $(AM_CFLAGS) -DEXPORT_SYMTAB
ni_tio_ko_CFLAGS = $(AM_CFLAGS) -DEXPORT_SYMTAB
ni_tiocmd_ko_CFLAGS = $(AM_CFLAGS) -DEXPORT_SYMTAB


EXTRA_DIST = \
 Kbuild \
 8253.h \
 8255.h \
 addi_apci_all.c \
 addi_apci_1710.c \
 addi_apci_3200.c \
 addi_apci_3300.c \
 am9513.h \
 amcc_s5933.h \
 comedi_fc.h \
 comedi_pci.h \
 das08.h \
 dt9812.h \
 icp_multi.h \
 jr3_pci.h \
 mite.h \
 me4000.h \
 ni_labpc.h \
 ni_mio_common.c \
 ni_stc.h \
 ni_tio.h \
 ni_tio_internal.h \
 plx9052.h \
 plx9080.h \
 rtd520.h \
 s626.h

# mpc8260cpm.ko

if CONFIG_COMEDI_RT
rt_modules = comedi_rt_timer.ko
else
rt_modules =
endif

if CONFIG_COMEDI_PCI
# split to build modules in correct order (for non-Kbuild builds)
pci_modules1= \
 mite.ko \
 ni_tiocmd.ko
pci_modules2= \
 addi_apci_035.ko \
 addi_apci_1032.ko \
 addi_apci_1500.ko \
 addi_apci_1516.ko \
 addi_apci_1564.ko \
 addi_apci_16xx.ko \
 addi_apci_2016.ko \
 addi_apci_2032.ko \
 addi_apci_2200.ko \
 addi_apci_3001.ko \
 addi_apci_3120.ko \
 addi_apci_3501.ko \
 addi_apci_3xxx.ko \
 adl_pci6208.ko \
 adl_pci7296.ko \
 adl_pci7432.ko \
 adl_pci8164.ko \
 adl_pci9111.ko \
 adl_pci9118.ko \
 adv_pci1710.ko \
 adv_pci1723.ko \
 adv_pci_dio.ko \
 amplc_pci224.ko \
 amplc_pci230.ko \
 cb_pcidas.ko \
 cb_pcidas64.ko \
 cb_pcidda.ko \
 cb_pcidio.ko \
 cb_pcimdas.ko \
 cb_pcimdda.ko \
 contec_pci_dio.ko \
 daqboard2000.ko \
 dt3000.ko \
 gsc_hpdi.ko \
 icp_multi.ko \
 ii_pci20kc.ko \
 jr3_pci.ko \
 ke_counter.ko \
 me_daq.ko \
 me4000.ko \
 ni_6527.ko \
 ni_65xx.ko \
 ni_660x.ko \
 ni_670x.ko \
 ni_pcidio.ko \
 ni_pcimio.ko \
 rtd520.ko \
 s626.ko
else
pci_modules1=
pci_modules2=
endif


if CONFIG_COMEDI_PCMCIA
pcmcia_modules= \
 cb_das16_cs.ko \
 das08_cs.ko \
 ni_daq_700.ko \
 ni_daq_dio24.ko \
 ni_mio_cs.ko \
 ni_labpc_cs.ko \
 quatech_daqp_cs.ko
else
pcmcia_modules=
endif

if CONFIG_COMEDI_USB
usb_modules= \
 usbdux.ko \
 usbduxfast.ko \
 dt9812.ko
else
usb_modules=
endif

if CONFIG_KBUILD
module_PROGRAMS =
else !CONFIG_KBUILD
module_PROGRAMS = \
 8255.ko \
 comedi_fc.ko \
 das08.ko \
 ni_tio.ko \
 $(pci_modules1) \
 ni_labpc.ko \
 acl7225b.ko \
 adq12b.ko \
 amplc_pc236.ko \
 amplc_pc263.ko \
 amplc_dio200.ko \
 aio_aio12_8.ko \
 aio_iiro_16.ko \
 c6xdigio.ko \
 comedi_bond.ko \
 das16.ko \
 das16m1.ko \
 das6402.ko \
 das800.ko \
 das1800.ko \
 dmm32at.ko \
 dt2801.ko \
 dt2811.ko \
 dt2814.ko \
 dt2815.ko \
 dt2817.ko \
 dt282x.ko \
 fl512.ko \
 mpc624.ko \
 multiq3.ko \
 ni_atmio.ko \
 ni_atmio16d.ko \
 ni_at_a2150.ko \
 ni_at_ao.ko \
 pcm3724.ko \
 pcm3730.ko \
 pcmad.ko \
 pcmda12.ko \
 poc.ko \
 pcl711.ko \
 pcl724.ko \
 pcl725.ko \
 pcl726.ko \
 pcl730.ko \
 pcl812.ko \
 pcl816.ko \
 pcl818.ko \
 pcmuio.ko \
 pcmmio.ko \
 comedi_parport.ko \
 rti800.ko \
 rti802.ko \
 serial2002.ko \
 skel.ko \
 ssv_dnp.ko \
 s526.ko \
 comedi_test.ko \
 unioxx5.ko \
 $(pci_modules2) \
 $(pcmcia_modules) \
 $(usb_modules) \
 $(rt_modules)
endif !CONFIG_KBUILD

8255_ko_SOURCES = 8255.c
acl7225b_ko_SOURCES = acl7225b.c
#addi_apci_all_ko_SOURCES = addi_apci_all.c
addi_apci_035_ko_SOURCES = addi_apci_035.c
addi_apci_1032_ko_SOURCES = addi_apci_1032.c
addi_apci_1500_ko_SOURCES = addi_apci_1500.c
addi_apci_1516_ko_SOURCES = addi_apci_1516.c
addi_apci_1564_ko_SOURCES = addi_apci_1564.c
addi_apci_16xx_ko_SOURCES = addi_apci_16xx.c
#addi_apci_1710_ko_SOURCES = addi_apci_1710.c
addi_apci_2016_ko_SOURCES = addi_apci_2016.c
addi_apci_2032_ko_SOURCES = addi_apci_2032.c
addi_apci_2200_ko_SOURCES = addi_apci_2200.c
addi_apci_3001_ko_SOURCES = addi_apci_3001.c
addi_apci_3120_ko_SOURCES = addi_apci_3120.c
#addi_apci_3200_ko_SOURCES = addi_apci_3200.c
#addi_apci_3300_ko_SOURCES = addi_apci_3300.c
addi_apci_3501_ko_SOURCES = addi_apci_3501.c
addi_apci_3xxx_ko_SOURCES = addi_apci_3xxx.c
adl_pci6208_ko_SOURCES = adl_pci6208.c
adl_pci7296_ko_SOURCES = adl_pci7296.c
adl_pci7432_ko_SOURCES = adl_pci7432.c
adl_pci8164_ko_SOURCES = adl_pci8164.c
adl_pci9111_ko_SOURCES = adl_pci9111.c
adl_pci9118_ko_SOURCES = adl_pci9118.c
adq12b_ko_SOURCES = adq12b.c
adv_pci1710_ko_SOURCES = adv_pci1710.c
adv_pci1723_ko_SOURCES = adv_pci1723.c
adv_pci_dio_ko_SOURCES = adv_pci_dio.c
aio_aio12_8_ko_SOURCES = aio_aio12_8.c
aio_iiro_16_ko_SOURCES = aio_iiro_16.c
amplc_pci224_ko_SOURCES = amplc_pci224.c
amplc_pci230_ko_SOURCES = amplc_pci230.c
amplc_pc236_ko_SOURCES = amplc_pc236.c
amplc_pc263_ko_SOURCES = amplc_pc263.c
amplc_dio200_ko_SOURCES = amplc_dio200.c
c6xdigio_ko_SOURCES = c6xdigio.c
cb_pcidas_ko_SOURCES = cb_pcidas.c
cb_pcidas64_ko_SOURCES = cb_pcidas64.c
cb_pcidda_ko_SOURCES = cb_pcidda.c
cb_pcidio_ko_SOURCES = cb_pcidio.c
cb_pcimdas_ko_SOURCES = cb_pcimdas.c
cb_pcimdda_ko_SOURCES = cb_pcimdda.c
cb_das16_cs_ko_SOURCES = cb_das16_cs.c
comedi_bond_ko_SOURCES = comedi_bond.c
comedi_fc_ko_SOURCES = comedi_fc.c
contec_pci_dio_ko_SOURCES = contec_pci_dio.c
daqboard2000_ko_SOURCES = daqboard2000.c
das08_ko_SOURCES = das08.c
das08_cs_ko_SOURCES = das08_cs.c
das16_ko_SOURCES = das16.c
das16m1_ko_SOURCES = das16m1.c
das6402_ko_SOURCES = das6402.c
das800_ko_SOURCES = das800.c
das1800_ko_SOURCES = das1800.c
dmm32at_ko_SOURCES = dmm32at.c
dt2801_ko_SOURCES = dt2801.c
dt2811_ko_SOURCES = dt2811.c
dt2814_ko_SOURCES = dt2814.c
dt2815_ko_SOURCES = dt2815.c
dt2817_ko_SOURCES = dt2817.c
dt282x_ko_SOURCES = dt282x.c
dt3000_ko_SOURCES = dt3000.c
dt9812_ko_SOURCES = dt9812.c
fl512_ko_SOURCES = fl512.c
gsc_hpdi_ko_SOURCES = gsc_hpdi.c
ii_pci20kc_ko_SOURCES = ii_pci20kc.c
icp_multi_ko_SOURCES = icp_multi.c
jr3_pci_ko_SOURCES = jr3_pci.c
ke_counter_ko_SOURCES = ke_counter.c
me4000_ko_SOURCES = me4000.c
me_daq_ko_SOURCES = me_daq.c
mite_ko_SOURCES = mite.c
mpc624_ko_SOURCES = mpc624.c
multiq3_ko_SOURCES = multiq3.c
ni_660x_ko_SOURCES = ni_660x.c
ni_670x_ko_SOURCES = ni_670x.c
ni_atmio_ko_SOURCES = ni_atmio.c
ni_mio_cs_ko_SOURCES = ni_mio_cs.c
ni_pcimio_ko_SOURCES = ni_pcimio.c
ni_pcidio_ko_SOURCES = ni_pcidio.c
ni_6527_ko_SOURCES = ni_6527.c
ni_65xx_ko_SOURCES = ni_65xx.c
ni_atmio16d_ko_SOURCES = ni_atmio16d.c
ni_at_a2150_ko_SOURCES = ni_at_a2150.c
ni_labpc_ko_SOURCES = ni_labpc.c
ni_labpc_cs_ko_SOURCES = ni_labpc_cs.c
ni_daq_700_ko_SOURCES = ni_daq_700.c
ni_daq_dio24_ko_SOURCES = ni_daq_dio24.c
ni_at_ao_ko_SOURCES = ni_at_ao.c
ni_tio_ko_SOURCES = ni_tio.c
ni_tiocmd_ko_SOURCES = ni_tiocmd.c
pcm3724_ko_SOURCES = pcm3724.c
pcm3730_ko_SOURCES = pcm3730.c
pcmad_ko_SOURCES = pcmad.c
pcl711_ko_SOURCES = pcl711.c
pcl724_ko_SOURCES = pcl724.c
pcl725_ko_SOURCES = pcl725.c
pcl726_ko_SOURCES = pcl726.c
pcl730_ko_SOURCES = pcl730.c
pcl812_ko_SOURCES = pcl812.c
pcl816_ko_SOURCES = pcl816.c
pcl818_ko_SOURCES = pcl818.c
pcmda12_ko_SOURCES = pcmda12.c
pcmuio_ko_SOURCES = pcmuio.c
pcmmio_ko_SOURCES = pcmmio.c
poc_ko_SOURCES = poc.c
quatech_daqp_cs_ko_SOURCES = quatech_daqp_cs.c
comedi_parport_ko_SOURCES = comedi_parport.c
comedi_rt_timer_ko_SOURCES = comedi_rt_timer.c
rtd520_ko_SOURCES = rtd520.c
rti800_ko_SOURCES = rti800.c
rti802_ko_SOURCES = rti802.c
serial2002_ko_SOURCES = serial2002.c
skel_ko_SOURCES = skel.c
ssv_dnp_ko_SOURCES = ssv_dnp.c
s526_ko_SOURCES = s526.c
s626_ko_SOURCES = s626.c
comedi_test_ko_SOURCES = comedi_test.c
unioxx5_ko_SOURCES = unioxx5.c
usbdux_ko_SOURCES = usbdux.c
usbduxfast_ko_SOURCES = usbduxfast.c

clean-modulePROGRAMS:
	-test -z "$(module_PROGRAMS)" || rm -f $(module_PROGRAMS)
	-rm -rf .mods

