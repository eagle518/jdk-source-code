/*
 * @(#)Trace.c	1.8 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "Trace.h"

static int j2dTraceLevel = J2D_TRACE_INVALID;
static FILE *j2dTraceFile = NULL; 

JNIEXPORT void JNICALL
J2dTraceImpl(int level, jboolean cr, const char *string, ...)
{
    va_list args;
    if (j2dTraceLevel < J2D_TRACE_OFF) {
	J2dTraceInit();
    }
    if (level <= j2dTraceLevel) {
        if (cr) {
            switch (level) {
            case J2D_TRACE_ERROR:
                fprintf(j2dTraceFile, "[E] ");
                break;
            case J2D_TRACE_WARNING:
                fprintf(j2dTraceFile, "[W] ");
                break;
            case J2D_TRACE_INFO:
                fprintf(j2dTraceFile, "[I] ");
                break;
            case J2D_TRACE_VERBOSE:
                fprintf(j2dTraceFile, "[V] ");
                break;
            case J2D_TRACE_VERBOSE2:
                fprintf(j2dTraceFile, "[X] ");
                break;
            default:
                break;
            }
        }

	va_start(args, string);
	vfprintf(j2dTraceFile, string, args);
	va_end(args);

	if (cr) {
	    fprintf(j2dTraceFile, "\n");
	}
	fflush(j2dTraceFile);
    }
}

JNIEXPORT void JNICALL
J2dTraceInit()
{
    char *j2dTraceLevelString = getenv("J2D_TRACE_LEVEL");
    char *j2dTraceFileName;
    j2dTraceLevel = J2D_TRACE_OFF;
    if (j2dTraceLevelString) {
        int traceLevelTmp = -1;
        int args = sscanf(j2dTraceLevelString, "%d", &traceLevelTmp);
        if (args > 0 && 
            traceLevelTmp > J2D_TRACE_INVALID && 
            traceLevelTmp < J2D_TRACE_MAX) 
	{
            j2dTraceLevel = traceLevelTmp;
        }
    }
    j2dTraceFileName = getenv("J2D_TRACE_FILE");
    if (j2dTraceFileName) {
	j2dTraceFile = fopen(j2dTraceFileName, "w");
	if (!j2dTraceFile) {
	    printf("[E]: Error opening trace file %s\n", j2dTraceFileName);
	}
    }
    if (!j2dTraceFile) {
	j2dTraceFile = stdout;
    }
}
