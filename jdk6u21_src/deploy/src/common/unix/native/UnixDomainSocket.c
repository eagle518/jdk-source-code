/*
 * @(#)UnixDomainSocket.c	1.5 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 * Native implementation of the Unix Domain Socket part
 * for com.sun.deploy.net.socket.UnixSocketImpl
 */

#include <errno.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h>

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

#ifndef SUN_LEN
# define SUN_LEN(ptr) ((size_t) (((struct sockaddr_un *) 0)->sun_path)        \
              + strlen ((ptr)->sun_path))
#endif

#include <jni.h>

#include "com_sun_deploy_net_socket_UnixSocketImpl.h"

#ifndef CHECK_NULL
# define CHECK_NULL(x) if ((x) == NULL) return;
#endif
#ifndef CHECK_NULL_RETURN
# define CHECK_NULL_RETURN(x, y) if ((x) == NULL) return y;
#endif

/** internal representation of
 *  the socket_un handle.
 */
typedef struct _jni_socket_un {
    uint64_t magic_id;         /* magic RTTI type signature */
    struct sockaddr_un addr;   /* the socket addr */
    unsigned int addrLen;      /* the socket addr len */
    int    fd;                 /* the socket file descriptor */ 
} jni_socket_un_t;

static const uint64_t UN_SOCK_MAGIC_ID = 0xFACE1010UL;

/** 
 * release all ressources ..
 */
static void _freeJNISocketUN(jni_socket_un_t* p) {
    if(NULL==p || UN_SOCK_MAGIC_ID!=p->magic_id) {
        return;
    }
    p->magic_id=0UL; // clear the magic id!
    free(p);
}

static const char * const ClazzNameRuntimeException = 
                            "java/lang/RuntimeException";
static jclass    runtimeExceptionClz=NULL;

static const char * const ClazzNameIllegalArgumentException = 
                            "java/lang/IllegalArgumentException";
static jclass    illegalArgumentExceptionClz=NULL;

static const char * const ClazzNameUnixDomainSocketException = 
                            "com/sun/deploy/net/socket/UnixDomainSocketException";
static const char * const ClazzNameUnixDomainSocketExceptionStaticCstrName = 
                            "createUnixDomainSocketException";
static const char * const ClazzNameUnixDomainSocketExceptionStaticCstrSignature = 
                            "(Ljava/lang/String;I)Lcom/sun/deploy/net/socket/UnixDomainSocketException;";
static jclass    unixDomainSocketExceptionClz=NULL;
static jmethodID unixDomainSocketExceptionCstr=NULL;

static void _initStatics(JNIEnv *env)
{
    if (runtimeExceptionClz != NULL) {
        return;
    }

    if (runtimeExceptionClz == NULL) {
        jclass c = (*env)->FindClass(env, ClazzNameRuntimeException);
        if(NULL==c) {
            fprintf(stderr, "FatalError: Java_com_sun_deploy_net_socket_UnixSocket: can't find %s\n", ClazzNameRuntimeException);
            (*env)->FatalError(env, ClazzNameRuntimeException);
        }
        runtimeExceptionClz = (jclass)(*env)->NewGlobalRef(env, c);
        if(NULL==runtimeExceptionClz) {
            fprintf(stderr, "FatalError: Java_com_sun_deploy_net_socket_UnixSocket: can't use %s\n", ClazzNameRuntimeException);
            (*env)->FatalError(env, ClazzNameRuntimeException);
        }
    }

    if (illegalArgumentExceptionClz == NULL) {
        jclass c = (*env)->FindClass(env, ClazzNameIllegalArgumentException);
        if(NULL==c) {
            fprintf(stderr, "FatalError: Java_com_sun_deploy_net_socket_UnixSocket: can't find %s\n", ClazzNameIllegalArgumentException);
            (*env)->FatalError(env, ClazzNameIllegalArgumentException);
        }
        illegalArgumentExceptionClz = (jclass)(*env)->NewGlobalRef(env, c);
        if(NULL==illegalArgumentExceptionClz) {
            fprintf(stderr, "FatalError: Java_com_sun_deploy_net_socket_UnixSocket: can't use %s\n", ClazzNameIllegalArgumentException);
            (*env)->FatalError(env, ClazzNameIllegalArgumentException);
        }
    }

    if (unixDomainSocketExceptionClz == NULL) {
        jclass c = (*env)->FindClass(env, ClazzNameUnixDomainSocketException);
        if(NULL==c) {
            fprintf(stderr, "FatalError: Java_com_sun_deploy_net_socket_UnixSocket: can't find %s\n", ClazzNameUnixDomainSocketException);
            (*env)->FatalError(env, ClazzNameUnixDomainSocketException);
        }
        unixDomainSocketExceptionClz = (jclass)(*env)->NewGlobalRef(env, c);
        if(NULL==unixDomainSocketExceptionClz) {
            fprintf(stderr, "FatalError: Java_com_sun_deploy_net_socket_UnixSocket: can't use %s\n", ClazzNameUnixDomainSocketException);
            (*env)->FatalError(env, ClazzNameUnixDomainSocketException);
        }
        unixDomainSocketExceptionCstr =  (*env)->GetStaticMethodID(env, unixDomainSocketExceptionClz, 
                                                ClazzNameUnixDomainSocketExceptionStaticCstrName,
                                                ClazzNameUnixDomainSocketExceptionStaticCstrSignature);
        if(NULL==unixDomainSocketExceptionCstr) {
            fprintf(stderr, "FatalError: Java_com_sun_deploy_net_socket_UnixSocket: can't use %s.%s %s\n", 
                ClazzNameUnixDomainSocketException, 
                ClazzNameUnixDomainSocketExceptionStaticCstrName, ClazzNameUnixDomainSocketExceptionStaticCstrSignature);
            (*env)->FatalError(env, ClazzNameUnixDomainSocketException);
        }
    }
}

