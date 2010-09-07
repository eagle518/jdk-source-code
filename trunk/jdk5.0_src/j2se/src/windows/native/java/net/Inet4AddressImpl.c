/*
 * @(#)Inet4AddressImpl.c	1.16 04/06/11
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <windows.h>
#include <winsock2.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys/types.h>

#include "java_net_InetAddress.h"
#include "java_net_Inet4AddressImpl.h"
#include "net_util.h"
#include "icmp.h"


/*
 * Returns true if hostname is in dotted IP address format. Note that this 
 * function performs a syntax check only. For each octet it just checks that
 * the octet is at most 3 digits.
 */
jboolean isDottedIPAddress(const char *hostname, unsigned int *addrp) {
    char *c = (char *)hostname;
    int octets = 0;
    unsigned int cur = 0;
    int digit_cnt = 0;

    while (*c) {
	if (*c == '.') {
	    if (digit_cnt == 0) {
		return JNI_FALSE;
	    } else {
		if (octets < 4) {
		    addrp[octets++] = cur;
		    cur = 0;
		    digit_cnt = 0;
		} else {		
		    return JNI_FALSE;
		}
	    }
	    c++;
	    continue;
	}

	if ((*c < '0') || (*c > '9')) {
	    return JNI_FALSE;	
	}

	digit_cnt++;
	if (digit_cnt > 3) {
	    return JNI_FALSE;
	}

	/* don't check if current octet > 255 */
	cur = cur*10 + (*c - '0');			  
			    
	/* Move onto next character and check for EOF */
	c++;
	if (*c == '\0') {
	    if (octets < 4) {
		addrp[octets++] = cur;
	    } else {		
		return JNI_FALSE;
	    }
	}
    }

    return (jboolean)(octets == 4);
}

/*
 * Inet4AddressImpl
 */

/*
 * Class:     java_net_Inet4AddressImpl
 * Method:    getLocalHostName
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL 
Java_java_net_Inet4AddressImpl_getLocalHostName (JNIEnv *env, jobject this) {
    char hostname[256];

    if (gethostname(hostname, sizeof hostname) == -1) {
	strcpy(hostname, "localhost");
    }
    return JNU_NewStringPlatform(env, hostname);
}

/*
 * Find an internet address for a given hostname.  Not this this
 * code only works for addresses of type INET. The translation
 * of %d.%d.%d.%d to an address (int) occurs in java now, so the
 * String "host" shouldn't be a %d.%d.%d.%d string. The only 
 * exception should be when any of the %d are out of range and
 * we fallback to a lookup.
 *
 * Class:     java_net_Inet4AddressImpl
 * Method:    lookupAllHostAddr
 * Signature: (Ljava/lang/String;)[[B
 *
 * This is almost shared code
 */

