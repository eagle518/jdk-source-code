#!/usr/sbin/dtrace -s

/*
* @(#)dtrace_helper.d	1.2 10/04/02
*
* Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* -Redistribution of source code must retain the above copyright notice, this
*  list of conditions and the following disclaimer.
*
* -Redistribution in binary form must reproduce the above copyright notice,
*  this list of conditions and the following disclaimer in the documentation
*  and/or other materials provided with the distribution.
*
* Neither the name of Oracle or the names of contributors may
* be used to endorse or promote products derived from this software without
* specific prior written permission.
*
* This software is provided "AS IS," without a warranty of any kind. ALL
* EXPRESS OR IMPLIED CONDITIONS, REPRESENTATIONS AND WARRANTIES, INCLUDING
* ANY IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
* OR NON-INFRINGEMENT, ARE HEREBY EXCLUDED. SUN MICROSYSTEMS, INC. ("SUN")
* AND ITS LICENSORS SHALL NOT BE LIABLE FOR ANY DAMAGES SUFFERED BY LICENSEE
* AS A RESULT OF USING, MODIFYING OR DISTRIBUTING THIS SOFTWARE OR ITS
* DERIVATIVES. IN NO EVENT WILL SUN OR ITS LICENSORS BE LIABLE FOR ANY LOST
* REVENUE, PROFIT OR DATA, OR FOR DIRECT, INDIRECT, SPECIAL, CONSEQUENTIAL,
* INCIDENTAL OR PUNITIVE DAMAGES, HOWEVER CAUSED AND REGARDLESS OF THE THEORY
* OF LIABILITY, ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE,
* EVEN IF SUN HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
*
* You acknowledge that this software is not designed, licensed or intended
* for use in the design, construction, operation or maintenance of any
* nuclear facility.
*/

/*
 * Description:
 * dtrace -c option launches the command specified in the -c argument and
 * starts tracing the process. Typically, you can run a D script and trace
 * a Java application as follows:
 *    dscript.d -Zc "java HelloWorld"
 *
 * The -Z option is needed to permit probe descriptions that match zero
 * probes because Hotspot probes definitions are located in libjvm.so which
 * has not been yet loaded and thus can't be enabled until the application
 * is started.
 *
 * Straightforward attempt to run D script may fail, e.g.: 
 *    dscript.d -c "java HelloWorld" 
 *    "probe description hotspotPID:::probename does not match any probes"
 *
 * This is because DTrace tries to enable probes before libjvm.so is loaded.
 * The -Z option requires Solaris patch 118822-30 installed on your system.
 *
 * In case you don't have this Solaris patch use dtrace_helper.d script.
 * This script waits until the Hotspot DTrace probes are loaded and then
 * stops the Java process (passed as '-c' options). After the process is
 * stopped, another D script (passed as first argument) is called to do real
 * trace of Java process.
 *
 * Usage example:
 *   dtrace_helper.d -c "java ..." ../hotspot/class_loading_stat.d
 */

#pragma D option quiet
#pragma D option destructive


pid$target::dlopen:entry
{
    self->filename = arg0;
}


pid$target::dlopen:return
/self->filename && basename(copyinstr(self->filename)) == "libjvm.so"/
{
    printf(" loaded %s\n", basename(copyinstr(self->filename)));
    self->filename = 0;

    stop();
    printf(" stopped java process with pid=%d \n", $target);

    printf(" run: %s -p %d &", $1, $target);
    system("(%s -p %d) &", $1, $target);
    exit(0);
}
