/*
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef JQS_API_SERVER_HPP
#define JQS_API_SERVER_HPP

#include "jqs_api.hpp"

/*
 * Starts JQS API server thread which listens for the incoming connections
 * on JQS_API_PORT and starts the communication thread for each connection
 * occurred.
 * Returns true on success. All error messages are reported inside on failures.
 */
extern bool startJQSAPIServer();

/*
 * Stops all active communication threads and the server thread.
 */
extern void stopJQSAPIServer();

#endif
