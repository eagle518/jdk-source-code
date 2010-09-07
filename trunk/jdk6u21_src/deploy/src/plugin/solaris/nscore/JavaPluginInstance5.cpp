/*
 * @(#)JavaPluginInstance5.cpp	1.60 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


/* 
 * A Plugin instance is a single instantiation of the plugin, with its
 * own window and its own corresponding MotifAppletViewer. It will
 * share a classloader with other plugin instances that have the same
 * document base 
 */
#include "commonhdr.h"
#include "commandprotocol.h"
#include "workerprotocol.h"
#include "urlprotocol.h"
#include <X11/Xlib.h>
#include "CWriteBuffer.h"
#include "remotejni.h"
#include "IPluginInstance.h"
#include "IPluginStreamInfo.h"
#include "IPluginStream.h"
#include "IJVMPluginInstance.h"
#include "IPluginTagInfo2.h"
#include "ILiveconnect.h"
#include "IUniqueIdentifier.h"
#include "JavaPluginInstance5.h"
#include "JavaPluginFactory5.h"
#include "CJavaStream.h"
#include "PluginPrint.h"
#include "plugin_defs.h"

JD_DEFINE_IID(jPluginInstanceIID, IPLUGININSTANCE_IID);
JD_DEFINE_IID(jIUniqueIdentifierIID, UNIQUE_IDENTIFIER_IID);
JD_DEFINE_IID(jJVMPluginInstanceIID, IJVMPLUGININSTANCE_IID);
JD_DEFINE_IID(jSupportsIID, ISUPPORTS_IID);
JD_DEFINE_IID(jPluginTagInfoIID, IPLUGINTAGINFO_IID);
JD_DEFINE_IID(jPluginTagInfo2IID, IPLUGINTAGINFO2_IID);
JD_DEFINE_IID(jILiveconnectIID, ILIVECONNECT_IID);
JD_DEFINE_CID(jCLiveconnectCID, CLIVECONNECT_CID);

JD_IMPL_ISUPPORTS4(JavaPluginInstance5, 
		   IJVMPluginInstance, 
		   IPluginInstance, 
		   INS4PluginInstance,
		   IUniqueIdentifier);

long JavaPluginInstance5::s_uniqueId = 1;
static const char *s_szDocument_locator_url = "javascript:document.location";
/*
 * Create a new instance - should only be called by the factory 
 */
JavaPluginInstance5::JavaPluginInstance5(JavaPluginFactory5 *plugin) {
    trace("JavaPluginInstance5:Create\n");
    JD_INIT_REFCNT();
    plugin_number = -1;
    current_request = NULL;
    plugin_factory = plugin;
    instance_peer = NULL;
    window = NULL;
    mIsDestroyed = false;
    m_pLiveconnect = NULL;
    m_status = APPLET_DISPOSE;
    m_destroyPending = false;
}

/*
 * Get a pointer that represents the Applet object (in Java)
 * associated with this plugin instance. Used as an entry point
 * by liveconnect
 */
JD_IMETHODIMP 
JavaPluginInstance5::GetJavaObject(jobject *result) {
  
    trace("JavaPluginInstance5 GetJavaObject %d\n", plugin_number);
    if (result == NULL) 
        return JD_ERROR_NULL_POINTER;

    *result = plugin_factory->GetJavaObjectForInstance(plugin_number);
 
    if (result == NULL) {
        plugin_error("GetJavaObject - could not find applet's java object");
        return JD_ERROR_NULL_POINTER;
    } 
    else {
        return JD_OK;
    }
}

/*
 * Return the value for an instance variable
 */
