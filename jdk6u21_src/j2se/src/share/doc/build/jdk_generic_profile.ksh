#!/bin/ksh

# @(#)jdk_generic_profile.ksh	1.10 05/09/18

#############################################################################
#
# Generic build profile.ksh for all platforms, works in bash, sh, and ksh.
#
# Copy this file to your own area, and edit it to suit your needs.
#
# Ideally you either won't need to set the ALT_* variables because the
#   build system will find what it needs through system provided paths
#   or environment variables, or you have installed the component in the
#   recommended default path. But if you need to set an ALT_* variable,
#   edit the "export YOUR_ALT_*" to "export ALT_*".
#
# If you find yourself forced to set an ALT_* environment variable and
#   suspect we could have figured it out automatically, please let us know.
#
# Most ALT_* directory defaults are based on being in the parent directory in
#    ALT_SLASH_JAVA, so it's possible to create for example a "C:/mustang"
#    directory, assign that to ALT_SLASH_JAVA, and place all the components
#    in that directory. See the use of 'slash_java' below. This could also
#    minimize the ALT_* environment variables.
#
########
#
# Detects CYGWIN or MKS usage on Windows automatically.
# Uses MKS dosname and CYGWIN cygpath to make sure paths are space-free.
# Assumes basic unix utilities are in the PATH already (uname, hostname, etc.).
# On Windows, assumes PROCESSOR_IDENTIFIER, ROOTDIR (if MKS), VS71COMNTOOLS,
#   SYSTEMROOT (or SystemRoot), COMPUTERNAME (or hostname works), and
#   USERNAME is defined in the environment.
# This profile does not rely on using vcvars32.bat and 64bit Setup.bat.
# The JDK Makefiles may change in the future, making some of these
#   settings unnecessary or redundant.
# This is just an example profile and may or may not work on all systems.
#
#############################################################################
#
# WARNING: Will clobber the value of some environment variables.
#
# Sets up these environment variables for doing JDK builds:
#    USERNAME
#    COMPUTERNAME
#    PATH
#    Windows Only:
#      LIB
#      INCLUDE
#      PS1
#      SHELL
#      MKS Only:
#        ENVIRON
#        ENV
#        PROMPT
#
# Attempts to set these variables for the JDK builds:           
#    ALT_COMPILER_PATH
#    ALT_DEVTOOLS_PATH
#    ALT_BOOTDIR
#    ALT_JDK_IMPORT_PATH
#    ALT_MOTIF_DIR
#    ALT_ODBCDIR
#    ALT_CACERTS_FILE
#    ALT_GCC_COMPILER_PATH
#    ALT_MOZILLA_HEADERS_PATH
#    ALT_GCC29_COMPILER_PATH
#    Windows Only:
#      ALT_UNIXCOMMAND_PATH
#      ALT_MSDEVTOOLS_PATH
#      ALT_DXSDK_PATH
#      ALT_MSVCRT_DLL_PATH
#      ALT_UNICOWS_LIB_PATH
#      ALT_UNICOWS_DLL_PATH
#
#############################################################################
#
# Keep in mind that at this point, we are running in some kind of shell
#   (sh, ksh, or bash). We don't know if it's solaris, linux, windows MKS, 
#    or windows CYGWIN. We need to figure that out.

# Find user name
if [ "${USERNAME}" = "" ] ; then
    USERNAME="${LOGNAME}"
fi
if [ "${USERNAME}" = "" ] ; then
    USERNAME="${USER}"
fi
export USERNAME

# Find machine name
if [ "${COMPUTERNAME}" = "" ] ; then
    COMPUTERNAME="$(hostname)"
fi
export COMPUTERNAME

