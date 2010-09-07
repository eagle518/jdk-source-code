/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef JQS_API_HPP
#define JQS_API_HPP

#include "jqs.hpp"
#include <vector>

/*
 * This module contains classes and data common for both JQS API client 
 * and server sides.
 */

#define JQS_API_PORT                    5152
#define JQS_API_TIMEOUT                 1000        /* 1 sec */

#define JQS_API_MESSAGE_MAGIC           0x4a51534d  /* JQSM */

#define JQS_API_MESSAGE_HEADER_SIZE     12
#define JQS_API_MESSAGE_MAX_DATA_LEN    256


/*
 * Supported JQS API messages' kinds.
 */
enum JQSAPIMessageKind {
    JMK_Unknown,
    JMK_Pause,
    JMK_Resume,
    JMK_Notify,
    JMK_JQSVersionRequest,
    JMK_JQSVersionResponse,
    JMK_JavaVersionRequest,
    JMK_JavaVersionResponse,
    JMK_CapabilitiesRequest,
    JMK_CapabilitiesResponse,
};

/*
 * Meaning of the bits sent by the server in JMK_CapabilitiesResponse message.
 */
enum JQSCapabilityBits {
    JCB_AdjustWorkingSetSize        = 0x00000001,
    JCB_LockMemoryPages             = 0x00000002,
    JCB_LargePages                  = 0x00000004,
    JCB_LowMemoryNotifications      = 0x00000008,
    JCB_SetLibraryPath              = 0x00000010,
    JCB_CheckPowerStatus            = 0x00000020,
    JCB_DeviceEventNotifications    = 0x00000040,
    JCB_UserLogonNotifications      = 0x00000080,
};


/*
 * JQS API message wrapper class responsible for proper message construction.
 */
class JQSAPIMessage {
    std::vector<char> buf;

public:
    JQSAPIMessage (JQSAPIMessageKind kind, uint32_t datalen = 0) {
        append (JQS_API_MESSAGE_MAGIC);
        append (kind);
        append (datalen);
    }

    void append(uint32_t x) {
        // big-endian order
        buf.push_back ((char)(x >> 24));
        buf.push_back ((char)(x >> 16));
        buf.push_back ((char)(x >>  8));
        buf.push_back ((char)(x >>  0));
    }

    void append(const char* s) {
        size_t len = strlen (s);
        buf.insert (buf.end (), s, s+len);
    }

    const std::vector<char>& getBytes () const { return buf; }
};

#endif
