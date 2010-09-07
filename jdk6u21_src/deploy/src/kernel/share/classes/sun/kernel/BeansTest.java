package sun.kernel;

import java.beans.*;

/** 
 * Determines the set of "core" classes necessary to provide basic JavaBeans 
 * support.  Run by SplitJRE with the -verbose JVM parameter.  
 */
public class BeansTest {
    public static void main(String[] arg) throws Exception {
        Introspector.getBeanInfo(java.awt.Component.class);
    }
}
