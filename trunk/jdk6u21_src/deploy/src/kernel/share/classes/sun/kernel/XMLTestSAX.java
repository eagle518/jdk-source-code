package sun.kernel;

import javax.xml.parsers.*;
import org.xml.sax.helpers.DefaultHandler;

/** 
 * Determines the set of "core" classes necessary to support basic XML parsing using SAX.  
 * Run by SplitJRE with the -verbose JVM parameter.  
 */
public class XMLTestSAX {
    public static void main(String[] arg) throws Exception {
		SAXParserFactory factory = SAXParserFactory.newInstance();
		SAXParser saxParser = factory.newSAXParser();
		saxParser.parse(
                    XMLTestSAX.class.getResourceAsStream("bundles.xml"), 
                    new DefaultHandler());
  }
}
