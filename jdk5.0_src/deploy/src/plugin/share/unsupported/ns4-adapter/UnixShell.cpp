/* -*- Mode: C; tab-width: 4; -*- */
/******************************************************************************
 * Copyright (c) 1996 Netscape Communications. All rights reserved.
 ******************************************************************************/
/*
 * UnixShell.c
 *
 * Netscape Client Plugin API
 * - Function that need to be implemented by plugin developers
 *
 * This file defines a "Template" plugin that plugin developers can use
 * as the basis for a real plugin.  This shell just provides empty
 * implementations of all functions that the plugin can implement
 * that will be called by Netscape (the NPP_xxx methods defined in 
 * npapi.h). 
 *
 * dp Suresh <dp@netscape.com>
 *
 */

#include <stdio.h>
#include <X11/Intrinsic.h>
#include <unistd.h>
#include <dlfcn.h>

#include "npapi.h"
#include "ns4common.h"
#include "adaptercommon.h"

#include "ICreater.h"
#include "IVersion.h"
#include "IJavaInstance.h"
#include "CJavaInstanceCB.h"
#include "CFDMonitor.h"

/***********************************************************************
 * Instance state information about the plugin.
 *
 * PLUGIN DEVELOPERS:
 *	Use this struct to hold per-instance information that you'll
 *	need in the various functions in this file.
 ***********************************************************************/

typedef struct _PluginInstance
{
    int nothing;
    IJavaInstance * ji;
} PluginInstance;

IEgo * javaService = NULL;

/***********************************************************************
 *
 * Empty implementations of plugin API functions
 *
 * PLUGIN DEVELOPERS:
 *	You will need to implement these functions as required by your
 *	plugin.
 *
 ***********************************************************************/

char*
NPP_GetMIMEDescription(void)
{
    char * mimeTable = NULL;

    ac_createMimeTable(javaService,&mimeTable);

    return(mimeTable);
}

NPError
NPP_GetValue(NPP future, NPPVariable variable, void *value)
{

	NPError err = NPERR_NO_ERROR;

	switch (variable) {
		case NPPVpluginNameString:
			*((char **)value) = "Java Plugin "PLUGIN_VERSION;
			break;
		case NPPVpluginDescriptionString:
			*((char **)value) =
				"Java Plugin for Netscape Navigator";
			break;
		default:
			err = NPERR_GENERIC_ERROR;
	}
	return err;
}


NPError
NPP_Initialize(void)
{
    static int initialized = 0;
    fprintf(stderr,"In NPP_Initialize\n");
    if (javaService != NULL && !initialized ) {
fprintf(stderr,"initializing in NPP_Initialize\n");
       Display * display;
       ICreater * creater;
       NPN_GetValue(NULL,NPNVxDisplay,&display);
       XtAppContext apctx = XtDisplayToApplicationContext(display);
       CFDMonitor * fdm = new CFDMonitor(apctx);
       javaService->QI(ICreater_IID, (void **) &creater);
       creater->setFDMonitor(fdm);
       creater->release();
       initialized = 1;
    }

    return NPERR_NO_ERROR;
}


jref
NPP_GetJavaClass()
{
    return NULL;
}

void
NPP_Shutdown(void)
{
}


NPError 
NPP_New(NPMIMEType pluginType,
	NPP instance,
	uint16 mode,
	int16 argc,
	char* argn[],
	char* argv[],
	NPSavedData* saved)
{
       PluginInstance* This;

        if (instance == NULL)
                return NPERR_INVALID_INSTANCE_ERROR;

        instance->pdata = NPN_MemAlloc(sizeof(PluginInstance));

        This = (PluginInstance*) instance->pdata;

        if (This != NULL) {
            This->ji = NULL;
            CJavaInstanceCB * cb = new CJavaInstanceCB(instance);
            ICreater * creater;
            javaService->QI(ICreater_IID, (void **) &creater);
            creater->createJavaInstance(NULL,argc,
                                 (const char **) argn,
                                 (const char **) argv,(IJavaInstanceCB *) cb,
                                                 &This->ji);
            creater->release();
		
            NPN_GetURLNotify(instance, "javascript:document.location", 
                             NULL, (void*) JA_DOCBASE);
            return NPERR_NO_ERROR;
        }
        else
            return NPERR_OUT_OF_MEMORY_ERROR;
}


NPError 
NPP_Destroy(NPP instance, NPSavedData** save)
{
     fprintf(stderr,"In NPP_Destroy\n");
	PluginInstance* This;

	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;

	This = (PluginInstance*) instance->pdata;

        This->ji->destroy();
        This->ji->release();

	/* PLUGIN DEVELOPERS:
	 *	If desired, call NP_MemAlloc to create a
	 *	NPSavedDate structure containing any state information
	 *	that you want restored if this plugin instance is later
	 *	recreated.
	 */

	if (This != NULL) {
		NPN_MemFree(instance->pdata);
		instance->pdata = NULL;
	}

	return NPERR_NO_ERROR;
}



