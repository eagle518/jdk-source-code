/*
 * @(#)WindowsEvent.java	1.5 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.ipc.windows;

import java.util.*;

import sun.plugin2.ipc.*;
import sun.plugin2.os.windows.*;

/** Implementation of the Event abstraction using a true Windows Event
    object underneath. */

public class WindowsEvent extends Event {
    private long handle;
    private String name;

    /** Constructor takes the handle to the Event object and its
        associated name. */
    public WindowsEvent(long handle, String name) {
        this.handle = handle;
        this.name = name;
    }

    public long getEventHandle() {
        return handle;
    }

    public void waitForSignal(long millisToWait) {
        Windows.WaitForSingleObject(handle,
                                    (int) ((millisToWait == 0) ? Windows.INFINITE : millisToWait));
    }

    public void signal() {
        if (handle != 0) {
            Windows.SetEvent(handle);
        }
    }

    public Map getChildProcessParameters() {
        Map ret = new HashMap();
        ret.put("evt_name", name);
        return ret;
    }

    public void dispose() {
        if (handle != 0) {
            Windows.CloseHandle(handle);
            handle = 0;
        }
    }
}
