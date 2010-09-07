/*
 * @(#)JSObject.cpp	1.11 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*
 * Native side implemenation to deliver the JSObject call to 
 * browser side (Java ----> Javascript communication through
 * netscape.javascript.JSObject.*
 *
 *  
 */

#include "commonhdr.h"
#include "protocol.h"
#include "jni.h"
#include "IPluginServiceProvider.h"
#include "ILiveconnect.h"
#include "ISecurityContext.h"
#include "IJVMManager.h"
#include "IBrowserAuthenticator.h"

#include "JavaPluginFactory5.h"
#include "CSecurityContext.h"
#include "remotejni.h"
#include "JSObject.h"

#ifndef MAX_PATH
#define MAX_PATH 1024
#endif

JD_DEFINE_IID(jISupportsIID, ISUPPORTS_IID);
JD_DEFINE_IID(jILiveconnectIID, ILIVECONNECT_IID);
JD_DEFINE_IID(jCLiveconnectCID, CLIVECONNECT_CID);
static JD_DEFINE_CID(jBrowserAuthenticatorIID, IBROWSERAUTHENTICATOR_IID);
static JD_DEFINE_CID(jBrowserAuthenticatorCID, IBROWSERAUTHENTICATOR_IID);


/* Read the JS message assuming the message structure described in 
   jvm_natives/native.c */
void 
UnpackJSMessage(RemoteJNIEnv* env, JSMessage* mesg) {
  int raw_msg_len;		/* Length of the data read */
  char *raw_msg;                /* Raw data read */
  int raw_mi = 0;	        /* Index into the raw message */

  trace("JSObject:UnpackJSMessage()");
  /* Read the length of the message */
  get_msg(env, &raw_msg_len, 4);

  raw_msg = (char *) checked_malloc(raw_msg_len);

    /* Swallow the whole message */
  get_msg(env, raw_msg, raw_msg_len);
  
  memcpy(&(mesg->requestID), raw_msg + raw_mi, 4); raw_mi += 4;
  memcpy(&(mesg->nativeJSObject), raw_msg + raw_mi, 4); raw_mi += 4;
  memcpy(&(mesg->slotindex), raw_msg + raw_mi, 4); raw_mi += 4;
  memcpy(&(mesg->utfstr_len), raw_msg + raw_mi, 4); raw_mi += 4;

  if (mesg->utfstr_len > 0) {
    mesg->utfstr = (char*) checked_malloc(mesg->utfstr_len + 1);
    memcpy(mesg->utfstr, raw_msg + raw_mi, mesg->utfstr_len);
    raw_mi += mesg->utfstr_len;
    mesg->utfstr[mesg->utfstr_len] = '\0';
  } else {
    mesg->utfstr = NULL;
  }
  memcpy(&(mesg->charstr_len), raw_msg + raw_mi, 4); raw_mi += 4;
  memcpy(&(mesg->charstr_sz), raw_msg + raw_mi, 4); raw_mi += 4;

  if (mesg->charstr_len > 0) {
    mesg->charstr = (jchar*) checked_malloc(mesg->charstr_sz);
    memcpy(mesg->charstr, raw_msg + raw_mi, mesg->charstr_sz);
    raw_mi += mesg->charstr_sz;
    /*
      for(int i = 0; i < mesg->charstr_len; i++) {
      fprintf(stderr, "rem: Char %d is %d\n", i, (int) mesg->charstr[i]);
      }
    */
  }
    
  memcpy(&(mesg->jarr), raw_msg + raw_mi, sizeof(jobjectArray));
  raw_mi += sizeof(jobjectArray);
  memcpy(&(mesg->value), raw_msg + raw_mi, sizeof(jobject));
  raw_mi += sizeof(jobject);
  memcpy(&(mesg->ctx), raw_msg + raw_mi, sizeof(jobject));
  raw_mi += sizeof(jobject);

  free(raw_msg);

  trace("UnpackJSMessage: received JS nativeJSObject=%d slot=%d utflen=%d\n\tjchar str=%X len=%d size=%d\n\tjarr=%X\n\tjval=%X ctx=%X raw_msg_len=%d\n",mesg->nativeJSObject,mesg->slotindex, mesg->utfstr_len,
	(int) mesg->charstr, mesg->charstr_len, mesg->charstr_sz,
	(int) mesg->jarr, 
	(int) mesg->value, (int) mesg->ctx, raw_msg_len);
}

