package sun.kernel;

/**
 * Called by SplitJRE to determine which classes are needed in order to support
 * the downloading of additional bundles.  Touches classes which are marked
 * (during the JRE build process) as "not part of the core JRE, but available
 * for download", causing the bundles to be downloaded and installed.  All
 * classes involved in this process are then incorporated into the core Kernel
 * JRE.
 */
public class DownloadTest {
    public static void main(String[] arg) throws Exception {
        // These dummy classes are bundled by SplitJRE, which also adds a
        // resource_map to the JRE copy claiming claiming that they are
        // in fact part of the JRE.  The test is configured so that the
        // "download' comes from resources embedded in the JAR.
        // Dummy2's bundle is marked as a dependency of Dummy1's bundle,
        // so the dependency download code is exercised as well.
        Class.forName("sun.kernel.Dummy1");
        Class.forName("sun.kernel.Dummy2");
    }
}
