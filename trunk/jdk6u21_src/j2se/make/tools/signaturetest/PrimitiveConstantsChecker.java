/*
 * @(#)PrimitiveConstantsChecker.java	1.9 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
        
package javasoft.sqe.tests.api.SignatureTest;

import java.io.ByteArrayInputStream;
import java.io.DataInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Properties;
import java.util.Vector;

/** This class scans class files for founding fields which are primitive constants.
 *  In definition of these fields PRIMITIVE_CONSTANT modifier will be added during
 *  formating. **/
class PrimitiveConstantsChecker extends DefinitionFormat implements ClassConstants {
    /** ClassCollection which stores names of the primitive constants for
     *  each class. **/
    private ClassCollection primitiveConstants = new ClassCollection();
    /** name of the scanned class which found in the class file. **/
    String ClassName = "N/A";
    /** represents class file as stream. **/
    private DataInputStream in;
    /** types founded in the constant pool.**/
    private byte types[];
    /** constant pool. **/
    private Object cpool[];

    /** creates new DefinitionFormat.
     *  @param isQualifiedName specify if qualified names will be used.
     *  @param isThrowsTracked specify if throws clause will be tracked.**/
    public PrimitiveConstantsChecker(boolean isQualifiedName, boolean isThrowsTracked) {
        super(isQualifiedName, isThrowsTracked, new Properties());
    }

    /** creates new DefinitionFormat.
     *  @param isQualifiedName specify if qualified names will be used.
     *  @param isThrowsTracked specify if throws clause will be tracked.
     *  @param removedModif modifiers which are required to be deleted. **/
    public PrimitiveConstantsChecker(boolean isQualifiedName, boolean isThrowsTracked,
                                     String[][] removedModif) {
        super(isQualifiedName, isThrowsTracked, removedModif);
    }
        
    /** return formatted definition. **/
    public String getDefinition(String definition) {
        if (definition.startsWith(FIELD) && isQualifiedName &&
            isPrimitiveConstant(definition) &&
            !definition.startsWith(FIELD + PRIMITIVE_CONSTANT + " "))
            return super.getDefinition(FIELD + PRIMITIVE_CONSTANT + " " +
                                       definition.substring(FIELD.length()));
        else
            return super.getDefinition(definition);
    }

    /** scans class file with given class name for founding primitive constants.
     *  @param className name of the class
     *  @param inClass class file as stream. **/
    public void checkPrimitiveConstants(String className, InputStream inClass)
        throws IOException { 
	this.in = new DataInputStream(inClass);
        ClassName = "N/A";
        
        Vector fieldNames = null;
        // Read the header
        int magic = in.readInt();
        int min_version = in.readUnsignedShort();
        int version = in.readUnsignedShort();

        // Read the constant pool
        readConstantPool();
        short access = in.readShort(); // don't care about sign 
        int this_cpx = in.readUnsignedShort();

        try {
            ClassName = (String)cpool[((Integer)cpool[this_cpx]).intValue()];
        } catch (Exception e) {
        }
        int super_cpx = in.readUnsignedShort();
        // skips the interface names
        int numinterfaces = in.readUnsignedShort();
        in.skipBytes(numinterfaces * 2);
        // Read the fields
        fieldNames = decodeMembers();
        this.in.close();
        inClass.close();
        primitiveConstants.putVector(className, fieldNames);
    }

    /** Returns true if before the field is founded as primitive constants or
     *  if definition contains modifier PRIMITIVE_CONSTANT
     *  @param def tested definition. **/
    public boolean isPrimitiveConstant(String def) {
        if (def.startsWith(FIELD)) {
            String name  = def.substring(def.lastIndexOf(' ') + 1);
            int pos = name.lastIndexOf('.');
            String className = (pos >= 0) ? name.substring(0, pos) : name;
            Vector h = primitiveConstants.get(className);
            if ((h != null) && 
                h.contains(name.substring(name.lastIndexOf('.') + 1)))
                return true;
            else {
                pos = def.lastIndexOf(' ') - 1;
                pos = def.lastIndexOf(' ') - 1;
                return (def.lastIndexOf(" " + PRIMITIVE_CONSTANT, pos) >= 0);
            }
        }
        return false;
    }

    /** read constant pool. **/
    private void readConstantPool() throws IOException {
	int length = in.readUnsignedShort();
	int CPlen = length;
	types = new byte[length];
	cpool = new Object[length];
	for (int i = 1 ; i < length ; i++) {
            byte tag;
            int v1, v2, n = i; long lv;
	    tag = in.readByte();
	    switch(types[i] = tag) {
            case CONSTANT_UTF8:
		cpool[i] = in.readUTF();
		break;
            case CONSTANT_INTEGER:
		v1 = in.readInt();
		cpool[i] = new Integer(v1);
		break;
            case CONSTANT_FLOAT:
		v1 = Float.floatToIntBits(in.readFloat());
		cpool[i] = new Integer(v1);
		break;
            case CONSTANT_LONG:
		lv = in.readLong();
		cpool[i] = new Long(lv);
		i++;
		break;
            case CONSTANT_DOUBLE:
		lv = Double.doubleToLongBits(in.readDouble());
		cpool[i] = new Long(lv);
		i++;
		break;
            case CONSTANT_CLASS:
            case CONSTANT_STRING:
		v1 = in.readUnsignedShort();
		cpool[i] = new Integer(v1);
		break;		
            case CONSTANT_INTERFACEMETHOD:
            case CONSTANT_FIELD:
            case CONSTANT_METHOD:
            case CONSTANT_NAMEANDTYPE:
		cpool[i] = "#"+in.readUnsignedShort()+" #"+in.readUnsignedShort();
		break;
            default:
		CPlen=i;
		throw new ClassFormatError();
	    }
	}
    }

    /** decode member record in the class file. **/
    private Vector decodeMembers() throws IOException {
        Vector retVal = new Vector();
	int nfields = in.readUnsignedShort();
        boolean isPrimitiveConstant = false;
        for (int i = 0 ; i < nfields ; i++) {
            int access = in.readShort();
            isPrimitiveConstant = ((((access & ACC_PUBLIC) != 0) || 
                                    ((access & ACC_PROTECTED) != 0)) &&
                                   ((access & ACC_STATIC) != 0) &&
                                   ((access & ACC_FINAL) !=0));
            int name_cpx = in.readUnsignedShort();
            int sig_cpx = in.readUnsignedShort();
            // Read the attributes
            if (decodeAttributes() && isPrimitiveConstant)
                retVal.addElement(cpool[name_cpx]);
        }
        return retVal;
    }

    /** decodes all attributes of the member. **/
    private boolean decodeAttributes() throws IOException {
	// Read the attributes
        boolean hasValue = false;
	int attr_num = in.readUnsignedShort();
	for (int i = 0 ; i < attr_num ; i++) {
            if (decodeAttribute())
                hasValue = true;
	}
        return hasValue;
    }

    /** decodes single attribute. **/
    private boolean decodeAttribute() throws IOException {
	// Read one attribute
      
        int name_cpx = in.readUnsignedShort(), tag, len;
        String AttrName = "";
        try {
            tag = types[name_cpx];
            if (tag == CONSTANT_UTF8) {
                AttrName = (String)cpool[name_cpx];
            }
        } catch (ArrayIndexOutOfBoundsException e) {
        }
        len = in.readInt();
        byte attr[] = new byte[len];
        in.read(attr);
        return AttrName.equals("ConstantValue");
    }
}

    