void 
FreeJSMessage(JSMessage* mesg) {
    /* Free the malloced strings */
    if (mesg->charstr_len > 0) 
	free(mesg->charstr);
    if (mesg->utfstr_len > 0)
	free(mesg->utfstr);
}

/* Send a final response on the pipe for this env */
static void
send_jnijsOK_res(RemoteJNIEnv *env, int replyID, void* data, int len) {
  char *msg;
  int return_code = JAVA_PLUGIN_RETURN;

  msg = (char *) checked_malloc(8 + len);
  memcpy(msg, &return_code, 4);
  memcpy(msg + 4, &replyID, 4);
  memcpy(msg + 8, data, len);

  //debug("Returning from JS call jnijsOK\n");

  send_msg(env, msg, 8 + len);

  free(msg);
}


// CreateSecurityContext is a function which creates the ISecurityContext
// object. It
JDresult 
CreateSecurityContext(const char* origin, JDBool isAllPermission, ISecurityContext** pContext) {
    if (pContext == NULL)
        return JD_ERROR_FAILURE;

    CSecurityContext::Create(NULL, origin, isAllPermission, jISupportsIID, 
			     (void**) pContext);
    return JD_OK;
}


const char *
jscode_to_str(int js_code) {
  
  switch(js_code) {
  case JAVA_PLUGIN_JNIJS_GET_NATIVE: return "JAVA_PLUGIN_JNIJS_GET_NATIVE" ; 
    case JAVA_PLUGIN_JNIJS_TOSTRING: return "JAVA_PLUGIN_JNIJS_TOSTRING" ; 
    case JAVA_PLUGIN_JNIJS_FINALIZE: return "JAVA_PLUGIN_JNIJS_FINALIZE" ; 
    case JAVA_PLUGIN_JNIJS_CALL: return "JAVA_PLUGIN_JNIJS_CALL" ; 
    case JAVA_PLUGIN_JNIJS_EVAL: return "JAVA_PLUGIN_JNIJS_EVAL" ; 
    case JAVA_PLUGIN_JNIJS_GETMEMBER: return "JAVA_PLUGIN_JNIJS_GETMEMBER" ; 
    case JAVA_PLUGIN_JNIJS_SETMEMBER: return "JAVA_PLUGIN_JNIJS_SETMEMBER" ; 
    case JAVA_PLUGIN_JNIJS_REMOVEMEMBER: 
      return "JAVA_PLUGIN_JNIJS_REMOVEMEMBER" ; 
    case JAVA_PLUGIN_JNIJS_GETSLOT: return "JAVA_PLUGIN_JNIJS_GETSLOT" ; 
    case JAVA_PLUGIN_JNIJS_SETSLOT: return "JAVA_PLUGIN_JNIJS_SETSLOT" ; 
	case JAVA_PLUGIN_GET_BROWSER_AUTHINFO: return "JAVA_PLUGIN_GET_BROWSER_AUTHINFO";
  default:
    return "UNKNOWN CODE";
  }

}

jcharArray GetBrowserAuthInfo(RemoteJNIEnv* env, jobjectArray args, IBrowserAuthenticator* pBrowserAuthenticator) {
        jstring protocol = (jstring)env->GetObjectArrayElement(args, 0);
        jstring host = (jstring)env->GetObjectArrayElement(args, 1);
        jstring port = (jstring)env->GetObjectArrayElement(args, 2);
        jstring scheme = (jstring)env->GetObjectArrayElement(args, 3);
        jstring realm = (jstring)env->GetObjectArrayElement(args, 4);

        const char* lpszProtocol = env->GetStringUTFChars(protocol, (jboolean*)0);
        const char* lpszHost = env->GetStringUTFChars(host, (jboolean*)0);
        const char* lpszScheme = env->GetStringUTFChars(scheme, (jboolean*)0);
        const char* lpszRealm = env->GetStringUTFChars(realm, (jboolean*)0);
        const char* lpszPort = env->GetStringUTFChars(port, (jboolean*)0);

        jcharArray ret = NULL;
        trace("Call browser authenticationInfo(%s, %s, %s, %s, %s)\n", 
                lpszProtocol, lpszHost, lpszPort, lpszScheme, lpszRealm);
		char szUserName[MAX_PATH];
		char szPassword[MAX_PATH];

        if(JD_SUCCEEDED(pBrowserAuthenticator->GetAuthInfo(lpszProtocol, lpszHost, atoi(lpszPort),
                lpszScheme, lpszRealm, szUserName, sizeof(szUserName), szPassword, sizeof(szPassword)))) {
			int len = strlen(szUserName) + strlen(szPassword);
			if(0 != len) {
				char* lpszBuf = new char[len + 2];
				sprintf(lpszBuf, "%s:%s", szUserName, szPassword);
				trace("Browser return: %s\n", lpszBuf);
				jstring jstrBuf = env->NewStringUTF(lpszBuf);
				const jchar * jcharsBuf = env->GetStringChars(jstrBuf, (jboolean*)0);
				const jsize unicode_len = env->GetStringLength(jstrBuf);
				ret = env->NewCharArray(unicode_len);
				env->SetCharArrayRegion(ret, 0, unicode_len, (jchar *)jcharsBuf);

				env->ReleaseStringChars(jstrBuf, jcharsBuf);
				delete[] lpszBuf;                   
            }
        }

        env->ReleaseStringUTFChars(protocol, lpszProtocol);
        env->ReleaseStringUTFChars(host, lpszHost);
        env->ReleaseStringUTFChars(scheme, lpszScheme);
        env->ReleaseStringUTFChars(realm, lpszRealm);
        env->ReleaseStringUTFChars(port, lpszPort);

        return ret;

}



