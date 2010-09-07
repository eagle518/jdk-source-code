import java.applet.*;
import java.io.*;
import java.net.*;
import java.util.*;
import javax.swing.JLabel;

/** Tests cookie fetching support from the browser. NOTE that this
    applet must be paired with the CookieAppletTest.html and be
    compiled into CookieAppletTest.jar and signed. <P>
*/

public class CookieAppletTest extends Applet {
    // NOTE: must match cookie in surrounding web page
    private static final String COOKIE   = "cookie_content=foo";
    private static final String DOCUMENT = "CookieAppletTest.html";

    public void init() {
        add(new JLabel("See Java Console for output"));

        final URL docBase = getDocumentBase();
        String protocol = docBase.getProtocol();
        if (!("http".equals(protocol) || "https".equals(protocol))) {
            System.out.println("Test failed: this test must be run from a web server (served up via HTTP)");
            return;
        }

        try {
            CookieHandler handler = CookieHandler.getDefault();
            URL url = new URL(docBase, DOCUMENT);
            Map cookieMap = handler.get(url.toURI(), null);
            if (cookieMap == null) {
                System.out.println("Test failed: no cookies set.");
            } else {
                List cookies = (List) cookieMap.get("Cookie");
                if (cookies == null) {
                    System.out.println("Test failed: \"Cookie\" not found in cookie map.");
                } else if (cookies.size() == 0) {
                    System.out.println("Test failed: cookie list in map was empty.");
                } else {
                    String cookie = (String) cookies.get(0);
                    if (cookie.indexOf(COOKIE) < 0) {
                        System.out.println("Test failed: cookie set to wrong value (got \"" +
                                           cookie + "\", expected \"" + COOKIE + "\")");
                    } else {
                        System.out.println("Test passed.");
                    }
                }
            }
        } catch (Exception ex) {
            System.out.println("Test failed:");
            ex.printStackTrace();
        }
    }
}