JDresult
JavaPluginInstance5::GetValue(JDPluginInstanceVariable var, 
			      void* res) {
  JDresult err = JD_OK;
  CWriteBuffer wb;
  int ret = 0;
  JavaVM5* vm = NULL;
  
  switch (var) {
  case JDPluginInstanceVariable_WindowlessBool:
     trace("JavaPluginInstance5:GetValue Windowlessbool\n");
     *(JDBool *)res = JD_FALSE;
     break;
  case JDPluginInstanceVariable_TransparentBool:
    trace("JavaPluginInstance5:Transparent\n");
     *(JDBool *)res = JD_FALSE;
     break;
  case JDPluginInstanceVariable_DoCacheBool:
    trace("JavaPluginInstance5:DoCacheBool\n");
    *(JDBool *)res = JD_FALSE;
    break;
  case JDPluginInstanceVariable_CallSetWindowAfterDestroyBool:
    trace("JavaPluginInstance5:CallSetWindowAfterDestroyBool\n");
    *(JDBool *)res = JD_FALSE;
    break;
  case JDPluginInstanceVariable_NeedsXEmbed:
    trace("JavaPluginInstance5:NeedsXEmbed\n");
	
    vm = plugin_factory->GetJavaVM();
    
    if (vm != NULL) {
        wb.putInt(JAVA_PLUGIN_QUERY_XEMBED);

        vm->SendRequest(wb, TRUE, TRUE, &ret);
	
        if (ret == JAVA_PLUGIN_XEMBED_TRUE)
            *(JDBool *)res = JD_TRUE;
        else
            *(JDBool *)res = JD_FALSE;
    } else {
        err = JD_ERROR_FAILURE;
    }
    break;

  default:
    err = JD_ERROR_NOT_IMPLEMENTED;
  }

  return err;
}

/*
 * Initialization essentially consists of determining the document base.
 * This is different for 3.0, 4.0 and 5.0.
 * In 5.0, query the tag info
 */
