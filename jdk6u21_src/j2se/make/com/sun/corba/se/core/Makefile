#
# @(#)Makefile	1.16 10/03/23
#
# Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

BUILDDIR = ../../../../..

PACKAGE = com.sun.corba.se.impl.core
PRODUCT = sun

include $(BUILDDIR)/common/Defs.gmk

#
# Files
#
CORBA_JMK_DIRECTORY=$(TOPDIR)/make/com/sun/corba/minclude/
include $(CORBA_JMK_DIRECTORY)com_sun_corba_se_impl_corba.jmk
include $(CORBA_JMK_DIRECTORY)com_sun_corba_se_impl_core.jmk
include $(CORBA_JMK_DIRECTORY)com_sun_corba_se_impl_dynamicany.jmk
include $(CORBA_JMK_DIRECTORY)com_sun_corba_se_impl_encoding.jmk
include $(CORBA_JMK_DIRECTORY)com_sun_corba_se_impl_legacy.jmk
include $(CORBA_JMK_DIRECTORY)com_sun_corba_se_spi_servicecontext.jmk
include $(CORBA_JMK_DIRECTORY)com_sun_corba_se_impl_ior.jmk
include $(CORBA_JMK_DIRECTORY)com_sun_corba_se_impl_orb.jmk
include $(CORBA_JMK_DIRECTORY)com_sun_corba_se_impl_orbutil.jmk
include $(CORBA_JMK_DIRECTORY)com_sun_corba_se_impl_protocol.jmk
include $(CORBA_JMK_DIRECTORY)com_sun_corba_se_impl_resolver.jmk
include $(CORBA_JMK_DIRECTORY)com_sun_corba_se_impl_transport.jmk
include $(CORBA_JMK_DIRECTORY)com_sun_corba_se_impl_util.jmk
include $(CORBA_JMK_DIRECTORY)com_sun_corba_se_impl_io.jmk
include $(CORBA_JMK_DIRECTORY)com_sun_corba_se_impl_presentation_rmi.jmk
include $(CORBA_JMK_DIRECTORY)com_sun_corba_se_internal_LegacyFiles.jmk



#
# Include
#

FILES_java =	$(com_sun_corba_se_impl_corba_java) \
		$(com_sun_corba_se_impl_core_java) \
		$(com_sun_corba_se_impl_dynamicany_java) \
		$(com_sun_corba_se_impl_encoding_java) \
		$(com_sun_corba_se_impl_legacy_java) \
		$(com_sun_corba_se_spi_servicecontext_java) \
		$(com_sun_corba_se_impl_ior_java) \
		$(com_sun_corba_se_impl_orb_java) \
		$(com_sun_corba_se_impl_orbutil_java) \
		$(com_sun_corba_se_impl_protocol_java) \
		$(com_sun_corba_se_impl_resolver_java) \
		$(com_sun_corba_se_impl_transport_java) \
		$(com_sun_corba_se_impl_util_java) \
		$(com_sun_corba_se_impl_io_java) \
		$(com_sun_corba_se_impl_presentation_rmi_java) \
		$(com_sun_corba_se_internal_LegacyFiles_java)

include $(BUILDDIR)/common/Classes.gmk
