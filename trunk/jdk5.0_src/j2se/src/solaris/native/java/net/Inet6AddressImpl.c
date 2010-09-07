/*
 * @(#)Inet6AddressImpl.c	1.19 04/01/13
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <ctype.h>

#include "jvm.h"
#include "jni_util.h"
#include "net_util.h"
#ifndef IPV6_DEFS_H
#include <netinet/icmp6.h>
#endif

#include "java_net_Inet4AddressImpl.h"
#include "java_net_Inet6AddressImpl.h"

/* the initial size of our hostent buffers */
#ifndef NI_MAXHOST
#define NI_MAXHOST 1025
#endif 

#ifndef __GLIBC__
/* gethostname() is in libc.so but I can't find a header file for it */
extern int gethostname(char *buf, int buf_len);
#endif

/************************************************************************
 * Inet6AddressImpl
 */

/*
 * Class:     java_net_Inet6AddressImpl
 * Method:    getLocalHostName
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL
Java_java_net_Inet6AddressImpl_getLocalHostName(JNIEnv *env, jobject this) {
    char hostname[NI_MAXHOST+1];

    hostname[0] = '\0';
    if (JVM_GetHostName(hostname, MAXHOSTNAMELEN)) {
	/* Something went wrong, maybe networking is not setup? */
	strcpy(hostname, "localhost");
    } else {
#ifdef __linux__
	/* On Linux gethostname() says "host.domain.sun.com".  On
	 * Solaris gethostname() says "host", so extra work is needed.
	 */
#else
	/* Solaris doesn't want to give us a fully qualified domain name.
	 * We do a reverse lookup to try and get one.  This works
	 * if DNS occurs before NIS in /etc/resolv.conf, but fails
	 * if NIS comes first (it still gets only a partial name).
	 * We use thread-safe system calls.
	 */
#ifdef AF_INET6
        if (NET_addrtransAvailable()) {
	    struct addrinfo  hints, *res;
	    int error;

	    bzero(&hints, sizeof(hints));
	    hints.ai_flags = AI_CANONNAME;
	    hints.ai_family = AF_UNSPEC;

	    error = (*getaddrinfo_ptr)(hostname, "domain", &hints, &res);

	    if (error == 0) {
		/* host is known to name service */
	        error = (*getnameinfo_ptr)(res->ai_addr, 
					   res->ai_addrlen, 
					   hostname, 
					   NI_MAXHOST, 
					   NULL, 
					   0, 
					   NI_NAMEREQD);

		/* if getnameinfo fails hostname is still the value
		   from gethostname */

	        (*freeaddrinfo_ptr)(res);
	    }
	}
#endif /* AF_INET6 */
#endif /* __linux__ */
    }
    return (*env)->NewStringUTF(env, hostname);
}

/*
 * Find an internet address for a given hostname.  Not this this
 * code only works for addresses of type INET. The translation
 * of %d.%d.%d.%d to an address (int) occurs in java now, so the
 * String "host" shouldn't *ever* be a %d.%d.%d.%d string
 *
 * Class:     java_net_Inet6AddressImpl
 * Method:    lookupAllHostAddr
 * Signature: (Ljava/lang/String;)[[B
 */

