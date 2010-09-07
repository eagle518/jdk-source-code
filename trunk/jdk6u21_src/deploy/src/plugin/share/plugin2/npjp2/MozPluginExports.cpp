/*
 * @(#)MozPluginExports.cpp	1.12 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


/* 
 * This file contains the entry points from Mozilla/Firefox browser.
 * It serves the interfaces between the browser and the Java Plug-in.
 */

#include "StdAfx.h"

#include "npapi.h"
#include "npfunctions.h"
#include "npruntime.h"

#include "MozExports.h"
#include "MozPluginInstance.h"

#ifdef _WINDOWS
#define CALL_CONV WINAPI
#define NP_EXPORT
#else
#define CALL_CONV
#undef  NP_EXPORT
#define NP_EXPORT
#endif

// Global data

// Browser side function table.
NPNetscapeFuncs*    g_pMozillaFuncs = NULL;
// Quick indication of whether NPN_GetValueForURL, NPN_SetValueForURL
// and NPN_GetAuthenticationInfo are available
bool g_haveCookieAndProxyNPAPIs = false;

#ifndef HIBYTE
#define HIBYTE(x)   ((((u_int)(x)) & 0xff00)>>8)
#endif

#ifndef LOBYTE
#define LOBYTE(x)   (((u_int)(x)) & 0x00ff)
#endif

#define RETURN_ERROR_IF_NULL(arg, err) \
  if (arg == NULL) return err;

// Method forward declarations.
NPError NPP_Initialize(void);
// NP_GetEntryPoints is used on both Windows and Mac
#ifndef XP_UNIX
NPError CALL_CONV NP_EXPORT NP_GetEntryPoints(NPPluginFuncs* pFuncs);
#endif

NPError getEntryPointsImpl(NPPluginFuncs* pFuncs);

#ifdef XP_UNIX
// FIXME: need to better figure out how to conditionally compile this
// for X11 platforms only; see below
static bool useXEmbed() {
    return (::getenv("JPI_PLUGIN2_NO_XEMBED") == NULL);
}

#endif // ifdef XP_UNIX