JNIEXPORT jobjectArray JNICALL 
Java_java_net_Inet4AddressImpl_lookupAllHostAddr(JNIEnv *env, jobject this, 
						jstring host) {
    const char *hostname;
    struct hostent *hp;
    unsigned int addr[4];

    jobjectArray ret = NULL;
    jclass byteArrayCls;

    if (IS_NULL(host)) {
	JNU_ThrowNullPointerException(env, "host argument");
	return NULL;
    }
    hostname = JNU_GetStringPlatformChars(env, host, JNI_FALSE);
    CHECK_NULL_RETURN(hostname, NULL);

    /*
     * The NT/2000 resolver tolerates a space in front of localhost. This
     * is not consistent with other implementations of gethostbyname.
     * In addition we must do a white space check on Solaris to avoid a
     * bug whereby 0.0.0.0 is returned if any host name has a white space.
     */
    if (isspace(hostname[0])) {
	JNU_ThrowByName(env, JNU_JAVANETPKG "UnknownHostException", hostname);
	goto cleanupAndReturn;
    } 

    /*
     * If the format is x.x.x.x then don't use gethostbyname as Windows
     * is unable to handle octets which are out of range.
     */
    if (isDottedIPAddress(hostname, &addr[0])) {
	unsigned int address;
	jbyteArray barray;
	jobjectArray oarray;  

	/* 
	 * Are any of the octets out of range?
	 */
	if (addr[0] > 255 || addr[1] > 255 || addr[2] > 255 || addr[3] > 255) {
	    JNU_ThrowByName(env, JNU_JAVANETPKG "UnknownHostException", hostname);
	    goto cleanupAndReturn;
	} 

	/*
	 * Return an byte array with the populated address.
	 */
	address = (addr[3]<<24) & 0xff000000;
	address |= (addr[2]<<16) & 0xff0000;
	address |= (addr[1]<<8) & 0xff00;
	address |= addr[0];

	byteArrayCls = (*env)->FindClass(env, "[B");
	if (byteArrayCls == NULL) {
	    goto cleanupAndReturn;
	}
	
	barray = (*env)->NewByteArray(env, 4);
	oarray = (*env)->NewObjectArray(env, 1, byteArrayCls, NULL);

	if (barray == NULL || oarray == NULL) {
	    /* pending exception */
	    goto cleanupAndReturn;
	}
	(*env)->SetByteArrayRegion(env, barray, 0, 4, (jbyte *)&address);
	(*env)->SetObjectArrayElement(env, oarray, 0, barray);

	JNU_ReleaseStringPlatformChars(env, host, hostname);
	return oarray;	
    }

    /*
     * Perform the lookup
     */
    if ((hp = gethostbyname((char*)hostname)) != NULL) {
	struct in_addr **addrp = (struct in_addr **) hp->h_addr_list;
	int len = sizeof(struct in_addr);
	int i = 0;

	while (*addrp != (struct in_addr *) 0) {
	    i++;
	    addrp++;
	}

	byteArrayCls = (*env)->FindClass(env, "[B");
	if (byteArrayCls == NULL) {
	    goto cleanupAndReturn;
	}

        ret = (*env)->NewObjectArray(env, i, byteArrayCls, NULL);
	if (IS_NULL(ret)) {
	    goto cleanupAndReturn;
	}

	addrp = (struct in_addr **) hp->h_addr_list;
	i = 0;
	while (*addrp != (struct in_addr *) 0) {
	    jbyteArray barray = (*env)->NewByteArray(env, len);
	    if (IS_NULL(barray)) {
		JNU_ThrowOutOfMemoryError(env, "lookupAllHostAddr");
		ret = NULL;
		goto cleanupAndReturn;
	    }
	    (*env)->SetByteArrayRegion(env, barray, 0, len, (jbyte *)(*addrp));
	    (*env)->SetObjectArrayElement(env, ret, i, barray);
	    addrp++;
	    i++;
	}
    } else {
        JNU_ThrowByName(env, JNU_JAVANETPKG "UnknownHostException", hostname);
    }

cleanupAndReturn:
    JNU_ReleaseStringPlatformChars(env, host, hostname);
    return ret;
}

/*
 * Class:     java_net_Inet4AddressImpl
 * Method:    getHostByAddr
 * Signature: (I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL
Java_java_net_Inet4AddressImpl_getHostByAddr(JNIEnv *env, jobject this, 
					    jbyteArray addrArray) {
    struct hostent *hp;
    jbyte caddr[4];
    jint addr;
    (*env)->GetByteArrayRegion(env, addrArray, 0, 4, caddr);
    addr = ((caddr[0]<<24) & 0xff000000);
    addr |= ((caddr[1] <<16) & 0xff0000);
    addr |= ((caddr[2] <<8) & 0xff00);
    addr |= (caddr[3] & 0xff); 
    addr = htonl(addr);

    hp = gethostbyaddr((char *)&addr, sizeof(addr), AF_INET);
    if (hp == NULL) {
	JNU_ThrowByName(env, JNU_JAVANETPKG "UnknownHostException", 0);
	return NULL;
    }
    if (hp->h_name == NULL) { /* Deal with bug in Windows XP */
	JNU_ThrowByName(env, JNU_JAVANETPKG "UnknownHostException", 0);
	return NULL;
    }
    return JNU_NewStringPlatform(env, hp->h_name);
}


/**
 * ping implementation.
 * Send a ICMP_ECHO_REQUEST packet every second until either the timeout
 * expires or a answer is received.
 * Returns true is an ECHO_REPLY is received, otherwise, false.
 */