NPError 
NPP_SetWindow(NPP instance, NPWindow* window)
{
	PluginInstance* This;

	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;

	if (window == NULL)
		return NPERR_NO_ERROR;

	This = (PluginInstance*) instance->pdata;

	/*
	 * PLUGIN DEVELOPERS:
	 *	Before setting window to point to the
	 *	new window, you may wish to compare the new window
	 *	info to the previous window (if any) to note window
	 *	size changes, etc.
	 */
         if (window != NULL) {
             NPSetWindowCallbackStruct * x;
             x = (NPSetWindowCallbackStruct *) window->ws_info;
             XSync(x->display,0);
             This->ji->window((int) window->window,window->width,window->height,
                          window->x,window->y);
         } else {
             This->ji->window(0,0,0,0,0);
         }

	return NPERR_NO_ERROR;
}


NPError 
NPP_NewStream(NPP instance,
			  NPMIMEType type,
			  NPStream *stream, 
			  NPBool seekable,
			  uint16 *stype)
{
	NPByteRange range;
	PluginInstance* This;

	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;

	This = (PluginInstance*) instance->pdata;

	return NPERR_NO_ERROR;
}


/* PLUGIN DEVELOPERS:
 *	These next 2 functions are directly relevant in a plug-in which
 *	handles the data in a streaming manner. If you want zero bytes
 *	because no buffer space is YET available, return 0. As long as
 *	the stream has not been written to the plugin, Navigator will
 *	continue trying to send bytes.  If the plugin doesn't want them,
 *	just return some large number from NPP_WriteReady(), and
 *	ignore them in NPP_Write().  For a NP_ASFILE stream, they are
 *	still called but can safely be ignored using this strategy.
 */

int32 STREAMBUFSIZE = 0X0FFFFFFF; /* If we are reading from a file in NPAsFile
				   * mode so we can take any size stream in our
				   * write call (since we ignore it) */

int32 
NPP_WriteReady(NPP instance, NPStream *stream)
{
	PluginInstance* This;
	if (instance != NULL)
		This = (PluginInstance*) instance->pdata;

	return STREAMBUFSIZE;
}


int32 
NPP_Write(NPP instance, NPStream *stream, int32 offset, int32 len, void *buffer)
{
	if (instance != NULL)
	{
	    PluginInstance* This = (PluginInstance*) instance->pdata;
            char * foo = (char *) NPN_MemAlloc(len + 1);
            memcpy((void *) foo, buffer, len);
            foo[len] = '\0';
            if (((int) stream->notifyData) == JA_DOCBASE) {
                This->ji->docbase(foo);
            } else if (((int) stream->notifyData) == JA_JSR) {
                This->ji->javascriptReply(foo);
            }
            NPN_MemFree(foo);
	}

	return len;		/* The number of bytes accepted */
}


NPError 
NPP_DestroyStream(NPP instance, NPStream *stream, NPError reason)
{
	PluginInstance* This;

	if (instance == NULL)
		return NPERR_INVALID_INSTANCE_ERROR;
	This = (PluginInstance*) instance->pdata;

	return NPERR_NO_ERROR;
}


void 
NPP_StreamAsFile(NPP instance, NPStream *stream, const char* fname)
{
	PluginInstance* This;
	if (instance != NULL)
		This = (PluginInstance*) instance->pdata;
}


void 
NPP_Print(NPP instance, NPPrint* printInfo)
{
	if(printInfo == NULL)
		return;

	if (instance != NULL) {
		PluginInstance* This = (PluginInstance*) instance->pdata;
	
		if (printInfo->mode == NP_FULL) {
		    /*
		     * PLUGIN DEVELOPERS:
		     *	If your plugin would like to take over
		     *	printing completely when it is in full-screen mode,
		     *	set printInfo->pluginPrinted to TRUE and print your
		     *	plugin as you see fit.  If your plugin wants Netscape
		     *	to handle printing in this case, set
		     *	printInfo->pluginPrinted to FALSE (the default) and
		     *	do nothing.  If you do want to handle printing
		     *	yourself, printOne is true if the print button
		     *	(as opposed to the print menu) was clicked.
		     *	On the Macintosh, platformPrint is a THPrint; on
		     *	Windows, platformPrint is a structure
		     *	(defined in npapi.h) containing the printer name, port,
		     *	etc.
		     */

			void* platformPrint =
				printInfo->print.fullPrint.platformPrint;
			NPBool printOne =
				printInfo->print.fullPrint.printOne;
			
			/* Do the default*/
			printInfo->print.fullPrint.pluginPrinted = FALSE;
		}
		else {	/* If not fullscreen, we must be embedded */
		    /*
		     * PLUGIN DEVELOPERS:
		     *	If your plugin is embedded, or is full-screen
		     *	but you returned false in pluginPrinted above, NPP_Print
		     *	will be called with mode == NP_EMBED.  The NPWindow
		     *	in the printInfo gives the location and dimensions of
		     *	the embedded plugin on the printed page.  On the
		     *	Macintosh, platformPrint is the printer port; on
		     *	Windows, platformPrint is the handle to the printing
		     *	device context.
		     */

			NPWindow* printWindow =
				&(printInfo->print.embedPrint.window);
			void* platformPrint =
				printInfo->print.embedPrint.platformPrint;
		}
	}
}

void NPP_URLNotify(NPP instance, const char* url, NPReason reason, void* notifyData) {

    PluginInstance* This;

    if (instance == NULL)
            return;

    This = (PluginInstance*) instance->pdata;

    int x = (int) notifyData;
    if (x == JA_JSR) {
        This->ji->javascriptReply(NULL);
    }

}