// Called by Mozilla plugin code immediately after the plugin library is loaded
NPError CALL_CONV NP_EXPORT
#ifdef XP_UNIX
NP_Initialize(NPNetscapeFuncs* pFuncs, NPPluginFuncs* pluginFuncs) {
#else
NP_Initialize(NPNetscapeFuncs* pFuncs) {
#endif
  
  RETURN_ERROR_IF_NULL(pFuncs, NPERR_INVALID_FUNCTABLE_ERROR);
  
  // if the plugin's major ver level is lower than the Navigator's,
  // then they are incompatible, and should return an error 
  if(HIBYTE(pFuncs->version) > NP_VERSION_MAJOR) {
    return NPERR_INCOMPATIBLE_VERSION_ERROR;
  }
  
  // We now require the minor version to support NPN_PluginThreadAsyncCall on all platforms
  if (LOBYTE(pFuncs->version) < NPVERS_HAS_PLUGIN_THREAD_ASYNC_CALL) {
#ifdef _WINDOWS
      // Try to provide slightly better information to the user
      ::MessageBox(NULL,
                   "The new Java Plug-In requires a recent version of\n"
                   "the Firefox browser (Firefox 3 or later)\n",
                   NULL,
                   MB_OK | MB_ICONSTOP);
#endif
      return NPERR_INCOMPATIBLE_VERSION_ERROR;
  }

  // save it for future referece
  g_pMozillaFuncs = pFuncs;

  // Check to see whether we should use the new NPAPIs for cookie,
  // proxy and browser authentication support
  if (LOBYTE(pFuncs->version) >= NPVERS_HAS_URL_AND_AUTH_INFO) {
      g_haveCookieAndProxyNPAPIs = true;
  }

#ifdef XP_UNIX
  NPError error = getEntryPointsImpl(pluginFuncs);
  if (error != NPERR_NO_ERROR)
      return error;

  // FIXME: is this #ifdef sufficient to compile this code for all X11 platforms?
  // Would like a more "positive" preprocessor check like #ifdef X_PROTOCOL
  if (useXEmbed()) {
      NPBool supportsXEmbed = 0;
      NPError ret = MozNPN_GetValue(NULL, NPNVSupportsXEmbedBool, &supportsXEmbed);
      if (ret != NPERR_NO_ERROR || !supportsXEmbed) {
          // Browser doesn't support XEmbed, but we need it
          return NPERR_INCOMPATIBLE_VERSION_ERROR;
      }
  }

#endif // ifdef XP_UNIX

  // NPP_Initialize is a standard (cross-platform) initialize function.
  return NPP_Initialize();
}

// The browser calls this function once after the last instance of the 
// plug-in is destroyed, before unloading the plug-in library itself. 
NPError CALL_CONV NP_EXPORT NP_Shutdown() {

  // Do nothing.

  return NPERR_NO_ERROR;
}

#ifdef XP_UNIX

// pluginversion.h is auto generated during the build process to contain the 
// unique mimetype etc.
#include "pluginversion.h"

// On UNIX platforms this is how we export our MIME description.
// Note that the addition of the x-java-vm-npruntime MIME type is a
// private contract between Mozilla and Sun in order to detect the
// presence of an NPRuntime-enabled Java Plug-In. The main reason this
// is needed is to tell the browser early in its startup sequence that
// it can use a new, alternate code path to support the Packages.* and
// java.* JavaScript keywords rather than the old OJI code path. See
// also version.rc for the Windows port. This MIME type is not exposed
// by the browser and must be the last one in the list.
char* NP_GetMIMEDescription() {
      return const_cast<char*>
          ("application/x-java-vm::Java&#153 Plug-in;"
           PLUGIN_MIMETABLE 
           ";application/x-java-vm-npruntime::Java&#153 Plug-in"
          );
}

NPError NP_GetValue(void * future, NPPVariable variable, void *ret_value) {
    switch (variable) {

        case NPPVpluginNameString: {
	    *((char **)ret_value) = "Java(TM) Plug-in "PLUGIN_VERSION;
            return NPERR_NO_ERROR;
        }

        case NPPVpluginDescriptionString: {
	    *((char **)ret_value) =
	      "The next generation <a href=\"http://java.sun.com\">Java</a> plug-in for Mozilla browsers.";
	    return NPERR_NO_ERROR;
        }

        default:
            return NPERR_GENERIC_ERROR;
    }
}
#endif // ifdef XP_UNIX

NPError NPP_Initialize() {
  return NPERR_NO_ERROR;
}


/*************************************************************************
 *                       Plugin provided methods                         *
 *************************************************************************/
NPError NPP_New(NPMIMEType pluginType, NPP instance,
		uint16_t mode, int16_t argc, char* argn[],
		char* argv[], NPSavedData* saved)
{
  // Prevent the browser from unloading our plugin. The main reason
  // for this is that the Java HotSpot VM doesn't currently support
  // being unloaded and reloaded.
  NPBool value = 1;
  MozNPN_SetValue(instance, NPPVpluginKeepLibraryInMemory, (void*) value);

  // Make 100% sure we're initialized before continuing; putting this
  // call here is a workaround for incorrect re-entrance to this code
  // caused by the error dialogs raised by Java Kernel's preJVMStart()
  if (!MozPluginInstance::GlobalInitialize()) {
    return NPERR_GENERIC_ERROR;
  }

#ifdef XP_MACOSX
  // On the Mac, we need to negotiate the drawing model to be
  // CoreGraphics instead of QuickDraw; later, we will need to also
  // negotiate the event model to be Cocoa, which will change how we
  // obtain the native window handle
  //
  // NOTE that if this code is changed, the code in NPP_SetWindow
  // below (as well as Java-side code) must change as well
  //
  // Check if the browser supports the CoreGraphics drawing model
  NPBool supportsCoreGraphics = FALSE;
  NPError err = MozNPN_GetValue(instance,
                             NPNVsupportsCoreGraphicsBool,
                             &supportsCoreGraphics);
  if (err != NPERR_NO_ERROR || !supportsCoreGraphics) {
      return NPERR_INCOMPATIBLE_VERSION_ERROR;
  }

  // Set the drawing model
  err = MozNPN_SetValue(instance,
                     NPPVpluginDrawingModel,
                     (void*)NPDrawingModelCoreGraphics);
  if (err != NPERR_NO_ERROR) {
      return NPERR_INCOMPATIBLE_VERSION_ERROR;
  }
#endif

  // Note:'mode' seems no long used anywhere.
  MozPluginInstance* pMozPluginInstance = 
    new MozPluginInstance(instance,
                          (const char*)pluginType, 
                          (short)argc, argn, argv);

  if (pMozPluginInstance == NULL) {
    return NPERR_OUT_OF_MEMORY_ERROR;
  }

  instance->pdata = (void*) pMozPluginInstance;

  if (saved != NULL) {
    MozNPN_MemFree(saved);
  }
  return NPERR_NO_ERROR;
}

NPError NPP_SetWindow(NPP instance, NPWindow* window)
{  
  MozPluginInstance* pMozPluginInstance = (MozPluginInstance*)instance->pdata;

  if (pMozPluginInstance != NULL) {
      void* win = window->window;
#ifdef XP_MACOSX
      // NOTE dependence on code in NPP_New above
      if (win != NULL) {
          win = ((NP_CGContext*) win)->window;
      }
#endif
      if (pMozPluginInstance->SetWindow(win, window->x, window->y,
                                        window->width, window->height,
                                        window->clipRect.top,
                                        window->clipRect.left,
                                        window->clipRect.bottom,
                                        window->clipRect.right)) {
          return NPERR_NO_ERROR;
      }
  }
  
  return NPERR_GENERIC_ERROR;
}

NPError NPP_Destroy(NPP instance, NPSavedData** save)
{
  MozPluginInstance* pMozPluginInstance = (MozPluginInstance*)instance->pdata;

  if (pMozPluginInstance != NULL) {
    pMozPluginInstance->Destroy();
    delete pMozPluginInstance;
  }
  return NPERR_NO_ERROR;
}


NPError NPP_NewStream(NPP instance, NPMIMEType type,
		      NPStream* stream, NPBool seekable,
    		      uint16_t* stype)
{
  return NPERR_GENERIC_ERROR;
}

		      
NPError NPP_DestroyStream(NPP instance, NPStream* stream,
			  NPReason reason)
{
  return NPERR_GENERIC_ERROR;
}


int32_t NPP_WriteReady(NPP instance, NPStream* stream)
{
  return -1;	    // or 8192 ??
}


int32_t NPP_Write(NPP instance, NPStream* stream, int32_t offset,
	        int32_t len, void* buffer)
{
  return -1;
}

void NPP_StreamAsFile(NPP instance, NPStream* stream,
		      const char* fname)
{
}


void NPP_Print(NPP instance, NPPrint* platformPrint)
{
    MozPluginInstance* pMozPluginInstance = (MozPluginInstance*)instance->pdata;

    if (pMozPluginInstance != NULL)
        pMozPluginInstance->Print(platformPrint);
}

void NPP_URLNotify(NPP instance, const char* url,
		  NPReason reason, void* notifyData)
{
}

NPError NPP_GetValue(NPP instance, NPPVariable variable, void *ret_value) {
    switch (variable) {
#ifdef XP_UNIX
        case NPPVpluginNeedsXEmbed: {
            // X11 platforms only, but the code compiles on all platforms
            // NOTE that this code needs to match the logic up in MozillaPlugin.java
            *((NPBool *) ret_value) = (NPBool) useXEmbed();
            return NPERR_NO_ERROR;
        }
#endif // ifdef XP_UNIX

        case NPPVpluginScriptableNPObject: {
            MozPluginInstance* plugin = (MozPluginInstance*) instance->pdata;
            // This should not happen, but can happen in rare circumstances with
            // incorrect re-entrance into this code caused by error dialogs displayed
            // by the Java Kernel's preJVMStart()
            if (plugin == NULL) {
                return NPERR_GENERIC_ERROR;
            }
            *((NPObject**) ret_value) = plugin->getAppletNPObject();
            return NPERR_NO_ERROR;
        }

        default:
            // FIXME: may need to implement more of these, including the name of the plugin, etc.
            return NPERR_GENERIC_ERROR;
    }
}

NPError NPP_SetValue(NPP instance, NPNVariable variable, void *ret_value) {
    return NPERR_NO_ERROR;
}

#ifdef XP_MACOSX
int16 NPP_HandleEvent(NPP instance, void* event) {
    EventRecord* ev = (EventRecord*) event;

    if (ev->what == activateEvt && (ev->modifiers & activeFlag)) {
        // Update the applet's location and bring it to the front
        MozPluginInstance* plugin = (MozPluginInstance*) instance->pdata;
        plugin->updateLocationAndClip();
        return 1;
    }

    return 0;
}
#endif

#ifndef XP_UNIX
// Called by Mozilla plugin code once during initialization phase.
NPError CALL_CONV NP_EXPORT
NP_GetEntryPoints(NPPluginFuncs* pFuncs) {
  return getEntryPointsImpl(pFuncs);
}
#endif // ifndef XP_UNIX

NPError getEntryPointsImpl(NPPluginFuncs* pFuncs) {
  RETURN_ERROR_IF_NULL(pFuncs, NPERR_INVALID_FUNCTABLE_ERROR);
  
  int expectedSize = sizeof(NPPluginFuncs);
  if (pFuncs->size < expectedSize) {
      // We're likely running on an earlier version of the browser than we were compiled against
      return NPERR_INVALID_FUNCTABLE_ERROR;
  }

  // Register callback functions provided by the plugin.
  pFuncs->version       = (NP_VERSION_MAJOR << 8) | NP_VERSION_MINOR;
  pFuncs->newp          = &NPP_New;
  pFuncs->destroy       = &NPP_Destroy;
  pFuncs->setwindow     = &NPP_SetWindow;
  pFuncs->newstream     = &NPP_NewStream;
  pFuncs->destroystream = &NPP_DestroyStream;
  pFuncs->asfile        = &NPP_StreamAsFile;
  pFuncs->writeready    = &NPP_WriteReady;
  pFuncs->write         = &NPP_Write;
  pFuncs->print         = &NPP_Print;
#ifdef XP_MACOSX
  pFuncs->event         = &NPP_HandleEvent;
#else
  pFuncs->event         = 0;           //reserved 
#endif

  // Note that we're assuming that if the NPPluginFuncs is the right size,
  // that these fields are present and will be observed by the browser
  pFuncs->urlnotify     = &NPP_URLNotify;
  pFuncs->getvalue      = &NPP_GetValue;
  pFuncs->setvalue      = &NPP_SetValue;

  return NPERR_NO_ERROR;
}
