#!/usr/sbin/dtrace -Zs

/*
* @(#)hotspot_jni_calls_tree.d	1.2 10/04/02
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
 *   1. hotspot_jni_calls_tree.d -c "java ..."
 *   2. hotspot_jni_calls_tree.d -p JAVA_PID
 *
 * The script prints tree of JNI method calls.
 *
 */

#pragma D option quiet
#pragma D option destructive
#pragma D option defaultargs
#pragma D option bufsize=16m
#pragma D option aggrate=100ms


self int indent;

:::BEGIN
{
    printf("BEGIN hotspot_jni tracing\n");
}


hotspot_jni$target:::*
/!self->indent/
{
    self->indent = 11;
}

hotspot_jni$target:::*-entry
{
    self->indent++;
    printf("%d %*s -> %s\n", curcpu->cpu_id, self->indent, "", probename);
}


hotspot_jni$target:::*-return
{
    printf("%d %*s <- %s\n", curcpu->cpu_id, self->indent, "", probename);
    self->indent--;
}

:::END
{
   printf("\nEND hotspot_jni tracing.\n");

}

syscall::rexit:entry,
syscall::exit:entry
/pid == $target/
{
   exit(0);
}