JD_IMETHODIMP JavaPluginInstance5::Initialize(IPluginInstancePeer *peer) {

  const char *pluginType;
  JDUint16 argc1 = 0, argc2 = 0, i, j;
  int argc;
  char **keys1, **vals1, **keys2, **vals2, **keys, **vals;
  JDSmartPtr<IPluginTagInfo>  tagInfo;
  JDSmartPtr<IPluginTagInfo2> extTagInfo;
  JDresult res = JD_OK; 
  JDPluginTagType tagType = JDPluginTagType_Unknown;
  const char *docbase = NULL;
  
  trace("JavaPluginInstance5:Initialize\n");
  
  peer->AddRef(); 
  instance_peer = peer;
  
  if (JD_OK != (res = peer->GetMIMEType(&pluginType))){
    plugin_error("Could not get the mime type to init instance");
    return res; 
  }

  peer->QueryInterface(jPluginTagInfoIID, (void **)&tagInfo);

  if (tagInfo != NULL) {
    // Cast to a reference to a constant pointer to an array 
    // of constant strings ! 
    tagInfo->GetAttributes(argc1, (const char* const* &)keys1, 
                           (const char* const* &) vals1);
  }

  // We need to figure out our document base from the taginfo
  // This is for Netscape 6+/Mozilla
  res = peer->QueryInterface(jPluginTagInfo2IID, (void **)&extTagInfo);
  if ((res == JD_OK) && (extTagInfo != NULL)) {
    if (JD_SUCCEEDED(extTagInfo->GetTagType(&tagType))) {
      if (tagType == JDPluginTagType_Applet || 
           tagType == JDPluginTagType_Object) {
          extTagInfo->GetParameters(argc2, (const char* const* &) keys2, 
                              (const char* const* &) vals2);
      }
    }
    
    if (JD_OK != (res = extTagInfo->GetDocumentBase(&docbase))) {
      plugin_error("[%d] Initialize. No docbase?", plugin_number); 
      return res;
    }
    trace("JavaPluginInstance5::Initialize. Docbase %s\n", docbase);
  } else { 
    // For Netscape 4.x browser which does not support IPluginInfoIID2;
    // This is a asynchronous call to request infor from Netscape 4.x browser
    // Browser will call NPP_NewStream and deliver the result to the plug-in
    // Comment from X.Lu
    plugin_factory->GetPluginManager()->GetURL((IPluginInstance*)this, s_szDocument_locator_url,
                      0, (IPluginStreamListener*)JAVA_PLUGIN_DOCBASE_QUERY);
  }
    
  plugin_number = plugin_factory->RegisterInstance(this);
    
  if (plugin_number < 0) 
    return JD_ERROR_OUT_OF_MEMORY;

  /* Lock this instance as "in use" and won't release it until it's ready for destroy*/
  plugin_factory->LockInstanceIndex(plugin_number);

  argc = argc1 + argc2 + 1;
  keys = (char**) malloc(argc*sizeof(char*));
  vals = (char**) malloc(argc*sizeof(char*));

  bool bObjectTagCodebase = false;
  bool bObjectTagClassid = false;
  bool replace_codebase = false;
  const char * s_code = "code";
  const char * s_codebase = "codebase";
  const char * s_classid = "classid";
  const char * s_clsid = "clsid:";

  if (tagType == JDPluginTagType_Object) {
    for (i=0; i < argc1; i++) {
        if (keys1[i] != NULL && vals1[i] != NULL) {
            // Check if CODEBASE exists in OBJECT tag
            if (!strncasecmp(keys1[i],s_codebase,strlen(s_codebase))) {
                bObjectTagCodebase = true;
                continue;
            }
            if (!strncasecmp(keys1[i],s_classid,strlen(s_classid)) &&
                !strncasecmp(vals1[i],s_clsid,strlen(s_clsid))) {
                bObjectTagClassid = true;
                continue;
            }
        }
    }
    replace_codebase = bObjectTagCodebase && bObjectTagClassid;
  }

  // Skim out any null key.
  // Plus fix for 4648112 for object tags:
  // check if we are being called because of classid="clsid"
  // if so, replace all codebase attributes with "."
  // any actual codebases paramaters given later will override
  if(replace_codebase) {
    for (i=0, j=0; i < argc1; i++) {
        if (keys1[i] != NULL) {
            if(!strncasecmp(keys1[i],s_codebase,strlen(s_codebase))) {
                vals[j] = (char*) ".";
            } else {
                vals[j] = vals1[i];
            }
            keys[j] = keys1[i];
            j++;
        }
    }
  } 
  else {
    for (i=0, j=0; i < argc1; i++) {
        if (keys1[i] != NULL) {
            keys[j] = keys1[i];
            vals[j] = vals1[i];
            j++;
        }
    }
  }

  // Adjust the total count for attribute key/value pairs
  // after all null keys had been trimmed out.
  argc1 = j;

  // Append the parameters to the end of the list.
  // Skim out any null value
  for (i=0, j=0; i < argc2; i++) {
      if (keys2[i] != NULL) {
          keys[j+argc1] = keys2[i];
          vals[j+argc1] = vals2[i];
          j++;
      }
  }

  // Adjust the total count for attribute and parameter 
  // key/value pairs after all null keys had been trimmed out.
  argc = argc1 + j + 1;

  // Mark the end of the list with unique identifier
  keys[argc - 1] = (char*)UNIQUE_IDENTIFIER_ID;
  if (docbase) {
    vals[argc - 1] = (char*)docbase;
  } else { // For NS4
    char szBuf[16];
    sprintf(szBuf, "%x", m_uniqueId);
    vals[argc - 1] =  szBuf;
  }

  plugin_factory->CreateApplet(pluginType, plugin_number, 
			       argc, keys, vals);
  free(keys);
  free(vals);

  if (docbase != NULL) {
    SetDocbase(docbase);
    return JD_OK;
  }
  return JD_ERROR_FAILURE;
}

/* 
 * Get the peer NPIPluginInstancePeer for this NPIPluginInstance
 */
JD_IMETHODIMP JavaPluginInstance5::GetPeer(IPluginInstancePeer*
					   *resultingPeer) {
    trace("JavaPluginInstance5:GetPeer\n");
    *resultingPeer = instance_peer;
    if (instance_peer == NULL) 
	plugin_error("Instance Peer is null! %d\n", plugin_number);
    instance_peer->AddRef();
    return JD_OK;
}

