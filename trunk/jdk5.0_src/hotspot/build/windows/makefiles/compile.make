# @(#)compile.make	1.24 04/04/12 14:10:17
# 
# Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
# 

# Generic compiler settings
CPP=cl.exe

!if "$(ARCH)" == "ia64"
MACHINE=IA64
CPP_FLAGS=/nologo /W3 /WX /D "CC_INTERP" /D "_LP64" /D "IA64"
!else
!if "$(ARCH)" == "amd64"
MACHINE=AMD64
CPP_FLAGS=/nologo /W3 /D "_LP64" /D "AMD64"
!else
CPP_FLAGS=/nologo /W3 /WX /Zi /D "IA32"
MACHINE=I386
!endif
!endif

# Generic linker settings
LINK=link.exe
LINK_FLAGS= kernel32.lib user32.lib gdi32.lib winspool.lib \
 comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib \
 uuid.lib Wsock32.lib winmm.lib /nologo /machine:$(MACHINE) /opt:REF \
 /opt:ICF,8 /map /debug

RC=rc.exe
RC_FLAGS=/D "HS_VER=$(HS_VER)" \
	 /D "HS_DOTVER=$(HS_DOTVER)" \
	 /D "HS_BUILD_ID=$(HS_BUILD_ID)" \
	 /D "HS_COMPANY=$(HS_COMPANY)" \
	 /D "HS_FILEDESC=$(HS_FILEDESC)" \
	 /D "HS_COPYRIGHT=$(HS_COPYRIGHT)" \
	 /D "HS_FNAME=$(HS_FNAME)" \
	 /D "HS_INTERNAL_NAME=$(HS_INTERNAL_NAME)" \
	 /D "HS_NAME=$(HS_NAME)"
