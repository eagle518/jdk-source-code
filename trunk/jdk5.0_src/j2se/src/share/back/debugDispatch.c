/*
 * @(#)debugDispatch.c	1.34 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "util.h"
#include "transport.h"
#include "debugDispatch.h"
#include "VirtualMachineImpl.h"
#include "ReferenceTypeImpl.h"
#include "ClassTypeImpl.h"
#include "ArrayTypeImpl.h"
#include "FieldImpl.h"
#include "MethodImpl.h"
#include "ObjectReferenceImpl.h"
#include "StringReferenceImpl.h"
#include "ThreadReferenceImpl.h"
#include "ThreadGroupReferenceImpl.h"
#include "ClassLoaderReferenceImpl.h"
#include "ClassObjectReferenceImpl.h"
#include "ArrayReferenceImpl.h"
#include "EventRequestImpl.h"
#include "StackFrameImpl.h"
#include "typedefs.h"

static void **l1Array;

void 
debugDispatch_initialize(void)
{
    /*
     * Create the level-one (CommandSet) dispatch table.
     * Zero the table so that unknown CommandSets do not
     * cause random errors.
     */
    l1Array = jvmtiAllocate((JDWP_HIGHEST_COMMAND_SET+1) * sizeof(void *));

    if (l1Array == NULL) {
	EXIT_ERROR(JVMTI_ERROR_OUT_OF_MEMORY,"command set array");
    }

    (void)memset(l1Array, 0, (JDWP_HIGHEST_COMMAND_SET+1) * sizeof(void *));

    /*
     * Create the level-two (Command) dispatch tables to the
     * corresponding slots in the CommandSet dispatch table..
     */
    l1Array[JDWP_COMMAND_SET(VirtualMachine)] = (void *)VirtualMachine_Cmds;
    l1Array[JDWP_COMMAND_SET(ReferenceType)] = (void *)ReferenceType_Cmds;
    l1Array[JDWP_COMMAND_SET(ClassType)] = (void *)ClassType_Cmds;
    l1Array[JDWP_COMMAND_SET(ArrayType)] = (void *)ArrayType_Cmds;

    l1Array[JDWP_COMMAND_SET(Field)] = (void *)Field_Cmds;
    l1Array[JDWP_COMMAND_SET(Method)] = (void *)Method_Cmds;
    l1Array[JDWP_COMMAND_SET(ObjectReference)] = (void *)ObjectReference_Cmds;
    l1Array[JDWP_COMMAND_SET(StringReference)] = (void *)StringReference_Cmds;
    l1Array[JDWP_COMMAND_SET(ThreadReference)] = (void *)ThreadReference_Cmds;
    l1Array[JDWP_COMMAND_SET(ThreadGroupReference)] = (void *)ThreadGroupReference_Cmds;
    l1Array[JDWP_COMMAND_SET(ClassLoaderReference)] = (void *)ClassLoaderReference_Cmds;
    l1Array[JDWP_COMMAND_SET(ArrayReference)] = (void *)ArrayReference_Cmds;
    l1Array[JDWP_COMMAND_SET(EventRequest)] = (void *)EventRequest_Cmds;
    l1Array[JDWP_COMMAND_SET(StackFrame)] = (void *)StackFrame_Cmds;
    l1Array[JDWP_COMMAND_SET(ClassObjectReference)] = (void *)ClassObjectReference_Cmds;
}

void
debugDispatch_reset(void)
{
}

CommandHandler 
debugDispatch_getHandler(int cmdSet, int cmd) 
{
    void **l2Array;

    if (cmdSet > JDWP_HIGHEST_COMMAND_SET) {
        return NULL;
    }

    l2Array = (void **)l1Array[cmdSet];

    /*
     * If there is no such CommandSet or the Command
     * is greater than the nummber of commands (the first
     * element) in the CommandSet, indicate this is invalid.
     */
    /*LINTED*/
    if (l2Array == NULL || cmd > (int)(intptr_t)(void*)l2Array[0]) {
        return NULL;
    }

    return (CommandHandler)l2Array[cmd];
}