/* 
 * Start this instance. Just propagate to the JVM. The start
 * may actually have to wait until other information such as
 * the document base etc. are available. See MotifAppletViewer.java
 * maybeInit()
 */
JD_IMETHODIMP JavaPluginInstance5::Start(void) {
    CWriteBuffer wb;
    trace("JavaPluginInstance5:Start\n");
    wb.putInt(JAVA_PLUGIN_START);
    wb.putInt(plugin_number);
    plugin_factory->SendRequest(wb, FALSE);
    return JD_OK;
}

/* 
 * Stop this instance. Just propagate to the JVM.
 */
JD_IMETHODIMP JavaPluginInstance5::Stop(void) {
    CWriteBuffer wb;
    trace("JavaPluginInstance5:Stop JAVA_PLUGIN_STOP\n");
    wb.putInt(JAVA_PLUGIN_STOP);
    wb.putInt(plugin_number);
    plugin_factory->SendRequest(wb, FALSE);
    return JD_OK;
}

/* 
 * Destroy this instance. Just propagate to the JVM.
 */
JD_IMETHODIMP JavaPluginInstance5::Destroy(void) {
  CWriteBuffer wb;
  trace("JavaPluginInstance5:Destroy\n");
  if(!mIsDestroyed) {
	trace("JavaPluginInstance5:Doing Destroy\n");
	// Tell the VM end to go die
	m_destroyPending = true;
	wb.putInt(JAVA_PLUGIN_DESTROY);
	wb.putInt(plugin_number);
	plugin_factory->SendRequest(wb, TRUE);

	if (plugin_number >= 0)
	  plugin_factory->UnregisterInstance(this);

	if (current_request != NULL) {
	  trace("JavaPluginInstance5:Destroying instance, abruptly terminating request!\n");
	  CWriteBuffer terminateRequest;
	  terminateRequest.putInt(JAVA_PLUGIN_REQUEST_ABRUPTLY_TERMINATED);
	  plugin_factory->SendRequest(terminateRequest, FALSE);
	}
    
	// The object destructor should be called later
	mIsDestroyed = true;
	m_destroyPending = false;
  }
  return JD_OK;
}

/* 
 *  Set the window parameters of this plugin instance. Called at
 *  creation and after a resize. 'window' holds dimension and other 
 * information
 */
