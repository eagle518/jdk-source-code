/*
 * @(#)IPCFactory.java	1.6 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.ipc;

import java.security.*;
import java.util.*;

import sun.plugin2.util.SystemUtil;

/** A factory for creating IPC related objects like named pipes and
    cross-process signalling primitives. */

public abstract class IPCFactory {
    public static final int KB = 1024;
    public static final int PIPE_BUF_SZ = 4 * KB;

    private static IPCFactory instance;
    protected IPCFactory() {}

    /** Fetches the sole instance of the IPCFactory. */
    public static IPCFactory getFactory() {
        if (instance == null) {
            try {
                String factoryClassName;
                switch (SystemUtil.getOSType()) {
                    case SystemUtil.WINDOWS:
                        factoryClassName = "sun.plugin2.ipc.windows.WindowsIPCFactory";
                        break;
                    case SystemUtil.UNIX:
                    case SystemUtil.MACOSX:
                        factoryClassName = "sun.plugin2.ipc.unix.UnixIPCFactory";
                        break;
                    default:
                        throw new RuntimeException("Unknown OS type from SystemUtil.getOSType()");
                }
                instance = (IPCFactory) Class.forName(factoryClassName).newInstance();
            } catch (Exception e) {
                if (e instanceof RuntimeException)
                    throw (RuntimeException) e;
                else
                    throw new RuntimeException(e);
            }
        }
        return instance;
    }

    /** Creates an Event object with the specified parameters. The
        keys and values are platform-dependent. The following table
        specifies which platforms look for which keys and values of
        what types. Note that other code in the system assumes that
        none of the keys or values contain the ',' or '='
        characters. <P>
<PRE>
WINDOWS OS
----------

On the Windows family of OSs, when creating the event in the parent
process, the map is optional and may be null. The created event will
be named in a way that a child process can open it.

When accessing the event in the child process, the name of the event
should be passed in using the evt_name key in the parameter map.

key          type            value type
---          ----            ----------
REQUIRED ONLY IN CHILD PROCESS:
evt_name     String          String (name of the event)
</PRE>
    */

    public abstract Event createEvent(Map parameters);

    /** Creates a NamedPipe object with the specified parameters. The
        keys and values are platform-dependent. The following table
        specifies which platforms look for which keys and values of
        what types. Note that other code in the system assumes that
        none of the keys or values contain the ',' or '='
        characters. <P>

<PRE>
WINDOWS OS
----------

On the Windows family of OSs, when creating the named pipe in the
parent process, the map is optional and may be null. The created named
pipe will be named in a way that a child process can open it.

When accessing the named pipe in the child process, the name of the
named pipe should be passed in using the pipe_name key in the
parameter map.

key          type            value type
---          ----            ----------
REQUIRED ONLY IN CHILD PROCESS:
pipe_name    String          String (name of the named pipe)
</PRE>
    */

    public abstract NamedPipe createNamedPipe(Map parameters);

    /** Helper function which turns a parameter map into a String for
        passing on the command-line to a subprocess. */
    public static String mapToString(Map/*<String,String>*/ map) {
        // Turns a map containing for example
        //        key        val
        //        shm_size   65536
        //        shm_name   jpi2_pid345_shm3
        // Into a string containing
        // shm_size=65536,shm_name=jpi2_pid345_shm3
        StringBuffer buf = new StringBuffer();
        boolean first = true;
        for (Iterator iter = map.keySet().iterator(); iter.hasNext(); ) {
            String key = (String) iter.next();
            if (first) {
                first = false;
            } else {
                buf.append(",");
            }
            buf.append(key);
            buf.append("=");
            buf.append((String) map.get(key));
        }
        return buf.toString();
    }

    /** Helper function which turns a String back into a parameter map
        after being passed on the command-line to this process. */
    public static Map/*<String,String>*/ stringToMap(String inputString) {
        // Turns a string containing for example
        // shm_size=65536,shm_name=jpi2_pid345_shm3
        // Into a map containing
        //        key        val
        //        shm_size   65536
        //        shm_name   jpi2_pid345_shm3
        Map/*<String,String>*/ map = new HashMap/*<String,String>*/();
        String[] strs = inputString.split(",");
        for (int i = 0; i < strs.length; i++) {
            String[] kv = strs[i].split("=");
            map.put(kv[0], kv[1]);
        }
        return map;
    }
}
