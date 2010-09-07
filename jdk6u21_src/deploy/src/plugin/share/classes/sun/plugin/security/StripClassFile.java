/*
 * @(#)StripClassFile.java	1.9 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.security;

import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.InputStream;
import java.io.IOException;

/**
 * An object of this class is used to strip class files of unneeded 
 * attributes sections, and also trims unneeded bytes from the end
 * of class files.
 */
public class StripClassFile {

    private static final int CONSTANT_Class = 7;
    private static final int CONSTANT_Fieldref = 9;
    private static final int CONSTANT_Methodref = 10;
    private static final int CONSTANT_InterfaceMethodref = 11;
    private static final int CONSTANT_String = 8;
    private static final int CONSTANT_Integer = 3;
    private static final int CONSTANT_Float = 4;
    private static final int CONSTANT_Long = 5;
    private static final int CONSTANT_Double = 6;
    private static final int CONSTANT_NameAndType = 12;
    private static final int CONSTANT_Utf8 = 1;

    private static final String[] requiredTypes =
    { "Code", "ConstantValue", "Exceptions", "InnerClasses", "Synthetic" };

    private ByteStream bs;
    private DataInputStream dis;
    int magic;
    short minor_version;
    short major_version;
    int constant_pool_count;
    ConstantPool constant_pool[];

    /**
     * Strip the provided class definition of unneeded attributes sections.
     * If unable to parse the class definition properly, the original
     * definition will be returned;
     *
     * @param      classDefinition is the definition of the class to be stipped.
     *
     * @return     a replacement class definition.
     */
    public byte[] strip(byte[] classDefinition) throws Exception {

	// Duplicate the original definition so not to tamper with the original.
	int length = classDefinition.length;
	byte[] tmpDefinition = new byte[length];
	System.arraycopy(classDefinition, 0, tmpDefinition, 0, length);

	try {
	    // Setup streams for processing.
	    bs = new ByteStream(tmpDefinition);
	    dis = new DataInputStream(bs);

	    // Parse and rewrite the class definition.
	    stripClassFile();

	} catch (Exception e) {
//	    System.out.println("Parse error at byte " + bs.getOffset());
//	    e.printStackTrace();
	    throw e;
//	    return classDefinition;
	}

	// Copy the new class definition to an array of the correct length.
	int newLength = bs.getOffset();
	byte[]newDefinition = new byte[newLength];
	tmpDefinition = bs.getArray();
	System.arraycopy(tmpDefinition, 0, newDefinition, 0, newLength);
	return newDefinition;
    }


    /**
     * Implement a variant of ByteArrayInputStream which allows
     * positions in the array to be obtained, so that the byte array can
     * be rewritten.
     */
    private class ByteStream extends InputStream {

	private byte[] array;
	private int offset;
	private int length;

	ByteStream(byte[] array) {
	    this.array = array;
	    offset = 0;
	    length = array.length;
	    //System.err.println("length = " + length);
	}

	public byte[] getArray() {
	    return array;
	}

	public int read() throws IOException {
	    if (offset == length) {
		throw new IOException();
	    } else {
		int res = (int)array[offset++] & 255;
		//System.err.println("read(): " + res);
		return res;
	    }
	}

	public void write(byte[] b) throws IOException {
	    if (offset + b.length > length) {
		throw new IOException();
	    }
	    for (int i = 0; i < b.length; ++i) {
		array[offset++] = b[i];
	    }
	}

	/**
	 * Return the current position in the byte array.
	 */
	public int getOffset() {
	    return offset;
	}

	/**
	 * Reset the position in the byte array to a new location.
	 */
	public void setOffset(int offset) {
	    this.offset = offset;
	}

	/**
	 * Set a byte in the array to a particular value.
	 */
	public void setByte(int offset, int value) {
	    array[offset] = (byte)value;
	}

	/**
	 * Decrement an unsigned short located at an offset in the byte array.
	 */
	public void decrementCount(int offset) {
	    int ch1 = array[offset] & 0xFF;
	    int ch2 = array[offset+1] & 0xFF;
	    int count = (ch1 << 8) + (ch2 << 0) - 1;
	    array[offset]   = (byte)((count >>> 8) & 0xFF);
	    array[offset+1] = (byte)((count >>> 0) & 0xFF);
	}

