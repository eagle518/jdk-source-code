/*
 * @(#)jexec.c	1.15 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * jexec for J2SE
 *
 * jexec is used by the system to allow execution of JAR files.
 *    Essentially jexec needs to run java and 
 *    needs to be a native ISA executable (not a shell script), although
 *    this native ISA executable requirement was a mistake that will be fixed.
 *    (<ISA> is sparc or i386).
 *
 *    When you execute a jar file, jexec is executed by the system as follows:
 *	/usr/java/jre/lib/<ISA>/jexec -jar JARFILENAME
 *    so this just needs to be turned into:
 *	/usr/java/jre/bin/java -jar JARFILENAME
 *
 * Solaris systems (new 7's and all 8's) will be looking for jexec at:
 *	/usr/java/jre/lib/<ISA>/jexec
 * Older systems may need to add this to their /etc/system file:
 *	set javaexec:jexec="/usr/java/jre/lib/<ISA>/jexec"
 *     and reboot the machine for this to work.
 *
 * This source should be compiled as:
 *	cc -o jexec jexec.c
 *
 * And jexec should be placed at the following location of the installation:
 *	<INSTALLATIONDIR>/jre/lib/<ISA>/jexec
 *
 * NOTE: Unless <INSTALLATIONDIR> is the "default" JDK on the system
 *	 (i.e. /usr/java -> <INSTALLATIONDIR>), this jexec will not be
 *	 found.  The 1.2 java is only the default on Solaris 8 and
 *       on systems where the 1.2 packages were installed and no 1.1
 *       java was found.
 *
 * NOTE: You must use 1.2 jar to build your jar files. The system
 *       doesn't seem to pick up 1.1 jar files.
 *
 * NOTE: We don't need to set LD_LIBRARY_PATH here, even though we
 *       are running the actual java binary because the java binary will
 *       look for it's libraries through it's own runpath, which uses
 *       $ORIGIN.
 *
 * NOTE: This jexec should NOT have any special .so library needs because
 *       it appears that this executable will NOT get the $ORIGIN of jexec
 *       but the $ORIGIN of the jar file being executed. Be careful to keep
 *       this program simple and with no .so dependencies.
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>

int 
main(int argc, char * argv []) {
   
    char java[PATH_MAX+1];

    /* Make sure we have something to work with */
    if (argc <= 0 || argv == 0) {
	/* Shouldn't happen... */
	return 1;
    }

    /* get the full path of this program
     * something like <FOO>/jre/lib/<ISA>/jexec */
    realpath(argv[0],java);

    /* truncate off /jexec */
    *(strrchr(java, '/')) = '\0';

    /* truncate off /<ISA> */
    *(strrchr(java, '/')) = '\0';

    /* truncate off /lib */
    *(strrchr(java, '/')) = '\0';
    
    /* append the relative location of java...
     * creating something like <FOO>/jre/bin/java */
    strcat(java, "/bin/java");

    /* Use the absolute path so the launcher can always find <FOO> */
    argv[0] = java;

    execv(java, argv) ;
    perror("execv()");
    exit(1);
}
