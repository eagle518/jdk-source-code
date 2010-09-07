#include <stdio.h>
#include <stdlib.h>
#include "CPluginInstance.h"
#include "nsIPluginTagInfo.h"
#include "nsIPluginTagInfo2.h"

#define UNIQUE_IDENTIFIER_ID    "A8F70EB5-AAEF-11d6-95A4-0050BAAC8BD3"

NS_IMETHODIMP CPluginInstance::Initialize(nsIPluginInstancePeer* peer) {
fprintf(stderr,"CPluginInstance::Initialize\n");

    static NS_DEFINE_IID(kPluginTagInfoIID, NS_IPLUGINTAGINFO_IID);
    static NS_DEFINE_IID(kPluginTagInfo2IID, NS_IPLUGINTAGINFO2_IID);

    nsresult rv = NS_OK;
    m_PluginInstancePeer = peer;
    m_PluginInstancePeer->AddRef();

    const char * mimeType;
    rv = m_PluginInstancePeer->GetMIMEType(&mimeType);
    if (NS_FAILED(rv)) {
        return rv;
    }

    nsIPluginTagInfo *tagInfo;
    PRUint16 argc1 = 0;
    char **keys1;
    char ** vals1; 
    rv = m_PluginInstancePeer->QueryInterface(kPluginTagInfoIID, 
                                              (void **) &tagInfo);
    if ( NS_SUCCEEDED(rv) && NULL != tagInfo) {
        tagInfo->GetAttributes(argc1, 
                               (const char* const* &) keys1,
                               (const char* const* &) vals1);
        tagInfo->Release();
    } 

    nsIPluginTagInfo2 *extTagInfo;
    PRUint16 argc2 = 0;
    char **keys2; 
    char ** vals2;
    rv = m_PluginInstancePeer->QueryInterface(kPluginTagInfo2IID,
                                              (void **) &extTagInfo);
    if (NS_OK == rv && NULL != extTagInfo) {
        nsPluginTagType tagType = nsPluginTagType_Unknown;
        if (NS_SUCCEEDED(extTagInfo->GetTagType(&tagType))) {
            if (tagType == nsPluginTagType_Applet ||
                tagType == nsPluginTagType_Object) {
                extTagInfo->GetParameters(argc2, 
                                          (const char* const* &) keys2,
                                          (const char* const* &) vals2);
            }
        }

        int argc;
        char ** keys;
        char ** vals;

        argc = argc1 + argc2 + 1;
        keys = (char**) malloc((argc)*sizeof(char*));
        vals = (char**) malloc((argc)*sizeof(char*));

        // Fix for 4648112
        // check if we are being called because of classid="clsid"
        // if so, replace all codebase attributes with "."
        // any actual codebases paramaters given later will override
        // This only applies to object tags

        int i;
        bool replace_codebase = false;
        const char * s_codebase = "codebase";
        const char * s_classid = "classid";
        const char * s_clsid = "clsid:";
        if (tagType == nsPluginTagType_Object) {
            for (i=0; i < argc1; i++) {
                if (!strncasecmp(keys1[i],s_classid,strlen(s_classid))) {
                    if(!strncasecmp(vals1[i],s_clsid,strlen(s_clsid))) {
                       replace_codebase = true;
                       break;
                    }
                }
            }
        }

        if(replace_codebase) {
            for (i=0; i < argc1; i++) {
                if(!strncasecmp(keys1[i],s_codebase,strlen(s_codebase))) {
                    vals[i] = strdup(".");
                } else {
                    vals[i] = vals1[i];
                }
                keys[i] = keys1[i];
            }
        } else {
            for (i=0; i < argc1; i++) {
                keys[i] = keys1[i];
                vals[i] = vals1[i];
            }
        }

        for (i=0; i < argc2; i++) {
          keys[i+argc1] = keys2[i];
          vals[i+argc1] = vals2[i];
        }
 
        const char *docbase = NULL;
        rv = extTagInfo->GetDocumentBase(&docbase);

        keys[argc-1] = (char*)UNIQUE_IDENTIFIER_ID;
        vals[argc-1] = (char*)docbase;

        m_Creater->createJavaInstance(mimeType, argc,
                                      (const char ** ) keys, 
                                      (const char **) vals, 
                                      this, &m_JavaInstance); 

        free(keys);
        free(vals);

        if (NULL != docbase && NULL != m_JavaInstance) {
            m_JavaInstance->docbase(docbase);
        }
        extTagInfo->Release();
    }
    
    return rv;
}

NS_IMETHODIMP CPluginInstance::GetPeer(nsIPluginInstancePeer* *resultingPeer) {
fprintf(stderr,"CPluginInstance::GetPeer\n");
     if (NULL != m_PluginInstancePeer) {
         m_PluginInstancePeer->AddRef();
     }
     *resultingPeer = m_PluginInstancePeer;
     return NS_OK;
}

NS_IMETHODIMP CPluginInstance::Start() {
fprintf(stderr,"CPluginInstance::Start()\n");
    m_JavaInstance->start();
    return NS_OK;
}

NS_IMETHODIMP CPluginInstance::Stop() {
fprintf(stderr,"CPluginInstance::Stop()\n");
    m_JavaInstance->stop();
    return NS_OK;
}

NS_IMETHODIMP CPluginInstance::Destroy() {
fprintf(stderr,"CPluginInstance::Destroy()\n");
    if(!m_destroyed) {
        m_JavaInstance->destroy();
        m_destroyed = true;
        m_JavaInstance->release();
        m_JavaInstance = NULL;
    }
    return NS_OK;
}

NS_IMETHODIMP CPluginInstance::SetWindow(nsPluginWindow* window) {
fprintf(stderr,"CPluginInstance::SetWindow\n");
    nsPluginSetWindowCallbackStruct *ws_info;

    if (NULL == window || NULL == window->window) {
        m_JavaInstance->window(0,0,0,0,0);
    } else {
        m_JavaInstance->window((int) window->window,
                                    window->width,
                                    window->height,
                                    window->x,
                                    window->y); 
    }
    return NS_OK;
}

NS_IMETHODIMP CPluginInstance::NewStream(nsIPluginStreamListener** listener) {
fprintf(stderr,"CPluginInstance::NewStream\n");
return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP CPluginInstance::Print(nsPluginPrint* platformPrint) {
fprintf(stderr,"CPluginInstance::Print\n");
return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP CPluginInstance::GetValue(nsPluginInstanceVariable variable, void *value) {
fprintf(stderr,"CPluginInstance::GetValue\n");
    if (variable == nsPluginInstanceVariable_WindowlessBool) {
        // Does not apply to UNIX
    } else if (variable == nsPluginInstanceVariable_TransparentBool) {
        // Does not apply to UNIX
    } else if (variable == nsPluginInstanceVariable_DoCacheBool) {
        *(PRBool *)value = PR_FALSE;
        return NS_OK;
    } else if (variable == nsPluginInstanceVariable_CallSetWindowAfterDestroyBool) {
        *(PRBool *)value = PR_FALSE;
        return NS_OK;
    }

    // should we be returning UNEXPECTED or something else?
    return NS_ERROR_NOT_IMPLEMENTED;
}

NS_IMETHODIMP CPluginInstance::HandleEvent(nsPluginEvent* event, PRBool* handled) {
fprintf(stderr,"CPluginInstance::HandleEvent\n");
return NS_ERROR_NOT_IMPLEMENTED;
}

