package sun.kernel;

import javax.xml.parsers.*;

/** 
 * Determines the set of "core" classes necessary to support basic XML parsing.  Run
 * by SplitJRE with the -verbose JVM parameter.  
 */
public class XMLTest {
    public static void main(String[] arg) throws Exception {
        DocumentBuilderFactory domFactory = DocumentBuilderFactory.newInstance(); 
        DocumentBuilder domBuilder = domFactory.newDocumentBuilder(); 
        domBuilder.parse(XMLTest.class.getResourceAsStream("bundles.xml")); 
        domBuilder.parse(XMLTest.class.getResource("bundles.xml").toString());
    }
}