JD_IMETHODIMP JavaPluginInstance5::SetWindow(JDPluginWindow* win) {
    JDPluginSetWindowCallbackStruct *ws_info;
    CWriteBuffer wb;

    window = win;

    trace("JavaPluginInstance5:SetWindow JAVA_PLUGIN_WINDOW\n");
    if (win == NULL || win->window == NULL) {
        /*
         * netscape send a setwindow with NULL, we take this as a clean up notification
         * need to use EnterMonitor?
         */
        wb.putInt(JAVA_PLUGIN_WINDOW);
        wb.putInt(plugin_number);
        wb.putInt(0);
        wb.putInt(0);
        wb.putInt(0);
        wb.putInt(0);
        wb.putInt(0);
        wb.putInt(0);
        plugin_factory->SendRequest(wb, TRUE);
        return JD_OK;
    }
    plugin_factory->EnterMonitor("SetWindow");

/* On my sparc box and on some linux boxes the plugin was displaying in its
   own window instead of inside the brower.  This started happening after
   netscape changed from passing a GTK widget to an XID.  As near as we can
   tell some kind of race condition exists between their creating the XID
   and the X server actually thinking it has it.  So I added these lines
   so we can push the info out to the X server before the plugin (VM) is told
   about it.

   As you can see below, if netscape provided a value in ws_info, this code
   would not be needed.  That XSync() would do the job.  We could probably
   delete that code since this code will work in all cases.  Also, we have asked
   netscape to do the XSync() on their side (which is really how it should be).
   If that happens this code and that code could go.

[smk]
*/
    Display *display = (Display *) NULL;

    trace("JavaPluginInstance5:Getting Display in SetWindow()\n");
    plugin_factory->GetPluginManager()->GetValue(JDPluginManagerVariable_XDisplay, &display);

    XSync(display, False);

    // PENDING(mark.lin@eng.sun.com): Sometimes ws_info isn't even 
    // set in Mozilla5!
    ws_info = (JDPluginSetWindowCallbackStruct *) win->ws_info;
    // Flush any pending X requests.  This is necessary because win
    // updates affecting the target win may not yet have reached the
    // server, and the javaplugin child process may get out of step.
    // PENDING(mark.lin@eng.sun.com): Sometimes ws_info isn't even set 
    // in Mozilla5!

    if(ws_info != NULL) {
        XSync(ws_info->display, 0);
    }

    // Getting XEmbed variable
    trace("JavaPluginInstance5:Getting XEmbed support variable in SetWindow()\n");
    JDBool xembed = JD_FALSE;
    plugin_factory->GetPluginManager()->GetValue(JDPluginManagerVariable_SupportsXEmbed, &xembed);

    // forward the request to the server 
    wb.putInt(JAVA_PLUGIN_WINDOW);
    wb.putInt(plugin_number);
    wb.putInt((int)win->window);
    wb.putInt(xembed);
    wb.putInt(win->width);
    wb.putInt(win->height);
    wb.putInt(win->x);
    wb.putInt(win->y);
    plugin_factory->SendRequest(wb, TRUE);
    plugin_factory->ExitMonitor("SetWindow");

    return JD_OK;
}

/*
 * Printing is not supported 
 */
JD_IMETHODIMP JavaPluginInstance5::Print(JDPluginPrint* printInfo) {
  
  UNUSED(printInfo);
  JDPluginEmbedPrint ep;
  ep = printInfo->print.embedPrint;

  if (&(ep.window) == NULL)
    return JD_ERROR_FAILURE;
  
  PluginPrint *printer;
  int result;
  printer = new PluginPrint(this, printInfo);
  result= printer->Print();
  delete printer;
  return result;
  
  //fprintf(stderr,"Print is not implemented yet\n");
  return JD_OK;
}

/* 
 * Destroy a plugin instance. Called after release?
 */
JavaPluginInstance5::~JavaPluginInstance5(void) {
    trace("JavaPluginInstance5:~destructor\n");
    
    if (m_pLiveconnect) {
      IPluginServiceProvider* pProvider = plugin_factory->GetServiceProvider();
      if (pProvider) {
        pProvider->ReleaseService(jCLiveconnectCID, m_pLiveconnect);
      }
    }

    if (plugin_number >= 0 && !mIsDestroyed)
	plugin_factory->UnregisterInstance(this);
    if (instance_peer != NULL)
	instance_peer->Release();

   
    plugin_factory = NULL;
}

/*
 * Return the associated plugin instance factory.
 */
JavaPluginFactory5* JavaPluginInstance5::GetPluginFactory(void){
    trace("JavaPluginInstance5:Getting plugin\n");
    return plugin_factory;
}

/*
 * Create a new stream for the browser to write data into.
 */
JD_IMETHODIMP 
JavaPluginInstance5::NewStream(IPluginStreamInfo* peer,
			       IPluginStream* *result) {

    trace("JavaPluginInstance::NewStream %d\n", plugin_number);

    *result = new CJavaStream(this, peer);

    return JD_OK;
}

/*
 * Set the document base for this instance to the url 'url'.
 * This is called during initialization
 * Send the data to the jvm, which might now have all the information
 * required to start an applet.
 */
JD_IMETHODIMP JavaPluginInstance5::SetDocbase(const char *url) {
    CWriteBuffer wb;
    trace("JavaPluginInstance::SetDocBase %s\n", url);

    wb.putInt(JAVA_PLUGIN_DOCBASE);
    wb.putInt(plugin_number);
    wb.putString(url);
    plugin_factory->SendRequest(wb, FALSE);

    return JD_OK;
}