# Uses 'uname -s', but only expect SunOS or Linux, assume Windows otherwise.
osname=$(uname -s)
if [ "${osname}" = SunOS ] ; then
   
    # Root of where components are located
    slash_java=/java

    # SOLARIS STEP #0: Sparc or X86
    osarch=$(uname -p)
    if [ "${osarch}" = sparc ] ; then
	solaris_arch=sparc
    else
	solaris_arch=i386
    fi

    # SOLARIS STEP #1: Get the SS11 compilers (and latest patches for them too)
    #   NOTE: Place compiler path early in PATH to avoid 'cc' conflicts.
    # Add to PATH and set ALT_COMPILER_PATH
    compiler_path=${slash_java}/devtools/mustang/${solaris_arch}/SUNWspro/SS11/bin
    path4sdk=${compiler_path}
    if [ -d ${compiler_path} ] ; then
        export YOUR_ALT_COMPILER_PATH=${compiler_path}
    else
	echo "WARNING: Cannot access compiler_path=${compiler_path}"
    fi

    # SOLARIS STEP #2: Get the devtools needed (gnumake, zip, unzipsfx)
    #   NOTE: Place devtools path early in PATH to avoid 'zip' conflicts.
    #   NOTE: If you have /opt/sfw/bin/gmake, you can alias gnumake=/opt/sfw/bin/gmake
    #   WARN: Do not use "/usr/ccs/bin/make"
    # Add to PATH and set ALT_DEVTOOLS_PATH.
    devtools_path=${slash_java}/devtools/mustang/${solaris_arch}/bin
    path4sdk=${path4sdk}:${devtools_path}
    if [ -d ${devtools_path} ] ; then
        export YOUR_ALT_DEVTOOLS_PATH=${devtools_path}
    else
	echo "WARNING: Cannot access devtools_path=${devtools_path}"
    fi

    # Add basic solaris system paths
    path4sdk=${path4sdk}:/usr/ccs/bin:/usr/ccs/lib:/usr/bin:/bin:/usr/sfw/bin

    # SOLARIS STEP #3: Get the previous JDK to be used to bootstrap the build
    #   NOTE: You could use jdk1.6.0 and skip this step (but you need to do step 4)
    # Use a local copy of jdk1.5.0 if possible.
    bootdir=/opt/java/jdk1.5.0
    if [ -d ${bootdir} ] ; then
	export YOUR_ALT_BOOTDIR=${bootdir}
    else
	echo "WARNING: Cannot access bootdir=${bootdir}"
    fi
    
    # SOLARIS STEP #4: Get the latest JDK build to be used to import pre-built binaries
    #   NOTE: Only if you want to skip parts of the build, e.g. like skip the hotspot build
    # Use a local copy of a recent jdk1.6.0 if possible.
    jdk_import_path=/opt/java/jdk1.6.0
    if [ -d ${jdk_import_path} ] ; then
	export YOUR_ALT_JDK_IMPORT_PATH=${jdk_import_path}
    else
	echo "WARNING: Cannot access jdk_import_path=${jdk_import_path}"
    fi
   
    # SOLARIS STEP #5: Directory that contains a motif21/{lib,include}
    motif_dir=${slash_java}/devtools/${solaris_arch}
    if [ -d ${motif_dir} ] ; then
	export YOUR_ALT_MOTIF_DIR=${motif_dir}
    else
	echo "WARNING: Cannot access motif_dir=${motif_dir}"
    fi
    
    # SOLARIS STEP #6: ODBC Driver
    odbcdir=${slash_java}/devtools/${solaris_arch}/jdbc-odbc
    if [ -d ${odbcdir} ] ; then
	export YOUR_ALT_ODBCDIR=${odbcdir}
    else
	echo "WARNING: Cannot access odbcdir=${odbcdir}"
    fi

    # SOLARIS STEP #7: A certificates file
    cacerts_file=${jdk_import_path}/jre/lib/security/cacerts
    if [ -f ${cacerts_file} ] ; then
	export YOUR_ALT_CACERTS_FILE=${cacerts_file}
    else
	echo "WARNING: Cannot access cacerts_file=${cacerts_file}"
    fi

    # SOLARIS STEP #8: Path to 2.95.2 gcc for building the plugin
    gcc_compiler_path=${slash_java}/devtools/${solaris_arch}/gnucc/bin/ 
    if [ -d ${gcc_compiler_path} ] ; then
	export YOUR_ALT_GCC_COMPILER_PATH=${gcc_compiler_path}
    else
	echo "WARNING: Cannot access gcc_compiler_path=${gcc_compiler_path}"
    fi

    # SOLARIS STEP #9: Mozilla header files for building the plugin
    mozilla_headers_path=${slash_java}/devtools/share/plugin
    if [ -d ${mozilla_headers_path} ] ; then
	export YOUR_ALT_MOZILLA_HEADERS_PATH=${mozilla_headers_path}
    else
	echo "WARNING: Cannot access mozilla_headers_path=${mozilla_headers_path}"
    fi

    # File creation mask
    umask 002

