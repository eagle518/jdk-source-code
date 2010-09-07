/*
 * @(#)MessagePassingExecutionContext.java	1.23 10/05/20 
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.main.client;

import java.io.IOException;
import java.net.PasswordAuthentication;
import java.net.URL;
import java.net.MalformedURLException;
import java.security.SecureRandom;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import netscape.javascript.*;
import sun.plugin2.applet.*;
import sun.plugin2.message.*;
import com.sun.deploy.net.cookie.*;
import com.sun.deploy.net.proxy.*;
import com.sun.deploy.security.BrowserAuthenticator;
import com.sun.deploy.util.URLUtil;
import com.sun.deploy.util.Trace;

/** An implementation of the Applet2ExecutionContext interface which
    provides its functionality by sending messages to and receiving
    messages back from the web browser. */

public class MessagePassingExecutionContext implements Applet2ExecutionContext {
    private Map/*<String,String>*/ params;
    private Pipe pipe;
    private int appletID;
    private String documentBase;

    /** Creates a new MessagePassingExecutionContext. It requires the
        set of applet parameters to be specified, along with the
        communications pipe back to the web browser and the applet ID
        associated with the applet we're executing. (FIXME: should
        rethink this -- the original idea was to be able to share the
        execution contexts between applets, but the need to supply the
        applet parameters and the applet ID defeat this. The applet ID
        can be trivially removed by putting it in the Plugin2Manager,
        which is supplied to for example getJSObject(), but the applet
        parameters may be more difficult to deal with. Perhaps all of
        the methods on this class need to take an Plugin2Manager as
        argument.) */

    public MessagePassingExecutionContext(Map/*<String,String>*/ appletParameters, Pipe pipe, int appletID, String docbase) {
        this.params = appletParameters;
        this.pipe = pipe;
        this.appletID = appletID;
        this.documentBase = docbase;
    }

    public Map/*<String,String>*/ getAppletParameters() {
        return params;
    }

    public void setAppletParameters(Map/*<String,String>*/ p) {
        params=p;
    }

    public String getDocumentBase(Plugin2Manager manager) {
        // Let's try implementing this via LiveConnect in all situations
        // (See sun.plugin.AppletViewer.getDocumentBase() for Netscape-specific code path)
        if (documentBase == null) {
            try {
                JSObject window = getJSObject(manager);
                if (window != null) {
                    JSObject document = (JSObject) window.getMember("document");
		    String url = null;
		    try {
			url = (String) document.getMember("URL");
		    } catch (JSException jse) {
			// try to get documentURI if no document.URL
			url = (String) document.getMember("documentURI");
		    }

		    if (url == null) throw new Exception("Can not get DocumentBase");
			
                    // Canonicalize URL in case the URL is in some
                    // weird form only recognized by the browsers
                    //
                    String tmpDocBase = URLUtil.canonicalize(url);
                    documentBase = URLUtil.canonicalizeDocumentBaseURL(tmpDocBase);
                    try {
                        // Need to "normalize" the document base by creating a temporary URL for it
                        documentBase = new URL(documentBase).toString();
                    } catch (MalformedURLException e) {
                    }
                }
            } catch (Exception e) {
                // FIXME: consider doing something else or printing this in another way
                Trace.ignored(e);
            }
        }

        return documentBase;
    }

    // This methods retrieves a proxy list from browser side to be used to connect through
    // in order to download the class files and resources etc.
    public List/*<java.net.Proxy>*/ getProxyList(URL url, boolean isSocketURI) {
        Conversation c = pipe.beginConversation();

        try {
            GetProxyMessage msg = new GetProxyMessage(c, appletID, url, isSocketURI);
            pipe.send(msg);
            ProxyReplyMessage reply = (ProxyReplyMessage) pipe.receive(0, c);
            
            if (reply != null) {
                return reply.getProxyList();
            }
        }  catch (Throwable ex) {
            Trace.ignored(ex);
        } finally {
            pipe.endConversation(c);
        }

        // Return NO_PROXY to behave better in error situations
        ArrayList res = new ArrayList();
        res.add(java.net.Proxy.NO_PROXY);
        return res;
    }