//=-------------------------------------------------------------------------=
// UnwrapJavaWrapper is a function which returns an opaque reference given a
// JNI type of java object
// 
// parameter:
// JNIEnv* env               [in]  the JNI enviroment variable
// jobject jobj              [in]  the JNI type of java object
// jint*   obj               [out] opaque reference of the java object
//
// out:
// JDresult                  [out] status code

//
JDresult UnwrapJavaWrapper(RemoteJNIEnv* env, jobject jobj, jint* obj)
{
    trace("JSObject::UnwrapJavaWrapper\n");
    if (env == NULL || jobj == NULL || obj == NULL)
	return JD_ERROR_NULL_POINTER;
    // Find class
    jclass clazz = env->FindClass("sun/plugin/javascript/navig5/JSObject");
    // Check if we may unwrap the JSObject
    if (env->IsInstanceOf(jobj, clazz) == JNI_FALSE)
    {
        return JD_ERROR_FAILURE;
    }
    jfieldID fid = env->GetFieldID(clazz, "nativeJSObject", "I");
    *obj = env->GetIntField(jobj, fid);
    return JD_OK;
}


/* Handle the next message from Java to JS */
void
JSHandler(RemoteJNIEnv* env) {
	/* 2 cases 
	   - if this is recursive then we have sole ownership of the
	   pipe. 
	   - If this is spontaneous, we should also have sole ownership of
	   the pipe since it is locked at the other end 
	   - Hence, there should be no need for locking here
	*/
	/* Get the plugin index */
	int pluginIndex;
	get_msg(env, (char*)&pluginIndex, 4);
	JavaPluginFactory5* plugin_factory = get_global_factory();
	JavaPluginInstance5* inst = plugin_factory->GetInstance(pluginIndex);
	int code;
	get_msg(env, (char*)&code, 4);
  
	/*
	 * when  JSObject is GCed after plugin is destroyed, inst is NULL 
	 * and code is JAVA_PLUGIN_JNIJS_FINALIZE. In this case, we still
	 * need consume message from spontaneous pipe and ask browser to 
	 * release native JSObject,  or spontaneous pipe will be corrupted
	 * and resource leak
	 */
	if (code != JAVA_PLUGIN_JNIJS_FINALIZE) {

		if (inst == NULL || inst->IsDestroyPending()) {

			// Consume the message
			int raw_msg_len;
			char* raw_msg;
			get_msg(env, &raw_msg_len, 4);
			raw_msg = (char *) checked_malloc(raw_msg_len);
			/* Swallow the whole message */
			get_msg(env, raw_msg, raw_msg_len);
			int replyID = 0;
			memcpy(&replyID, raw_msg, 4);
			free(raw_msg);
			jobject nullret = NULL;
			send_jnijsOK_res(env, replyID, &nullret, sizeof(jobject));
			return;
		}
	}
  
	JSMessage msg;
	trace("JSObject:Entering JSHandler()\n");

	UnpackJSMessage(env, &msg);

	// Create ProxyJNIEnv
	JNIEnv* pProxyJNIEnv = NULL;
	IJVMManager* jvm_manager;
	jvm_manager = plugin_factory->GetJVMManager();
    
	ILiveconnect* pLiveConnect = NULL;
	ISecurityContext* pContext = NULL;
    
	if (JD_SUCCEEDED(jvm_manager->CreateProxyJNI(NULL, &pProxyJNIEnv)) ) {
		trace("JSHandler(): JS command: %X %s\n", code, jscode_to_str(code));
	
		if (inst == NULL) {
			IPluginServiceProvider* pProvider = plugin_factory->GetServiceProvider();

			if (pProvider == NULL) {
				trace("JSHandler(): cannot get pProvider when inst is NULL\n");
				return;
			}

			if (JD_FAILED(pProvider->QueryService(jCLiveconnectCID,
					jILiveconnectIID, (ISupports**)&pLiveConnect))) {
				trace("JSHandler(): cannot get liveconnect when inst is NULL\n");
				return;
			}

		} else {
	
			if (JD_FAILED(inst->GetJSDispatcher(&pLiveConnect))) {
				return;
			}
		}

		if (msg.utfstr != NULL)
			CreateSecurityContext(msg.utfstr, (int) msg.ctx, &pContext);

		int replyID = msg.requestID;

		switch(code){
		case JAVA_PLUGIN_JNIJS_GET_NATIVE:
			{
				// CLiveconnect	  
				jsobject ret = 0;

				/* Get the JS object, which represents a window associated
				   with the given plugin instance */

				JDresult nr = pLiveConnect->GetWindow(pProxyJNIEnv, 
													  (IPluginInstance*) inst, 
													  (void **)NULL, 
													  0, 
													  (ISecurityContext*) pContext, 
													  &ret);
				if(JD_FAILED(nr) || ret == 0) {
					trace("JSObject::ILiveConnect::GetWindow FAILED\n");
				}
  
				send_jnijsOK_res(env, replyID, &ret, sizeof(jsobject));
				break;
			}
		case JAVA_PLUGIN_JNIJS_TOSTRING:
			{
				jstring ret = NULL;
	  
				JDresult nr = pLiveConnect->ToString(pProxyJNIEnv,
													 (jsobject) msg.nativeJSObject,
													 &ret);
				if(!JD_SUCCEEDED(nr)) {
					trace("JSObject::ILiveConnect::ToString FAILED\n");
				}
	  	  	
				send_jnijsOK_res(env, replyID, &ret, sizeof(jstring));
				break;
			}
		case JAVA_PLUGIN_JNIJS_FINALIZE:
			{		 
				jobject dummy = NULL;
				JDresult nr = pLiveConnect->FinalizeJSObject(pProxyJNIEnv, 
															 (jsobject) msg.nativeJSObject);
				if(!JD_SUCCEEDED(nr)) {
					trace("JSObject::ILiveConnect::FinalizeJSObject() FAILED\n");
				}
		
				send_jnijsOK_res(env, replyID, &dummy, sizeof(jobject));
				break;
			}
		case JAVA_PLUGIN_GET_BROWSER_AUTHINFO: 
			{
				IBrowserAuthenticator* pBrowserAuthenticator;
				jobject ret = NULL;
				if (inst != NULL) {
					IPluginServiceProvider* service_provider =  plugin_factory->GetServiceProvider();

					trace("Handle native call: GetBrowserAuthenticat()");
					if(service_provider != NULL && 
					   JD_SUCCEEDED(service_provider->QueryService(jBrowserAuthenticatorCID, 
																   jBrowserAuthenticatorIID,
																   (ISupports**)&pBrowserAuthenticator))) {                       
						trace("Interface IBrowserAuthenticator found");
						ret = GetBrowserAuthInfo(env, msg.jarr, pBrowserAuthenticator);
						service_provider->ReleaseService(jBrowserAuthenticatorCID, pBrowserAuthenticator);
					} else {
						trace("Interface IBrowserAuthenticator not found");
					}
				}

				send_jnijsOK_res(env, replyID, &ret, sizeof(jobject));
				break;
			}
		case JAVA_PLUGIN_JNIJS_CALL:
			{
				jobject ret = NULL;
				JDresult nr = pLiveConnect->Call(pProxyJNIEnv, 
												 (jsobject)msg.nativeJSObject, 
												 msg.charstr, msg.charstr_len, 
												 msg.jarr,
												 (void**)NULL, 0, 
												 (ISecurityContext*)pContext, &ret);
				if(!JD_SUCCEEDED(nr)) {
					trace("JSObject::ILiveConnect::Call() FAILED\n");
				}
			
				send_jnijsOK_res(env, replyID, &ret, sizeof(jobject));
				break;
			}
		case JAVA_PLUGIN_JNIJS_EVAL:
			{
				jobject ret = NULL;
				JDresult nr = pLiveConnect->Eval(pProxyJNIEnv, 
												 (jsobject)msg.nativeJSObject,
												 msg.charstr, msg.charstr_len,
												 (void**)NULL, 0,
												 (ISecurityContext*)pContext, &ret);
				if(!JD_SUCCEEDED(nr)) {
					trace("JSObject::ILiveConnect::Eval() FAILED\n");
				}
		
				send_jnijsOK_res(env, replyID, &ret, sizeof(jobject));
				break;
			}
		case JAVA_PLUGIN_JNIJS_GETMEMBER:
			{
				jobject ret = NULL;
		 
				JDresult nr = pLiveConnect->GetMember(pProxyJNIEnv,
													  (jsobject)msg.nativeJSObject,
													  msg.charstr, msg.charstr_len,
													  (void**)NULL, 0,
													  (ISecurityContext*)pContext, &ret);
				if(!JD_SUCCEEDED(nr)) {
					trace("JSObject::ILiveConnect::GetMember() FAILED\n");
				}
		
				send_jnijsOK_res(env, replyID, &ret, sizeof(jobject));
				break;
			}
		case JAVA_PLUGIN_JNIJS_SETMEMBER:
			{
				jobject dummy = NULL;
	 
				JDresult nr = pLiveConnect->SetMember(pProxyJNIEnv,
													  (jsobject)msg.nativeJSObject,
													  msg.charstr, msg.charstr_len, 
													  msg.value,
													  (void**)NULL, 0,
													  (ISecurityContext*)pContext);
				if(!JD_SUCCEEDED(nr)) {
					trace("JSObject::ILiveConnect::SetMember() FAILED\n");
				}
	  
				send_jnijsOK_res(env, replyID, &dummy, sizeof(jobject));
				break;
			}
		case JAVA_PLUGIN_JNIJS_REMOVEMEMBER:
			{
				jobject dummy = NULL;
		 
				JDresult nr = pLiveConnect->RemoveMember(pProxyJNIEnv, 
														 (jsobject)msg.nativeJSObject, 
														 msg.charstr, msg.charstr_len, 
														 (void**)NULL, 0, 
														 (ISecurityContext*)pContext);
				if(!JD_SUCCEEDED(nr)) {
					trace("JSObject::ILiveConnect::RemoveMember() FAILED\n");
				}
		  
				send_jnijsOK_res(env, replyID, &dummy, sizeof(jobject));
				break;
			}
		case JAVA_PLUGIN_JNIJS_GETSLOT:
			{
				jobject ret = NULL;
	 
				JDresult nr = pLiveConnect->GetSlot(pProxyJNIEnv, 
													(jsobject)msg.nativeJSObject, 
													msg.slotindex,
													(void**)NULL, 0, 
													(ISecurityContext*)pContext, &ret);
				if(!JD_SUCCEEDED(nr)) {
					trace("JSObject::ILiveConnect::GetSlot() FAILED");
				}
				send_jnijsOK_res(env, replyID, &ret, sizeof(jobject));
				break;
			}
		case JAVA_PLUGIN_JNIJS_SETSLOT:
			{
				jobject dummy = NULL;
	  
				JDresult nr = pLiveConnect->SetSlot(pProxyJNIEnv, 
													(jsobject)msg.nativeJSObject, 
													msg.slotindex, 
													msg.value,
													(void**)NULL, 0, 
													(ISecurityContext*)pContext);
				if(!JD_SUCCEEDED(nr)) {
					trace("JSObject::ILiveConnect::SetSlot() FAILED\n");
				}
				send_jnijsOK_res(env, replyID, &dummy, sizeof(jobject));
				break;
			}
		default:
			plugin_error("Error in handler for JS calls!\n");
	
			if (pContext != NULL)
				pContext->Release();
			/* End of the handler for JS method calls */
		}
	}
	else {
		trace("Can not get ProxyJNI\n");
	}
	if(pLiveConnect != NULL)
		pLiveConnect->Release();

	if (pContext)
		pContext->Release();

	FreeJSMessage(&msg);
}

  