elif [ "${osname}" = Linux ] ; then
   
    # Root of where components are located
    slash_java=/java

    # LINUX STEP #0: X86, AMD64
    osarch=$(uname -m)
    if [ "${osarch}" = i686 ] ; then
	linux_arch=i586
    elif [ "${osarch}" = x86_64 ] ; then
	linux_arch=amd64
    fi

    # LINUX STEP #1: Verify the linux you have has the right compilers, we assume so.
    path4sdk=${compiler_path}
    compiler_path=/usr/bin
    if [ -d ${compiler_path} ] ; then
        export YOUR_ALT_COMPILER_PATH=${compiler_path}
    else
	echo "WARNING: Cannot access compiler_path=${compiler_path}"
    fi
    
    # LINUX STEP #2: Get the devtools needed (gnumake, zip, unzipsfx)
    #   NOTE: You can alias gnumake=/usr/bin/make, or just use "make"
    # Add to PATH and set ALT_DEVTOOLS_PATH.
    #gnumake=/usr/bin/make
    devtools_path=${slash_java}/devtools/linux/bin
    path4sdk=${path4sdk}:${devtools_path}
    if [ -d ${devtools_path} ] ; then
        export YOUR_ALT_DEVTOOLS_PATH=${devtools_path}
    else
	echo "WARNING: Cannot access devtools_path=${devtools_path}"
    fi

    # Add basic paths
    path4sdk=${path4sdk}:/usr/bin:/bin:/usr/sbin:/sbin

    # LINUX STEP #3: Get the previous JDK to be used to bootstrap the build
    #   NOTE: You could use jdk1.6.0 and skip this step (but you need to do step 4)
    # Use a local copy of jdk1.5.0 if possible.
    bootdir=/opt/java/jdk1.5.0
    if [ -d ${bootdir} ] ; then
	export YOUR_ALT_BOOTDIR=${bootdir}
    else
	echo "WARNING: Cannot access bootdir=${bootdir}"
    fi
    
    # LINUX STEP #4: Get the latest JDK build to be used to import pre-built binaries
    #   NOTE: Only if you want to skip parts of the build, e.g. like skip the hotspot build
    # Use a local copy of a recent jdk1.6.0 if possible.
    jdk_import_path=/opt/java/jdk1.6.0
    if [ -d ${jdk_import_path} ] ; then
	export YOUR_ALT_JDK_IMPORT_PATH=${jdk_import_path}
    else
	echo "WARNING: Cannot access jdk_import_path=${jdk_import_path}"
    fi
    
    # LINUX STEP #5: Directory that contains a {lib,include}
    #   NOTE: You normally build this from the motif sources in control build.
    #   NOTE: Be careful you get the right arch binaries.
    motif_dir=${slash_java}/devtools/linux/motif-${linux_arch}
    if [ -d ${motif_dir} ] ; then
	export YOUR_ALT_MOTIF_DIR=${motif_dir}
    else
	echo "WARNING: Cannot access motif_dir=${motif_dir}"
    fi
    
    # LINUX STEP #6: A certificates file
    cacerts_file=${jdk_import_path}/jre/lib/security/cacerts
    if [ -f ${cacerts_file} ] ; then
	export YOUR_ALT_CACERTS_FILE=${cacerts_file}
    else
	echo "WARNING: Cannot access cacerts_file=${cacerts_file}"
    fi
    
    # LINUX STEP #7: Mozilla header files for building the plugin
    mozilla_headers_path=${slash_java}/devtools/share/plugin
    if [ -d ${mozilla_headers_path} ] ; then
	export YOUR_ALT_MOZILLA_HEADERS_PATH=${mozilla_headers_path}
    else
	echo "WARNING: Cannot access mozilla_headers_path=${mozilla_headers_path}"
    fi
   
    # LINUX STEP #8: OJI Plugin needs gcc 2.9 or ALT_GCC29_PLUGIN_LIB_PATH
    gcc29_compiler_path=${slash_java}/devtools/linux/gcc29/usr
    if [ -d ${gcc29_compiler_path} ] ; then
	export YOUR_ALT_GCC29_COMPILER_PATH=${gcc29_compiler_path}
    else
	echo "WARNING: Cannot access gcc29_compiler_path=${gcc29_compiler_path}"
    fi

    umask 002