	/**
	 * Decrement an int located at an offset in the byte array.
	 */
	public void decrementLength(int offset, int diff) {
	    int ch1 = array[offset]   & 0xFF;
	    int ch2 = array[offset+1] & 0xFF;
	    int ch3 = array[offset+2] & 0xFF;
	    int ch4 = array[offset+3] & 0xFF;
	    int length = (ch1 << 24) + (ch2 << 16) + (ch3 << 8) + (ch4 << 0);
	    length = length - diff;
	    array[offset]   = (byte)((length >>> 24) & 0xFF);
	    array[offset+1] = (byte)((length >>> 16) & 0xFF);
	    array[offset+2] = (byte)((length >>>  8) & 0xFF);
	    array[offset+3] = (byte)((length >>>  0) & 0xFF);
	}

	/**
	 * Remove bytes (between two offsets) from the array.
	 */
	public void removeBytes(int startOffset, int endOffset) {
	    System.arraycopy(array, endOffset, array, startOffset,
			     length - endOffset);
	    length -= (endOffset - startOffset);
	}

	/**
	 * Remove bytes (between two offsets) from the array.
	 */
	public void addBytes(int startOffset, int newSpace) {
	    byte[] newArray = new byte[length + newSpace];
	    System.arraycopy(array, 0, newArray, 0, startOffset);
	    System.arraycopy(array, startOffset, newArray,
			     startOffset + newSpace, length - startOffset);
	    array = newArray;
	    length += newSpace;
	}
    }


    /**
     * Validate a class name -- if the name at the specified constant pool
     * index is invalid, mark that constant pool entry as invalid, so that
     * on the second pass, it can be "corrected".
     */
    private boolean validateClassName(int cpIndex)
    throws IOException {
	ConstantPool entry = constant_pool[cpIndex-1];
	cpIndex = entry.nameIndex;
	return validateName(cpIndex);
    }


    /**
     * Return true if passed in a character which is valid in a Java
     * descriptor.
     */
    private static boolean validJavaCharacter(char c, boolean first) {
	if (c == '/' || c == '(' || c == ')' || c == ';' || c == '[' || c == '<') {
	    return true;
	}
	return first ? Character.isJavaIdentifierStart(c)
	             : Character.isJavaIdentifierPart(c);
    }


    /**
     * Validate an identifier or descriptor -- if the name at the
     * specified constant pool index is invalid, mark that constant pool
     * entry as invalid, so that on the second pass, it can be
     * "corrected".
     */
    private boolean validateName(int cpIndex)
    throws IOException {
	ConstantPool entry = constant_pool[cpIndex-1];

	// Allow well known names.
	if (entry.string == null ||
	    entry.string.equals("<init>") ||
	    entry.string.equals("<clinit>")) return true;

	// Examine each character -- if an illegal character is found,
	// mark the constant pool entry invalid return false.
	for (int i = 0; i < entry.string.length(); ++i) {
	    char c = entry.string.charAt(i);
	    if (!validJavaCharacter(c, i == 0)) {
		entry.invalid = true;
		return false;
	    }
	}
	return true;
    }


    /**
     * Append a string Uxxxx (x = hex digit) to a string buffer for the
     * unicode value of a given character.
     */
    static void charToUnicode(char c, StringBuffer sb) {
	sb.append("U");
	String hex = Integer.toHexString(c);
	for (int j = hex.length(); j < 4; ++j)
	    sb.append('0');
	sb.append(hex);
    }


    /**
     * Return a valid identifier for an invalid one bby substituting
     * a string Uxxxx (x = hex digit) for the illegal characters.
     */
    static String makeValid(String identifier) {

	int length = identifier.length();
	if (length == 0) {
	    return "";
	}
	StringBuffer sb = new StringBuffer(length * 5);
	char[] chars = identifier.toCharArray();

	for (int i = 0; i < length; ++i) {
	    char c = chars[i];
	    if (validJavaCharacter(c, i == 0)) {
		sb.append(chars[i]);
	    } else {
		charToUnicode(chars[i], sb);
	    }
	}

	return sb.toString();
    }


