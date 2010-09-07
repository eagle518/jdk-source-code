package sun.kernel;

/** 
 * Used to create a "dummy" bundle while SplitJRE is running.  The dummy bundle,
 * which contains only this class, is downloaded with "-verbose" in order to see
 * which classes are involved in the download process.
 *
 * The Dummy2 bundle is marked as a dependency of the Dummy1 bundle, so that the
 * dependency download code is exercised as well.
 */
public class Dummy1 {
}