static void _throwNewRuntimeException(JNIEnv *env, const char* msg)
{
    (*env)->ThrowNew(env, runtimeExceptionClz, msg);
}

static void _throwNewIllegalArgumentException(JNIEnv *env, const char* msg)
{
    (*env)->ThrowNew(env, illegalArgumentExceptionClz, msg);
}

static void _throwNewUnixDomainSocketException(JNIEnv *env, const char* msg, int errornumber)
{
    jobject unixDomainSocketException = NULL;
    jstring jmsg = (*env)->NewStringUTF(env, msg);
    if(NULL==jmsg) {
        _throwNewRuntimeException(env, msg);
    }
    unixDomainSocketException = (*env)->CallStaticObjectMethod(env, 
                unixDomainSocketExceptionClz, unixDomainSocketExceptionCstr, jmsg, errornumber);
    if(NULL==unixDomainSocketException) {
        _throwNewRuntimeException(env, msg);
    }
    if( (*env)->Throw(env, unixDomainSocketException) < 0 ) {
        _throwNewRuntimeException(env, msg);
    }
}

static jni_socket_un_t* _getUnSocketByHandle(JNIEnv *env, jlong unSocketHandle)
{
    jni_socket_un_t * _socket = NULL;
    if(0==unSocketHandle) {
        _throwNewIllegalArgumentException(env, "unSocketHandle is null");
        return NULL;
    }
    _socket = (jni_socket_un_t*)((void *)((intptr_t)unSocketHandle));
    if(UN_SOCK_MAGIC_ID!=_socket->magic_id) {
        fprintf(stderr, "Error: unidentified unSocket: %p: magic %lX != has %lX\n", 
            _socket, (unsigned long)UN_SOCK_MAGIC_ID, (unsigned long)_socket->magic_id);
        _throwNewIllegalArgumentException(env, "unSocketHandle does not reference unSocket");
        return NULL;
    }
    return _socket;
}

static jlong _createUnSocketHandleByFilename(JNIEnv *env, jstring fileName, jboolean abstractNamespace)
{
  jlong socketHandle;
  jni_socket_un_t* _socket;
  const char *_fileName;

  /* create our socket object, copy the filename, and tag it valid with the RTTI id.*/
  _socket = (jni_socket_un_t*) malloc(sizeof(*_socket));
  if(NULL==_socket) {
    _throwNewRuntimeException(env, strerror(ENOMEM));
    return 0;
  } 
  memset((void *)_socket, 0, sizeof(*_socket));
  socketHandle = (jlong)((intptr_t)((void *)_socket));

  _fileName = (*env)->GetStringUTFChars(env, fileName, 0);
  if(NULL==_fileName) {
    free(_socket);
    _throwNewIllegalArgumentException(env, "fileName invalid");
    return 0;
  }

  /* setup our socket object's socket addr */
  memset(&_socket->addr, 0, sizeof(_socket->addr));
  _socket->addr.sun_family = AF_UNIX;
  if(JNI_TRUE==abstractNamespace) {
      // abstract namespace + rest of filename
      _socket->addr.sun_path[0] = '\0'; 
      strncpy (_socket->addr.sun_path+1, _fileName, sizeof(_socket->addr.sun_path)-2);
  } else {
      strncpy (_socket->addr.sun_path, _fileName, sizeof(_socket->addr.sun_path)-1);
  }

  (*env)->ReleaseStringUTFChars(env, fileName, _fileName);

  _socket->addrLen = SUN_LEN (&_socket->addr);

  _socket->magic_id = UN_SOCK_MAGIC_ID;

  return socketHandle;
}