    /**
     * Parse and strip the class file.
     */
    private void stripClassFile() throws IOException {
	magic = dis.readInt();
	minor_version = dis.readShort();
	major_version = dis.readShort();
	constant_pool_count = dis.readUnsignedShort();
	constant_pool = new ConstantPool[constant_pool_count-1];

	// Loop through this parseing twice - on the first pass,
	// note illegal identifier names to "correct" on the second pass.

	int constantPoolOffset = bs.getOffset();
	for (int pass = 0; pass < 2; ++pass) {

	    boolean pass1 = (pass == 0);
	    boolean pass2 = (pass != 0);

	    // Parse the constant pool.

	    bs.setOffset(constantPoolOffset);
	    for (int i = 0; i < constant_pool_count - 1; ++i) {
		ConstantPool entry = constant_pool[i];
		entry = new ConstantPool(pass2 && entry.invalid);
		constant_pool[i] = entry;

		// Some constants take multiple entries .....
		if (entry.tag == CONSTANT_Long
		    || entry.tag == CONSTANT_Double) ++i;
	    }

	    // Verify the constant pool (for illegal Java identifiers)

	    for (int i = 0; i < constant_pool_count - 1; ++i) {
		ConstantPool entry = constant_pool[i];
		entry.verify(constant_pool);

		// Some constants take multiple entries .....
		if (entry.tag == CONSTANT_Long
		    || entry.tag == CONSTANT_Double) ++i;
	    }

	    // Parse methods, fields, etc.

	    dis.skip(2);				// access_flags
	    int classIndex = dis.readUnsignedShort();	// this_class
	    int superIndex = dis.readUnsignedShort();	// super_class

	    short interfaces_count = dis.readShort();
	    for (int i = 0; i < interfaces_count; ++i) {
		int name_index = dis.readUnsignedShort();
	    }

	    short fields_count = dis.readShort();
	    for (int i = 0; i < fields_count; ++i) {
		parseFieldInfo();
	    }

	    short methods_count = dis.readShort();
	    for (int i = 0; i < methods_count; ++i) {
		parseMethodInfo();
	    }
	}
	    
	// The attribbutes are parsed only once.  Remove those which
	// are not required.

	int attributeCountOffset = bs.getOffset();
	short attributes_count = dis.readShort();

	for (int i = 0; i < attributes_count; ++i) {
	    int attributeOffset = bs.getOffset();

	    // If this attribute is not required, remove it.
	    // -- decrement the attribute count.
	    // -- compress the attribute out of the file.
	    if (!parseAttributeInfo()) {
		bs.decrementCount(attributeCountOffset);
		bs.removeBytes(attributeOffset, bs.getOffset());
		bs.setOffset(attributeOffset);
	    }
	}
    }

    private class ConstantPool {
	byte tag;
	int nameIndex;
	String string;
	int stringOffset;
	int descriptorIndex;
	boolean invalid;

	ConstantPool(boolean fixMe) throws IOException {
	    tag = (byte) dis.read();
	    //System.err.print("new ConstantPool(" + tag + ")");
	    switch (tag) {
	    case CONSTANT_Class:
	    case CONSTANT_String:
		nameIndex = dis.readUnsignedShort();
		break;
	    case CONSTANT_Fieldref:
	    case CONSTANT_Methodref:
	    case CONSTANT_InterfaceMethodref:
		nameIndex = dis.readUnsignedShort();
		descriptorIndex = dis.readUnsignedShort();
		break;
	    case CONSTANT_Integer:
	    case CONSTANT_Float:
		dis.skip(4);
		break;

	    case CONSTANT_NameAndType:
		nameIndex = dis.readUnsignedShort();
		descriptorIndex = dis.readUnsignedShort();
		break;
	    case CONSTANT_Long:
	    case CONSTANT_Double:
		dis.skip(8);
		break;
	    case CONSTANT_Utf8:
		stringOffset = bs.getOffset();
		string = dis.readUTF();

		if (fixMe) {
		    int utfLength = bs.getOffset() - stringOffset;
		    ByteArrayOutputStream baos = new ByteArrayOutputStream();
		    DataOutputStream dos = new DataOutputStream(baos);
		    string = makeValid(string);
		    dos.writeUTF(string);
		    byte[] newUTF = baos.toByteArray();
		    baos = null;

		    bs.addBytes(stringOffset, newUTF.length - utfLength);
		    bs.setOffset(stringOffset);
		    bs.write(newUTF);
		}
		break;
	    default:
		//System.out.println();
		throw new IOException();
	    }
	    //System.out.println();
	}