static jboolean
ping4(JNIEnv *env, jint fd, struct sockaddr_in* him, jint timeout,
      struct sockaddr_in* netif, jint ttl) {
    jint size;
    jint n, len, hlen1, icmplen;
    char sendbuf[1500];
    char recvbuf[1500];
    struct icmp *icmp;
    struct ip *ip;
    WSAEVENT hEvent;
    struct sockaddr sa_recv;
    jint tmout2;
    u_short pid, seq=1;
    int read_rv = 0;

    pid = (u_short) getpid();
    size = 60*1024;
    setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (const char *) &size, sizeof(size));
    /**
     * A TTL was specified, let's set the socket option.
     */
    if (ttl > 0) {
      setsockopt(fd, IPPROTO_IP, IP_TTL, (const char *) &ttl, sizeof(ttl));
    }

    /**
     * A network interface was specified, let's bind to it.
     */
    if (netif != NULL) {
      if (bind(fd, (struct sockaddr*)netif, sizeof(struct sockaddr_in)) < 0) {
	NET_ThrowNew(env, WSAGetLastError(), "Can't bind socket");
	closesocket(fd);
	return JNI_FALSE;
      }
    }

    /**
     * Let's make the socket non blocking
     */
    hEvent = WSACreateEvent();
    WSAEventSelect(fd, hEvent, FD_READ|FD_CONNECT|FD_CLOSE);

    /**
     * send 1 ICMP REQUEST every second until either we get a valid reply
     * or the timeout expired.
     */
    do {
      /**
       * construct the ICMP header
       */
      memset(sendbuf, 0, 1500);
      icmp = (struct icmp *) sendbuf;
      icmp->icmp_type = ICMP_ECHO;
      icmp->icmp_code = 0;
      icmp->icmp_id = htons(pid);
      icmp->icmp_seq = htons(seq);
      seq++;
      /**
       * checksum has to be set to zero before we can calculate the
       * real checksum!
       */
      icmp->icmp_cksum = 0;
      icmp->icmp_cksum = in_cksum((u_short *)icmp, 64);
      /**
       * Ping!
       */
      n = sendto(fd, sendbuf, 64, 0, (struct sockaddr *)him,
		 sizeof(struct sockaddr));
      if (n < 0 && WSAGetLastError() != WSAEWOULDBLOCK) {
	NET_ThrowNew(env, WSAGetLastError(), "Can't send ICMP packet");
	closesocket(fd);
	return JNI_FALSE;
      }

      /*
       * wait for 1 second at most
       */
      tmout2 = timeout > 1000 ? 1000 : timeout;
      do {
	tmout2 = NET_Wait(env, fd, NET_WAIT_READ, tmout2);
	if (tmout2 >= 0) {
	  len = sizeof(sa_recv);
	  n = recvfrom(fd, recvbuf, sizeof(recvbuf), 0, &sa_recv, &len);
	  ip = (struct ip*) recvbuf;
	  hlen1 = (ip->ip_hl) << 2;
	  icmp = (struct icmp *) (recvbuf + hlen1);
	  icmplen = n - hlen1;
	  /**
	   * Is that a proper ICMP reply?
	   */
	  if (icmplen >= 8 && icmp->icmp_type == ICMP_ECHOREPLY &&
	      ntohs(icmp->icmp_id) == pid) {
	    closesocket(fd);
	    return JNI_TRUE;
	  }
	}
      } while (tmout2 > 0);
      timeout -= 1000;
    } while (timeout > 0);
    closesocket(fd);
    return JNI_FALSE;
}

/*
 * Class:     java_net_Inet4AddressImpl
 * Method:    isReachable0
 * Signature: ([bI[bI)Z
 */
