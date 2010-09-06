/*
 * @(#)ConstantPrinter.java	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * Provides printing related access to the BinaryConstantPool.
 * Prints constant and provides other utilities.
 *
 */

package sun.tools.javap.oldjavap;

import java.io.PrintWriter;

import sun.tools.java.BinaryConstantPool;
import sun.tools.java.ClassDeclaration;
import sun.tools.java.ClassDefinition;
import sun.tools.java.Constants;

/**
 * This class is used to print constants in the constant pool
 */
public final
class ConstantPrinter implements Constants {

    BinaryConstantPool cpool;
    private JavaPEnvironment env;
    private PrintWriter output;
    private boolean decodeTypeSignatures;


    /**
     * Constructor
     */
    public
    ConstantPrinter(BinaryConstantPool cpool, JavaPEnvironment env, PrintWriter output, boolean decodeTypeSignatures) {
	this.cpool = cpool;
	this.env = env;
	this.output = output;
	this.decodeTypeSignatures = decodeTypeSignatures;
    }

    /**
     * get a string
     */
    public String getString(int n) {
	return cpool.getString(n);
    }

    /**
     * Print a constant name and its type
     */
    private void printNameAndType(int key, String separator, boolean decodeType) {
	int fieldnameindex = key >> 16;
	int fieldtypeindex = key & 0xFFFF;
	output.print(cpool.getString(fieldnameindex) + separator);
    	if (decodeType) 
	    output.print(cpool.getType(fieldtypeindex));
    	else 
	    output.print(cpool.getString(fieldtypeindex));
    }

    /**
     * Print a constant name and its type from an index
     */
    private void printNameAndTypeFromIndex(int index, String separator) {
	int key = cpool.getInteger(index); 
	printNameAndType(key, separator, decodeTypeSignatures);
    }
    
    /**
     * Print a reference
     */
    private void printRef(int index, String separator) {
    	if (decodeTypeSignatures) {
            output.print(cpool.getConstant(index, env));
    	} else {
            int key = cpool.getInteger(index);
	    int classkey = key >> 16; 
	    int nametypekey = key & 0xFFFF;
	    output.print(cpool.getDeclaration(env, classkey).getName());
	    output.print(".");
	    printNameAndTypeFromIndex(nametypekey, separator);
	}
    }
    	    
    /**
     * Print a constant in the constant pool
     */
    public void printClassDeclaration(ClassDeclaration cdcl) {
    	ClassDefinition cdef = cdcl.getClassDefinition();
    	if (cdef != null && cdef.isInterface())
	    output.print("<Interface ");
	else
	    output.print("<Class ");
    	output.print(cdcl.getName());
	output.print(">");
    }
    	    
    /**
     * Print a constant in the constant pool
     */
    public void printConstant(int index) {
    	int typeId = cpool.getConstantType(index);
        switch(typeId) {
        case CONSTANT_STRING: 
	    output.print("<String \"" + cpool.getConstant(index, env) + "\"");
	    break;
	   
        case CONSTANT_UTF8:
            output.print("<\"" + cpool.getString(index) + "\"");
	    break;
	
        case CONSTANT_INTEGER:
	    output.print("<Integer " + cpool.getInteger(index));
	    break;

        case CONSTANT_FLOAT:
	    output.print("<Real " + cpool.getValue(index));
	    break;

        case CONSTANT_LONG: 
	    output.print("<Long " + cpool.getValue(index));
	    break;

        case CONSTANT_DOUBLE:
	    output.print("<Double " + cpool.getValue(index));
	    break;

        case CONSTANT_CLASS:  
	    printClassDeclaration(cpool.getDeclaration(env, index));
	    return;

	case CONSTANT_METHOD: 
	    output.print("<Method ");
	    printRef(index, "");
	    break;
	
        case CONSTANT_FIELD:  
	    output.print("<Field ");
	    printRef(index, " ");
	    break;
	    
	case CONSTANT_INTERFACEMETHOD: 
	    output.print("<InterfaceMethod ");
	    printRef(index, "");
	    break;
	    	    
        case CONSTANT_NAMEANDTYPE:
	    output.print("<NameAndType");
	    printNameAndTypeFromIndex(index, " ");
	    break;
	    
	default:
	    output.print("<Unknown " + typeId);
	    break;
	
        }
        output.print('>');
    }

}
