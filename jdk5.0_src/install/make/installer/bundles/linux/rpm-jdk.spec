#
# Copyright 2003 Sun Microsystems, Inc. All rights reserved.
# SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#

#
# @(#)rpm-jdk.spec	1.43 04/06/03
#
%prep
%ifos Linux
%ifarch %{arch}

%setup -n %{jdk_name}%{jdk_version}



%build



%install
    if [ -d $RPM_BUILD_ROOT/%{jdk_prefix}/%{jdk_name}%{jdk_version} ]; then
        rm -rf $RPM_BUILD_ROOT/%{jdk_prefix}/%{jdk_name}%{jdk_version}
    fi

    mkdir -p $RPM_BUILD_ROOT/%{jdk_prefix}/%{jdk_name}%{jdk_version}
    cp -dpr $RPM_BUILD_DIR/%{jdk_name}%{jdk_version}/* \
            $RPM_BUILD_ROOT/%{jdk_prefix}/%{jdk_name}%{jdk_version}/



%files
%defattr(-,root,root)
%{jdk_prefix}/%{jdk_name}%{jdk_version}/*



%clean
    rm -rf $RPM_BUILD_DIR/%{jdk_name}%{jdk_version}
    rm -rf $RPM_BUILD_ROOT/%{jdk_prefix}/%{jdk_name}%{jdk_version}
%else
    echo "This package is for %{arch}."
    echo "To override add the \"--ignorearch\" option"
%endif
%else
    echo "This package is for the Linux operating system."
    echo "To override add the "--ignoreos" option"
%endif



%post
# Make sure any new files created in /etc are created with a secure access
# mask.  Using chmod is problematic, since that would also change the rights
# of any existing file, and we are only interested in setting the rights for
# new files.
umask 022

#
# ExpandPrefix ( release )
#
# These two shell routines expand JVM release identifier prefixes to the
# full, four element tuple.  ExpandPrefix zero extends as per JSR 56.
#
# Parameters:
#   $1      release         Partial or complete release name tuple.
#
ExpandPrefix() {
    echo $1 | sed -e "s/_/\./g" | \
      awk 'BEGIN { FS = "." } { printf "%d.%d.%d_%d\n", $1, $2, $3, $4 }'
}
# End of ExpandPrefix

#
# GetRel ( filename )
#
# A little utility routine to strip viable prefixes from release names.
# Note that this only works for release names by Sun convention, not the
# whole, generalized JSR 56 name set.
#
# Parameters:
#   $1      filename        Filesystem filename ( j(re|dk|(2(re|sdk))<version> )
#
# Returns:
#   Version portion of the file name.
#
GetRel() {
    if [ "`echo $1 | cut -c 1-3`" = "jre" ]; then
        echo $1 | cut -c 4-
    elif [ "`echo $1 | cut -c 1-3`" = "jdk" ]; then
        echo $1 | cut -c 4-
    elif [ "`echo $1 | cut -c 1-4`" = "j2re" ]; then
        echo $1 | cut -c 5-
    elif [ "`echo $1 | cut -c 1-5`" = "j2sdk" ]; then
        echo $1 | cut -c 6-
    else
        echo $1
    fi
}
# End of GetRel

#Returns the instpath of the latest javaws or ""
#With linux we could have both a jre and jdk installed.
#Therefore we choose the jre javaws over the one with jdk.
GetLatestJavaWSPath() {
    tlist=
    jre_list=`ls -d %{jre_prefix}/%{jre_name}* %{jre_prefix}/j2re* 2> /dev/null`
    for dir in $jre_list; do
        rel=`GetRel \`basename $dir\``
        rel=`ExpandPrefix $rel | sed -e "s/[\._-]/ /g"`

        # Note: we prefer a JRE over a JDK, so we assign a 9 in the 5th column
        if [ -x ${dir}/bin/javaws ]; then
            tlist="${rel} 9 ${dir}/bin/javaws\n${tlist}"
        elif [ -x ${dir}/javaws/javaws ]; then
            tlist="${rel} 9 ${dir}/javaws/javaws\n${tlist}"
        fi
    done

    jre_list=`ls -d %{jdk_prefix}/%{jdk_name}* %{jdk_prefix}/j2sdk* 2> /dev/null`
    for dir in $jre_list; do
        rel=`GetRel \`basename $dir\``
        rel=`ExpandPrefix $rel | sed -e "s/[\._-]/ /g"`

        # Note: a JRE is preferred, so we assign a 0 in the 5th column
        if [ -x ${dir}/jre/bin/javaws ]; then
            tlist="${rel} 0 ${dir}/jre/bin/javaws\n${tlist}"
        elif [ -x ${dir}/jre/javaws/javaws ]; then
            tlist="${rel} 0 ${dir}/jre/javaws/javaws\n${tlist}"
        fi
    done

    echo -e "${tlist}" | \
        sort -k 1,1n -k 2,2n -k 3,3n -k 4,4n -k 5,5n | \
        tail -1 | \
        cut -d " " -f 6
}
# End of GetLatestJavaWSPath

#=================================================
# Add Java Web Start entry to /etc/mailcap
#     param - $1 - mailcap file
#-------------------------------------------------
UpdateMailcap() {
    MAILCAP_FILE=$1

    MC_COMMENT="# Java Web Start"
    MC_TEXT=

    if [ -w ${MAILCAP_FILE} ]; then
        # Remove existing entry, if present
        MC_TEXT=`grep -v "${MIME_TYPE}" ${MAILCAP_FILE} | \
                 grep -v "${MC_COMMENT}"`
    fi
    # Add new entry
    if [ -w `dirname ${MAILCAP_FILE}` ]; then
        MC_TEXT="${MC_TEXT}\n${MC_COMMENT}"
        MC_TEXT="${MC_TEXT}\n${MIME_TYPE}; $LATEST_JAVAWS_PATH %s"
        echo -e "${MC_TEXT}" > ${MAILCAP_FILE}
    else
        echo "WARNING - cannot write to file:"
        echo "       ${MAILCAP_FILE}"
        echo "Check permissions."
    fi
}
# End of UpdateMailcap

#=================================================
# Add Java Web Start entry to /etc/.mime.types
#     param - $1 - mime file
#-------------------------------------------------
UpdateMimeTypes() {
    MIME_FILE=$1

    NS_COMMENT1="#--Netscape Communications Corporation MIME Information"
    NS_COMMENT2="#Do not delete the above line. It is used to identify the file type."
    NS_COMMENT3="#mime types added by Netscape Helper"
    JNLP_ENTRY="type=${MIME_TYPE} desc=\"Java Web Start\" exts=\"jnlp\""

    # Create the file if it does not exist
    if [ ! -w ${MIME_FILE} ]; then
        if [ -w `dirname ${MIME_FILE}` ]; then
            echo "${NS_COMMENT1}"  > ${MIME_FILE}
            echo "${NS_COMMENT2}" >> ${MIME_FILE}
            echo "${NS_COMMENT3}" >> ${MIME_FILE}
        else
            echo "WARNING - cannot write to file:"
            echo "       ${MIME_FILE}"
            echo "Check permissions."
            return
        fi
    fi
    # Add the jnlp entry if it does not already exist.
    if [ -z "`grep ${MIME_TYPE} ${MIME_FILE}`" ]; then
        echo ${JNLP_ENTRY} >> ${MIME_FILE}
    fi
}
# End of UpdateMimeTypes

#=================================================
# Get the location of the GNOME top directory.  This should either be the
# directory specified in GNOMEDIR, or /usr.
#
# Given an input directory, determine if it appears to be a valid GNOME top
# level directory.  If it is valid, then return the input; otherwise return
# an empty string.
#
# Note: The user's home directory is always a valid location for the GNOME
# association files.
#
#     param - $1 - gnome top dir
#-------------------------------------------------
GetGNOMETopDir() {
    _gnome_dir=$1

    if [ "${_gnome_dir}" != "${HOME}" -a \
         ! \( -d ${_gnome_dir}/${APP_REGISTRY} -a \
              -d ${_gnome_dir}/${MIME_INFO} \) ]; then

        # This isn't the user's home directory, and one or both of the
        # application directories are missing, but this could still be
        # a valid.  If this directory contains bin/gnome* files,
        # lib/libgnome* files, and a share directory, then assume it is
        # a valid GNOME top level directory.
        _gnomebin="`ls ${_gnome_dir}/bin/gnome* 2> /dev/null`"
        _gnomelib="`ls ${_gnome_dir}/lib/libgnome* 2> /dev/null`"
        if [ -z "${_gnomebin}" -o \
             -z "${_gnomelib}" -o \
             ! -d ${_gnome_dir}/${GNOME_SHARE} ]; then

            # This doesn't seem to be a valid GNOME top level directory.
            _gnome_dir=
        fi
    fi

    echo ${_gnome_dir}
}
# End of GetGNOMETopDir

#=================================================
# Add an entry to GNOME application registry
#     param - $1 - mime type     associated mime type
#     param - $2 - extension     associated file extension
#     param - $3 - name          used to name GNOME registry files
#     param - $4 - command       what to invoke when launching file/mime type
#     param - $5 - icon          base name of icon file
#     param - $6 - icon ext      extension for icon file
#     param - $7 - description   description for association
#-------------------------------------------------
AddGNOME() {
    _mime_type=$1
    _extension=$2
    _name=$3
    _command=$4
    _icon=$5
    _icon_ext=$6
    _description=$7

    if [ -n "${_mime_type}" -a \
         -n "${_extension}" -a \
         -n "${_name}" -a \
         -n "${_command}" -a \
         -n "${_icon}" -a \
         -n "${_icon_ext}" ]; then

        # check the GNOMEDIR environment variable to see if it points to a valid
        # GNOME top level directory.
        _gnome_dir=`GetGNOMETopDir ${GNOMEDIR}`
        if [ -z "${_gnome_dir}" ]; then
            # GNOMEDIR was invalid, so check in /usr
            _gnome_dir=`GetGNOMETopDir ${GNOMEDIR_DEFAULT}`
            if [ -z "${_gnome_dir}" ]; then
                # GNOME doesn't seem to be installed
                return 1
            fi
        fi

        if [ -d $LATEST_JRE_PATH/lib/images/icons ]; then
            # the various directories may need to be created if no themes
            # have ever been installed before
            mkdir -p ${_gnome_dir}/${BASE_ICONS}
            mkdir -p ${_gnome_dir}/${HIGH_CONTRAST_ICONS}
            mkdir -p ${_gnome_dir}/${HIGH_CONTRAST_INVERSE_ICONS}
            mkdir -p ${_gnome_dir}/${LOW_CONTRAST_ICONS}

            # copy the icons used on the GNOME desktop
            cp -f $LATEST_JRE_PATH/lib/images/icons/${_icon}.${_icon_ext} \
                  ${_gnome_dir}/${BASE_ICONS}/${_icon}.${_icon_ext}
            cp -f $LATEST_JRE_PATH/lib/images/icons/${_icon}_HighContrast.${_icon_ext} \
                  ${_gnome_dir}/${HIGH_CONTRAST_ICONS}/${_icon}.${_icon_ext}
            cp -f $LATEST_JRE_PATH/lib/images/icons/${_icon}_HighContrastInverse.${_icon_ext} \
                  ${_gnome_dir}/${HIGH_CONTRAST_INVERSE_ICONS}/${_icon}.${_icon_ext}
            cp -f $LATEST_JRE_PATH/lib/images/icons/${_icon}_LowContrast.${_icon_ext} \
                  ${_gnome_dir}/${LOW_CONTRAST_ICONS}/${_icon}.${_icon_ext}
        fi

        # since we are pretty certain this is a valid GNOME installation, create
        # the required association directories in case they don't already exist.
        mkdir -p ${_gnome_dir}/${MIME_INFO}
        mkdir -p ${_gnome_dir}/${APP_REGISTRY}

        GNOME_KEYS_FILE=${_gnome_dir}/${MIME_INFO}/${_name}.keys
        GNOME_MIME_FILE=${_gnome_dir}/${MIME_INFO}/${_name}.mime
        GNOME_APPS_FILE=${_gnome_dir}/${APP_REGISTRY}/${_name}.applications

        # Create the keys file.
        echo "${_mime_type}"                                           > $GNOME_KEYS_FILE
        echo "        description=${_description}"                    >> $GNOME_KEYS_FILE
        echo "        icon_filename=${_icon}"                         >> $GNOME_KEYS_FILE
        echo "        default_action_type=application"                >> $GNOME_KEYS_FILE
        echo "        default_application_id=${_name}"                >> $GNOME_KEYS_FILE
        echo "        short_list_application_user_additions=${_name}" >> $GNOME_KEYS_FILE

        # Create the mime file.
        echo "${_mime_type}"               > $GNOME_MIME_FILE
        echo "        ext: ${_extension}" >> $GNOME_MIME_FILE

        # Create the apps file.
        echo "${_name}"                               > $GNOME_APPS_FILE
        echo "        command=${_command}"           >> $GNOME_APPS_FILE
        echo "        name=${_name}"                 >> $GNOME_APPS_FILE
        echo "        can_open_multiple_files=false" >> $GNOME_APPS_FILE
        echo "        requires_terminal=false"       >> $GNOME_APPS_FILE
        echo "        mime_types=${_mime_type}"      >> $GNOME_APPS_FILE
    fi
}
# End of AddGNOME

#
# MakeLink ( _src _dst )
#
# Create a link if a file/directory doesn't exist at the given _dst.
#
# Fix for 4938584 - Java links need to be set up on Java Desktop System.
# This will do until mJRE is implemented on Linux
#
# Parameters:
#   $1  _src  the source for the link
#   $2  _dst  the destination for the link
#
MakeLink () {
    if [ \( ! -f $2 -a ! -d $2 \) -o -h $2 ]; then
        rm -f $2
        if [ -n $1 ]; then
            ln -s $1 $2
        fi
    fi
}

#
# Create a symlink if install dir is not /usr/java
#
if [ "${RPM_INSTALL_PREFIX}" != "%{jdk_prefix}" ]; then
    if [ -d $RPM_INSTALL_PREFIX/%{jdk_name}%{jdk_version} ]; then
        ln -s $RPM_INSTALL_PREFIX/%{jdk_name}%{jdk_version} \
              %{jdk_prefix}/%{jdk_name}%{jdk_version}
    fi
fi

UNPACK_EXE=%{jdk_prefix}/%{jdk_name}%{jdk_version}/bin/unpack200
if [ -f $UNPACK_EXE ]; then
    chmod +x $UNPACK_EXE

    PACKED_JARS=%{packed_jars}
    for i in $PACKED_JARS; do
        srcFile=%{jdk_prefix}/%{jdk_name}%{jdk_version}/`dirname $i`/`basename $i .jar`.pack
        dstFile=%{jdk_prefix}/%{jdk_name}%{jdk_version}/$i
        $UNPACK_EXE $srcFile $dstFile 
        if [ ! -f  $dstFile ]; then
            printf "Error: unpack could not create %s. Please refer to the Troubleshooting\n" $dstFile
            printf "Section of the Installation Instructions on the download page.\n"
            exit 1
        fi

        rm -f $srcFile
    done

else
    printf "Error: unpack command could not be found. Please refer to the \n"
    printf "TroubleShooting Section of the Installation Instructions on \n"
    printf "the download page.\n"
    printf "Please do not attempt to install this archive file.\n"
    exit 2
fi

if [ ! -d /usr/local/man/man1 ]; then mkdir -p /usr/local/man/man1; fi
cp -dpr %{jdk_prefix}/%{jdk_name}%{jdk_version}/man/man1/* %{jdk_manpath}

userid=`expr "\`id\`" : ".*uid=[0-9]*(\(.[0-9a-z]*\)) .*"`
PREFS_LOCATION=%{jdk_prefix}/%{jdk_name}%{jdk_version}/jre
if [ "$userid" = "root" ]; then
    PREFS_LOCATION=/etc/.java
fi
if [ ! -d $PREFS_LOCATION ]; then
    mkdir -m 755 $PREFS_LOCATION
fi
if [ ! -d $PREFS_LOCATION/.systemPrefs ]; then
    mkdir -m 755 $PREFS_LOCATION/.systemPrefs
fi
if [ ! -f $PREFS_LOCATION/.systemPrefs/.system.lock ]; then
    touch $PREFS_LOCATION/.systemPrefs/.system.lock
    chmod 644 $PREFS_LOCATION/.systemPrefs/.system.lock
fi
if [ ! -f $PREFS_LOCATION/.systemPrefs/.systemRootModFile ]; then
    touch $PREFS_LOCATION/.systemPrefs/.systemRootModFile
    chmod 644 $PREFS_LOCATION/.systemPrefs/.systemRootModFile
fi
%ifarch ia64 || x86_64
    # No GNOME icons, javaws, or shared class files on 64-bit
%else
    # fix for: 4728032 - Install needs to generate shared class files
    %{jdk_prefix}/%{jdk_name}%{jdk_version}/bin/java -client -Xshare:dump > /dev/null 2>&1

    MIME_TYPE=%{mime_type}

    GNOMEDIR_DEFAULT=$RPM_BUILD_ROOT/usr
    GNOME_SHARE=share
    MIME_INFO=${GNOME_SHARE}/mime-info
    APP_REGISTRY=${GNOME_SHARE}/application-registry

    BASE_ICONS=share/pixmaps
    HIGH_CONTRAST_ICONS=share/icons/HighContrast/48x48/apps
    HIGH_CONTRAST_INVERSE_ICONS=share/icons/HighContrastInverse/48x48/apps
    LOW_CONTRAST_ICONS=share/icons/LowContrast/48x48/apps

    # Try to set latest path to Java Web Start
    LATEST_JAVAWS_PATH=`GetLatestJavaWSPath`

    if [ \( -n "${LATEST_JAVAWS_PATH}" -a -x "${LATEST_JAVAWS_PATH}" \) ]; then
        # get the path to the "best choice" JRE
        LATEST_JRE_PATH=`dirname \`dirname $LATEST_JAVAWS_PATH\``

        # Setup the mailcap file
        UpdateMailcap $RPM_BUILD_ROOT/etc/mailcap

        # Setup the mime.type file
        UpdateMimeTypes $RPM_BUILD_ROOT/etc/mime.types

        # Setup the GNOME associations
        AddGNOME application/java-archive \
                 jar \
                 java-archive \
                 "${LATEST_JRE_PATH}/bin/java -jar" \
                 sun-java \
                 png \
                 "Java Archive"

        AddGNOME ${MIME_TYPE} \
                 jnlp \
                 java-web-start \
                 ${LATEST_JAVAWS_PATH} \
                 sun-java \
                 png \
                 "Java Web Start Application"

        # Let's wait for CCC approval on this - MWR
        # MakeLink ${LATEST_JRE_PATH}/bin/java /usr/bin/java
        # MakeLink ${LATEST_JAVAWS_PATH} /usr/bin/javaws
    fi

    #
    #copy sun_java.desktop file to /usr/share/control-center-2.0/capplets 
    #if it exists; this is the proper location for GNOME 2.0+ (java control
    #panel is for configuration, not an application)  see bug 4891973
    #
    #copy sun_java.desktop file to /usr/share/applications directory,
    #since this is the default location for all desktop files starting with
    #GNOME 2.0
    #
    DESKTOP_FILE_PATH=`echo $DESKTOP_FILE_PATH`
    if [ -z "${DESKTOP_FILE_PATH}" ]; then
        DESKTOP_FILE_PATH=/usr/share/control-center-2.0/capplets
        if [ ! -d $DESKTOP_FILE_PATH ]; then
            DESKTOP_FILE_PATH=/usr/share/applications
            if [ ! -d $DESKTOP_FILE_PATH ]; then
                DESKTOP_FILE_PATH=/usr/share/gnome/apps/Settings
                if [ ! -d $DESKTOP_FILE_PATH ]; then
                    mkdir -p $DESKTOP_FILE_PATH > /dev/null 2>&1
                fi
            fi
        fi
    fi

    if [ -f ${DESKTOP_FILE_PATH}/sun_java.desktop ]; then
        rm ${DESKTOP_FILE_PATH}/sun_java.desktop
    fi

    if [ -n "$LATEST_JRE_PATH" ]; then
        # use the "best choice" selected by GetLatestJavaWSPath for the control panel
        INSTALL_PATH=`dirname $LATEST_JRE_PATH`
        JRE_STRING=`basename $LATEST_JRE_PATH`
    else
        # use the JRE in the JDK for the control panel
        INSTALL_PATH=%{jre_prefix}
        JRE_STRING=%{jre_name}%{jre_version}/jre
    fi

    sed "s|INSTALL_DIR|$INSTALL_PATH|g" \
             $INSTALL_PATH/$JRE_STRING/plugin/desktop/sun_java.desktop \
         | sed "s|JRE_NAME_VERSION|$JRE_STRING|g" \
         > ${DESKTOP_FILE_PATH}/sun_java.desktop
%endif



%preun
if [ ! -d %{jdk_prefix}/%{jdk_name}%{jdk_version}/man/man1 ]; then
    mkdir -p %{jdk_prefix}/%{jdk_name}%{jdk_version}/man/man1;
fi
if [ -f %{jdk_manpath}/appletviewer.1 ]; then
    mv -f %{jdk_manpath}/appletviewer.1 %{jdk_prefix}/%{jdk_name}%{jdk_version}/man/man1; 
fi
if [ -f %{jdk_manpath}/extcheck.1 ]; then
    mv -f %{jdk_manpath}/extcheck.1 %{jdk_prefix}/%{jdk_name}%{jdk_version}/man/man1;
fi
if [ -f %{jdk_manpath}/jar.1 ]; then
    mv -f %{jdk_manpath}/jar.1 %{jdk_prefix}/%{jdk_name}%{jdk_version}/man/man1;
fi
if [ -f %{jdk_manpath}/jarsigner.1 ]; then
    mv -f %{jdk_manpath}/jarsigner.1 %{jdk_prefix}/%{jdk_name}%{jdk_version}/man/man1;
fi
if [ -f %{jdk_manpath}/java.1 ]; then
    mv -f %{jdk_manpath}/java.1 %{jdk_prefix}/%{jdk_name}%{jdk_version}/man/man1;
fi
if [ -f %{jdk_manpath}/javac.1 ]; then
    mv -f %{jdk_manpath}/javac.1 %{jdk_prefix}/%{jdk_name}%{jdk_version}/man/man1;
fi
if [ -f %{jdk_manpath}/javadoc.1 ]; then
    mv -f %{jdk_manpath}/javadoc.1 %{jdk_prefix}/%{jdk_name}%{jdk_version}/man/man1;
fi
if [ -f %{jdk_manpath}/javah.1 ]; then
    mv -f %{jdk_manpath}/javah.1 %{jdk_prefix}/%{jdk_name}%{jdk_version}/man/man1;
fi
if [ -f %{jdk_manpath}/javap.1 ]; then
    mv -f %{jdk_manpath}/javap.1 %{jdk_prefix}/%{jdk_name}%{jdk_version}/man/man1;
fi
if [ -f %{jdk_manpath}/jdb.1 ]; then
    mv -f %{jdk_manpath}/jdb.1 %{jdk_prefix}/%{jdk_name}%{jdk_version}/man/man1;
fi
if [ -f %{jdk_manpath}/keytool.1 ]; then
    mv -f %{jdk_manpath}/keytool.1 %{jdk_prefix}/%{jdk_name}%{jdk_version}/man/man1;
fi
if [ -f %{jdk_manpath}/native2ascii.1 ]; then
    mv -f %{jdk_manpath}/native2ascii.1 %{jdk_prefix}/%{jdk_name}%{jdk_version}/man/man1;
fi
if [ -f %{jdk_manpath}/rmic.1 ]; then
    mv -f %{jdk_manpath}/rmic.1 %{jdk_prefix}/%{jdk_name}%{jdk_version}/man/man1;
fi
if [ -f %{jdk_manpath}/rmid.1 ]; then
    mv -f %{jdk_manpath}/rmid.1 %{jdk_prefix}/%{jdk_name}%{jdk_version}/man/man1;
fi
if [ -f %{jdk_manpath}/rmiregistry.1 ]; then
    mv -f %{jdk_manpath}/rmiregistry.1 %{jdk_prefix}/%{jdk_name}%{jdk_version}/man/man1;
fi
if [ -f %{jdk_manpath}/serialver.1 ]; then
    mv -f %{jdk_manpath}/serialver.1 %{jdk_prefix}/%{jdk_name}%{jdk_version}/man/man1;
fi
if [ -f %{jdk_manpath}/tnameserv.1 ]; then
    mv -f %{jdk_manpath}/tnameserv.1 %{jdk_prefix}/%{jdk_name}%{jdk_version}/man/man1;
fi

PACKED_JARS=%{packed_jars}
for i in $PACKED_JARS; do
    touch %{jdk_prefix}/%{jdk_name}%{jdk_version}/`dirname $i`/`basename $i .jar`.pack
    rm -f %{jdk_prefix}/%{jdk_name}%{jdk_version}/$i
done

%ifarch ia64 || x86_64
    # No shared class files on 64-bit
%else
    ARCH=i386    # I hate to hardcode this, but the %{arch} macro is defined as i586
    SHARED_CLASS_PATH=%{jdk_prefix}/%{jdk_name}%{jdk_version}/jre/lib/${ARCH}/client
    SHARED_CLASS_FILE=classes.jsa

    # remove the shared class file that was generated during install
    if [ -f ${SHARED_CLASS_PATH}/${SHARED_CLASS_FILE} ]; then
        rm -f ${SHARED_CLASS_PATH}/${SHARED_CLASS_FILE}
    fi
%endif



%postun
# Make sure any new files created in /etc are created with a secure access
# mask.  Using chmod is problematic, since that would also change the rights
# of any existing file, and we are only interested in setting the rights for
# new files.
umask 022

#
# ExpandPrefix ( release )
#
# These two shell routines expand JVM release identifier prefixes to the
# full, four element tuple.  ExpandPrefix zero extends as per JSR 56.
#
# Parameters:
#   $1      release         Partial or complete release name tuple.
#
ExpandPrefix() {
    echo $1 | sed -e "s/_/\./g" | \
      awk 'BEGIN { FS = "." } { printf "%d.%d.%d_%d\n", $1, $2, $3, $4 }'
}
# End of ExpandPrefix

#
# GetRel ( filename )
#
# A little utility routine to strip viable prefixes from release names.
# Note that this only works for release names by Sun convention, not the
# whole, generalized JSR 56 name set.
#
# Parameters:
#   $1      filename        Filesystem filename ( j(re|dk|(2(re|sdk))<version> )
#
# Returns:
#   Version portion of the file name.
#
GetRel() {
    if [ "`echo $1 | cut -c 1-3`" = "jre" ]; then
        echo $1 | cut -c 4-
    elif [ "`echo $1 | cut -c 1-3`" = "jdk" ]; then
        echo $1 | cut -c 4-
    elif [ "`echo $1 | cut -c 1-4`" = "j2re" ]; then
        echo $1 | cut -c 5-
    elif [ "`echo $1 | cut -c 1-5`" = "j2sdk" ]; then
        echo $1 | cut -c 6-
    else
        echo $1
    fi
}
# End of GetRel

#Returns the instpath of the latest javaws or ""
#With linux we could have both a jre and jdk installed.
#Therefore we choose the jre javaws over the one with jdk.
GetLatestJavaWSPath() {
    tlist=
    jre_list=`ls -d %{jre_prefix}/%{jre_name}* %{jre_prefix}/j2re* 2> /dev/null`
    for dir in $jre_list; do
        rel=`GetRel \`basename $dir\``
        rel=`ExpandPrefix $rel | sed -e "s/[\._-]/ /g"`

        # Note: we prefer a JRE over a JDK, so we assign a 9 in the 5th column
        if [ -x ${dir}/bin/javaws ]; then
            tlist="${rel} 9 ${dir}/bin/javaws\n${tlist}"
        elif [ -x ${dir}/javaws/javaws ]; then
            tlist="${rel} 9 ${dir}/javaws/javaws\n${tlist}"
        fi
    done

    jre_list=`ls -d %{jdk_prefix}/%{jdk_name}* %{jdk_prefix}/j2sdk* 2> /dev/null`
    for dir in $jre_list; do
        rel=`GetRel \`basename $dir\``
        rel=`ExpandPrefix $rel | sed -e "s/[\._-]/ /g"`

        # Note: a JRE is preferred, so we assign a 0 in the 5th column
        if [ -x ${dir}/jre/bin/javaws ]; then
            tlist="${rel} 0 ${dir}/jre/bin/javaws\n${tlist}"
        elif [ -x ${dir}/jre/javaws/javaws ]; then
            tlist="${rel} 0 ${dir}/jre/javaws/javaws\n${tlist}"
        fi
    done

    echo -e "${tlist}" | \
        sort -k 1,1n -k 2,2n -k 3,3n -k 4,4n -k 5,5n | \
        tail -1 | \
        cut -d " " -f 6
}
# End of GetLatestJavaWSPath

#=================================================
# Add Java Web Start entry to /etc/mailcap
#     param - $1 - mailcap file
#-------------------------------------------------
UpdateMailcap() {
    MAILCAP_FILE=$1

    MC_COMMENT="# Java Web Start"
    MC_TEXT=

    if [ -w ${MAILCAP_FILE} ]; then
        # Remove existing entry, if present
        MC_TEXT=`grep -v "${MIME_TYPE}" ${MAILCAP_FILE} | \
                 grep -v "${MC_COMMENT}"`
    fi
    # Add new entry
    if [ -w `dirname ${MAILCAP_FILE}` ]; then
        MC_TEXT="${MC_TEXT}\n${MC_COMMENT}"
        MC_TEXT="${MC_TEXT}\n${MIME_TYPE}; $LATEST_JAVAWS_PATH %s"
        echo -e "${MC_TEXT}" > ${MAILCAP_FILE}
    else
        echo "WARNING - cannot write to file:"
        echo "       ${MAILCAP_FILE}"
        echo "Check permissions."
    fi
}
# End of UpdateMailcap
  
#=================================================
# Add Java Web Start entry to /etc/.mime.types
#     param - $1 - mime file
#-------------------------------------------------
UpdateMimeTypes() {
    MIME_FILE=$1

    NS_COMMENT1="#--Netscape Communications Corporation MIME Information"
    NS_COMMENT2="#Do not delete the above line. It is used to identify the file type."
    NS_COMMENT3="#mime types added by Netscape Helper"
    JNLP_ENTRY="type=${MIME_TYPE} desc=\"Java Web Start\" exts=\"jnlp\""

    # Create the file if it does not exist
    if [ ! -w ${MIME_FILE} ]; then
        if [ -w `dirname ${MIME_FILE}` ]; then
            echo "${NS_COMMENT1}"  > ${MIME_FILE}
            echo "${NS_COMMENT2}" >> ${MIME_FILE}
            echo "${NS_COMMENT3}" >> ${MIME_FILE}
        else
            echo "WARNING - cannot write to file:"
            echo "       ${MIME_FILE}"
            echo "Check permissions."
            return
        fi
    fi
    # Add the jnlp entry if it does not already exist.
    if [ -z "`grep ${MIME_TYPE} ${MIME_FILE}`" ]; then
        echo ${JNLP_ENTRY} >> ${MIME_FILE}
    fi
}
# End of UpdateMimeTypes

#=================================================
# Remove Java Web Start entry to /etc/mailcap
#     param - $1 - mailcap file
#-------------------------------------------------
RemoveMailcap() {
    MAILCAP_FILE=$1

    MC_COMMENT="# Java Web Start"

    if [ -w ${MAILCAP_FILE} ]; then
        # Remove existing entry, if present
        if [ -n "`grep ${MIME_TYPE} ${MAILCAP_FILE}`" ] ; then
            MC_TEXT=`grep -v "${MIME_TYPE}" ${MAILCAP_FILE} | \
                     grep -v "${MC_COMMENT}"`
            echo -e "${MC_TEXT}" > ${MAILCAP_FILE}
        fi
    fi
}
# End of RemoveMailcap
  
#=================================================
# Remove Java Web Start entry to /etc/.mime.types
#     param - $1 - mime file
#-------------------------------------------------
RemoveMimeTypes() {
    MIME_FILE=$1

    if [ -w ${MIME_FILE} ]; then
        # Remove existing entry, if present
        if [ -n "`grep ${MIME_TYPE} ${MIME_FILE}`" ]; then
            MC_TEXT=`grep -v ${MIME_TYPE} ${MIME_FILE}`
            echo "${MC_TEXT}" > ${MIME_FILE}
        fi
    fi
}
# End of RemoveMimeTypes

#=================================================
# Get the location of the GNOME top directory.  This should either be the
# directory specified in GNOMEDIR, or /usr.
#
# Given an input directory, determine if it appears to be a valid GNOME top
# level directory.  If it is valid, then return the input; otherwise return
# an empty string.
#
# Note: The user's home directory is always a valid location for the GNOME
# association files.
#
#     param - $1 - gnome top dir
#-------------------------------------------------
GetGNOMETopDir() {
    _gnome_dir=$1

    if [ "${_gnome_dir}" != "${HOME}" -a \
         ! \( -d ${_gnome_dir}/${APP_REGISTRY} -a \
              -d ${_gnome_dir}/${MIME_INFO} \) ]; then

        # This isn't the user's home directory, and one or both of the
        # application directories are missing, but this could still be
        # a valid.  If this directory contains bin/gnome* files,
        # lib/libgnome* files, and a share directory, then assume it is
        # a valid GNOME top level directory.
        _gnomebin="`ls ${_gnome_dir}/bin/gnome* 2> /dev/null`"
        _gnomelib="`ls ${_gnome_dir}/lib/libgnome* 2> /dev/null`"
        if [ -z "${_gnomebin}" -o \
             -z "${_gnomelib}" -o \
             ! -d ${_gnome_dir}/${GNOME_SHARE} ]; then

            # This doesn't seem to be a valid GNOME top level directory.
            _gnome_dir=
        fi
    fi

    echo ${_gnome_dir}
}
# End of GetGNOMETopDir

#=================================================
# Add an entry to GNOME application registry
#     param - $1 - mime type     associated mime type
#     param - $2 - extension     associated file extension
#     param - $3 - name          used to name GNOME registry files
#     param - $4 - command       what to invoke when launching file/mime type
#     param - $5 - icon          base name of icon file
#     param - $6 - icon ext      extension for icon file
#     param - $7 - description   description for association
#-------------------------------------------------
AddGNOME() {
    _mime_type=$1
    _extension=$2
    _name=$3
    _command=$4
    _icon=$5
    _icon_ext=$6
    _description=$7

    if [ -n "${_mime_type}" -a \
         -n "${_extension}" -a \
         -n "${_name}" -a \
         -n "${_command}" -a \
         -n "${_icon}" -a \
         -n "${_icon_ext}" ]; then

        # check the GNOMEDIR environment variable to see if it points to a valid
        # GNOME top level directory.
        _gnome_dir=`GetGNOMETopDir ${GNOMEDIR}`
        if [ -z "${_gnome_dir}" ]; then
            # GNOMEDIR was invalid, so check in /usr
            _gnome_dir=`GetGNOMETopDir ${GNOMEDIR_DEFAULT}`
            if [ -z "${_gnome_dir}" ]; then
                # GNOME doesn't seem to be installed
                return 1
            fi
        fi

        if [ -d $LATEST_JRE_PATH/lib/images/icons ]; then
            # the various directories may need to be created if no themes
            # have ever been installed before
            mkdir -p ${_gnome_dir}/${BASE_ICONS}
            mkdir -p ${_gnome_dir}/${HIGH_CONTRAST_ICONS}
            mkdir -p ${_gnome_dir}/${HIGH_CONTRAST_INVERSE_ICONS}
            mkdir -p ${_gnome_dir}/${LOW_CONTRAST_ICONS}

            # copy the icons used on the GNOME desktop
            cp -f $LATEST_JRE_PATH/lib/images/icons/${_icon}.${_icon_ext} \
                  ${_gnome_dir}/${BASE_ICONS}/${_icon}.${_icon_ext}
            cp -f $LATEST_JRE_PATH/lib/images/icons/${_icon}_HighContrast.${_icon_ext} \
                  ${_gnome_dir}/${HIGH_CONTRAST_ICONS}/${_icon}.${_icon_ext}
            cp -f $LATEST_JRE_PATH/lib/images/icons/${_icon}_HighContrastInverse.${_icon_ext} \
                  ${_gnome_dir}/${HIGH_CONTRAST_INVERSE_ICONS}/${_icon}.${_icon_ext}
            cp -f $LATEST_JRE_PATH/lib/images/icons/${_icon}_LowContrast.${_icon_ext} \
                  ${_gnome_dir}/${LOW_CONTRAST_ICONS}/${_icon}.${_icon_ext}
        fi

        # since we are pretty certain this is a valid GNOME installation, create
        # the required association directories in case they don't already exist.
        mkdir -p ${_gnome_dir}/${MIME_INFO}
        mkdir -p ${_gnome_dir}/${APP_REGISTRY}

        GNOME_KEYS_FILE=${_gnome_dir}/${MIME_INFO}/${_name}.keys
        GNOME_MIME_FILE=${_gnome_dir}/${MIME_INFO}/${_name}.mime
        GNOME_APPS_FILE=${_gnome_dir}/${APP_REGISTRY}/${_name}.applications

        # Create the keys file.
        echo "${_mime_type}"                                           > $GNOME_KEYS_FILE
        echo "        description=${_description}"                    >> $GNOME_KEYS_FILE
        echo "        icon_filename=${_icon}"                         >> $GNOME_KEYS_FILE
        echo "        default_action_type=application"                >> $GNOME_KEYS_FILE
        echo "        default_application_id=${_name}"                >> $GNOME_KEYS_FILE
        echo "        short_list_application_user_additions=${_name}" >> $GNOME_KEYS_FILE

        # Create the mime file.
        echo "${_mime_type}"               > $GNOME_MIME_FILE
        echo "        ext: ${_extension}" >> $GNOME_MIME_FILE

        # Create the apps file.
        echo "${_name}"                               > $GNOME_APPS_FILE
        echo "        command=${_command}"           >> $GNOME_APPS_FILE
        echo "        name=${_name}"                 >> $GNOME_APPS_FILE
        echo "        can_open_multiple_files=false" >> $GNOME_APPS_FILE
        echo "        requires_terminal=false"       >> $GNOME_APPS_FILE
        echo "        mime_types=${_mime_type}"      >> $GNOME_APPS_FILE
    fi
}
# End of AddGNOME

#=================================================
# Remove an entry fromt GNOME application registry
#     param - $1 - name  used to name GNOME registry files
#-------------------------------------------------
RemoveGNOME() {
    _name=$1

    if [ -n "${_name}" ]; then
        # check the GNOMEDIR environment variable to see if it points to a valid
        # GNOME top level directory.
        _gnome_dir=`GetGNOMETopDir ${GNOMEDIR}`
        if [ -z "${_gnome_dir}" ]; then
            # GNOMEDIR was invalid, so check in /usr
            _gnome_dir=`GetGNOMETopDir ${GNOMEDIR_DEFAULT}`
            if [ -z "${_gnome_dir}" ]; then
                # GNOME doesn't seem to be installed
                return 1
            fi
        fi

        GNOME_KEYS_FILE=${_gnome_dir}/${MIME_INFO}/${_name}.keys
        GNOME_MIME_FILE=${_gnome_dir}/${MIME_INFO}/${_name}.mime
        GNOME_APPS_FILE=${_gnome_dir}/${APP_REGISTRY}/${_name}.applications

        rm -f $GNOME_KEYS_FILE $GNOME_MIME_FILE $GNOME_APPS_FILE
    fi
}
# End of RemoveGNOME

#
# MakeLink ( _src _dst )
#
# Create a link if a file/directory doesn't exist at the given _dst.
#
# Fix for 4938584 - Java links need to be set up on Java Desktop System.
# This will do until mJRE is implemented on Linux
#
# Parameters:
#   $1  _src  the source for the link
#   $2  _dst  the destination for the link
#
MakeLink () {
    if [ \( ! -f $2 -a ! -d $2 \) -o -h $2 ]; then
        rm -f $2
        if [ -n $1 ]; then
            ln -s $1 $2
        fi
    fi
}

if [ "${RPM_INSTALL_PREFIX}" != "%{jdk_prefix}" ]; then
    # Remove the symlink and install dir if installed in non-default location
    if [ -h %{jdk_prefix}/%{jdk_name}%{jdk_version} ]; then
        rm -f %{jdk_prefix}/%{jdk_name}%{jdk_version}
    fi
    if [ -d $RPM_INSTALL_PREFIX/%{jdk_name}%{jdk_version} ]; then
        rm -rf $RPM_INSTALL_PREFIX/%{jdk_name}%{jdk_version}
    fi
else
    rm -rf %{jdk_prefix}/%{jdk_name}%{jdk_version}
fi

%ifarch ia64 || x86_64
    # No GNOME icons, javaws, or shared class files on 64-bit
%else
    MIME_TYPE=%{mime_type}

    GNOMEDIR_DEFAULT=$RPM_BUILD_ROOT/usr
    GNOME_SHARE=share
    MIME_INFO=${GNOME_SHARE}/mime-info
    APP_REGISTRY=${GNOME_SHARE}/application-registry

    BASE_ICONS=share/pixmaps
    HIGH_CONTRAST_ICONS=share/icons/HighContrast/48x48/apps
    HIGH_CONTRAST_INVERSE_ICONS=share/icons/HighContrastInverse/48x48/apps
    LOW_CONTRAST_ICONS=share/icons/LowContrast/48x48/apps

    # Try to set latest path to Java Web Start
    LATEST_JAVAWS_PATH=`GetLatestJavaWSPath`

    if [ \( -n "${LATEST_JAVAWS_PATH}" -a -x "${LATEST_JAVAWS_PATH}" \) ]; then
        # get the path to the "best choice" JRE
        LATEST_JRE_PATH=`dirname \`dirname $LATEST_JAVAWS_PATH\``

        # Setup the mailcap file
        UpdateMailcap $RPM_BUILD_ROOT/etc/mailcap

        # Setup the mime.type file
        UpdateMimeTypes $RPM_BUILD_ROOT/etc/mime.types

        # Setup the GNOME associations
        AddGNOME application/java-archive \
                 jar \
                 java-archive \
                 "${LATEST_JRE_PATH}/bin/java -jar" \
                 sun-java \
                 png \
                 "Java Archive"

        AddGNOME ${MIME_TYPE} \
                 jnlp \
                 java-web-start \
                 ${LATEST_JAVAWS_PATH} \
                 sun-java \
                 png \
                 "Java Web Start Application"

        # Let's wait for CCC approval on this - MWR
        # MakeLink ${LATEST_JRE_PATH}/bin/java /usr/bin/java
        # MakeLink ${LATEST_JAVAWS_PATH} /usr/bin/javaws
    else
        # no JaWS left on system so remove old entries

        # Remove the mailcap file
        RemoveMailcap $RPM_BUILD_ROOT/etc/mailcap

        # Remove the mime.type file
        RemoveMimeTypes $RPM_BUILD_ROOT/etc/mime.types

        # Remove the GNOME associations
        RemoveGNOME java-web-start
        RemoveGNOME java-archive
    fi

    #
    # remove sun_java.desktop file from common desktop file location 
    # if it belongs to this jre.
    #
    DESKTOP_FILE_PATH=`echo $DESKTOP_FILE_PATH`
    if [ -z "${DESKTOP_FILE_PATH}" ]; then
        DESKTOP_FILE_PATH=/usr/share/control-center-2.0/capplets
        if [ ! -d $DESKTOP_FILE_PATH ]; then
            DESKTOP_FILE_PATH=/usr/share/applications
            if [ ! -d $DESKTOP_FILE_PATH ]; then
                DESKTOP_FILE_PATH=/usr/share/gnome/apps/Settings
            fi
        fi
    fi

    INSTALL_PATH=%{jre_prefix}
    JRE_STRING=%{jre_name}%{jre_version}
    FOUND_CNT=`grep "$INSTALL_PATH/$JRE_STRING" $DESKTOP_FILE_PATH/sun_java.desktop 2> /dev/null`
    if [ -n "$FOUND_CNT" ]; then
        # remove the old desktop file
        rm -f $DESKTOP_FILE_PATH/sun_java.desktop

        if [ -n "$LATEST_JRE_PATH" -a "$INSTALL_PATH/$JRE_STRING" != "$LATEST_JRE_PATH" ]; then
            # update sun_java.desktop to use the "best choice" selected by
            # GetLatestJavaWSPath for the control panel
            INSTALL_PATH=`dirname $LATEST_JRE_PATH`
            JRE_STRING=`basename $LATEST_JRE_PATH`

            sed "s|INSTALL_DIR|$INSTALL_PATH|g" \
                     $INSTALL_PATH/$JRE_STRING/plugin/desktop/sun_java.desktop \
                 | sed "s|JRE_NAME_VERSION|$JRE_STRING|g" \
                 > ${DESKTOP_FILE_PATH}/sun_java.desktop
        fi
    fi
%endif