static jlong _createUnSocketHandleByUnSocket(JNIEnv *env, jni_socket_un_t* _socketServer)
{
  jlong socketHandleClient;
  jni_socket_un_t *_socketClient;

  /* create our client socket object, copy the server addr, copy the filename, 
     and tag it valid with the RTTI id.*/
  _socketClient = (jni_socket_un_t*) malloc(sizeof(*_socketClient));
  if(NULL==_socketClient) {
    _throwNewRuntimeException(env, strerror(ENOMEM));
    return 0;
  }
  memset((void *)_socketClient, 0, sizeof(*_socketClient));
  socketHandleClient = (jlong)((intptr_t)((void *)_socketClient));

  _socketClient->addrLen=sizeof(struct sockaddr_un);
  memcpy(&(_socketClient->addr), &(_socketServer->addr), _socketServer->addrLen);

  _socketClient->magic_id = _socketServer->magic_id;

  return socketHandleClient;
}

JNIEXPORT jlong JNICALL Java_com_sun_deploy_net_socket_UnixSocketImpl_unStreamSocketCreate
  (JNIEnv *env, jclass _unused, jstring fileName, jboolean abstractNamespace, jint protocol)
{
  jlong socketHandle;
  jni_socket_un_t* _socket;

  _initStatics(env);

  /* create & init socket, and verify the type cast */
  socketHandle = _createUnSocketHandleByFilename(env, fileName, abstractNamespace);
  if(0==socketHandle) return 0;
  _socket = _getUnSocketByHandle(env, socketHandle);
  if(NULL==_socket) return 0;

  /* open the socket: Full duplex reliable byte streams */
  if( (_socket->fd = socket(AF_UNIX, SOCK_STREAM, protocol)) < 0 ) {   
      _throwNewUnixDomainSocketException(env, strerror(errno), errno);
      _freeJNISocketUN(_socket);
      return 0;
  }

  return socketHandle;
}

JNIEXPORT void JNICALL Java_com_sun_deploy_net_socket_UnixSocketImpl_unStreamSocketClose
  (JNIEnv *env, jclass _unused, jlong unSocketHandle)
{
  int res;
  jni_socket_un_t* _socket = NULL;
  _initStatics(env);
  _socket = _getUnSocketByHandle(env, unSocketHandle);
  if(NULL==_socket) return;

  fsync(_socket->fd); /* just to make sure, last chance .. */
  res = close(_socket->fd);

  _freeJNISocketUN(_socket);

  if( res < 0 ) {
      _throwNewUnixDomainSocketException(env, strerror(errno), errno);
  } 
}

JNIEXPORT jboolean JNICALL Java_com_sun_deploy_net_socket_UnixSocketImpl_unStreamSocketIsValid
  (JNIEnv *env, jclass _unused, jlong unSocketHandle)
{
  jni_socket_un_t* _socket = NULL;
  _initStatics(env);
  _socket = _getUnSocketByHandle(env, unSocketHandle);
  if(NULL==_socket) return JNI_FALSE;

  int soType=0;
  socklen_t soTypeLen=sizeof(soType);

  if( getsockopt(_socket->fd, SOL_SOCKET, SO_TYPE, (void *)&soType, &soTypeLen) < 0 ) {
      _throwNewUnixDomainSocketException(env, strerror(errno), errno);
  }
  return (SOCK_STREAM==soType)?JNI_TRUE:JNI_FALSE;
}