else

    # Root of where components are located
    slash_java=J:

    # Windows: Differs on CYGWIN vs. MKS, and the compiler available.
    #   Also, blanks in pathnames gives gnumake headaches, so anything placed
    #   in any ALT_* variable should be the short windows dosname.
   
    # WINDOWS STEP #0: Install and use MKS or CYGWIN (should have already been done)
    #   Assumption here is that you are in a shell window via MKS or cygwin.
    #   MKS install should have defined the environment variable ROOTDIR.
    #   We also need to figure out which one we have: X86, AMD64
    if [ "$(echo ${PROCESSOR_IDENTIFIER} | fgrep AMD64)" != "" ] ; then
	windows_arch=amd64
    else
	windows_arch=i586
    fi
    # We need to determine if we are running a CYGWIN shell or an MKS shell
    #    (if uname isn't available, then it will be unix_toolset=unknown)
    unix_toolset=unknown
    if [ "$(uname -a | fgrep Cygwin)" = "" -a -d "${ROOTDIR}" ] ; then
        # We kind of assume ROOTDIR is where MKS is and it's ok
        unix_toolset=MKS
        mkshome=$(dosname -s "${ROOTDIR}")
	# Utility to convert to short pathnames without spaces
	dosname="${mkshome}/mksnt/dosname -s"
        # Most unix utilities are in the mksnt directory of ROOTDIR
        unixcommand_path="${mkshome}/mksnt"
        # Make the prompt tell you MKS
        export PS1="MKS:${COMPUTERNAME}:${USERNAME}[!] "
        # Get tab completion
        set -o tabcomplete
    elif [ "$(uname -a | fgrep Cygwin)" != "" -a -f /bin/cygpath ] ; then
        # For CYGWIN, uname will have "Cygwin" in it, and /bin/cygpath should exist
        unix_toolset=CYGWIN
	# Utility to convert to short pathnames without spaces
	dosname="/usr/bin/cygpath -a -m -s"
        # Most unix utilities are in the /usr/bin
        unixcommand_path="/usr/bin"
        # Make the prompt tell you CYGWIN
        export PS1="CYGWIN:${COMPUTERNAME}:${USERNAME}[\!] "
    else
      echo "WARNING: Cannot figure out if this is MKS or CYGWIN"
    fi
    if [ "${ALT_UNIXCOMMAND_PATH}" != "" ] ; then
        unixcommand_path=${ALT_UNIXCOMMAND_PATH}
    fi
    
    # Default shell
    export SHELL="${unixcommand_path}/sh"

    # WINDOWS STEP #1: Setup path to the unix utilities
    path4sdk="${unixcommand_path}"
    if [ ! -d "${unixcommand_path}" ] ; then
	echo "WARNING: Cannot access unixcommand_path=${unixcommand_path}"
    fi

    # WINDOWS STEP #2: Get the devtools needed (gnumake, zip, unzipsfx)
    #   NOTE: If you use CYGWIN, alias gnumake=make, or just use 'make'
    # Add to PATH and set ALT_DEVTOOLS_PATH.
    if [ "${ALT_DEVTOOLS_PATH}" != "" ] ; then
        devtools_path=${ALT_UNIXCOMMAND_PATH}
    else
        if [ "${unix_toolset}" = MKS ] ; then
            devtools_path="C:/UTILS"
        else
            devtools_path="${unixcommand_path}"
        fi
    fi
    if [ "${unix_toolset}" = MKS ] ; then
	path4sdk="${path4sdk};${devtools_path}"
    fi
    if [ ! -d "${devtools_path}" ] ; then
	echo "WARNING: Cannot access devtools_path=${devtools_path}"
    fi

    # WINDOWS STEP #3: Setup path system (verify this is right)
    # System root paths
    if [ "${SystemRoot}" != "" ] ; then
        sys_root=$(${dosname} "${SystemRoot}")
    elif [ "${SYSTEMROOT}" != "" ] ; then
        sys_root=$(${dosname} "${SYSTEMROOT}")
    else
        sys_root=$(${dosname} "C:/WINNT")
    fi
    path4sdk="${path4sdk};${sys_root}/system32;${sys_root};${sys_root}/System32/Wbem"
    if [ ! -d "${sys_root}" ] ; then
        echo "WARNING: No system root found at: ${sys_root}"
    fi

    # Special stuff for MKS (do we really need this stuff?)
    if [ "${unix_toolset}" = MKS ] ; then
        export ENVIRON="${HOME%/}/environ.ksh"
        export ENV="$ENVIRON"
        # If LOGNAME is com or cmd, then we really want to run command.com or
        # cmd.exe, the standard command interpreters provided with DOS and OS/2
        # respectively.
        case "$LOGNAME" in
        com)	. "$ROOTDIR/etc/dospath.ksh"
	        export PROMPT='$p$g'
	        unset SHELL
	        exec $COMSPEC /e:1024;;	# adjust environment space to suit.
        cmd)	. "$ROOTDIR/etc/dospath.ksh"
	        export PROMPT='$p$g'
	        unset SHELL
	        exec $COMSPEC;;
        esac
    fi

    # Special setup for CYGWIN
    if [ "${unix_toolset}" = CYGWIN ] ; then
        # Make sure you use make with CYGWIN
        gnumake=/usr/bin/make
    fi

    # WINDOWS STEP #4: Compiler setup (nasty part)
    #   NOTE: You can use vcvars32.bat to set PATH, LIB, and INCLUDE.
    #   NOTE: CYGWIN has a link.exe too, make sure the compilers are first
    if [ "${windows_arch}" = i586 ] ; then
        # 32bit Windows compiler settings
        # VisualStudio .NET 2003 VC++ 7.1 (VS71COMNTOOLS should be defined)
        vs_root=$(${dosname} "${VS71COMNTOOLS}/../..")
        # Fill in PATH, LIB, and INCLUDE (unset all others to make sure)
        msdev_root="${vs_root}/Common7/Tools"
        msdevtools_path="${msdev_root}/bin"
        vc7_root="${vs_root}/Vc7"
        compiler_path="${vc7_root}/bin"
        platform_sdk="${vc7_root}/PlatformSDK"
          
        # LIB and INCLUDE must use ; as a separator
        include4sdk="${vc7_root}/atlmfc/include"
        include4sdk="${include4sdk};${vc7_root}/include"
        include4sdk="${include4sdk};${platform_sdk}/include/prerelease"
        include4sdk="${include4sdk};${platform_sdk}/include"
        include4sdk="${include4sdk};${vs_root}/SDK/v1.1/include"
        lib4sdk="${vc7_root}/atlmfc/lib"
        lib4sdk="${lib4sdk};${vc7_root}/lib"
        lib4sdk="${lib4sdk};${platform_sdk}/lib/prerelease"
        lib4sdk="${lib4sdk};${platform_sdk}/lib"
        lib4sdk="${lib4sdk};${vs_root}/SDK/v1.1/lib"
        # Search path and DLL locating path
        #   WARNING: CYGWIN has a link.exe too, make sure compilers are first
        path4sdk="${vs_root}/Common7/Tools/bin;${path4sdk}"
	path4sdk="${vs_root}/SDK/v1.1/bin;${path4sdk}"
        path4sdk="${vs_root}/Common7/Tools;${path4sdk}"
	path4sdk="${vs_root}/Common7/Tools/bin/prerelease;${path4sdk}"
        path4sdk="${vs_root}/Common7/IDE;${path4sdk}"
	path4sdk="${compiler_path};${path4sdk}"
    elif [ "${windows_arch}" = amd64 ] ; then
        # AMD64 64bit Windows compiler settings
	if [ "${ALT_DEPLOY_MSSDK}" != "" ] ; then
	    platform_sdk=${ALT_DEPLOY_MSSDK}
        else
	    platform_sdk=$(${dosname} "C:/Program Files/Microsoft Platform SDK/")
	fi
	if [ "${ALT_COMPILER_PATH}" != "" ] ; then
	    compiler_path=${ALT_COMPILER_PATH}
	    if [ "${ALT_DEPLOY_MSSDK}" = "" ] ; then
	        platform_sdk=${ALT_COMPILER_PATH}/../../../..
	    fi
	else
	    compiler_path="${platform_sdk}/Bin/win64/x86/AMD64"
	fi
	if [ "${ALT_MSDEVTOOLS_PATH}" != "" ] ; then
	    msdevtools_path=${ALT_MSDEVTOOLS_PATH}
	else
	    msdevtools_path="${platform_sdk}/Bin/win64/x86/AMD64"
	fi
        msdevtools_path="${compiler_path}"
        # LIB and INCLUDE must use ; as a separator
        include4sdk="${platform_sdk}/Include"
	include4sdk="${include4sdk};${platform_sdk}/Include/crt/sys"
	include4sdk="${include4sdk};${platform_sdk}/Include/mfc"
	include4sdk="${include4sdk};${platform_sdk}/Include/atl"
	include4sdk="${include4sdk};${platform_sdk}/Include/crt"
        lib4sdk="${platform_sdk}/Lib/AMD64"
        lib4sdk="${lib4sdk};${platform_sdk}/Lib/AMD64/atlmfc"
        # Search path and DLL locating path
        #   WARNING: CYGWIN has a link.exe too, make sure compilers are first
        path4sdk="${platform_sdk}/bin;${path4sdk}"
        path4sdk="${compiler_path};${path4sdk}"
    fi
    # Export LIB and INCLUDE
    unset lib
    unset Lib
    LIB="${lib4sdk}"
    export LIB
    unset include
    unset Include
    INCLUDE="${include4sdk}"
    export INCLUDE
    # Set the ALT variable
    if [ -d "${compiler_path}" ] ; then
        export YOUR_ALT_COMPILER_PATH=$(${dosname} "${compiler_path}")
    else
	echo "WARNING: Cannot access compiler_path=${compiler_path}"
    fi
    if [ -d "${msdevtools_path}" ] ; then
        export YOUR_ALT_MSDEVTOOLS_PATH=$(${dosname} "${msdevtools_path}")
    else
	echo "WARNING: Cannot access msdevtools_path=${msdevtools_path}"
    fi
    
    # WINDOWS STEP #5: Get Microsoft SDKs
    if [ "${windows_arch}" = i586 ] ; then
    dx_sdk=$(${dosname} "${DXSDK_DIR}")
    if [ -d "${dx_sdk}" ] ; then
        export YOUR_ALT_DXSDK_PATH=${dx_sdk}
    else
	echo "WARNING: Cannot access dx_sdk=${dx_sdk}"
    fi
    fi

    # WINDOWS STEP #6: Get the previous JDK to be used to bootstrap the build
    #   NOTE: You could use jdk1.6.0 and skip this step (but you need step 4)
    # Use a local copy of jdk1.5.0 if possible.
    bootdir="C:/jdk1.5.0"
    if [ -d "${bootdir}" ] ; then
	export YOUR_ALT_BOOTDIR=$(${dosname} "${bootdir}")
    else
	echo "WARNING: Cannot access bootdir=${bootdir}"
    fi
    
    # WINDOWS STEP #7: Get latest JDK to be used to import pre-built binaries
    #   NOTE: Only if you want to skip parts of the build, e.g. skip hotspot
    # Use a local copy of a recent jdk1.6.0 if possible.
    jdk_import_path="C:/jdk1.6.0"
    if [ -d "${jdk_import_path}" ] ; then
	export YOUR_ALT_JDK_IMPORT_PATH=$(${dosname} "${jdk_import_path}")
    else
	echo "WARNING: Cannot access jdk_import_path=${jdk_import_path}"
    fi
    
    # WINDOWS STEP #8: A certificates file
    cacerts_file="${jdk_import_path}/jre/lib/security/cacerts"
    if [ -f "${cacerts_file}" ] ; then
	export YOUR_ALT_CACERTS_FILE=$(${dosname} "${cacerts_file}")
    else
	echo "WARNING: Cannot access cacerts_file=${cacerts_file}"
    fi

    # WINDOWS STEP #9: ALT_MSVCRT_DLL_PATH
    if [ "${windows_arch}" = i586 ] ; then
        msvcrt_dll_path="${sys_root}/system32"
    elif [ "${windows_arch}" = amd64 ] ; then
        microsoft_sdk="${platform_sdk}"
	if [ ! -d "${microsoft_sdk}" ] ; then
	    echo "WARNING: Cannot access microsoft_sdk=${microsoft_sdk}"
	fi
        msvcrt_dll_path="${microsoft_sdk}/redist/win64/amd64"
    fi
    if [ -d "${msvcrt_dll_path}" ] ; then
	export YOUR_ALT_MSVCRT_DLL_PATH=$(${dosname} "${msvcrt_dll_path}")
    else
	echo "WARNING: Cannot access msvcrt_dll_path=${msvcrt_dll_path}"
    fi
    
    if [ "${windows_arch}" = i586 ] ; then
    
	# WINDOWS STEP #10: ALT_UNICOWS_LIB_PATH/ALT_UNICOWS_DLL_PATH
        #unicows_lib_path=$(${dosname} "C:/Program Files/Microsoft SDK/lib")
        #unicows_dll_path="${jdk_import_path}/jre/bin"
        unicows_lib_path="C:/MSLU"
        unicows_dll_path="C:/MSLU"
        if [ -d "${unicows_lib_path}" ] ; then
            export YOUR_ALT_UNICOWS_LIB_PATH=$(${dosname} "${unicows_lib_path}")
        else
	    echo "WARNING: Cannot access unicows_lib_path=${unicows_lib_path}"
        fi
        if [ -d "${unicows_dll_path}" ] ; then
	    export YOUR_ALT_UNICOWS_DLL_PATH=$(${dosname} "${unicows_dll_path}")
        else
	    echo "WARNING: Cannot access unicows_dll_path=${unicows_dll_path}"
        fi
        
        # WINDOWS STEP #11: Mozilla header files for building the plugin
        mozilla_headers_path="${slash_java}/devtools/share/plugin"
        if [ -d "${mozilla_headers_path}" ] ; then
	    export YOUR_ALT_MOZILLA_HEADERS_PATH=$(${dosname} "${mozilla_headers_path}")
        else
	    echo "WARNING: Cannot access mozilla_headers_path=${mozilla_headers_path}"
        fi
    fi
    
    # Turn all \\ into /, remove duplicates and trailing /
    slash_path="$(echo ${path4sdk} | sed -e 's@\\\\@/@g' -e 's@//@/@g' -e 's@/$@@' -e 's@/;@;@g')"
    path4sdk="${slash_path}"
    
    # Convert path4sdk to cygwin style
    if [ "${unix_toolset}" = CYGWIN ] ; then
	path4sdk="$(/usr/bin/cygpath -p ${path4sdk})"
    fi

fi

# Export PATH setting
PATH="${path4sdk}"
export PATH