JNIEXPORT jobjectArray JNICALL
Java_java_net_Inet6AddressImpl_lookupAllHostAddr(JNIEnv *env, jobject this,
						jstring host) {
    const char *hostname;
    jobjectArray ret = 0;
    int retLen = 0;
    jclass byteArrayCls;
    jboolean preferIPv6Address;

    int error=0;
#ifdef AF_INET6
    struct addrinfo hints, *res, *resNew = NULL;
#endif /* AF_INET6 */

    if (IS_NULL(host)) {
	JNU_ThrowNullPointerException(env, "host is null");
	return 0;
    }
    hostname = JNU_GetStringPlatformChars(env, host, JNI_FALSE);
    CHECK_NULL_RETURN(hostname, NULL);

#ifdef AF_INET6
    if (NET_addrtransAvailable()) {
	static jfieldID ia_preferIPv6AddressID;
	if (ia_preferIPv6AddressID == NULL) {
	    jclass c = (*env)->FindClass(env,"java/net/InetAddress");
	    if (c)  {
		ia_preferIPv6AddressID =
                    (*env)->GetStaticFieldID(env, c, "preferIPv6Address", "Z");
	    }
	    if (ia_preferIPv6AddressID == NULL) {
		JNU_ReleaseStringPlatformChars(env, host, hostname);
		return NULL;
	    }
	} 
	/* get the address preference */
	preferIPv6Address 
	    = (*env)->GetStaticBooleanField(env, ia_class, ia_preferIPv6AddressID); 

	/* Try once, with our static buffer. */
	bzero(&hints, sizeof(hints));
	hints.ai_flags = AI_CANONNAME;
	hints.ai_family = AF_UNSPEC;
	
	/* 
	 * Workaround for Solaris bug 4160367 - if a hostname contains a
	 * white space then 0.0.0.0 is returned
	 */
	if (isspace(hostname[0])) {
	    JNU_ThrowByName(env, JNU_JAVANETPKG "UnknownHostException",
			    (char *)hostname);
	    JNU_ReleaseStringPlatformChars(env, host, hostname);
	    return NULL;
	}
	
	error = (*getaddrinfo_ptr)(hostname, "domain", &hints, &res);
	
	if (error) {
	    /* report error */
	    JNU_ThrowByName(env, JNU_JAVANETPKG "UnknownHostException",
			    (char *)hostname);
	    JNU_ReleaseStringPlatformChars(env, host, hostname);
	    return NULL;
	} else {
	    int i = 0;
	    struct addrinfo *itr, *last, *iterator = res;
	    while (iterator != NULL) {
		int skip = 0;
		itr = resNew;
		while (itr != NULL) {
		    if (iterator->ai_family == itr->ai_family &&
			iterator->ai_addrlen == itr->ai_addrlen) {
			if (itr->ai_family == AF_INET) { /* AF_INET */
			    struct sockaddr_in *addr1, *addr2;
			    addr1 = (struct sockaddr_in *)iterator->ai_addr;
			    addr2 = (struct sockaddr_in *)itr->ai_addr;
			    if (addr1->sin_addr.s_addr == 
				addr2->sin_addr.s_addr) {
				skip = 1;
				break;
			    } 
			} else {
			    int t;
			    struct sockaddr_in6 *addr1, *addr2;
			    addr1 = (struct sockaddr_in6 *)iterator->ai_addr;
			    addr2 = (struct sockaddr_in6 *)itr->ai_addr;
			    
			    for (t = 0; t < 16; t++) {
				if (addr1->sin6_addr.s6_addr[t] !=
				    addr2->sin6_addr.s6_addr[t]) {
				    break;
				} 
			    }
			    if (t < 16) {
				itr = itr->ai_next;
				continue;
			    } else {
				skip = 1;
				break;
			    }
			}
		    } else if (iterator->ai_family != AF_INET && 
			       iterator->ai_family != AF_INET6) { 
			/* we can't handle other family types */
			skip = 1;
			break;	
		    }
		    itr = itr->ai_next;
		}
		
		if (!skip) {
		    struct addrinfo *next 
			= (struct addrinfo*) malloc(sizeof(struct addrinfo));
		    if (!next) {
			JNU_ThrowOutOfMemoryError(env, "heap allocation failed");
			ret = NULL;
			goto cleanupAndReturn;
		    }
		    memcpy(next, iterator, sizeof(struct addrinfo));
		    next->ai_next = NULL;
		    if (resNew == NULL) {
			resNew = next;
		    } else {
			last->ai_next = next;
		    }
		    last = next;
		    i++;
		}
		iterator = iterator->ai_next;
	    }
	    retLen = i;
	    iterator = resNew;
	    i = 0;
	    byteArrayCls = (*env)->FindClass(env, "[B");
	    if (byteArrayCls == NULL) {
		/* pending exception */
		goto cleanupAndReturn;
	    }
	    ret = (*env)->NewObjectArray(env, retLen, byteArrayCls, NULL);
	    
	    if (IS_NULL(ret)) {
		/* we may have memory to free at the end of this */
		goto cleanupAndReturn;
	    }

	    while (iterator != NULL) {
		/* if ai_addrlen is 16 bytes, it points to sockaddr_in;
		 * if it is 24 bytes, it points to sockaddr_in6. 
		 * in the former case, we need 4 bytes to store ipv4 address;
		 * in the latter case, we need 16 bytes to store ipv6 address.
		 */
		int len = iterator->ai_addrlen== 16 ? 4:16;
		jbyteArray barray = (*env)->NewByteArray(env, len);
		
		if (IS_NULL(barray)) {
		    /* we may have memory to free at the end of this */
		    ret = NULL;
		    goto cleanupAndReturn;
		}
		if (iterator->ai_family == AF_INET) {
		    (*env)->SetByteArrayRegion(env, barray, 0, len,
					       (jbyte *) &((struct sockaddr_in*)(iterator->ai_addr))->sin_addr);
		} else {
		    (*env)->SetByteArrayRegion(env, barray, 0, len,
					       (jbyte *) ((struct sockaddr_in6*)(iterator->ai_addr))->sin6_addr.s6_addr);
		}
		if (preferIPv6Address) {
		    (*env)->SetObjectArrayElement(env, ret, i, barray);
		} else {
		    (*env)->SetObjectArrayElement(env, ret, retLen - i -1, barray);
		}
		i++;
		iterator = iterator->ai_next;
	    }
	}
    }
    
#endif /* AF_INET6 */
    
cleanupAndReturn:
    {
	struct addrinfo *iterator, *tmp;
	iterator = resNew;
	while (iterator != NULL) {
	    tmp = iterator;
	    iterator = iterator->ai_next;
	    free(tmp);
	}
	JNU_ReleaseStringPlatformChars(env, host, hostname);
    }
   
#ifdef AF_INET6
    if (NET_addrtransAvailable())
	(*freeaddrinfo_ptr)(res);
#endif /* AF_INET6 */
    
    return ret;
}