JNIEXPORT void JNICALL Java_com_sun_deploy_net_socket_UnixSocketImpl_unStreamSocketBind
  (JNIEnv *env, jclass _unused, jlong unSocketHandle)
{
  jni_socket_un_t* _socket = NULL;
  _initStatics(env);
  _socket = _getUnSocketByHandle(env, unSocketHandle);
  if(NULL==_socket) return;

  if( bind(_socket->fd, (struct sockaddr *)&(_socket->addr), _socket->addrLen ) < 0 ) {
      _throwNewUnixDomainSocketException(env, strerror(errno), errno);
  } 
}

JNIEXPORT void JNICALL Java_com_sun_deploy_net_socket_UnixSocketImpl_unStreamSocketListen
  (JNIEnv *env, jclass _unused, jlong unSocketHandle, jint backlog)
{
  jni_socket_un_t* _socket = NULL;
  _initStatics(env);
  _socket = _getUnSocketByHandle(env, unSocketHandle);
  if(NULL==_socket) return;

  if( listen(_socket->fd, backlog) < 0 ) {
      _throwNewUnixDomainSocketException(env, strerror(errno), errno);
  } 
}

JNIEXPORT jlong JNICALL Java_com_sun_deploy_net_socket_UnixSocketImpl_unStreamSocketAccept
  (JNIEnv *env, jclass _unused, jlong unSocketHandleServer)
{
  jni_socket_un_t* _socketServer;
  jlong             socketHandleClient;
  jni_socket_un_t* _socketClient;
  _initStatics(env);
  _socketServer = _getUnSocketByHandle(env, unSocketHandleServer);
  if(NULL==_socketServer) return 0;

  /* create & init client socket, and verify the type cast */
  socketHandleClient  = _createUnSocketHandleByUnSocket(env, _socketServer);
  if(0==socketHandleClient) return 0;
  _socketClient = _getUnSocketByHandle(env, socketHandleClient);
  if(NULL==_socketClient) return 0;

  /* wait for client to connect .. */
  _socketClient->fd = accept( _socketServer->fd, 
                              (struct sockaddr *) &(_socketClient->addr), 
                              &(_socketClient->addrLen));
  if( _socketClient->fd < 0 ) {
      _throwNewUnixDomainSocketException(env, strerror(errno), errno);
      _freeJNISocketUN(_socketClient);
      return 0;
  }

  return socketHandleClient;
}

JNIEXPORT void JNICALL Java_com_sun_deploy_net_socket_UnixSocketImpl_unStreamSocketConnect
  (JNIEnv *env, jclass _unused, jlong unSocketHandle)
{
  jni_socket_un_t* _socket = NULL;
  _initStatics(env);
  _socket = _getUnSocketByHandle(env, unSocketHandle);
  if(NULL==_socket) return;

  if( connect(_socket->fd, (struct sockaddr *)&(_socket->addr), _socket->addrLen ) < 0 ) {
      _throwNewUnixDomainSocketException(env, strerror(errno), errno);
  }
}

JNIEXPORT jint JNICALL Java_com_sun_deploy_net_socket_UnixSocketImpl_unStreamSocketRead(
                         JNIEnv *env, jclass _unused,
                         jlong unSocketHandle, jobject buffer, jint offset, jint count) 
{
  ssize_t _n;
  char * _buffer;
  jni_socket_un_t* _socket = NULL;
  _initStatics(env);
  _socket = _getUnSocketByHandle(env, unSocketHandle);
  if(NULL==_socket) return -1;

  _buffer = (char*) (*env)->GetDirectBufferAddress(env, buffer);
  if(NULL==_buffer) {
      _throwNewIllegalArgumentException(env, "buffer invalid direct buffer");
  }
  _buffer += offset;
  _n = read(_socket->fd, (void *)_buffer, (size_t)count);
  if( _n < 0 ) {
      _throwNewUnixDomainSocketException(env, strerror(errno), errno);
  } 
  return (jint)_n;
}

JNIEXPORT jint JNICALL Java_com_sun_deploy_net_socket_UnixSocketImpl_unStreamSocketWrite(
                         JNIEnv *env, jclass _unused,
                         jlong unSocketHandle, jobject buffer, jint offset, jint count) 
{
  ssize_t _n;
  char * _buffer;
  jni_socket_un_t* _socket = NULL;
  _initStatics(env);
  _socket = _getUnSocketByHandle(env, unSocketHandle);
  if(NULL==_socket) return -1;

  _buffer = (char*) (*env)->GetDirectBufferAddress(env, buffer);
  if(NULL==_buffer) {
      _throwNewIllegalArgumentException(env, "buffer invalid direct buffer");
  }
  _buffer += offset;
  _n = write(_socket->fd, (void *)_buffer, (size_t)count);
  if( _n < 0 ) {
      _throwNewUnixDomainSocketException(env, strerror(errno), errno);
  } 
  return (jint)_n;
}

