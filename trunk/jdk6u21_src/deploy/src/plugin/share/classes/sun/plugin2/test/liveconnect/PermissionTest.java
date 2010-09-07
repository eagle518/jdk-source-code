import java.applet.*;
import java.io.*;
import java.lang.reflect.*;
import java.net.*;
import java.security.*;

/** This class tests the new LiveConnect security model. Calls coming
    from JavaScript are treated as though they come from unsigned
    applets hosted at the document (not the code) base. This implies
    that if the document base and the code base come from the same
    host that (without more code) network connections are allowed back
    to the host, but if the hosts are different that network
    connections are not allowed. */

public class PermissionTest extends Applet {
    /** Called from JavaScript. */
    public void runTest(boolean shouldThrowException) {
        URL codeBase = getCodeBase();
        if (codeBase == null) {
            System.out.println("TEST FAILED -- null codebase");
            return;
        }
        Socket socket = null;
        InetAddress address = null;
        try {
            // We aren't testing deeper failures of the networking stack
            address = getAddress(codeBase.getHost());
        } catch (UnknownHostException e) {
            e.printStackTrace();
            System.out.println("TEST FAILED -- unknown host");
            return;
        } catch (SecurityException e) {
            e.printStackTrace();
            System.out.println("TEST FAILED -- should be able to resolve host name in doPrivileged() block");
            return;
        } catch (RuntimeException e) {
            e.printStackTrace();
            System.out.println("TEST FAILED -- unexpected exception");
            return;
        }
        try {
            socket = new Socket(address, 80);
            if (shouldThrowException) {
                System.out.println("TEST FAILED -- no SecurityException");
            } else {
                System.out.println("Test passed -- no SecurityException");
            }
        } catch (SecurityException e) {
            if (shouldThrowException) {
                System.out.println("Test passed -- caught SecurityException");
            } else {
                e.printStackTrace();
                System.out.println("TEST FAILED -- caught SecurityException");
            }
        } catch (IOException e) {
            e.printStackTrace();
            System.out.println("TEST FAILED -- caught IOException");
        }
        try {
            if (socket != null)
                socket.close();
        } catch (IOException e) {
        }
    }

    /** Called from JavaScript. */
    public void runDoPrivilegedTest() {
        AccessController.doPrivileged(new PrivilegedAction() {
                public Object run() {
                    runTest(false);
                    return null;
                }
            });
    }

    // Helper function to look up address in a privileged context
    private InetAddress getAddress(final String host) throws UnknownHostException {
        try {
            return (InetAddress) AccessController.doPrivileged(new PrivilegedExceptionAction() {
                    public Object run() throws Exception {
                        return InetAddress.getByName(host);
                    }
                });
        } catch (PrivilegedActionException e) {
            Throwable cause = e.getCause();
            if (cause instanceof UnknownHostException) {
                throw (UnknownHostException) cause;
            }
            throw new RuntimeException(cause);
        }
    }
}
