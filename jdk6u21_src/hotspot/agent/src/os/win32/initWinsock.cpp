/*
 * Copyright (c) 2000, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

#include <iostream>
#include <winsock2.h>

using namespace std;

void
initWinsock()
{
  static int initted = 0;
  WORD wVersionRequested;
  WSADATA wsaData;
  int err;

  if (!initted) {
    wVersionRequested = MAKEWORD( 2, 0 );

    err = WSAStartup( wVersionRequested, &wsaData );
    if ( err != 0 ) {
      {
        /* Tell the user that we couldn't find a usable */
        /* WinSock DLL.                                 */
        cerr << "SocketBase::SocketBase: unable to find usable "
             << "WinSock DLL" << endl;
        exit(1);
      }
    }

    /* Confirm that the WinSock DLL supports 2.0.*/
    /* Note that if the DLL supports versions greater    */
    /* than 2.0 in addition to 2.0, it will still return */
    /* 2.0 in wVersion since that is the version we      */
    /* requested.                                        */

    if ( LOBYTE( wsaData.wVersion ) != 2 ||
         HIBYTE( wsaData.wVersion ) != 0 ) {
      /* Tell the user that we couldn't find a usable */
      /* WinSock DLL.                                  */
      {
        cerr << "Unable to find suitable version of WinSock DLL" << endl;
        WSACleanup( );
        exit(1);
      }
    }

    initted = 1;
  }
}
