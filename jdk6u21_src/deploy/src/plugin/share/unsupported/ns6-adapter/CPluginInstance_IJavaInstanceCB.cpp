#include <stdio.h>
#include <stdlib.h>
#include "CPluginInstance.h"
#include "nsICookieStorage.h"

NS_DEFINE_CID(kCPluginManagerCID, NS_PLUGINMANAGER_CID);
NS_DEFINE_IID(kIPluginManager2IID, NS_IPLUGINMANAGER2_IID);
NS_DEFINE_IID(kICookieStorageIID, NS_ICOOKIESTORAGE_IID);

void CPluginInstance::showStatus(char * mess) {
fprintf(stderr,"CPluginInstance::showStatus: %s\n",mess);
    if (!m_destroyed) {
        m_PluginInstancePeer->ShowStatus(mess);
    }
}

void CPluginInstance::showDocument(char * url, char * target) {
fprintf(stderr,"CPluginInstance::showDocument %s - %s\n",url,target);

    nsresult rv;
    nsIPluginManager2 * pm2 = NULL;
    rv = m_pServiceManager->GetService(kCPluginManagerCID,
                                       kIPluginManager2IID,
                                       (nsISupports **) &pm2);
    if (NS_SUCCEEDED(rv) && (NULL != pm2)) {
        pm2->GetURL((nsIJVMPluginInstance*)this, url, target, NULL);
        pm2->Release();
    }
}

void CPluginInstance::findProxy(char * url, 
                                char ** proxyResult) {
fprintf(stderr,"CPluginInstance::findProxy:\n  url=%s\n",url);

    nsresult rv;
    nsIPluginManager2 * pm2 = NULL;
    rv = m_pServiceManager->GetService(kCPluginManagerCID,
                                  kIPluginManager2IID,
                                  (nsISupports **) &pm2);

    if (NS_SUCCEEDED(rv) && (NULL != pm2)) {
        pm2->FindProxyForURL(url, proxyResult);
        pm2->Release();
    fprintf(stderr,"proxyResult=%s\n",*proxyResult);
    }
}

void CPluginInstance::findCookie(char * url, char ** cookieResult) {
fprintf(stderr,"CPluginInstance::findCookie\n");

    const PRUint32 MAX_COOKIE = 8192;
    PRUint32 cookieSize = MAX_COOKIE;

    nsresult rv;
    nsICookieStorage * cs = NULL;
    rv = m_pServiceManager->GetService(kCPluginManagerCID,
                                       kICookieStorageIID,
                                       (nsISupports **) &cs);
    if (NS_SUCCEEDED(rv) && (NULL != cs)) {
        char * cres = (char *) malloc(MAX_COOKIE);
        rv = cs->GetCookie(url, cres, cookieSize);
        cs->Release();
        if(NS_SUCCEEDED(rv)) {
            *cookieResult = cres;
        } else {
            *cookieResult = strdup(" ");
            free(cres);
        }
    } else {
        *cookieResult = strdup(" ");
    }

    fprintf(stderr,"cookie=%s\n",*cookieResult);
}

void CPluginInstance::javascriptRequest(char * buff) {
fprintf(stderr,"CPluginInstance::javascriptRequest\n");
}

void CPluginInstance::setCookie(char * url, char * cookie) {
fprintf(stderr,"CPluginInstance::setCookie\n");

    nsresult rv;
    nsICookieStorage * cs = NULL;
    rv = m_pServiceManager->GetService(kCPluginManagerCID,
                                  kICookieStorageIID,
                                  (nsISupports **) &cs);

    if (NS_SUCCEEDED(rv) && (NULL != cs)) {
        cs->SetCookie(url, cookie, strlen(cookie));
        cs->Release();
    }
}
