import netscape.javascript.*;
import org.w3c.dom.*;
import org.w3c.dom.html.*;
import com.sun.java.browser.plugin2.*;

/** Simple applet which dumps the DOM of the web page it's on,
    including node names and their attributes, to System.out. */

public class DOMDump extends java.applet.Applet {
    public void start() {
        try {
            HTMLDocument doc = (HTMLDocument) DOM.getDocument(this);
            HTMLBodyElement body = (HTMLBodyElement) doc.getBody();
            dump(body, INDENT);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private static final String INDENT = "    ";
    private static final String HALF_INDENT = "  ";

    private void dump(Node root, String prefix) {
        if (root instanceof Element) {
            System.out.println(prefix + ((Element) root).getTagName() + " / " + root.getClass().getName());
        } else if (root instanceof CharacterData) {
            String data = ((CharacterData) root).getData().trim();
            if (!data.equals("")) {
                System.out.println(prefix + "CharacterData: " + data);
            }
        } else {
            System.out.println(prefix + root.getClass().getName());
        }
        NamedNodeMap attrs = root.getAttributes();
        if (attrs != null) {
            int len = attrs.getLength();
            for (int i = 0; i < len; i++) {
                Node attr = attrs.item(i);
                System.out.print(prefix + HALF_INDENT + "attribute " + i + ": " +
                                 attr.getNodeName());
                if (attr instanceof Attr) {
                    System.out.print(" = " + ((Attr) attr).getValue());
                }
                System.out.println();
            }
        }

        if (root.hasChildNodes()) {
            NodeList children = root.getChildNodes();
            if (children != null) {
                int len = children.getLength();
                for (int i = 0; i < len; i++) {
                    dump(children.item(i), prefix + INDENT);
                }
            }
        }
    }
}
