/*
 * @(#)XPane.java	1.4 04/04/09
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.tools.jconsole.inspector;

import javax.management.MBeanInfo;
import java.io.IOException;

public abstract class XPane extends XTabbedPane {

    public abstract void init();
    public abstract void load(XMBean mbean, 
			      MBeanInfo mbeanInfo) throws Exception;
    public abstract void clear();
    public abstract void refresh() throws IOException;

}
