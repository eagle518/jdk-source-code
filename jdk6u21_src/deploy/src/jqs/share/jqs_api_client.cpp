/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#include <vector>

#include "jqs_api_client.hpp"
#include "sockets.hpp"
#include "thread.hpp"
#include "os_defs.hpp"
#include "utils.hpp"
#include "service.hpp"
#include "prefetch.hpp"

using namespace std;

/*
 * Establishes connection to JQS API server and sends given command.
 * Returns true on success.
 */
bool sendJQSAPICommand(JQSAPIMessageKind cmd) {
    assert((cmd == JMK_Pause) || (cmd == JMK_Resume) || (cmd == JMK_Notify));

    try {
        Connection conn(JQS_API_PORT);
        conn.setTimeout (JQS_API_TIMEOUT);

        JQSAPIMessage msg (cmd);
        conn.sendMessage(msg.getBytes ());

    } catch (const ConnectionRefusedException&) {
        jqs_error ("Unable to connect to JQS service: connection refused\n");
        return false;

    } catch (const SocketException& e) {
        jqs_error("Unable to send message to JQS service: %s (Socket error %d)\n",
                  e.getMessage(), e.getSocketError());
        return false;
    }

    return true;
}
