/*
 * @(#)NetworkUtil.c	1.5 04/28/2005
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "NetworkUtil.h"
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

/*
 * Returns the Fully Qualified domain name of a computer from the DNS.
 *   Exmaple: Host Name = emma  
 *            Domain = east.sun.com
 *   will return emma.east.sun.com
 */         
JNIEXPORT jstring JNICALL Java_com_sun_deploy_net_proxy_WebProxyAutoDetection_getFQHostName
  (JNIEnv *env, jclass cl ) {

    jstring str = NULL;
    struct hostent* hp1 = NULL;
    struct hostent* hp2 = NULL;
    struct in_addr * pAddress = NULL;
    int MAX_HOST_LENGTH = 512;
    char host[MAX_HOST_LENGTH];

    // Get the computer hostname
    gethostname( host, MAX_HOST_LENGTH );
 
    // Get the IP Information for the host
    hp1  = gethostbyname( host );

    // If the host name was retrieved
    if( hp1 != NULL )
    {
        // Now take the raw IP and let the DNS resolve it to ASCII
        hp2 = gethostbyaddr( hp1->h_addr_list[0], sizeof(long), AF_INET);    
    }   

    // If the IP was resolved into Text based FQDN
    if( hp2 != NULL )
    {  
        //Put the result in the Java String Object
        str = (*env)->NewStringUTF(env, hp2->h_name); 
    }
  
    return str;
}    
