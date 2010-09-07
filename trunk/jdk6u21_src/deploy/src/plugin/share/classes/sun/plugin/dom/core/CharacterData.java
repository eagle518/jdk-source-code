/*
 * @(#)CharacterData.java	1.10 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.plugin.dom.core;

import org.w3c.dom.*;
import sun.plugin.dom.*;
import sun.plugin.dom.exception.*;


/**
 *  The <code>CharacterData</code> interface extends Node with a set of 
 * attributes and methods for accessing character data in the DOM.  For 
 * clarity this set is defined here rather than on each object that uses 
 * these attributes and methods. No DOM objects correspond directly to 
 * <code>CharacterData</code> , though <code>Text</code> and others do 
 * inherit the interface from it. All <code>offsets</code> in this interface 
 * start from 0.
 * <p> As explained in the <code>DOMString</code> interface, text strings in 
 * the DOM are represented in UTF-16, i.e. as a sequence of 16-bit units.  In 
 * the following, the term  16-bit units is used whenever necessary to 
 * indicate that indexing on CharacterData is done in 16-bit units.
 * <p>See also the <a href='http://www.w3.org/TR/2000/CR-DOM-Level-2-20000510'>Document Object Model (DOM) Level 2 Specification</a>.
 */
class CharacterData extends sun.plugin.dom.core.Node 
			   implements org.w3c.dom.CharacterData {

    private static final String ATTR_DATA = "data";
    private static final String ATTR_LENGTH = "length";

    private static final String FUNC_SUBSTRING_DATA = "substringData";
    private static final String FUNC_APPEND_DATA = "appendData";
    private static final String FUNC_INSERT_DATA = "insertData";
    private static final String FUNC_DELETE_DATA = "deleteData";
    private static final String FUNC_REPLACE_DATA = "replaceData";

    /**
     * Construct a CharacterData object.
     */
    protected CharacterData(DOMObject obj, org.w3c.dom.Document doc) {
	super(obj, doc);
    }

    /**
     *  The character data of the node that implements this interface. The DOM 
     * implementation may not put arbitrary limits on the amount of data that 
     * may be stored in a  <code>CharacterData</code> node. However, 
     * implementation limits may  mean that the entirety of a node's data may 
     * not fit into a single <code>DOMString</code> . In such cases, the user 
     * may call <code>substringData</code> to retrieve the data in 
     * appropriately sized pieces.
     * @exception DOMException
     *    NO_MODIFICATION_ALLOWED_ERR: Raised when the node is readonly.
     * @exception DOMException
     *    DOMSTRING_SIZE_ERR: Raised when it would return more characters 
     *   than fit in a <code>DOMString</code> variable on the implementation 
     *   platform.
     */
    public String getData() throws DOMException {
	return DOMObjectHelper.getStringMemberNoEx(obj, ATTR_DATA);
    }

    public void setData(String data) throws DOMException {
	DOMObjectHelper.setStringMember(obj, ATTR_DATA, data);
    }

    /**
     *  The number of  16-bit units that are available through 
     * <code>data</code> and the <code>substringData</code> method below.  
     * This may have the value zero, i.e., <code>CharacterData</code> nodes 
     * may be empty.
     */
    public int getLength() {
	return DOMObjectHelper.getIntMemberNoEx(obj, ATTR_LENGTH);
    }

    /**
     *  Extracts a range of data from the node.
     * @param offset  Start offset of substring to extract.
     * @param count  The number of 16-bit units to extract.
     * @return  The specified substring. If the sum of <code>offset</code> and 
     *   <code>count</code> exceeds the <code>length</code> , then all 16-bit 
     *   units to the end of the data are returned.
     * @exception DOMException
     *    INDEX_SIZE_ERR: Raised if the specified <code>offset</code> is 
     *   negative or greater than the number of 16-bit units in 
     *   <code>data</code> , or if the specified <code>count</code> is 
     *   negative.
     *   <br> DOMSTRING_SIZE_ERR: Raised if the specified range of text does 
     *   not fit into a <code>DOMString</code> .
     */
    public String substringData(int offset, 
                                int count)
                                throws DOMException {
	Object[] args = {new Integer(offset), new Integer(count)};
	return DOMObjectHelper.callStringMethod(obj, FUNC_SUBSTRING_DATA, args);
    }


    /**
     *  Append the string to the end of the character data of the node. Upon 
     * success, <code>data</code> provides access to the concatenation of 
     * <code>data</code> and the <code>DOMString</code> specified.
     * @param arg  The <code>DOMString</code> to append.
     * @exception DOMException
     *    NO_MODIFICATION_ALLOWED_ERR: Raised if this node is readonly.
     */
    public void appendData(String arg)
	throws DOMException {
	Object[] args = {arg};

	obj.call(FUNC_APPEND_DATA, args);
    }

    /**
     *  Insert a string at the specified character offset.
     * @param offset  The character offset at which to insert.
     * @param arg  The <code>DOMString</code> to insert.
     * @exception DOMException
     *    INDEX_SIZE_ERR: Raised if the specified <code>offset</code> is 
     *   negative or greater than the number of 16-bit units in 
     *   <code>data</code> .
     *   <br> NO_MODIFICATION_ALLOWED_ERR: Raised if this node is readonly.
     */
    public void insertData(int offset, 
                           String arg)
                           throws DOMException {
	Object[] args = {new Integer(offset), arg};
	obj.call(FUNC_INSERT_DATA, args);
    }

    /**
     *  Remove a range of  16-bit units from the node. Upon success, 
     * <code>data</code> and <code>length</code> reflect the change.
     * @param offset  The offset from which to start removing.
     * @param count  The number of 16-bit units to delete. If the sum of 
     *   <code>offset</code> and <code>count</code> exceeds 
     *   <code>length</code> then all 16-bit units from <code>offset</code> 
     *   to the end of the data are deleted.
     * @exception DOMException
     *    INDEX_SIZE_ERR: Raised if the specified <code>offset</code> is 
     *   negative or greater than the number of 16-bit units in 
     *   <code>data</code> , or if the specified <code>count</code> is 
     *   negative.
     *   <br> NO_MODIFICATION_ALLOWED_ERR: Raised if this node is readonly.
     */
    public void deleteData(int offset, 
                           int count)
                           throws DOMException {
	Object[] args = {new Integer(offset), new Integer(count)};
	obj.call(FUNC_DELETE_DATA, args);
    }

    /**
     *  Replace the characters starting at the specified  16-bit unit offset 
     * with the specified string.
     * @param offset  The offset from which to start replacing.
     * @param count  The number of 16-bit units to replace. If the sum of 
     *   <code>offset</code> and <code>count</code> exceeds 
     *   <code>length</code> , then all 16-bit units to the end of the data 
     *   are replaced; (i.e., the effect is the same as a <code>remove</code> 
     *   method call with the same range, followed by an <code>append</code> 
     *   method invocation).
     * @param arg  The <code>DOMString</code> with which the range must be 
     *   replaced.
     * @exception DOMException
     *    INDEX_SIZE_ERR: Raised if the specified <code>offset</code> is 
     *   negative or greater than the number of 16-bit units in 
     *   <code>data</code> , or if the specified <code>count</code> is 
     *   negative.
     *   <br> NO_MODIFICATION_ALLOWED_ERR: Raised if this node is readonly.
     */
    public void replaceData(int offset, 
                            int count, 
                            String arg)
                            throws DOMException {
	Object[] args = {new Integer(offset), new Integer(count), arg};
	obj.call(FUNC_REPLACE_DATA, args);
    }


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
    public String getNodeValue() throws DOMException {
	return getData();
    }

    public void setNodeValue(String nodeValue)
	throws DOMException {
	setData(nodeValue);
    }
}

