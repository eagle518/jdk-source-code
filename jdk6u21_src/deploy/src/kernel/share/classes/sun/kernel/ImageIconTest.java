package sun.kernel;

/** 
 * Determines the set of ImageIcon related classes 
 * Workaround for:
 *   plugin loads ImageIcon explicitly as workaround for memory leak.  
 * Run by SplitJRE with the -verbose JVM parameter.  
 */
public class ImageIconTest {
    public static void main(String[] args) throws Exception {
       Class lnfClass = Class.forName("javax.swing.ImageIcon");
    }
}
