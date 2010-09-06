/*
 * @(#)ConnectionParameters.java	1.3 04/06/02
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.tools.jconsole;

import javax.management.remote.JMXServiceURL;

import java.util.Map;

/**
 * URL connections from command line.
 */
class ConnectionParameters {
    JMXServiceURL jmxUrl;
    Map map;
}
