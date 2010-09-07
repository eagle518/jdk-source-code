/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef MESSAGES_HPP
#define MESSAGES_HPP


/*
 * Existing JQS messages.
 */
enum MessageID {
    MSG_Unknown, 
    MSG_JQSUsage,
    MSG_JQSRegistered,
    MSG_JQSRegisterFailed,
    MSG_JQSUnregistered,
    MSG_JQSUnregisterFailed,
    MSG_JQSEnabled,
    MSG_JQSEnableFailed,
    MSG_JQSDisabled,
    MSG_JQSDisableFailed,
    MSG_JQSPaused,
    MSG_JQSPauseFailed,
    MSG_JQSResumed,
    MSG_JQSResumeFailed,
    MSG_JQSRequiresAdminPrivileges,
};


/*
 * Defines the relative path to the directory with message files.
 */
#define MESSAGES_DIR_RELATIVE_LOCATION  JAVA_HOME_RELATIVE_TO_JQS "/lib/deploy/jqs"


/*
 * Returns message string by given message ID, taking into account system locale.
 */
extern const char* getMsgString(MessageID messageID);

#endif