    public void setCookie(URL url, String value) throws CookieUnavailableException {
        doCookieOp(false, url, value);
    }

    public String getCookie(URL url) throws CookieUnavailableException {
        return doCookieOp(true, url, null);
    }

    private String doCookieOp(boolean doGet, URL url, String value) throws CookieUnavailableException {
        Conversation c = pipe.beginConversation();
        try {
            CookieOpMessage msg = new CookieOpMessage(c, appletID,
                                                      (doGet ? CookieOpMessage.GET_COOKIE : CookieOpMessage.SET_COOKIE),
                                                      url,
                                                      value);
            pipe.send(msg);
            CookieReplyMessage reply = (CookieReplyMessage) pipe.receive(0, c);
            if (reply.getExceptionMessage() != null) {
                throw new CookieUnavailableException(reply.getExceptionMessage());
            }
            return reply.getCookie();
        }  catch (Throwable ex) {
            Trace.ignored(ex);
        } finally {
            pipe.endConversation(c);
        }

        return null;
    }

    public com.sun.deploy.security.CertStore getBrowserSigningRootCertStore() {
        return ServiceDelegate.get().getBrowserSigningRootCertStore();
    }

    public com.sun.deploy.security.CertStore getBrowserSSLRootCertStore() {
        return ServiceDelegate.get().getBrowserSSLRootCertStore();
    }

    public com.sun.deploy.security.CertStore getBrowserTrustedCertStore() {
        return ServiceDelegate.get().getBrowserTrustedCertStore();
    }

    public java.security.KeyStore getBrowserClientAuthKeyStore() {
        return ServiceDelegate.get().getBrowserClientAuthKeyStore();
    }

    public com.sun.deploy.security.CredentialManager getCredentialManager() {
        return ServiceDelegate.get().getCredentialManager();
    }

    public SecureRandom getSecureRandom() {
        return ServiceDelegate.get().getSecureRandom();
    }

    public boolean isIExplorer() {
        return ServiceDelegate.get().isIExplorer();
    }

    public boolean isNetscape() {
        return ServiceDelegate.get().isNetscape();
    }

    public com.sun.deploy.net.offline.OfflineHandler getOfflineHandler() {
        return ServiceDelegate.get().getOfflineHandler();
    }

    // FIXME: should indicate in the SetJVMIDMessage whether the
    // current plugin actually has support for browser authentication
    // and return null for the BrowserAuthenticator if not; this will
    // ease porting to new browsers
    class BrowserAuthenticatorImpl implements BrowserAuthenticator {
        public PasswordAuthentication getAuthentication(String protocol, String host, int port, 
                                                        String scheme, String realm, URL requestURL,
                                                        boolean proxyAuthentication) {
            Conversation c = pipe.beginConversation();
 
            try {
                GetAuthenticationMessage msg = new GetAuthenticationMessage(c, appletID,
                                                                            protocol,
                                                                            host,
                                                                            port,
                                                                            scheme,
                                                                            realm,
                                                                            requestURL,
                                                                            proxyAuthentication);
                pipe.send(msg);
                GetAuthenticationReplyMessage reply =
                    (GetAuthenticationReplyMessage) pipe.receive(0, c);
                if (reply.getErrorMessage() != null) {
                    throw new RuntimeException(reply.getErrorMessage());
                }
                return reply.getAuthentication();
            } catch (InterruptedException ex) {
                throw new RuntimeException(ex);
            } catch (IOException ex) {
                throw new RuntimeException(ex);
            } finally {
                pipe.endConversation(c);
            }
        }
    }
    private BrowserAuthenticator authenticator;
    public BrowserAuthenticator getBrowserAuthenticator() {
        if (authenticator == null) {
            authenticator = new BrowserAuthenticatorImpl();
        }
        return authenticator;
    }

    //----------------------------------------------------------------------
    // Methods implementing support for the
    // sun.plugin.services.BrowserService interface
    //

    /**
     * Return browser version.
     */
    public float getBrowserVersion() {
        return 1.0f;
    }
    
    /**
     * Check if console should be iconified on close.
     */
    public boolean isConsoleIconifiedOnClose() {
        return false;
    }

