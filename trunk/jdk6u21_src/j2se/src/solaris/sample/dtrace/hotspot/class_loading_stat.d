#!/usr/sbin/dtrace -Zs

/*
* @(#)class_loading_stat.d	1.2 10/04/02
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
 * Usage:
 *    1. class_loading_stat.d -c "java ..." INTERVAL_SECS
 *    2. class_loading_stat.d -p JAVA_PID INTERVAL_SECS
 *
 * This script collects statistics about loaded and unloaded Java classes
 * and dump current state to stdout every INTERVAL_SECS seconds.  If
 * INTERVAL_SECS is not set then 10 seconds interval is used.
 *
 */

#pragma D option quiet
#pragma D option destructive
#pragma D option defaultargs
#pragma D option aggrate=100ms


self char *str_ptr;
self string class_name;
self string package_name;

int INTERVAL_SECS;

:::BEGIN
{
    SAMPLE_NAME = "hotspot class loadin tracing";

    INTERVAL_SECS = $1 ? $1 : 10;
    SAMPLING_TIME = timestamp + INTERVAL_SECS * 1000000000ull;

    LOADED_CLASSES_CNT = 0;
    UNLOADED_CLASSES_CNT = 0;

    LINE_SEP =
    "------------------------------------------------------------------------";

    printf("BEGIN %s\n\n", SAMPLE_NAME);
}

/*
 * hotspot:::class-loaded, hotspot:::class-unloaded probe arguments:
 *  arg0: char*,        class name passed as mUTF8 string
 *  arg1: uintptr_t,    class name length
 *  arg2: void*,        class loader ID, which is unique identifier for
 *                      a class loader in the VM.
 *  arg3: uintptr_t,    class is shared or not
 */
hotspot$target:::class-loaded
{
    LOADED_CLASSES_CNT ++;

    self->str_ptr = (char*) copyin(arg0, arg1+1);
    self->str_ptr[arg1] = '\0';
    self->class_name = (string) self->str_ptr;

    self->package_name = dirname(self->class_name);

    @classes_loaded[self->package_name] = count();
}

hotspot$target:::class-unloaded
{
    UNLOADED_CLASSES_CNT ++;

    self->str_ptr = (char*) copyin(arg0, arg1+1);
    self->str_ptr[arg1] = '\0';
    self->class_name = (string) self->str_ptr;

    self->package_name = dirname(self->class_name);

    @classes_unloaded[self->package_name] = count();
}


tick-1sec
/timestamp > SAMPLING_TIME/
{
    printf("%s\n", LINE_SEP);
    printf("%Y\n", walltimestamp);
    printf("%s\n", LINE_SEP);

    printf("Loaded classes by package:\n");
    printa("%10@d %s\n", @classes_loaded);

    printf("\n");
    printf("Unloaded classes by package:\n");
    printa("%10@d %s\n", @classes_unloaded);

    printf("\n");
    printf("Number of loaded classes: %10d\n", LOADED_CLASSES_CNT);
    printf("Number of unloaded classes: %10d\n", UNLOADED_CLASSES_CNT);

    SAMPLING_TIME = timestamp + INTERVAL_SECS * 1000000000ull;
}


:::END
{
    printf("%s\n", LINE_SEP);
    printf("%Y\n", walltimestamp);
    printf("%s\n", LINE_SEP);

    printf("Loaded classes by package:\n");
    printa("%10@d %s\n", @classes_loaded);

    printf("\n");
    printf("Unloaded classes by package:\n");
    printa("%10@d %s\n", @classes_unloaded);

    printf("\n");
    printf("Number of loaded classes: %10d\n", LOADED_CLASSES_CNT);
    printf("Number of unloaded classes: %10d\n", UNLOADED_CLASSES_CNT);

    printf("\nEND of %s\n", SAMPLE_NAME);
}

syscall::rexit:entry,
syscall::exit:entry
/pid == $target/
{
   exit(0);
}