JNIEXPORT jstring JNICALL Java_com_sun_deploy_net_socket_UnixSocketImpl_unStreamSocketGetNativeInfo
  (JNIEnv *env, jclass _unused, jlong unSocketHandle)
{
  jstring res;
  jni_socket_un_t* _socket = NULL;
  char buffer[256];
  socklen_t intTypeLen, timeValTypeLen;
  int soType=-1; // SO_TYPE -> SOCK_STREAM
  int soAcceptConnection=-1; // SO_ACCEPTCONN: 0: not listening, 1: listening
  int soRecvBuffSz=-1; // SO_RCVBUF
  int soSndBuffSz=-1; // SO_SNDBUF
  struct timeval soTOVal; // SO_RCVTIMEO, SO_SNDTIMEO
  uint64_t soRecTO_ms=0; // SO_RCVTIMEO
  uint64_t soSndTO_ms=0; // SO_SNDTIMEO

  _initStatics(env);
  _socket = _getUnSocketByHandle(env, unSocketHandle);
  if(NULL==_socket) return NULL;

  intTypeLen=sizeof(int);
  if( getsockopt(_socket->fd, SOL_SOCKET, SO_TYPE, (void *)&soType, &intTypeLen) < 0 ) {
      fprintf(stderr, "Error(%d): getsockopt(SO_TYPE): %p: fd %d\n", errno, _socket, _socket->fd);
  }
  intTypeLen=sizeof(int);
  if( getsockopt(_socket->fd, SOL_SOCKET, SO_ACCEPTCONN, (void *)&soAcceptConnection, &intTypeLen) < 0 ) {
      fprintf(stderr, "Error(%d): getsockopt(SO_ACCEPTCONN): %p: fd %d\n", errno, _socket, _socket->fd);
  }
  intTypeLen=sizeof(int);
  if( getsockopt(_socket->fd, SOL_SOCKET, SO_RCVBUF, (void *)&soRecvBuffSz, &intTypeLen) < 0 ) {
      fprintf(stderr, "Error(%d): getsockopt(SO_RCVBUF): %p: fd %d\n", errno, _socket, _socket->fd);
  }
  intTypeLen=sizeof(int);
  if( getsockopt(_socket->fd, SOL_SOCKET, SO_SNDBUF, (void *)&soSndBuffSz, &intTypeLen) < 0 ) {
      fprintf(stderr, "Error(%d): getsockopt(SO_SNDBUF): %p: fd %d\n", errno, _socket, _socket->fd);
  }
  timeValTypeLen=sizeof(struct timeval);
  if( getsockopt(_socket->fd, SOL_SOCKET, SO_RCVTIMEO, (void *)&soTOVal, &timeValTypeLen) < 0 ) {
      fprintf(stderr, "Error(%d): getsockopt(SO_RCVTIMEO): %p: fd %d\n", errno, _socket, _socket->fd);
  }
  soRecTO_ms = soTOVal.tv_sec*1000UL + soTOVal.tv_usec/1000UL;
  timeValTypeLen=sizeof(struct timeval);
  if( getsockopt(_socket->fd, SOL_SOCKET, SO_SNDTIMEO, (void *)&soTOVal, &timeValTypeLen) < 0 ) {
      fprintf(stderr, "Error(%d): getsockopt(SO_SNDTIMEO): %p: fd %d\n", errno, _socket, _socket->fd);
  }
  soSndTO_ms = soTOVal.tv_sec*1000UL + soTOVal.tv_usec/1000UL;

  snprintf(buffer, sizeof(buffer)-1, "type %d, accept %d, rcvBufSz %d, sndBufSz %d, rcvTO %lums, sndTO %lums",
    soType, soAcceptConnection, soRecvBuffSz, soSndBuffSz, (unsigned long)soRecTO_ms, (unsigned long)soSndTO_ms);
  buffer[sizeof(buffer)-1] = '\0' ; // EOS .. safeguard

  res = (*env)->NewStringUTF(env, buffer);
  if(NULL==res) {
    _throwNewRuntimeException(env, strerror(ENOMEM));
  }
  return res;
}