    /**
     * Install browser event listener
     * @since 1.4.1
     */
    public boolean installBrowserEventListener() {
        return false;
    }

    /**
     * Browser element mapping
     * @since 1.4.2
     */
    public String mapBrowserElement(String rawName) {
        return rawName;
    }

    //----------------------------------------------------------------------
    // Methods implementing support for the AppletContext interface
    //

    public void showDocument(URL url) {
        showDocument(url, "_self");
    }
    public void showDocument(URL url, String target) {
        Conversation c = pipe.beginConversation();
 
        try {
            ShowDocumentMessage msg = new ShowDocumentMessage(c, appletID, 
                                                              url.toString(), 
                                                              target);
            pipe.send(msg);
        } catch (IOException ex) {
            Trace.ignored(ex);
        } finally {
            pipe.endConversation(c);
        }
    }

    // show status in browser bar
    public void showStatus(String status) {
	Conversation c = pipe.beginConversation();
	
	try {
	    ShowStatusMessage msg = new ShowStatusMessage(c, appletID, status);
	    pipe.send(msg);
	} catch (IOException ex) {
	    Trace.ignored(ex);
	} finally {
	    pipe.endConversation(c);
	}
    }

    public JSObject getJSObject(Plugin2Manager manager) throws JSException {
        // Note: because we (currently) create a different
        // MessagePassingExecutionContext for each applet we start,
        // passing the Plugin2Manager is only a convenience, not a
        // necessity. We could consider changing this in the future
        // for memory consumption savings, etc.
        manager.stopWaitingForAppletStart();

        // Note that this code is similar to that in the
        // MessagePassingJSObject; could consider refactoring it (note
        // that I don't think that round-trip JavaScript-to-Java calls
        // should be allowed on this thread at this point, but maybe
        // I'm wrong)
        Conversation c = pipe.beginConversation();
        try {
            JavaScriptGetWindowMessage msg = new JavaScriptGetWindowMessage(c, appletID);
            pipe.send(msg);
            JavaScriptReplyMessage reply = (JavaScriptReplyMessage) pipe.receive(0, c);
            if (reply.getExceptionMessage() != null) {
                throw new JSException(reply.getExceptionMessage());
            }
            return (JSObject) LiveConnectSupport.importObject(reply.getResult(), appletID);
        } catch (InterruptedException e) {
            throw (JSException) new JSException().initCause(e);
        } catch (IOException e) {
            throw (JSException) new JSException().initCause(e);
        } finally {
            pipe.endConversation(c);
        }
    }

    public JSObject getOneWayJSObject(Plugin2Manager manager) throws JSException {

        // Note that this code is similar to that in the
        // MessagePassingJSObject; could consider refactoring it (note
        // that I don't think that round-trip JavaScript-to-Java calls
        // should be allowed on this thread at this point, but maybe
        // I'm wrong)
        Conversation c = pipe.beginConversation();
        try {
            JavaScriptGetWindowMessage msg = new JavaScriptGetWindowMessage(c, appletID);
            pipe.send(msg);
            JavaScriptReplyMessage reply = (JavaScriptReplyMessage) pipe.receive(0, c);
            if (reply.getExceptionMessage() != null) {
                throw new JSException(reply.getExceptionMessage());
            }
            return (JSObject) LiveConnectSupport.importOneWayJSObject(reply.getResult(), appletID, manager);
        } catch (InterruptedException e) {
            throw (JSException) new JSException().initCause(e);
        } catch (IOException e) {
            throw (JSException) new JSException().initCause(e);
        } finally {
            pipe.endConversation(c);
        }
    }

    //----------------------------------------------------------------------
    // The following methods are only needed in the case of
    // disconnected applets and are therefore not implemented here

    public com.sun.deploy.net.proxy.BrowserProxyConfig getProxyConfig()   { return null; }
    public com.sun.deploy.net.proxy.ProxyHandler getSystemProxyHandler()  { return null; }
    public com.sun.deploy.net.proxy.ProxyHandler getAutoProxyHandler()    { return null; }
    public com.sun.deploy.net.proxy.ProxyHandler getBrowserProxyHandler() { return null; }
}