	void verify(ConstantPool[] constant_pool) throws IOException {
	    //System.err.print("new ConstantPool(" + tag + ")");
	    switch (tag) {
	    case CONSTANT_Class:
	    case CONSTANT_String:
	    case CONSTANT_Integer:
	    case CONSTANT_Float:
	    case CONSTANT_Long:
	    case CONSTANT_Double:
	    case CONSTANT_Utf8:
		// do nothing.
		break;
	    case CONSTANT_Fieldref:
		validateClassName(nameIndex);
		break;
	    case CONSTANT_Methodref:
		validateClassName(nameIndex);
		break;
	    case CONSTANT_InterfaceMethodref:
		validateClassName(nameIndex);
		break;
	    case CONSTANT_NameAndType:
		validateName(nameIndex);
		validateName(descriptorIndex);
		break;
	    default:
		throw new IOException();
	    }
	}
    }

    private void parseFieldInfo() throws IOException {
	int accessFlags = dis.readUnsignedShort();
	int nameIndex = dis.readUnsignedShort();
	validateName(nameIndex);
	int descriptorIndex = dis.readUnsignedShort();
	validateName(descriptorIndex);
	int attributes_count = dis.readUnsignedShort();
	for (int i = 0; i < attributes_count; ++i) {
	    parseAttributeInfo();
	}
    }

    private void parseMethodInfo() throws IOException {
	dis.skip(2);
	int nameIndex = dis.readUnsignedShort();
	validateName(nameIndex);
	int descriptorIndex = dis.readUnsignedShort();
	validateName(descriptorIndex);
	int attributes_count = dis.readUnsignedShort();
	for (int i = 0; i < attributes_count; ++i) {
	    parseAttributeInfo();
	}
    }

    // From the VM spec - about required attributes:
    //
    // Of the predefined attributes, the Code, ConstantValue, and
    // Exceptions attributes must be recognized and correctly read by a
    // class file reader for correct interpretation of the class file by
    // a Java virtual machine implementation. The InnerClasses and
    // Synthetic attributes must be recognized and correctly read by a
    // class file reader in order to properly implement the Java and
    // Java 2 platform class libraries (section 3.12). Use of the remaining
    // predefined attributes is optional; a class file reader may use
    // the information they contain, or otherwise must silently ignore
    // those attributes.

    private boolean parseAttributeInfo() throws IOException {
	int attribute_name_index = dis.readUnsignedShort();
	ConstantPool entry = constant_pool[attribute_name_index-1];
	String type = entry.string;

	// Code attribute requires additional parsing to remove nested
	// attributes.
	if (type.equals("Code")) {
	    parseCodeAttribute();
	    return true;
	}

	int length = dis.readInt();
	dis.skip(length);
	return attributeRequired(type);
    }

    /**
     * Return true if this attribute is one if the types required for
     * execution of this class by the VM.
     */
    boolean attributeRequired(String type) {
	for (int i = 0; i < requiredTypes.length; ++i) {
	    if (type.equals(requiredTypes[i])) {
		return true;
	    }
	}
	return false;
    }


    private void parseCodeAttribute() throws IOException {
	int attributeLengthOffset = bs.getOffset();
	int length = dis.readInt();
	dis.skip(2);
	dis.skip(2);
	int codeLength = dis.readInt();
	dis.skip(codeLength);
	int exceptionTableLength = dis.readUnsignedShort();
	dis.skip(8 * exceptionTableLength);

	int attributeCountOffset = bs.getOffset();
	int attributes_count = dis.readUnsignedShort();
	for (int i = 0; i < attributes_count; ++i) {
	    int attributeOffset = bs.getOffset();

	    // If this attribute is not required, remove it.
	    // -- decrement the attribute count.
	    // -- compress the attribute out of the file.
	    if (!parseAttributeInfo()) {
		int diff = bs.getOffset() - attributeOffset;
		bs.decrementCount(attributeCountOffset);
		bs.removeBytes(attributeOffset, bs.getOffset());
		bs.setOffset(attributeOffset);
		bs.decrementLength(attributeLengthOffset, diff);
	    }
	}
    }
}
