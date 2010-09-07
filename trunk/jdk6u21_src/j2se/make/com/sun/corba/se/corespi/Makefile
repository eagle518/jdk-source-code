# @(#)Makefile	1.4 03/12/16

BUILDDIR = ../../../../..

PACKAGE = com.sun.corba.se.impl.corespi
PRODUCT = sun

include $(BUILDDIR)/common/Defs.gmk

#
# Files
#
CORBA_JMK_DIRECTORY=$(TOPDIR)/make/com/sun/corba/minclude/
include $(CORBA_JMK_DIRECTORY)com_sun_corba_se_spi_ior.jmk
include $(CORBA_JMK_DIRECTORY)com_sun_corba_se_spi_oa.jmk
include $(CORBA_JMK_DIRECTORY)com_sun_corba_se_spi_orb.jmk
include $(CORBA_JMK_DIRECTORY)com_sun_corba_se_spi_protocol.jmk
include $(CORBA_JMK_DIRECTORY)com_sun_corba_se_spi_resolver.jmk
include $(CORBA_JMK_DIRECTORY)com_sun_corba_se_spi_transport.jmk
include $(CORBA_JMK_DIRECTORY)com_sun_corba_se_spi_orbutil.jmk
include $(CORBA_JMK_DIRECTORY)com_sun_corba_se_spi_presentation_rmi.jmk

#
# Include
#

FILES_java =    $(com_sun_corba_se_spi_ior_java) \
                $(com_sun_corba_se_spi_oa_java) \
                $(com_sun_corba_se_spi_orb_java) \
                $(com_sun_corba_se_spi_protocol_java) \
                $(com_sun_corba_se_spi_resolver_java) \
                $(com_sun_corba_se_spi_transport_java) \
                $(com_sun_corba_se_spi_orbutil_java) \
                $(com_sun_corba_se_spi_presentation_rmi_java)

include $(BUILDDIR)/common/Classes.gmk
