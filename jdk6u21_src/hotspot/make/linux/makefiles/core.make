#
# Copyright (c) 1999, 2008, Oracle and/or its affiliates. All rights reserved.
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

# Sets make macros for making core version of VM

# Note the effect on includeDB lists in top.make:
# includeDB_compiler* and ad_<arch>.*pp are excluded from the build,
TYPE=CORE

# There is no "core" directory in JDK. Install core build in server directory.
VM_SUBDIR = server

# Note:  macros.hpp defines CORE
