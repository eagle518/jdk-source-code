#
# Copyright (c) 2008, 2010, Oracle and/or its affiliates. All rights reserved.
# ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
#
# @(#)generic-rpm.spec	1.15 10/05/21
#
# Now setup the generic portion of the spec file
#
%define target_os               TARGET_OS
%define target_arch             TARGET_ARCH
%define legacy_arch             i386
%define major                   MAJOR_VERSION
%define minor                   MINOR_VERSION
%define micro                   MICRO_VERSION
%define update                  UPDATE_VERSION
%define milestone_name          MILESTONE
%define build_number            BUILD_NUMBER
USE_UPDATE_IF_DEFINED
%define short_market_version    %{minor}.%{micro}
%define long_market_version     %{minor}.%{micro}%{?use_update:u%{update}}
%define tech_version            %{major}.%{minor}.%{micro}%{?use_update:_%{update}}

%define epoch                   2000
%define use_epoch               USE_EPOCH_IF_JDK
%define name                    RPM_PKG_NAME
%define official_name           OFFICIAL_PRODUCT_NAME
%define version                 %{tech_version}
%define release                 %{milestone_name}
%define url_ref			URL_REF
%define license_type		LICENSE_TYPE

%define java_prefix             /usr/java
%define man_prefix              /usr/local/man/man1
%define preference_prefix       /etc/.java/.systemPrefs
%define java_type		JAVA_TYPE
%define install_dir             %{java_type}%{version}
%define install_root            %{java_prefix}/%{install_dir}
%define latest_link             %{java_prefix}/latest
%define default_link            %{java_prefix}/default
%define bin_default_dir         /usr/bin
%define bin_default_jre_links   java javaws jcontrol
%define bin_default_jdk_links   %{bin_default_jre_links} javac jar javadoc
%define bin_default_links       PACKAGE_BIN_LINKS
%define first_desktop_integ     1.5.0
%define first_default_integ     1.6.0
%define first_jexec_integ       1.6.0

%define init_d                  /etc/init.d
%define init_script             jexec
%define lsb_init_installer      /usr/lib/lsb/install_initd
%define lsb_init_uninstaller    /usr/lib/lsb/remove_initd
%define chkconfig               /sbin/chkconfig


%define jre_classes_jsa         lib/%{legacy_arch}/client/classes.jsa
%define jdk_classes_jsa         jre/lib/%{legacy_arch}/client/classes.jsa
%define classes_jsa             CLASSES_JSA

%define tarfile                 SOURCE_TARFILE
%define packed_jars             PACKED_JARS
%define jre_docs                CHANGES|COPYRIGHT|README|Welcome.html
%define jdk_docs                COPYRIGHT|README.html|README_ja.html|README_zh_CN.html
%define package_docs            PACKAGE_DOCS

%define default_gnomedir        /usr
%define default_desktop_path    ${GNOMEDIR}/share/applications

%define jar_mime_type           application/x-java-archive

%define javaws_bin              JAVAWS_BIN
%define javaws_mime_type        application/x-java-jnlp-file
%define javaws_mime_desc        "Java Web Start"
%define javaws_mime_exts        jnlp

%define tmpdir                  %{_tmppath}
%define package_filelist        %{tmpdir}/%{name}.files

#
# Define macros that formated descriptions of the packages
#
%define package_description     %{expand:%%(%{_sourcedir}/PACKAGE_DESCRIPTION)}

#
# Define macros that contain common/frequently used shell scripts for  use
# within the various package scriptlets.
#
# this is an embedded tab ------------------v so don't convert tabs to spaces
%define expand_cmd              sed -e '/^[ 	]*\\(#.*\\)\\{0,1\\}$/d'
%define unpack_jars_function    %{expand:%%(%{expand_cmd} %{_sourcedir}/unpack_jars)}
%define legacy_mime_support     %{expand:%%(%{expand_cmd} %{_sourcedir}/legacy_mime_support)}
%define gnome_support           %{expand:%%(%{expand_cmd} %{_sourcedir}/gnome_support)}
%define parse_release_info1     %{expand:%%(%{expand_cmd} %{_sourcedir}/parse_release_info.part1)}
%define parse_release_info2     %{expand:%%(%{expand_cmd} %{_sourcedir}/parse_release_info.part2)}
%define link_support            %{expand:%%(%{expand_cmd} %{_sourcedir}/link_support)}


