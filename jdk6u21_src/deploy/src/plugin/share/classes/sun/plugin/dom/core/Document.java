/*
 * @(#)Document.java	1.13 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.dom.core;

import org.w3c.dom.*;
import sun.plugin.dom.*;
import sun.plugin.dom.exception.*;

/**
 *  The <code>Document</code> interface represents the entire HTML or XML 
 * document. Conceptually, it is the root of the document tree, and provides 
 * the  primary access to the document's data.
 * <p> Since elements, text nodes, comments, processing instructions, etc. 
 * cannot exist outside the context of a <code>Document</code> , the 
 * <code>Document</code> interface also contains the factory methods needed 
 * to create these objects. The <code>Node</code> objects created have a 
 * <code>ownerDocument</code> attribute which associates them with the 
 * <code>Document</code> within whose context they were created.
 * <p>See also the <a href='http://www.w3.org/TR/2000/CR-DOM-Level-2-20000510'>Document Object Model (DOM) Level 2 Specification</a>.
 */
public abstract class Document extends sun.plugin.dom.core.Node
			       implements org.w3c.dom.Document {

    private static final String FUNC_CREATE_DOC_FRAGMENT = "createDocumentFragment";
    private static final String FUNC_CREATE_TEXT_NODE = "createTextNode";
    private static final String FUNC_CREATE_COMMENT = "createComment";

    /**
     * Construct a Document object.
     */
    protected Document(DOMObject obj, org.w3c.dom.Document doc) {
	super(obj, doc);
    }


    /**
     *  The Document Type Declaration (see <code>DocumentType</code> ) 
     * associated with this document. For HTML documents as well as XML 
     * documents without a document type declaration this returns 
     * <code>null</code> . The DOM Level 2 does not support editing the 
     * Document Type Declaration, therefore <code>docType</code> cannot be 
     * altered in any way, including through the use of methods, such as 
     * <code>insertNode</code> or <code>removeNode</code> , which are 
     * inherited from the <code>Node</code> interface.
     */
    public org.w3c.dom.DocumentType getDoctype() {
	throw new PluginNotSupportedException("Document.getDoctype() is not supported.");
    }

    /**
     *  The <code>DOMImplementation</code> object that handles this document. 
     * A DOM application may use objects from multiple  implementations.
     */
    public org.w3c.dom.DOMImplementation getImplementation() {
	return new sun.plugin.dom.DOMImplementation((DOMObject) obj.getMember("implementation"));
    }

    /**
     *  This is a convenience attribute that allows direct access to the child 
     * node that is the root element of  the document. For HTML documents, 
     * this is the element with the tagName "HTML".
     */
    public abstract org.w3c.dom.Element getDocumentElement();

    /**
     *  Creates an element of the type specified. Note that the instance 
     * returned implements the <code>Element</code> interface, so attributes 
     * can be specified directly  on the returned object.
     * <br> In addition, if there are known attributes with default values, 
     * <code>Attr</code> nodes representing them are automatically created and
     *  attached to the element.
     * <br> To create an element with a qualified name and namespace URI, use 
     * the <code>createElementNS</code> method.
     * @param tagName  The name of the element type to instantiate. For XML, 
     *   this is case-sensitive. For HTML, the  <code>tagName</code> 
     *   parameter may be provided in any case,  but it must be mapped to the 
     *   canonical uppercase form by  the DOM implementation. 
     * @return  A new <code>Element</code> object with the 
     *   <code>nodeName</code> attribute set to <code>tagName</code> , and 
     *   <code>localName</code> , <code>prefix</code> , and 
     *   <code>namespaceURI</code> set to <code>null</code> .
     * @exception DOMException
     *    INVALID_CHARACTER_ERR: Raised if the specified name contains an 
     *   illegal character.
     */
    public abstract org.w3c.dom.Element createElement(String tagName)
					    throws DOMException;

    /**
     *  Creates an empty <code>DocumentFragment</code> object. 
     * @return  A new <code>DocumentFragment</code> .
     */
    public org.w3c.dom.DocumentFragment createDocumentFragment() {
	Object result = obj.call(FUNC_CREATE_DOC_FRAGMENT, null);

	if (result != null && result instanceof DOMObject) {
	    return new sun.plugin.dom.core.DocumentFragment((DOMObject) result, null);
	}
	return null;
    }

    /**
     *  Creates a <code>Text</code> node given the specified string.
     * @param data  The data for the node.
     * @return  The new <code>Text</code> object.
     */
    public org.w3c.dom.Text createTextNode(String data)
    {
	Object[] args = {data};

	Object result = obj.call(FUNC_CREATE_TEXT_NODE, args);

	if (result != null && result instanceof DOMObject) {
	    return new sun.plugin.dom.core.Text((DOMObject) result, null);
	}
	return null;
    }

    /**
     *  Creates a <code>Comment</code> node given the specified string.
     * @param data  The data for the node.
     * @return  The new <code>Comment</code> object.
     */
    public org.w3c.dom.Comment createComment(String data)
    {
	Object[] args = {data};
	Object result = obj.call(FUNC_CREATE_COMMENT, args);

	if (result != null && result instanceof DOMObject) {
	    return new sun.plugin.dom.core.Comment((DOMObject) result, null);
	}
	return null;
    }

    /**
     *  Creates a <code>CDATASection</code> node whose value  is the specified 
     * string.
     * @param data  The data for the <code>CDATASection</code> contents.
     * @return  The new <code>CDATASection</code> object.
     * @exception DOMException
     *    NOT_SUPPORTED_ERR: Raised if this document is an HTML document.
     */
    public org.w3c.dom.CDATASection createCDATASection(String data)
						       throws DOMException
    {
	throw new PluginNotSupportedException("Document.createCDATASection() is not supported");
    }

    /**
     *  Creates a <code>ProcessingInstruction</code> node given the specified 
     * name and data strings.
     * @param target  The target part of the processing instruction.
     * @param data  The data for the node.
     * @return  The new <code>ProcessingInstruction</code> object.
     * @exception DOMException
     *    INVALID_CHARACTER_ERR: Raised if the specified target contains an 
     *   illegal character.
     *   <br> NOT_SUPPORTED_ERR: Raised if this document is an HTML document.
     */
    public ProcessingInstruction createProcessingInstruction(String target, 
                                                             String data)
                                                             throws DOMException
    {
	throw new PluginNotSupportedException("Document.createProcessingInstruction() is not supported");
    }

    /**
     *  Creates an <code>Attr</code> of the given name. Note that the 
     * <code>Attr</code> instance can then be set on an <code>Element</code> 
     * using the <code>setAttributeNode</code> method. 
     * <br> To create an attribute with a qualified name and namespace URI, use
     *  the <code>createAttributeNS</code> method.
     * @param name  The name of the attribute.
     * @return  A new <code>Attr</code> object with the <code>nodeName</code> 
     *   attribute set to <code>name</code> , and <code>localName</code> , 
     *   <code>prefix</code> , and <code>namespaceURI</code> set to 
     *   <code>null</code> .
     * @exception DOMException
     *    INVALID_CHARACTER_ERR: Raised if the specified name contains an 
     *   illegal character.
     */
    public org.w3c.dom.Attr createAttribute(String name)
					    throws DOMException
    {
	throw new PluginNotSupportedException("Document.createAttribute() is not supported");
    }	

    /**
     *  Creates an <code>EntityReference</code> object. In addition, if the 
     * referenced entity is known, the child list of the 
     * <code>EntityReference</code> node is made the same as that of the 
     * corresponding <code>Entity</code> node. If any descendant of the 
     * <code>Entity</code> node has an unbound  namespace prefix , the 
     * corresponding descendant of the created <code>EntityReference</code> 
     * node is also unbound; (its <code>namespaceURI</code> is 
     * <code>null</code> ). The DOM Level 2 does not support any mechanism to 
     * resolve namespace prefixes.
     * @param name  The name of the entity to reference. 
     * @return  The new <code>EntityReference</code> object.
     * @exception DOMException
     *    INVALID_CHARACTER_ERR: Raised if the specified name contains an 
     *   illegal character.
     *   <br> NOT_SUPPORTED_ERR: Raised if this document is an HTML document.
     */
    public org.w3c.dom.EntityReference createEntityReference(String name)
							     throws DOMException
    {
	throw new PluginNotSupportedException("Document.createEntityReference() is not supported");
    }


    /**
     *  Returns a <code>NodeList</code> of all the <code>Elements</code> with 
     * a given tag name in the order in which they are encountered in a 
     * preorder traversal of the <code>Document</code> tree. 
     * @param tagname  The name of the tag to match on. The special value "*" 
     *   matches all tags.
     * @return  A new <code>NodeList</code> object containing all the matched 
     *   <code>Elements</code> .
     */
    public abstract org.w3c.dom.NodeList getElementsByTagName(String tagname);

    /**
     *  Imports a node from another document to this document. The returned 
     * node has no parent; (<code>parentNode</code> is <code>null</code> ). 
     * The source node is not altered or removed from the original document; 
     * this method creates a new copy of the source node.
     * <br> For all nodes, importing a node creates a node object owned by the 
     * importing document, with attribute values identical to the source 
     * node's <code>nodeName</code> and <code>nodeType</code> , plus the 
     * attributes related to namespaces (<code>prefix</code> , 
     * <code>localName</code> , and <code>namespaceURI</code> ). As in the 
     * <code>cloneNode</code> operation on a <code>Node</code> , the source 
     * node is not altered.
     * <br> Additional information is copied as appropriate to the 
     * <code>nodeType</code> , attempting to mirror the behavior expected if a
     *  fragment of XML or HTML source was copied from one document to 
     * another, recognizing that the two documents may have different DTDs in 
     * the XML case. The following list describes the specifics for each type 
     * of node. 
     * <dl>
     * <dt> ATTRIBUTE_NODE</dt>
     * <dd> The <code>ownerElement</code> attribute is 
     * set to <code>null</code> and the <code>specified</code> flag is set to 
     * <code>true</code> on the generated <code>Attr</code> . The descendants 
     * of the source <code>Attr</code> are recursively imported and the 
     * resulting nodes reassembled to form the corresponding subtree. Note 
     * that the <code>deep</code> parameter has no effect on  
     * <code>Attr</code> nodes; they always carry their children with them 
     * when imported.</dd>
     * <dt> DOCUMENT_FRAGMENT_NODE</dt>
     * <dd> If the <code>deep</code> option 
     * was set to <code>true</code> , the descendants of the source element 
     * will be recursively imported and the resulting nodes reassembled to 
     * form the corresponding subtree. Otherwise, this simply generates an 
     * empty <code>DocumentFragment</code> .</dd>
     * <dt> DOCUMENT_NODE</dt>
     * <dd>
     * <code>Document</code> nodes cannot be imported.</dd>
     * <dt> DOCUMENT_TYPE_NODE</dt>
     * <dd>
     * <code>DocumentType</code> nodes cannot be imported.</dd>
     * <dt> ELEMENT_NODE</dt>
     * <dd> 
     * Specified attribute nodes of the source element are imported, and the 
     * generated <code>Attr</code> nodes are attached to the generated 
     * <code>Element</code> . Default attributes are  not copied, though if 
     * the document being imported into defines default attributes for this 
     * element name, those are assigned. If the <code>importNode</code> 
     * <code>deep</code> parameter was set to <code>true</code> , the 
     * descendants of the source element will be recursively imported and the 
     * resulting nodes reassembled to form the corresponding subtree.</dd>
     * <dt> 
     * ENTITY_NODE</dt>
     * <dd><code>Entity</code> nodes can be imported, however in the 
     * current release of the DOM the <code>DocumentType</code> is readonly. 
     * Ability to add these imported nodes to a <code>DocumentType</code> 
     * will be considered for addition to a future release of the DOM. On 
     * import, the <code>publicId</code> , <code>systemId</code> , and 
     * <code>notationName</code> attributes are copied. If a <code>deep</code>
     *  import is requested, the descendants of the the source 
     * <code>Entity</code> is recursively imported and the resulting nodes 
     * reassembled to form the corresponding subtree.</dd>
     * <dt> ENTITY_REFERENCE_NODE</dt>
     * <dd> 
     * Only the <code>EntityReference</code> itself is copied, even if a 
     * <code>deep</code> import is requested, since the source and 
     * destination documents might have defined the entity differently. If 
     * the document being imported into provides a definition for this entity 
     * name, its value is assigned.</dd>
     * <dt> NOTATION_NODE</dt>
     * <dd><code>Notation</code> nodes 
     * can be imported, however in the current release of the DOM the 
     * <code>DocumentType</code> is readonly. Ability to add these imported 
     * nodes to a <code>DocumentType</code> will be considered for addition 
     * to a future release of the DOM. On import, the <code>publicId</code> 
     * and <code>systemId</code> attributes are copied. Note that the 
     * <code>deep</code> parameter has no effect on  <code>Notation</code> 
     * nodes since they never have any children.</dd>
     * <dt> PROCESSING_INSTRUCTION_NODE</dt>
     * <dd> 
     * The imported node copies its <code>target</code> and <code>data</code> 
     * values from those of the source node.</dd>
     * <dt> TEXT_NODE, CDATA_SECTION_NODE, 
     * COMMENT_NODE</dt>
     * <dd> These three types of nodes inheriting from 
     * <code>CharacterData</code> copy their <code>data</code> and 
     * <code>length</code> attributes from those of the source node.</dd>
     * </dl> 
     * @param importedNode  The node to import.
     * @param deep  If <code>true</code> , recursively import the subtree 
     *   under the specified node; if <code>false</code> , import only the 
     *   node itself, as explained above. This has no effect on 
     *   <code>Attr</code> , <code>EntityReference</code> , and 
     *   <code>Notation</code> nodes.
     * @return  The imported node that belongs to this <code>Document</code> .
     * @exception DOMException
     *    NOT_SUPPORTED_ERR: Raised if the type of node being imported is not 
     *   supported.
     * @since DOM Level 2
     */
    public org.w3c.dom.Node importNode(org.w3c.dom.Node importedNode, 
				       boolean deep)
				       throws DOMException
    {
	throw new PluginNotSupportedException("Document.importNode() is not supported");
    }

    /**
     *  Creates an element of the given qualified name and namespace URI. 
     * HTML-only DOM implementations do not need to implement this method.
     * @param namespaceURI  The  namespace URI of the element to create.
     * @param qualifiedName  The  qualified name of the element type to 
     *   instantiate.
     * @return  A new <code>Element</code> object with the following 
     *   attributes: Attribute Value<code>Node.nodeName</code>
     *   <code>qualifiedName</code><code>Node.namespaceURI</code>
     *   <code>namespaceURI</code><code>Node.prefix</code> prefix, extracted 
     *   from <code>qualifiedName</code> , or <code>null</code> if there is no
     *    prefix<code>Node.localName</code> local name , extracted from 
     *   <code>qualifiedName</code><code>Element.tagName</code>
     *   <code>qualifiedName</code>
     * @exception DOMException
     *    INVALID_CHARACTER_ERR: Raised if the specified qualified name 
     *   contains an illegal character.
     *   <br> NAMESPACE_ERR: Raised if the <code>qualifiedName</code> is 
     *   malformed, if the <code>qualifiedName</code> has a prefix and the 
     *   <code>namespaceURI</code> is <code>null</code> or an empty string, 
     *   or if the <code>qualifiedName</code> has a prefix that is "xml" and 
     *   the <code>namespaceURI</code> is different from " 
     *   http://www.w3.org/XML/1998/namespace "  .
     * @since DOM Level 2
     */
    public abstract org.w3c.dom.Element createElementNS(String namespaceURI, 
					       String qualifiedName)
	throws DOMException;

    /**
     *  Creates an attribute of the given qualified name and namespace URI. 
     * HTML-only DOM implementations do not need to implement this method.
     * @param namespaceURI  The  namespace URI of the attribute to create.
     * @param qualifiedName  The  qualified name of the attribute to 
     *   instantiate.
     * @return  A new <code>Attr</code> object with the following attributes: 
     *   Attribute Value<code>Node.nodeName</code> qualifiedName
     *   <code>Node.namespaceURI</code><code>namespaceURI</code>
     *   <code>Node.prefix</code> prefix, extracted from 
     *   <code>qualifiedName</code> , or <code>null</code> if there is no 
     *   prefix<code>Node.localName</code> local name , extracted from 
     *   <code>qualifiedName</code><code>Attr.name</code>
     *   <code>qualifiedName</code>
     * @exception DOMException
     *    INVALID_CHARACTER_ERR: Raised if the specified qualified name 
     *   contains an illegal character.
     *   <br> NAMESPACE_ERR: Raised if the <code>qualifiedName</code> is 
     *   malformed, if the <code>qualifiedName</code> has a prefix and the 
     *   <code>namespaceURI</code> is <code>null</code> or an empty string, 
     *   if the <code>qualifiedName</code> has a prefix that is "xml" and the 
     *   <code>namespaceURI</code> is different from " 
     *   http://www.w3.org/XML/1998/namespace ", or if the 
     *   <code>qualifiedName</code> is "xmlns" and the 
     *   <code>namespaceURI</code> is different from " 
     *   http://www.w3.org/2000/xmlns/ ".
     * @since DOM Level 2
     */
    public org.w3c.dom.Attr createAttributeNS(String namespaceURI, 
					      String qualifiedName)
					      throws DOMException
    {
	throw new PluginNotSupportedException("Document.createAttributeNS() is not supported");
    }


    /**
     *  Returns a <code>NodeList</code> of all the <code>Elements</code> with 
     * a given  local name and namespace URI in the order in which they are 
     * encountered in a preorder traversal of the <code>Document</code> tree.
     * @param namespaceURI  The  namespace URI of the elements to match on. 
     *   The special value "*" matches all namespaces.
     * @param localName  The  local name of the elements to match on. The 
     *   special value "*" matches all local names.
     * @return  A new <code>NodeList</code> object containing all the matched 
     *   <code>Elements</code> .
     * @since DOM Level 2
     */
    public org.w3c.dom.NodeList getElementsByTagNameNS(String namespaceURI, 
						       String localName)
    {
	throw new PluginNotSupportedException("Document.getElementsByTagNameNS() is not supported");
    }

    /**
     *  Returns the <code>Element</code> whose <code>ID</code> is given by 
     * <code>elementId</code> . If no such element exists, returns 
     * <code>null</code> . Behavior is not defined if more than one element 
     * has this <code>ID</code> .  The DOM implementation must have 
     * information that says which attributes are of type ID. Attributes with 
     * the name "ID" are not of type ID unless so defined. Implementations 
     * that do not know whether attributes are of type ID or not are expected 
     * to return <code>null</code> .
     * @param elementId  The unique <code>id</code> value for an element.
     * @return  The matching element.
     * @since DOM Level 2
     */
    public abstract org.w3c.dom.Element getElementById(String elementId);

    /**
     *  The value of this node, depending on its type; see the table above. 
     * When it is defined to be <code>null</code> , setting it has no effect.
     * @exception DOMException
     *    NO_MODIFICATION_ALLOWED_ERR: Raised when the node is readonly.
     * @exception DOMException
     *    DOMSTRING_SIZE_ERR: Raised when it would return more characters 
     *   than fit in a <code>DOMString</code> variable on the implementation 
     *   platform.
     */
    public String getNodeValue() throws DOMException
    {
	throw new PluginNotSupportedException("Document.getNodeValue() is not supported");
    }

    public void setNodeValue(String nodeValue) throws DOMException
    {
	throw new PluginNotSupportedException("Document.setNodeValue() is not supported");
    }

    //Start DOM L3 Stub methods 

   /**
     * An attribute specifying the encoding used for this document at the time 
     * of the parsing. This is <code>null</code> when it is not known, such 
     * as when the <code>Document</code> was created in memory.
     * @since DOM Level 3
     */
    public String getInputEncoding() {
    throw new PluginNotSupportedException("Document.getInputEncoding() is not supported"); 
    }


    /**
     * An attribute specifying, as part of the XML declaration, the encoding
     * of this document. This is <code>null</code> when unspecified.
     * @since DOM Level 3
     */
    public String getXmlEncoding() {
     throw new PluginNotSupportedException("Document.getXmlEncoding() is not supported"); 
    }

    /**
     * An attribute specifying, as part of the XML declaration, the encoding
     * of this document. This is <code>null</code> when unspecified.
     * @since DOM Level 3
     */
    public void setXmlEncoding(String xmlEncoding) {
        throw new PluginNotSupportedException("Document.setXmlEncoding() is not supported"); 
    }


    /**
     * An attribute specifying, as part of the XML declaration, whether this
     * document is standalone.
     * @since DOM Level 3
     */
    public boolean getXmlStandalone() {
       throw new PluginNotSupportedException("Document.getXmlStandalone() is not supported"); 
    }

    /**
     * An attribute specifying, as part of the XML declaration, whether this
     * document is standalone.
     * @exception DOMException
     *    NOT_SUPPORTED_ERR: Raised if this document does support the "XML"
     *   feature.
     * @since DOM Level 3
     */
    public void setXmlStandalone(boolean xmlStandalone)
                                  throws DOMException {
       throw new PluginNotSupportedException("Document.setXmlStandalone() is not supported"); 
    }


    /**
     *  An attribute specifying, as part of the XML declaration, the version
     * number of this document. If there is no declaration, the value is
     * <code>"1.0"</code>. Changing this attribute will affect methods that
     * check for illegal characters in XML names. Application should invoke
     * <code>Document.normalizeDocument()</code> in order to check for
     * illegal characters in the <code>Node</code>s that are already part of
     * this <code>Document</code>.
     * @since DOM Level 3
     */
    public String getXmlVersion() {
        throw new PluginNotSupportedException("Document.getXmlVersion() is not supported"); 
    }

    /**
     *  An attribute specifying, as part of the XML declaration, the version
     * number of this document. If there is no declaration, the value is
     * <code>"1.0"</code>. Changing this attribute will affect methods that
     * check for illegal characters in XML names. Application should invoke
     * <code>Document.normalizeDocument()</code> in order to check for
     * illegal characters in the <code>Node</code>s that are already part of
     * this <code>Document</code>.
     * @exception DOMException
     *    NOT_SUPPORTED_ERR: Raised if the version is set to a value that is
     *   not supported by this <code>Document</code> or if this document
     *   does support the "XML" feature.
     * @since DOM Level 3
     */
    public void setXmlVersion(String xmlVersion)
                                  throws DOMException {
        throw new PluginNotSupportedException("Document.setXmlVersion() is not supported"); 
    }


 /**
     * An attribute specifying whether error checking is enforced or not. When
     * set to <code>false</code>, the implementation is free to not test
     * every possible error case normally defined on DOM operations, and not
     * raise any <code>DOMException</code> on DOM operations or report
     * errors while using <code>Document.normalizeDocument()</code>. In case
     * of error, the behavior is undefined. This attribute is
     * <code>true</code> by default.
     * @since DOM Level 3
     */
    public boolean getStrictErrorChecking() {
        throw new PluginNotSupportedException("Document.getStrictErrorChecking() is not supported"); 
    }

    /**
     * An attribute specifying whether error checking is enforced or not. When
     * set to <code>false</code>, the implementation is free to not test
     * every possible error case normally defined on DOM operations, and not
     * raise any <code>DOMException</code> on DOM operations or report
     * errors while using <code>Document.normalizeDocument()</code>. In case
     * of error, the behavior is undefined. This attribute is
     * <code>true</code> by default.
     * @since DOM Level 3
     */
    public void setStrictErrorChecking(boolean strictErrorChecking) {
        throw new PluginNotSupportedException("Document.setStrictErrorChecking() is not supported"); 
    }


    /**
     * The location of the document or <code>null</code> if undefined.
     * <br>Beware that when the <code>Document</code> supports the feature
     * "HTML" [<a href='http://www.w3.org/TR/2003/REC-DOM-Level-2-HTML-20030109'>DOM Level 2 HTML</a>]
     * , the href attribute of the HTML BASE element takes precedence over
     * this attribute.
     * @since DOM Level 3
     */
    public String getDocumentURI() {
       throw new PluginNotSupportedException("Document.getDocumentURI() is not supported"); 
    }

    /**
     * The location of the document or <code>null</code> if undefined.
     * <br>Beware that when the <code>Document</code> supports the feature
     * "HTML" [<a href='http://www.w3.org/TR/2003/REC-DOM-Level-2-HTML-20030109'>DOM Level 2 HTML</a>]
     * , the href attribute of the HTML BASE element takes precedence over
     * this attribute.
     * @since DOM Level 3
     */
    public void setDocumentURI(String documentURI) {
        throw new PluginNotSupportedException("Document.setDocumentURI() is not supported"); 
    }

    public org.w3c.dom.Node adoptNode(org.w3c.dom.Node source)
        throws DOMException {
        throw new PluginNotSupportedException("Document.adoptNode() is not supported"); 
    }


    /**
     *  The configuration used when <code>Document.normalizeDocument()</code>
     * is invoked.
     * @since DOM Level 3
     */
    public org.w3c.dom.DOMConfiguration getDomConfig() {
        throw new PluginNotSupportedException("Document.getDomConfig() is not supported"); 
    }

    public void normalizeDocument() {
        throw new PluginNotSupportedException("Document.normalizeDocument() is not supported"); 
    }

    public org.w3c.dom.Node renameNode(org.w3c.dom.Node n,
                           String namespaceURI,
                           String qualifiedName)
                           throws DOMException {
        throw new PluginNotSupportedException("Document.renameNode() is not supported"); 
    }

    // End DOM L3 Stub methods
}