/*
 * Class:     java_net_Inet6AddressImpl
 * Method:    getHostByAddr
 * Signature: (I)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL
Java_java_net_Inet6AddressImpl_getHostByAddr(JNIEnv *env, jobject this,
					    jbyteArray addrArray) {

    jstring ret = NULL;

#ifdef AF_INET6
    char host[NI_MAXHOST+1];
    jfieldID fid;
    int error = 0;
    jint family;
    struct sockaddr *him ; 
    int len = 0;
    jbyte caddr[16];

    if (NET_addrtransAvailable()) {
	struct sockaddr_in him4;
        struct sockaddr_in6 him6;
	struct sockaddr *sa;

	/* 
	 * For IPv4 addresses construct a sockaddr_in structure.
	 */
	if ((*env)->GetArrayLength(env, addrArray) == 4) {
	    jint addr;
	    (*env)->GetByteArrayRegion(env, addrArray, 0, 4, caddr);
	    addr = ((caddr[0]<<24) & 0xff000000);
    	    addr |= ((caddr[1] <<16) & 0xff0000);
            addr |= ((caddr[2] <<8) & 0xff00);
    	    addr |= (caddr[3] & 0xff);
	    memset((char *) &him4, 0, sizeof(him4));
	    him4.sin_addr.s_addr = (uint32_t) htonl(addr);
	    him4.sin_family = AF_INET;
	    sa = (struct sockaddr *) &him4;
	    len = sizeof(him4);
	} else {
	    /*
	     * For IPv6 address construct a sockaddr_in6 structure.
	     */
	    (*env)->GetByteArrayRegion(env, addrArray, 0, 16, caddr); 
            memset((char *) &him6, 0, sizeof(him6)); 
            memcpy((void *)&(him6.sin6_addr), caddr, sizeof(struct in6_addr) ); 
            him6.sin6_family = AF_INET6; 
            sa = (struct sockaddr *) &him6 ;
            len = sizeof(him6) ; 
        }
    
        error = (*getnameinfo_ptr)(sa, len, host, NI_MAXHOST, NULL, 0,
				   NI_NAMEREQD);

        if (!error) {
	    ret = (*env)->NewStringUTF(env, host);
        }
    }
#endif /* AF_INET6 */

    if (ret == NULL) {
	JNU_ThrowByName(env, JNU_JAVANETPKG "UnknownHostException", NULL);
    }

    return ret;
}

#define SET_NONBLOCKING(fd) {		\
        int flags = fcntl(fd, F_GETFL);	\
        flags |= O_NONBLOCK; 		\
        fcntl(fd, F_SETFL, flags);	\
}

