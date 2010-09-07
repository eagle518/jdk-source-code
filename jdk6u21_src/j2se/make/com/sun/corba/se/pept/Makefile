# @(#)Makefile	1.7 05/08/13

BUILDDIR = ../../../../..
PACKAGE = com.sun.corba.se.pept
PRODUCT = sun

include $(BUILDDIR)/common/Defs.gmk

#
# Files
#
CORBA_JMK_DIRECTORY=$(TOPDIR)/make/com/sun/corba/minclude/
include $(CORBA_JMK_DIRECTORY)com_sun_corba_se_pept.jmk
include $(CORBA_JMK_DIRECTORY)com_sun_corba_se_spi_logging.jmk
include $(CORBA_JMK_DIRECTORY)com_sun_corba_se_impl_logging.jmk

FILES_java=\
        $(com_sun_corba_se_pept_java) \
        $(com_sun_corba_se_impl_logging_java) \
        $(com_sun_corba_se_spi_logging_java)

#
# Include
#
include $(BUILDDIR)/common/Classes.gmk
