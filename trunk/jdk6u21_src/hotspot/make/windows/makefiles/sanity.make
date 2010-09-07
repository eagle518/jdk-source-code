#
# Copyright (c) 2006, 2009, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#
#  
#

!include local.make

all: checkCL checkLink

checkCL:
	@ if "$(MSC_VER)" NEQ "1310" if "$(MSC_VER)" NEQ "1399" if "$(MSC_VER)" NEQ "1400" if "$(MSC_VER)" NEQ "1500" \
	echo *** WARNING *** unrecognized cl.exe version $(MSC_VER) ($(RAW_MSC_VER)).  Use FORCE_MSC_VER to override automatic detection.

checkLink:
	@ if "$(LINK_VER)" NEQ "710" if "$(LINK_VER)" NEQ "800" if "$(LINK_VER)" NEQ "900" \
	echo *** WARNING *** unrecognized link.exe version $(LINK_VER) ($(RAW_LINK_VER)).  Use FORCE_LINK_VER to override automatic detection.
