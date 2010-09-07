/*
 * @(#)CJavaStream.cpp	1.28 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Modified from Stanley Ho's Windows version by RS, BG.
 *
 */

#include <stdio.h>
#include <stdlib.h>

#include "urlprotocol.h"
#include "workerprotocol.h"
#include "JDSmartPtr.h"
#include "IPluginInstance.h"
#include "INS4PluginInstance.h"
#include "CJavaStream.h"
#include "Debug.h"

#define UNUSED(x) x=x

static JD_DEFINE_IID(jINS4PluginInstanceIID, INS4PLUGININSTANCE_IID);
/*
 * Construct and initialize a new stream
 */
CJavaStream::CJavaStream(IPluginInstance* pPluginInstance,
			 IPluginStreamInfo* peer):	
    CNetscapeStream(peer), m_PluginInstance(pPluginInstance), m_iCount(0)
{
    trace("CJavaStream::CJavaStream Creating a new stream\n");

    const char* szURL;
    peer->GetURL(&szURL);

    if (szURL == NULL) {

	m_pszURL = NULL;  // Patch for Opera Browser

    } 
    else 
    {
	int len = strlen(szURL);
	m_pszURL = (char*) malloc(sizeof(char) * len);
	memcpy(m_pszURL, szURL, len);
	m_pszURL[len] = '\0';
    }
}


/*
 * Destroy the stream
 */
CJavaStream::~CJavaStream()
{
    trace("CJavaStream::~CJavaStream\n");

    free(m_pszURL);
}

/*
 * StreamAsFile is called to return the filename in the cache.
 */
JD_IMETHODIMP CJavaStream::AsFile(const char *) 
{
    return JD_OK;
}


/* 
 * Write is called to handle a piece of data in the plugin stream.
 */
JD_IMETHODIMP CJavaStream::Write(const char* buffer, JDint32 offset, JDint32
				 len, JDint32 *bytesWritten)  
{
    /* 
       Notice that the offset only represents where the buffer is in 
       the entire stream, NOT where the data is in the buffer. Therefore,
       offset should never never be used, because the buffer already 
       contains all the data we need. 
    */
    trace("CJavaStream::Write %s\n", buffer);
    (void) offset; // defeat compiler "not used" warning.
    JDresult errorResult = JD_OK;
    int key;
    *bytesWritten = 0;

    // Get the notify key, which we use to indicate the type of request
    // from the stream peer 
    if (JD_OK != (errorResult = mStreamPeer->GetNotifyData((void **)&key)))
	return errorResult;

    // Dump the write request data

    char head[81];
    int clen = strlen(m_pszURL);
    if (clen > 30) {
	clen = 30;
    }
    memcpy(head, m_pszURL, clen);
    head[clen] = 0;
    
    //trace("CJavaStream::NPP_Write \"%s\" %d\n", head, len);
    if (len > 70) {	
	memcpy(head, buffer, 70);
	head[70] = 0;
        //fprintf(stderr, "%s...\n", head);
    } else {
	memcpy(head, buffer, len);
	head[len] = 0;
        //fprintf(stderr, "%s\n", head);
    }	

    JDSmartPtr<INS4PluginInstance> spNS4PluginInstance;
    if (m_PluginInstance)
        errorResult = m_PluginInstance->QueryInterface(jINS4PluginInstanceIID,
                                                       (void**)&spNS4PluginInstance);

    if (JD_FAILED(errorResult))
        return errorResult;

    if (key == JAVA_PLUGIN_DOCBASE_QUERY) {
	// We've just executed a javascript command to find the docbase.
	// Send the result to the java process.
	spNS4PluginInstance->SetDocbase(buffer);	
	return errorResult;

    } else if (key == JAVA_PLUGIN_JAVASCRIPT_REQUEST) {

	// A general java script reply has arrived
	spNS4PluginInstance->JavascriptReply(buffer);

    } else {
      //trace(" Other Stream Write %X \n",  key);
    }
    *bytesWritten = len;

    // Number of bytes accepted
    return errorResult;        
}

/*
 * Corresponds to NPP_NewStream's stype return parameter.
 */
JD_IMETHODIMP CJavaStream::GetStreamType(JDPluginStreamType *result)
{

    int key;
    JDresult res;

    trace("CJavaStream::GetStreamType\n");

    // Get the notify data
    if ( JD_OK != (res = mStreamPeer->GetNotifyData((void **) &key)))
	return res;

    return CNetscapeStream::GetStreamType(result);
}