/*
 * Notification as the result of a query into the browser i.e.
 * the return of a GetURL query.
 * The sequence of events is:
 *     GetURLNotify
 *     NewStream, 
 *     A sequence of writes to the stream
 *     Notify the instance that the writes are done
 * The notification happens *after* all writes to the stream
 * have completed.
 */
JD_IMETHODIMP 
JavaPluginInstance5::URLNotify(const char* url, 
			       const char*target, 
			       JDPluginReason reason, 
			       void* notifyData) {
    int key = (int)notifyData;
    if (tracing) {
	char head[31];
	int clen = slen(url);
	if (clen > 30) {
	    clen = 30;
	}
	memcpy(head, url, clen);
	head[clen] = 0;
	UNUSED(target);
        trace("%d: NPP_URLNotify: key=0x%X %s => %d\n", plugin_number,
	      key, head, reason);
	    
    }
    if (key == JAVA_PLUGIN_JAVASCRIPT_REQUEST) {
        // A general 4.0 JS request has just finished giving us all its data
       CWriteBuffer wb;
       wb.putInt(JAVA_PLUGIN_JAVASCRIPT_END);
       wb.putInt(plugin_number);
       plugin_factory->SendRequest(wb, FALSE);	
    } else if (key == JAVA_PLUGIN_DOCBASE_QUERY) {
	// In 4.0 we get the docbase through a JS request.
    } else {
	trace("[%d] Other URLNotify %X \n", plugin_number, key);
    }
    return JD_OK;
}

JD_METHOD_(void) 
JavaPluginInstance5::JavascriptReply(const char *reply) {
    trace("[%d] CJavaPluginInstance::JavascriptReply JAVA_PLUGIN_JAVASCRIPT_REPLY\n", plugin_number);

    CWriteBuffer wb;
    trace("JavaPluginInstance5:JavascriptReply\n");
    wb.putInt(JAVA_PLUGIN_JAVASCRIPT_REPLY);
    wb.putInt(plugin_number);
    wb.putString(reply);

    plugin_factory->SendRequest(wb, FALSE);
}

JD_IMETHODIMP JavaPluginInstance5::SetUniqueId(long uniqueId) {
	// already set
	if(0 != m_uniqueId)
	    return JD_OK;

	if(0 == uniqueId) {
	    m_uniqueId =  s_uniqueId ++;
	} else {
	    m_uniqueId = uniqueId;
	}
	return JD_OK;
}

JD_IMETHODIMP JavaPluginInstance5::GetUniqueId(long* pId) {
	*pId = m_uniqueId;
	return JD_OK;
}

JD_METHOD JavaPluginInstance5::GetJSDispatcher(ILiveconnect* *pLiveconnect) {
    if (pLiveconnect == NULL)
        return JD_ERROR_NULL_POINTER;
    
    *pLiveconnect = NULL;
 
    if (m_pLiveconnect == NULL) {
	JDresult ret = JD_ERROR_FAILURE;
	IPluginServiceProvider* pProvider = plugin_factory->GetServiceProvider();
	if (pProvider == NULL)
	  return ret;

        ret = pProvider->QueryService(jCLiveconnectCID, jILiveconnectIID, (ISupports**)&m_pLiveconnect);

	if (ret != JD_OK || m_pLiveconnect == NULL)
	  return ret;
    }

    *pLiveconnect = m_pLiveconnect;
    (*pLiveconnect)->AddRef();
	
    return JD_OK;
}
 
/*
 * Inform this plugin instance that a cache or https 
 * request is starting up so that it may be terminated
 * properly when the plugin quits
 */
void JavaPluginInstance5::EnterRequest(char* requestName) {
    current_request = requestName;
}

void JavaPluginInstance5::ExitRequest(char* unused_requestName) {
    UNUSED(unused_requestName);
    current_request = NULL;
}

void JavaPluginInstance5::SetStatus(short status) {
    m_status = status;
}