JNIEXPORT jboolean JNICALL
Java_java_net_Inet4AddressImpl_isReachable0(JNIEnv *env, jobject this,
					   jbyteArray addrArray,
					   jint timeout, 
					   jbyteArray ifArray,
					   jint ttl) {
    jint addr;
    jbyte caddr[4];
    jint fd;
    struct sockaddr_in him;
    struct sockaddr_in* netif = NULL;
    struct sockaddr_in inf;
    int len = 0;
    WSAEVENT hEvent;
    int connect_rv = -1;
    int sz;

    /**
     * Convert IP address from byte array to integer
     */
    sz = (*env)->GetArrayLength(env, addrArray);
    if (sz != 4) {
      return JNI_FALSE;
    }
    memset((char *) &him, 0, sizeof(him));
    (*env)->GetByteArrayRegion(env, addrArray, 0, 4, caddr);
    addr = ((caddr[0]<<24) & 0xff000000);
    addr |= ((caddr[1] <<16) & 0xff0000);
    addr |= ((caddr[2] <<8) & 0xff00);
    addr |= (caddr[3] & 0xff); 
    addr = htonl(addr);
    /**
     * Socket address
     */
    him.sin_addr.s_addr = addr;
    him.sin_family = AF_INET;
    len = sizeof(him);

    /**
     * If a network interface was specified, let's convert its address
     * as well.
     */
    if (!(IS_NULL(ifArray))) {
      (*env)->GetByteArrayRegion(env, ifArray, 0, 4, caddr);
      addr = ((caddr[0]<<24) & 0xff000000);
      addr |= ((caddr[1] <<16) & 0xff0000);
      addr |= ((caddr[2] <<8) & 0xff00);
      addr |= (caddr[3] & 0xff); 
      addr = htonl(addr);
      inf.sin_addr.s_addr = addr;
      inf.sin_family = AF_INET;
      inf.sin_port = 0;
      netif = &inf;
    }

#if 0
    /*
     * Windows implementation of ICMP & RAW sockets is too unreliable for now.
     * Therefore it's best not to try it at all and rely only on TCP
     * We may revisit and enable this code in the future.
     */

    /*
     * Let's try to create a RAW socket to send ICMP packets
     * This usually requires "root" privileges, so it's likely to fail.
     */
    fd = NET_Socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (fd != -1) {
      /*
       * It didn't fail, so we can use ICMP_ECHO requests.
       */
	return ping4(env, fd, &him, timeout, netif, ttl);
    }
#endif

    /*
     * Can't create a raw socket, so let's try a TCP socket
     */
    fd = NET_Socket(AF_INET, SOCK_STREAM, 0);
    if (fd == JVM_IO_ERR) {
	/* note: if you run out of fds, you may not be able to load
	 * the exception class, and get a NoClassDefFoundError
	 * instead.
	 */
	NET_ThrowNew(env, WSAGetLastError(), "Can't create socket");
	return JNI_FALSE;
    }
    if (ttl > 0) {
      setsockopt(fd, IPPROTO_IP, IP_TTL, (const char *)&ttl, sizeof(ttl));
    }
    /*
     * A network interface was specified, so let's bind to it.
     */
    if (netif != NULL) {
      if (bind(fd, (struct sockaddr*)netif, sizeof(struct sockaddr_in)) < 0) {
	NET_ThrowNew(env, WSAGetLastError(), "Can't bind socket");
	closesocket(fd);
	return JNI_FALSE;
      }
    }

    /*
     * Make the socket non blocking so we can use select/poll.
     */
    hEvent = WSACreateEvent();
    WSAEventSelect(fd, hEvent, FD_READ|FD_CONNECT|FD_CLOSE);

    /* no need to use NET_Connect as non-blocking */
    him.sin_port = htons(7);	/* Echo */
    connect_rv = connect(fd, (struct sockaddr *)&him, len);

    /**
     * connection established or refused immediately, either way it means
     * we were able to reach the host!
     */
    if (connect_rv == 0 || WSAGetLastError() == WSAECONNREFUSED) {
	closesocket(fd);
	return JNI_TRUE;
    } else {
	int optlen;

        switch (WSAGetLastError()) {
        case WSAEHOSTUNREACH:	/* Host Unreachable */
        case WSAENETUNREACH:	/* Network Unreachable */
        case WSAENETDOWN:	/* Network is down */
        case WSAEPFNOSUPPORT:	/* Protocol Family unsupported */
	  closesocket(fd);   
	  return JNI_FALSE;
        }

	if (WSAGetLastError() != WSAEWOULDBLOCK) {
	    NET_ThrowByNameWithLastError(env, JNU_JAVANETPKG "ConnectException",
					 "connect failed");
	    closesocket(fd);
	    return JNI_FALSE;
	}

	timeout = NET_Wait(env, fd, NET_WAIT_CONNECT, timeout);
	/* has connection been established */
	closesocket(fd);
	if (timeout >= 0) {
	    return JNI_TRUE;
	}
	return JNI_FALSE;
    }
}