#ifdef AF_INET6
static jboolean
ping6(JNIEnv *env, jint fd, struct sockaddr_in6* him, jint timeout,
      struct sockaddr_in6* netif, jint ttl) {
    jint size;
    jint n, len, hlen1, icmplen;
    char sendbuf[1500];
    unsigned char recvbuf[1500];
    struct icmp6_hdr *icmp6;
    struct ip6_hdr *ip6;
    struct sockaddr_in6 sa_recv;
    jint pid, tmout2, seq = 1;
    int read_rv = 0;

#ifdef __linux__
    {
    int csum_offset;
    /**
     * For some strange reason, the linux kernel won't calculate the
     * checksum of ICMPv6 packets unless you set this socket option
     */
    csum_offset = 2;
    setsockopt(fd, SOL_RAW, IPV6_CHECKSUM, &csum_offset, sizeof(int));
    }
#endif
    pid = getpid();
    size = 60*1024;
    setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
    if (ttl > 0) {
      setsockopt(fd, IPPROTO_IPV6, IPV6_UNICAST_HOPS, &ttl, sizeof(ttl));
    }
    if (netif != NULL) {
      if (bind(fd, (struct sockaddr*)netif, sizeof(struct sockaddr_in6)) <0) {
	NET_ThrowNew(env, errno, "Can't bind socket");
	close(fd);
	return JNI_FALSE;
      }
    }
    SET_NONBLOCKING(fd);

    do {
      icmp6 = (struct icmp6_hdr *) sendbuf;
      icmp6->icmp6_type = ICMP6_ECHO_REQUEST;
      icmp6->icmp6_code = 0;
      /* let's tag the ECHO packet with our pid so we can identify it */
      icmp6->icmp6_id = htons(pid);
      icmp6->icmp6_seq = htons(seq);
      seq++;
      icmp6->icmp6_cksum = 0;
      gettimeofday((struct timeval *) (icmp6 + 1), NULL);
      n = sendto(fd, sendbuf, 64, 0, (struct sockaddr*) him, sizeof(struct sockaddr_in6));
      if (n < 0 && errno != EINPROGRESS) {
	NET_ThrowNew(env, errno, "Can't send ICMP packet");
	return JNI_FALSE;
      }

      tmout2 = timeout > 1000 ? 1000 : timeout;
      do {
	tmout2 = NET_Wait(env, fd, NET_WAIT_READ, tmout2);
	
	if (tmout2 >= 0) {
	  len = sizeof(sa_recv);
	  n = recvfrom(fd, recvbuf, sizeof(recvbuf), 0, (struct sockaddr*) &sa_recv, &len);
	  icmp6 = (struct icmp6_hdr *) (recvbuf);
	  if (n >= 8 && icmp6->icmp6_type == ICMP6_ECHO_REPLY &&
	      icmp6->icmp6_id == ntohs(pid)) {
	    close(fd);
	    return JNI_TRUE;
	  }
	}
      } while (tmout2 > 0);
      timeout -= 1000;
    } while (timeout > 0);
    close(fd);
    return JNI_FALSE;
}
#endif /* AF_INET6 */

/*
 * Class:     java_net_Inet6AddressImpl
 * Method:    isReachable0
 * Signature: ([bII[bI)Z
 */
