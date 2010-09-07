/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include "os_layer.hpp"
#include "os_utils.hpp"
#include "jqs.hpp"
#include "jqs_api_client.hpp"
#include "sockets.hpp"


/*
 * The JQS notification utility entry point.
 */
int WINAPI WinMain(HINSTANCE hInstance, 
                   HINSTANCE hPrevInstance, 
                   LPSTR lpCmdLine, 
                   int nCmdShow) 
{
    TRY {
        initSocketLibrary ();
        sendJQSAPICommand (JMK_Notify);
        cleanupSocketLibrary ();

    } CATCH_SYSTEM_EXCEPTIONS {
        jqs_exit(1);
    }
}

