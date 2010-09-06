/*
 * @(#)AWTSecurityManager.java	1.7 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt;

/**
  * The AWTSecurityManager class provides the ability to secondarily
  * index AppContext objects through SecurityManager extensions.
  * As noted in AppContext.java, AppContexts are primarily indexed by
  * ThreadGroup.  In the case where the ThreadGroup doesn't provide
  * enough information to determine AppContext (e.g. system threads),
  * if a SecurityManager is installed which derives from
  * AWTSecurityManager, the AWTSecurityManager's getAppContext()
  * method is called to determine the AppContext.
  * 
  * A typical example of the use of this class is where an applet
  * is called by a system thread, yet the system AppContext is
  * inappropriate, because applet code is currently executing.
  * In this case, the getAppContext() method can walk the call stack
  * to determine the applet code being executed and return the applet's
  * AppContext object.
  * 
  * @author  Fred Ecks
  * @version 1.7 12/19/03
  */
public class AWTSecurityManager extends SecurityManager {

    /**
      * Get the AppContext corresponding to the current context.
      * The default implementation returns null, but this method
      * may be overridden by various SecurityManagers
      * (e.g. AppletSecurity) to index AppContext objects by the
      * calling context.
      * 
      * @return  the AppContext corresponding to the current context.
      * @see     sun.awt.AppContext
      * @see     java.lang.SecurityManager
      * @since   JDK1.2.1
      */
    public AppContext getAppContext() {
        return null; // Default implementation returns null
    }

} /* class AWTSecurityManager */