%if %{use_epoch}
Epoch:          %{epoch}
%endif
Name:           %{name}
Version:        %{version}
Release:        %{release}
# Copyright was deprecated because the proper use was ambiguous
License:        %{license_type}
Vendor:         Oracle and/or its affiliates.
URL:            %{url_ref}
Packager:       Java Software <jre-comments@java.sun.com>
Group:          Development/Tools
Summary:        %{official_name}
AutoReqProv:    no
Provides:       jaxp_parser_impl
Provides:       xml-commons-apis
Requires:       /bin/basename
Requires:       /bin/cat
Requires:       /bin/cp
Requires:       /bin/gawk
Requires:       /bin/grep
Requires:       /bin/ln
Requires:       /bin/ls
Requires:       /bin/mkdir
Requires:       /bin/mv
Requires:       /bin/pwd
Requires:       /bin/rm
Requires:       /bin/sed
Requires:       /bin/sort
Requires:       /bin/touch
Requires:       /usr/bin/cut
Requires:       /usr/bin/dirname
Requires:       /usr/bin/expr
Requires:       /usr/bin/find
Requires:       /usr/bin/tail
Requires:       /usr/bin/tr
Requires:       /usr/bin/wc
BuildPreReq:    /usr/bin/find
BuildPreReq:    /usr/bin/install
BuildPreReq:    /bin/cat
BuildPreReq:    /bin/chmod
BuildPreReq:    /bin/cp
BuildPreReq:    /bin/gawk
BuildPreReq:    /bin/rm
BuildPreReq:    /bin/sed
BuildPreReq:    /bin/touch
Source0:        %{tarfile}
Source1:        unpack_jars
Source2:        legacy_mime_support
Source3:        gnome_support
Source4:        parse_release_info.part1
Source4:        parse_release_info.part2
Source5:        link_support
Source6:        PACKAGE_DESCRIPTION
Buildroot:      USE_THIS_BUILDROOT
Prefix:         %{java_prefix}

#
# The description tag can span more than one line.  In addition, a primitive
# formatting capability exists. If a line starts with a space, that line will
# be displayed verbatim by RPM. Lines that do not start with a space are
# assumed to be part of a paragraph and will be formatted by RPM. It's even
# possible to mix and match formatted and unformatted lines.
#
%description
%{package_description}

#
# Prepare for the build and install phase.
#
%prep
%setup -n %{install_dir}

    chmod -R go=u-w *
    chmod -R u+w *


