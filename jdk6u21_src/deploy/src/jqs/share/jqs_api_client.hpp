/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef JQS_API_CLIENT_HPP
#define JQS_API_CLIENT_HPP


#include "jqs_api.hpp"

/*
 * Establishes connection to JQS API server and sends given command.
 * Returns true on success.
 */
extern bool sendJQSAPICommand(JQSAPIMessageKind cmd);

#endif