JNIEXPORT jboolean JNICALL
Java_java_net_Inet6AddressImpl_isReachable0(JNIEnv *env, jobject this,
					   jbyteArray addrArray,
					   jint scope,
					   jint timeout,
					   jbyteArray ifArray,
					   jint ttl, jint if_scope) {
#ifdef AF_INET6
    jint addr;
    jbyte caddr[16];
    jint fd, sz;
    struct sockaddr_in6 him6;
    struct sockaddr_in6 inf6;
    struct sockaddr_in6* netif = NULL;
#ifdef __linux__
    struct sockaddr_in6_ext *him6_ext = (struct sockaddr_in6_ext *)&him6;
    struct sockaddr_in6_ext *if6_ext = NULL;
#endif
    int len = 0;
    int connect_rv = -1;

    /*
     * If IPv6 is not enable, then we can't reach an IPv6 address, can we?
     */
    if (!ipv6_available()) {
      return JNI_FALSE;
    }
    /*
     * If it's an IPv4 address, ICMP won't work with IPv4 mapped address,
     * therefore, let's delegate to the Inet4Address method.
     */
    sz = (*env)->GetArrayLength(env, addrArray);
    if (sz == 4) {
      return Java_java_net_Inet4AddressImpl_isReachable0(env, this,
							 addrArray,
							 timeout,
							 ifArray, ttl);
    }

    memset((char *) caddr, 0, 16);
    memset((char *) &him6, 0, sizeof(him6));
    (*env)->GetByteArrayRegion(env, addrArray, 0, 16, caddr);
    memcpy((void *)&(him6.sin6_addr), caddr, sizeof(struct in6_addr) );
    him6.sin6_family = AF_INET6; 
#ifdef __linux__
    if (scope > 0)
      him6_ext->sin6_scope_id = scope;
    else
      him6_ext->sin6_scope_id = getDefaultIPv6Interface( &(him6.sin6_addr));
    len = sizeof(struct sockaddr_in6_ext);
#else
    if (scope > 0)
      him6.sin6_scope_id = scope;
    len = sizeof(struct sockaddr_in6);
#endif
    /*
     * If a network interface was specified, let's create the address
     * for it.
     */
    if (!(IS_NULL(ifArray))) {
      memset((char *) caddr, 0, 16);
      memset((char *) &inf6, 0, sizeof(inf6));
      (*env)->GetByteArrayRegion(env, ifArray, 0, 16, caddr);
      memcpy((void *)&(inf6.sin6_addr), caddr, sizeof(struct in6_addr) );
      inf6.sin6_family = AF_INET6; 
#ifdef __linux__
      if6_ext = (struct sockaddr_in6_ext *)&inf6;
      if6_ext->sin6_scope_id = if_scope;
#else
      inf6.sin6_scope_id = if_scope;
#endif
      netif = &inf6;
    }
    /*
     * If we can create a RAW socket, then when can use the ICMP ECHO_REQUEST
     * otherwise we'll try a tcp socket to the Echo port (7).
     * Note that this is empiric, and not connecting could mean it's blocked
     * or the echo servioe has been disabled.
     */

    fd = JVM_Socket(AF_INET6, SOCK_RAW, IPPROTO_ICMPV6);

    if (fd != -1) { /* Good to go, let's do a ping */
	return ping6(env, fd, &him6, timeout, netif, ttl);
    }

    /* No good, let's fall back on TCP */
    fd = JVM_Socket(AF_INET6, SOCK_STREAM, 0);
    if (fd == JVM_IO_ERR) {
	/* note: if you run out of fds, you may not be able to load
	 * the exception class, and get a NoClassDefFoundError
	 * instead.
	 */
	NET_ThrowNew(env, errno, "Can't create socket");
	return JNI_FALSE;
    }
    if (ttl > 0) {
      setsockopt(fd, IPPROTO_IPV6, IPV6_UNICAST_HOPS, &ttl, sizeof(ttl));
    }

    /*
     * A network interface was specified, so let's bind to it.
     */
    if (netif != NULL) {
      if (bind(fd, (struct sockaddr*)netif, sizeof(struct sockaddr_in6)) <0) {
	NET_ThrowNew(env, errno, "Can't bind socket");
	close(fd);
	return JNI_FALSE;
      }
    }
    SET_NONBLOCKING(fd);

    /* no need to use NET_Connect as non-blocking */
    him6.sin6_port = htons((short) 7); /* Echo port */
    connect_rv = JVM_Connect(fd, (struct sockaddr *)&him6, len);

    /**
     * connection established or refused immediately, either way it means
     * we were able to reach the host!
     */
    if (connect_rv == 0 || errno == ECONNREFUSED) {
	close(fd);
	return JNI_TRUE;
    } else {
	int optlen;

	switch (errno) {
	case ENETUNREACH: /* Network Unreachable */
	case EAFNOSUPPORT: /* Address Family not supported */
	case EADDRNOTAVAIL: /* address is not available on  the  remote machine */
#ifdef __linux__
	case EINVAL:
	  /*
	   * On some Linuxes, when bound to the loopback interface, connect
	   * will fail and errno will be set to EINVAL. When that happens,
	   * don't throw an exception, just return false.
	   */
#endif /* __linux__ */
	  close(fd);
	  return JNI_FALSE;
	}

	if (errno != EINPROGRESS) {
	    NET_ThrowByNameWithLastError(env, JNU_JAVANETPKG "ConnectException",
					 "connect failed");
	    close(fd);
	    return JNI_FALSE;
	}

	timeout = NET_Wait(env, fd, NET_WAIT_CONNECT, timeout);
	
	if (timeout >= 0) {
	  /* has connection been established */
	  optlen = sizeof(connect_rv);
	  if (JVM_GetSockOpt(fd, SOL_SOCKET, SO_ERROR, (void*)&connect_rv, 
			     &optlen) <0) {
	    connect_rv = errno;
	  }
	  if (connect_rv == 0 || ECONNREFUSED) {
	    close(fd);
	    return JNI_TRUE;
	  }
	}
	close(fd);
	return JNI_FALSE;
    }
#else /* AF_INET6 */
    return JNI_FALSE;
#endif /* AF_INET6 */
}