#
# Do a mock install of the files in the RPM_BUILD_ROOT
#
%install
    #
    # Remove any deadwood from the last build.
    #
    rm -Rf $RPM_BUILD_ROOT/*

    #
    # Create a variable containing the path to the "build".  This is the source
    # files that this installation is based on.
    #
    build_path=${RPM_BUILD_DIR}/%{install_dir}

    #
    # Create the basic installation structure by installing the contents
    # of the tarball bundle into the <install-root>
    # (i.e. /usr/java/<name><tech_version>).
    #
    install -d -m 755 ${RPM_BUILD_ROOT}%{install_root}
    cp -dpR ${build_path}/* ${RPM_BUILD_ROOT}%{install_root}

    #
    # Only create the shared class archive ghost if this is a 32-bit package
    #
%ifnarch ia64 x86_64
    touch ${RPM_BUILD_ROOT}%{install_root}/%{classes_jsa}
%endif

    #
    # Setup the structure required by the Preferences API.  This will only be
    # put on the destination system if it doesn't already exist.
    #
    install -d -m 755 ${RPM_BUILD_ROOT}%{preference_prefix}
    touch ${RPM_BUILD_ROOT}%{preference_prefix}/.system.lock
    touch ${RPM_BUILD_ROOT}%{preference_prefix}/.systemRootModFile

    #
    # Copy in the init file for supporting the JAR binary type using binfmt_misc
    #
    install -d -m 755 ${RPM_BUILD_ROOT}%{init_d}
    cp %{_sourcedir}/%{init_script} ${RPM_BUILD_ROOT}%{init_d}/%{init_script}
    chmod a=rx,u+w ${RPM_BUILD_ROOT}%{init_d}/%{init_script}

    #
    # Build a proper file list.  This prevents us from needing to do anything
    # ugly in the post-uninstall (%postun), like `rm -Rf`.
    #
    # Start by defining the default file attributes, and adding the Preferences
    # API config files, so they will be added to the system only when they don't
    # already exist.
    #
    cat << EOF > %{package_filelist}
%defattr(-,root,root)
%config(noreplace) %verify(not md5 size mtime) %{preference_prefix}/.system.lock
%config(noreplace) %verify(not md5 size mtime) %{preference_prefix}/.systemRootModFile
%config %verify(not md5 size mtime) %attr(755,root,root) %{init_d}/%{init_script}
%ifnarch ia64 x86_64
%ghost %attr(444,root,root) %{install_root}/%{classes_jsa}
%endif
EOF

    #
    # Add all the files in <install-root>, but ignore the directories for now.
    # Also make sure we account for the unpacked JAR files and the PACK files
    # that will get deleted in post-install (%post).
    #
    find ${build_path} -depth ! -type d -print |                          \
        sed "s|${build_path}|%{install_root}|" |                          \
        awk '/^.+\.pack$/                                                 \
             {                                                            \
                 split($0, parts, ".pack$");                              \
                 printf("%%%%config(missingok) \%s.pack\n", parts[1]);    \
                 printf("%%%%ghost \%s.jar\n", parts[1]);                 \
                 next;                                                    \
             }                                                            \
             /^.+\/man1\/.+\.1$/                                          \
             {                                                            \
                 printf("%%%%doc \%s\n", $0);                             \
                 next;                                                    \
             }                                                            \
             /%{package_docs}/                                            \
             {                                                            \
                 # ignore anything in the demo/sample directory           \
                 split($0, parts, "/");                                   \
                 if ((parts[5] != "demo") && (parts[5] != "sample"))      \
                 {                                                        \
                     printf("%%%%doc \%s\n", $0);                         \
                     next;                                                \
                 }                                                        \
             }                                                            \
             {                                                            \
                 print $0;                                                \
             }'                                                           \
            >> %{package_filelist}

    #
    # create ghost files for the unpacked JAR files
    #
    for file in %{packed_jars}; do
        touch ${RPM_BUILD_ROOT}%{install_root}/${file}
        chmod a=r,u+w ${RPM_BUILD_ROOT}%{install_root}/${file}
    done

    #
    # Create a list of directories that are added by this package.  Include all
    # directories on the off chance that this is the first package installed.
    #
    additional_dirs=

    #
    # Start by adding the <install-root> parent directory.
    #
    path=%{java_prefix}
    while [ -n "${path}" ]; do
        if [ "${path}" != "/" ]; then
            additional_dirs="%%%%dir ${path}\n${additional_dirs}"
            path=`dirname ${path}`
        else
            path=
        fi
    done

    #
    # Next add the directories used by the Preferences API.
    #
    path=%{preference_prefix}
    while [ -n "${path}" ]; do
        if [ "${path}" != "/" ]; then
            additional_dirs="%%%%dir ${path}\n${additional_dirs}"
            path=`dirname ${path}`
        else
            path=
        fi
    done

    #
    # Add the additional directories to the list of files.
    #
    printf "${additional_dirs}" >> %{package_filelist}

    #
    # Finally, add the <install-root> subdirectories
    #
    find ${build_path} -type d -print |             \
        sed "s|${build_path}|%dir %{install_root}|" \
            >> %{package_filelist}



%files -f %{package_filelist}



%clean
    rm -Rf $RPM_BUILD_ROOT
    rm -Rf $RPM_BUILD_DIR/%{install_dir}

%post
    #
    # Make sure any new files are created with a secure access mask.  Do not use
    # chmod, since that would also change the rights of any existing files, and
    # we are only interested in setting the rights for new files.
    #
    umask 022

    #
    # The package assumes that Gnome is either installed, or going to be
    # installed, so if nothing currently exists, then install in the default
    # location.
    #
    # NOTE: These variables must be defined before all the shell function macros
    #       are included.
    #
    if [ -z "${GNOMEDIR}" ]; then
        GNOMEDIR=%{default_gnomedir}
    fi

    #
    # RPM_INSTALL_PREFIX doesn't seem to be set by "alien" so the following
    # minor kludge allows some functionality on debian-like systems (such
    # a Ubuntu) which don't support packages.
    #
    if [ -z "${RPM_INSTALL_PREFIX}" ]; then
	RPM_INSTALL_PREFIX="/usr/java"
    fi

    #
    # Gross kludge for old SuSE distros: Even though they set the environment
    # variable GNOMEDIR to /opt/gnome, Gnome may really be in /opt/gnome2.
    # Go figure,... (I feel so unclean....)
    #
    if [ "${GNOMEDIR}" = "/opt/gnome" ] && [ -d "/opt/gnome2" ]; then
	GNOMEDIR="/opt/gnome2"
    fi

    INSTALL_JRE_PATH=${RPM_INSTALL_PREFIX}/%{install_dir}
    if [ -e ${INSTALL_JRE_PATH}/jre/bin/java ]; then
	INSTALL_JRE_PATH=${INSTALL_JRE_PATH}/jre
    fi

    #
    # Add the shell function and related variables used by the post-install.
    #
    %{unpack_jars_function}
    %{legacy_mime_support}
%ifnarch ia64 x86_64
    %{gnome_support}
%endif
    %{parse_release_info1}
    %{parse_release_info2}
    %{link_support}

    #
    # Unpack the packed JAR files.
    #
    unpack_jars "${RPM_INSTALL_PREFIX}/%{install_dir}/bin/unpack200" \
                "${RPM_INSTALL_PREFIX}/%{install_dir}" \
                %{packed_jars}

%ifnarch ia64 x86_64
    # fix for: 4728032 - Install needs to generate shared class files
    ${RPM_INSTALL_PREFIX}/%{install_dir}/bin/java -client -Xshare:dump > /dev/null 2>&1
%endif

    # Create a service tag if supported on the system.
    # No product registration is done.
    #
    # If the package is being relocated, the installer will not create 
    # a service tag since the service tag registry client 
    # (see /usr/bin/stclient) only supports local system use.
    # A RFE# 6576434 is created for stclient to support the remote installation
    # support.
    #
    if [ "${RPM_INSTALL_PREFIX}" = "%{java_prefix}" ]; then
         ${RPM_INSTALL_PREFIX}/%{install_dir}/bin/java com.sun.servicetag.Installer -source "%{name}" > /dev/null 2>&1
    fi

    #
    # Find out what version of Java is the latest.  Don't do any system
    # integration unless this is the latest version.  Otherwise, we make it
    # difficult for future installers.
    #
    LATEST_JAVA_PATH="`find_latest_release`"
    if [ "${LATEST_JAVA_PATH}" == "${RPM_INSTALL_PREFIX}/%{install_dir}" ] ||
       [ "${LATEST_JAVA_PATH}" == "%{install_root}" ]
    then
        #
        # Make sure the %{latest_link} link points to LATEST_JAVA_PATH, and
	# update it if it doesn't.
        #
        setup_latest_link "${LATEST_JAVA_PATH}" "%{latest_link}"

        #
        # Make sure the %{default_link} and %{bin_default_links} exist.
	# If anything is missing, create it.
        #
        setup_default_links "%{latest_link}" "%{default_link}" \
                            "%{bin_default_dir}" %{bin_default_links}

        #
        # If the "latest" link is a JDK, then the latest JRE is a subdir;
	# otherwise it is the same dir.
        #
        DEFAULT_JRE_PATH="%{default_link}"
        if [ -e "%{default_link}/jre/bin/java" ]; then
            DEFAULT_JRE_PATH="%{default_link}/jre"
        fi

%ifnarch ia64 x86_64
	#
	# Perform all integrations with the freedesktop.org desktop integration
	# specifications and Gnome legacy implementations.
	#
	IntegrateWithGnome

        # setup the mailcap file
        UpdateMailcap /etc/mailcap %{javaws_mime_type} "/usr/bin/javaws %s"

        # setup the mime.type file
        UpdateMimeTypes /etc/mime.types %{javaws_mime_type} \
			%{javaws_mime_desc} %{javaws_mime_exts}
%endif
    fi

    #
    # If the package is being relocated, then create a link in the default
    # location (%{java_prefix}) to the actual install directory.  Do this
    # last, so it doesn't add unnecessary complexity to the search for the
    # latest release.
    #
    if [ "${RPM_INSTALL_PREFIX}" != "%{java_prefix}" ] &&
       ( [ ! -e "%{install_root}" ] || [ -h "%{install_root}" ] )
    then
        rm -f "%{install_root}"
        ln -s "${RPM_INSTALL_PREFIX}/%{install_dir}" "%{install_root}"
    fi

    #
    # Next, make sure the files required for the Prferences API are setup
    # correctly.  Any files from an old, uninstalled version will have left
    # files with a .rpmsave extension.  If there was an older version currently
    # installed when this version installed, there will be a set of files with
    # a .rpmnew extension.  Try to use the best possible file (i.e. save old
    # preference settings).
    #
    if [ -f %{preference_prefix}/.system.lock.rpmsave ] &&
       [ ! -s %{preference_prefix}/.system.lock ]
    then
        #
        # Only overwrite if old file is empty (rpmsave is only created if it is
        # non-empty).
        #
        rm -f %{preference_prefix}/.system.lock
        mv %{preference_prefix}/.system.lock.rpmsave \
           %{preference_prefix}/.system.lock
    elif [ -f %{preference_prefix}/.system.lock.rpmnew ]
    then
        if [ -s %{preference_prefix}/.system.lock ]; then
            #
            # The existing lock is non-empty, so there is no reason to keep the
            # .rpmnew one created during this install.
            #
            rm -f %{preference_prefix}/.system.lock.rpmnew
        else
            #
            # The existing lock is empty, so replace it with the new one.  This
            # makes future installs a little cleaner, since the file in use is
            # the file in the RPM database.
            #
            rm -f %{preference_prefix}/.system.lock
            mv %{preference_prefix}/.system.lock.rpmnew \
               %{preference_prefix}/.system.lock
        fi
    fi

    if [ -f %{preference_prefix}/.systemRootModFile.rpmsave ] &&
       [ ! -s %{preference_prefix}/.systemRootModFile ]
    then
        #
        # Only overwrite if old file is empty (rpmsave is only created if it is
        # non-empty).
        #
        rm -f %{preference_prefix}/.systemRootModFile
        mv %{preference_prefix}/.systemRootModFile.rpmsave \
           %{preference_prefix}/.systemRootModFile
    elif [ -f %{preference_prefix}/.systemRootModFile.rpmnew ]
    then
        if [ -s %{preference_prefix}/.systemRootModFile ]; then
            #
            # The existing lock is non-empty, so there is no reason to keep the
            # .rpmnew one created during this install.
            #
            rm -f %{preference_prefix}/.systemRootModFile.rpmnew
        else
            #
            # The existing lock is empty, so replace it with the new one.  This
            # makes future installs a little cleaner, since the file in use is
            # the file in the RPM database.
            #
            rm -f %{preference_prefix}/.systemRootModFile
            mv %{preference_prefix}/.systemRootModFile.rpmnew \
               %{preference_prefix}/.systemRootModFile
        fi
    fi

    #
    # Try to register the init script to the various run levels.  If possible
    # this is accomplished using an LSB defined install tool.  If that isn't
    # available, then try to use chkconfig, which is supported by Red Hat and
    # Debian.  The feature of automatic jar file execution is not support on
    # systems which don't support either of these interfaces.
    #
    if [ -x %{lsb_init_installer} ]; then
        %{lsb_init_installer} %{init_script} > /dev/null 2>&1

        # start the service for the current session
        %{init_d}/%{init_script} start > /dev/null 2>&1
    elif [ -x %{chkconfig} ]; then
        %{chkconfig} --add %{init_script} > /dev/null 2>&1

        # start the service for the current session
        %{init_d}/%{init_script} start > /dev/null 2>&1
    fi


%preun
    #
    # Add the shell function and related variables used by the pre-uninstall.
    #
    %{parse_release_info1}
    %{parse_release_info2}
    %{link_support}

    # Delete a service tag if exists 
    #
    # If the package is being relocated, the installer will not create 
    # a service tag since the service tag registry client 
    # (see /usr/bin/stclient) only supports local system use.
    # A RFE# 6576434 is created for stclient to support the remote installation
    # support.
    #
    if [ "${RPM_INSTALL_PREFIX}" = "%{java_prefix}" ]; then 
        ${RPM_INSTALL_PREFIX}/%{install_dir}/bin/java com.sun.servicetag.Installer -delete 
    fi

    #
    # Dereference and follow any links that might have been created when this
    # package was installed.  If a link ultimately points to this installation
    # or the link is dead, then we should remove the link.  Important links,
    # like default and latest can be remade in post-uninstall (%postun).
    #
    # This is done in the reverse order the links were initially created in, in
    # case there are any partial loops.
    #
    if [ -h "%{default_link}" ]; then
        DEFAULT_LINK="`dereference --follow \"%{default_link}\"`"
        if [ $? -ne 0 ] ||
           [ "${DEFAULT_LINK}" = "${RPM_INSTALL_PREFIX}/%{install_dir}" ]
        then
            cleanup_default_links "%{default_link}" \
                                  "%{bin_default_dir}" %{bin_default_jdk_links}
        fi
    fi

    #
    # If the latest link still points to this installation it must mean one of
    # the following:
    #
    #     * No newer version of Java has been installed.  This is known because
    #       any such version would have already changed the latest to point
    #       to itself.
    #
    #     * No older version is installed.  We don't check this now, but if this
    #       is the case, now is the best time to remove the latest link, since
    #       anything pointing to this installation in post-uninstall will be a
    #       dead link.
    #
    #     * There is an older version of Java installed.  In this case we need
    #       to handle the latest link differently depending on what version
    #       remains.
    #
    if [ -h "%{latest_link}" ]; then
        LATEST_LINK="`dereference --follow \"%{latest_link}\"`"
        if [ $? -ne 0 ] ||
           [ "${LATEST_LINK}" = "${RPM_INSTALL_PREFIX}/%{install_dir}" ]
        then
            #
            # If this version is the latest, and the first version with jexec
            # support, then stop and remove the jexec service.  If this isn't
            # done now, there might not be an init script left when %postun is
            # called, and if there is, we can restart/reinstall the service
            # easy enough then.
            #
            if [ `compare_java_by_version ${LATEST_LINK} \
                                  version-%{first_jexec_integ}` -ge 0 ] &&
               [ -x %{init_d}/%{init_script} ]
            then
                %{init_d}/%{init_script} stop > /dev/null 2>&1

                if [ -x %{lsb_init_uninstaller} ]; then
                    %{lsb_init_uninstaller} %{init_script} > /dev/null 2>&1
                elif [ -x %{chkconfig} ]; then
                    %{chkconfig} --del %{init_script} > /dev/null 2>&1
                fi
            fi

            rm -f "%{latest_link}" 2> /dev/null
        fi
    fi

    #
    # If the package was relocated when it was installed, there should be a link
    # in %{java_prefix}.  So, if there is a link named %{install_root} that is
    # dead, or points back to ${RPM_INSTALL_PREFIX}, delete it.
    #
    if [ "${RPM_INSTALL_PREFIX}" != "%{java_prefix}" ] &&
       [ -h "%{install_root}" ]
    then
        THIS_LINK="`dereference --follow \"%{install_root}\"`"
        if [ $? -ne 0 ] ||
           [ "${THIS_LINK}" = "${RPM_INSTALL_PREFIX}/%{install_dir}" ]
        then
            rm -f "%{install_root}" 2> /dev/null
        fi
    fi


%postun
    #
    # Make sure any new files are created with a secure access mask.  Do not use
    # chmod, since that would also change the rights of any existing files, and
    # we are only interested in setting the rights for new files.
    #
    umask 022

    #
    # The package assumes that Gnome is either installed, or going to be
    # installed, so if nothing currently exists, then install in the default
    # location.
    #
    # NOTE: These variables must be defined before all the shell function macros
    #       are included.
    #
    if [ -z "${GNOMEDIR}" ]; then
        GNOMEDIR=%{default_gnomedir}
    fi

    #
    # RPM_INSTALL_PREFIX doesn't seem to be set by "alien" so the following
    # minor kludge allows some functionality on debian-like systems (such
    # a Ubuntu) which don't support packages.
    #
    if [ -z "${RPM_INSTALL_PREFIX}" ]; then
	RPM_INSTALL_PREFIX="/usr/java"
    fi

    #
    # Gross kludge for old SuSE distros: Even though they set the environment
    # variable GNOMEDIR to /opt/gnome, Gnome may really be in /opt/gnome2.
    # Go figure,... (I feel so unclean....)
    #
    if [ "${GNOMEDIR}" = "/opt/gnome" ] && [ -d "/opt/gnome2" ]; then
	GNOMEDIR="/opt/gnome2"
    fi

    INSTALL_JRE_PATH=${RPM_INSTALL_PREFIX}/%{install_dir}
    if [ -e ${INSTALL_JRE_PATH}/jre/bin/java ]; then
	INSTALL_JRE_PATH=${INSTALL_JRE_PATH}/jre
    fi

    #
    # Add the shell function and related variables used by the post-uninstall.
    #
    %{legacy_mime_support}
%ifnarch ia64 x86_64
    %{gnome_support}
%endif
    %{parse_release_info1}
    %{parse_release_info2}
    %{link_support}

    #
    # The RPM update command installs a given version of a package, and then
    # uninstalls all other versions of a package.
    #
    # The command does the following:
    #
    #     1) Run the %pre for the package being installed
    #     2) Install the new package's files
    #     3) Run the %post for the package being installed
    #     4) Run the %preun for each package being uninstalled
    #     5) Delete any old files not overwitten by the package being installed
    #     6) Run the %postun for each package being uninstalled
    #
    # Note: Because each version of Java installs into its own unique directory,
    #       the only files in step 5 that might not be deleted are the files in
    #       %{preference_prefix} that are used for the Preferences API.
    #
    # Note: The order described above is also the same order that occurs when a
    #       user installs a new version, then uninstalls an old version at a
    #       later date.  The only difference is the ammount of time that passes
    #       between steps 3 and 4.
    #
    # Because of this order, all changes made to the system by the package
    # being installed are made *before* any changes made by packages being
    # uninstalled. This means that it is important that the %preun and %postun
    # scriptlets are written in a way that does not break any integration just
    # setup by the new package.  This makes it very difficult to determine what
    # should and shouldn't be removed during %preun and %postun scriptlets.
    #
    # Packages written in the past have no idea what the future will hold.  This
    # is obvious, but it doesn't make it easy.  One option is to assume users
    # will always install newer versions over older versions, and will never
    # keep multiple versions of the same package installed at the same time.
    # This is actually the assumption that RPM is designed upon.
    #
    # However, the --force option can be used to force RPM to install an older
    # package; a so called downgrade.  In the past, Java RPM packages have
    # always attempted to provide special support for downgrades.  This can
    # cause a lot of trouble given the design of RPM.
    #
    # This spec follows the recomended RPM practice.  If the version being
    # uninstalled is not the latest version, then nothing is done.  However, if
    # the version being uninstalled is the latest, then anything setup by the
    # %post scriptlet that is not also tracked by the RPM database is removed.
    #
    # Unfortunately there are two damned kinds of Java installations for every
    # given release, i.e. a JDK and a JRE.  Because of this, it is possible that
    # this version being uninstalled is the latest version, and that the version
    # being left behind is the *same* version as this!
    #
    # In this case, it is necessary to fix anything just broken by the %preun
    # scriptlet.  This will only happen when the JDK is uninstalled, and the
    # JRE of the same version is still on the system.  For this, all that needs
    # to be done is to repair the default and latest links.

    #
    # Determine if a new latest link should be created.  This is done if
    # there is still an installed version of Java that is 1.6 up to the
    # the version of this package.
    #
    LATEST_JAVA_PATH="`find_latest_release`"
    if [ -n "${LATEST_JAVA_PATH}" ] &&
       [ `compare_java_by_version ${LATEST_JAVA_PATH} \
                                  version-%{tech_version}` -lt 0 ]
    then
        #
        # Only maintain the latest link if the latest version left on
        # the system is the ugly stepsister to this one, i.e. this is
	# the JDK, and the JRE of the same version remains.
        #
        # Note: if the latest is higher than the version of this
        #       package, then latest will either A) already exist,
        #       so there is nothing that needs to be done, or B) the
        #       latest link is no longer supported buy those versions,
        #       so this package shouldn't set it up.
        #
        setup_latest_link "${LATEST_JAVA_PATH}" "%{latest_link}"
    fi

%ifnarch ia64 x86_64
    if [ -z "${LATEST_JAVA_PATH}" ] ||
       [ `compare_java_by_version ${LATEST_JAVA_PATH} \
                                  version-%{tech_version}` -lt 0 ]
    then
        #
        # Only maintain the latest link if the latest version left on
        # the system is the ugly stepsister to this one, i.e. this is
	# the JDK, and the JRE of the same version remains.
        #
        # Note: again, the best policy is to assume that higher
        #       releases can take better care of themselves
        #

        #
        # Remove all system integration.
        #
        RemoveMailcap /etc/mailcap %{javaws_mime_type}
        RemoveMimeTypes /etc/mime.types %{javaws_mime_type}
	DisintegrateWithGnome
    fi
%endif

    if [ -n "${LATEST_JAVA_PATH}" ]; then
        #
        # We just removed the Prefernce API files, so restore them since there
        # is still a version of Java installed.
        #
        mkdir -p %{preference_prefix}
        if [ -f %{preference_prefix}/.system.lock.rpmsave ]; then
            mv %{preference_prefix}/.system.lock.rpmsave \
               %{preference_prefix}/.system.lock
        elif [ ! -f %{preference_prefix}/.system.lock ]; then
            touch %{preference_prefix}/.system.lock
        fi
        if [ -f %{preference_prefix}/.systemRootModFile.rpmsave ]; then
            mv %{preference_prefix}/.systemRootModFile.rpmsave \
               %{preference_prefix}/.systemRootModFile
        elif [ ! -f %{preference_prefix}/.systemRootModFile ]; then
            touch %{preference_prefix}/.systemRootModFile
        fi
    fi

    if [ -e "%{latest_link}" ]; then
        #
        # If the latest link exists, then make sure the default link exists.
        #
        # Note: instead of trying to determine whether or not the current latest
        #       installation is a JDK or a JRE, just assume it's a JDK.
        #
        setup_default_links "%{latest_link}" "%{default_link}" \
                            "%{bin_default_dir}" "%{bin_default_jdk_links}"

        #
        # If there is still an init script kicking around then restart/reinstall
        # it in case it was stopped/uninstalled during %preun.
        #
        if [ -x %{init_d}/%{init_script} ]; then
            #
            # Try to register the init script to the various run levels.  If
            # possible this is accomplished using an LSB defined install tool.
            # If that isn't available, then try to use chkconfig, which is
            # supported by Red Hat and Debian.  Otherwise it is up to the user
            # to get the script setup for their distribution.
            #
            if [ -x %{lsb_init_installer} ]; then
                %{lsb_init_installer} %{init_script} > /dev/null 2>&1

                # start the service for the current session
                %{init_d}/%{init_script} start > /dev/null 2>&1
            elif [ -x %{chkconfig} ]; then
                %{chkconfig} --add %{init_script} > /dev/null 2>&1

                # start the service for the current session
                %{init_d}/%{init_script} start > /dev/null 2>&1
            fi
        fi
    fi


%verifyscript
    #
    # Make sure any new files are created with a secure access mask.  Do not use
    # chmod, since that would also change the rights of any existing files, and
    # we are only interested in setting the rights for new files.
    #
    umask 022

    #
    # The package assumes that Gnome is either installed, or going to be
    # installed, so if nothing currently exists, then install in the default
    # location.
    #
    # NOTE: These variables must be defined before all the shell function macros
    #       are included.
    #
    if [ -z "${GNOMEDIR}" ]; then
        GNOMEDIR=%{default_gnomedir}
    fi

    #
    # RPM_INSTALL_PREFIX doesn't seem to be set by "alien" so the following
    # minor kludge allows some functionality on debian-like systems (such
    # a Ubuntu) which don't support packages.
    #
    if [ -z "${RPM_INSTALL_PREFIX}" ]; then
	RPM_INSTALL_PREFIX="/usr/java"
    fi

    #
    # Gross kludge for old SuSE distros: Even though they set the environment
    # variable GNOMEDIR to /opt/gnome, Gnome may really be in /opt/gnome2.
    # Go figure,... (I feel so unclean....)
    #
    if [ "${GNOMEDIR}" = "/opt/gnome" ] && [ -d "/opt/gnome2" ]; then
	GNOMEDIR="/opt/gnome2"
    fi

    INSTALL_JRE_PATH=${RPM_INSTALL_PREFIX}/%{install_dir}
    if [ -e ${INSTALL_JRE_PATH}/jre/bin/java ]; then
	INSTALL_JRE_PATH=${INSTALL_JRE_PATH}/jre
    fi
    #
    # Add the shell function and related variables used by the verify script.
    #
    %{legacy_mime_support}
%ifnarch ia64 x86_64
    %{gnome_support}
%endif
    %{parse_release_info1}
    %{parse_release_info2}
    %{link_support}

    #
    # It isn't possible to repair any missing packed JAR files from --verify.
    # This is because the PACK source files are removed during post-install.
    # If an administrator needs to restore missing packed JAR files, they will
    # need to do a --reinstall.
    #

    #
    # If the package was relocated, then temporarily remove the %{java_prefix}
    # link, but only if it really points to this package.
    #
    if [ "${RPM_INSTALL_PREFIX}" != "%{java_prefix}" ] &&
       [ "`dereference --follow \"%{install_root}\"`" = "${RPM_INSTALL_PREFIX}" ]
    then
        rm -f "%{install_root}"
    fi

    #
    # Find out what version of Java is the latest.  Don't do any system
    # integration unless this is the latest version.  Otherwise, we make
    # it difficult for future installers.
    #
    LATEST_JAVA_PATH="`find_latest_release`"
    if [ "${LATEST_JAVA_PATH}" == "${RPM_INSTALL_PREFIX}/%{install_dir}" ] ||
       [ "${LATEST_JAVA_PATH}" == "%{install_root}" ]
    then
        #
        # Make sure the %{latest_link} link points to LATEST_JAVA_PATH, and
	# update it if it doesn't.
        #
        setup_latest_link "${LATEST_JAVA_PATH}" "%{latest_link}"

        #
        # Make sure the %{default_link} and %{bin_default_links} exist.
	# If anything is missing, create it.
        #
        setup_default_links "%{latest_link}" "%{default_link}" \
                            "%{bin_default_dir}" %{bin_default_links}

        #
        # If the "latest" link is a JDK, then the latest JRE is a subdir;
	# otherwise it is the same dir.
        #
        DEFAULT_JRE_PATH="%{default_link}"
        if [ -e "%{default_link}/jre/bin/java" ]; then
            DEFAULT_JRE_PATH="%{default_link}/jre"
        fi

	#
%ifnarch ia64 x86_64
	IntegrateWithGNOME

        # setup the mailcap file
        UpdateMailcap /etc/mailcap %{javaws_mime_type} "/usr/bin/javaws %s"

        # setup the mime.type file
        UpdateMimeTypes /etc/mime.types	%{javaws_mime_type} \
			%{javaws_mime_desc} %{javaws_mime_exts}
%endif
    fi

    #
    # If the package is being relocated, then create a link in the default
    # location (%{java_prefix}) to the actual install directory.  Do this
    # last, so it doesn't add unnecessary complexity to the search for the
    # latest release.
    #
    if [ "${RPM_INSTALL_PREFIX}" != "%{java_prefix}" ] &&
       ( [ ! -e "%{install_root}" ] || [ -h "%{install_root}" ] )
    then
        rm -f "%{install_root}"
        ln -s "${RPM_INSTALL_PREFIX}/%{install_dir}" "%{install_root}"
    fi

    #
    # There should be an init script for jexec on the system.  If it is, then
    # make sure it's installed and running
    #
    if [ -x %{init_d}/%{init_script} ]; then
        #
        # Try to register the init script to the various run levels.  If
	# possible this is accomplished using an LSB defined install tool.
	# If that isn't available, then try to use chkconfig, which is
	# supported by Red Hat and Debian.  The feature of automatic jar
	# file execution is not support on systems which don't support
	# either of these interfaces.
        #
        if [ -x %{lsb_init_installer} ]; then
            %{lsb_init_installer} %{init_script} > /dev/null 2>&1

            # start the service for the current session
            %{init_d}/%{init_script} start > /dev/null 2>&1
        elif [ -x %{chkconfig} ]; then
            %{chkconfig} --add %{init_script} > /dev/null 2>&1

            # start the service for the current session
            %{init_d}/%{init_script} start > /dev/null 2>&1
        fi
    fi
