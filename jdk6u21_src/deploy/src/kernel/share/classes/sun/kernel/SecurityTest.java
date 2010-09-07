package sun.kernel;

import java.beans.*;

/** 
 * Determines the set of "core" classes necessary to support basic signature
 * verification.  Run by SplitJRE with the -verbose JVM parameter.  
 */
public class SecurityTest {
    public static void main(String[] arg) throws Exception {
        com.sun.crypto.provider.SunJCE.class.getSigners();
    }
}
