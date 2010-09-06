/*
 * @(#)InnerClassData.java	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */


package sun.tools.javap;

import java.io.*;
import java.util.*;

/**
 * Strores InnerClass data informastion.
 *
 * @author  Sucheta Dambalkar (Adopted code from jdis)
 */
class InnerClassData  implements RuntimeConstants {
    ClassData cls;
    
    
    int inner_class_info_index
	,outer_class_info_index
	,inner_name_index
	,access
	;
    
    public InnerClassData(ClassData cls) {
	this.cls=cls;
	
    }
    
    /**
     * Read Innerclass attribute data.
     */
    public void read(DataInputStream in) throws IOException {
	inner_class_info_index = in.readUnsignedShort();
	outer_class_info_index = in.readUnsignedShort();
	inner_name_index = in.readUnsignedShort();
	access = in.readUnsignedShort();
    }  // end read

    /**
     * Returns the access of this class or interface.
     */
    public String[] getAccess(){
	Vector v = new Vector();
	if ((access & ACC_PUBLIC)   !=0) v.addElement("public");
	if ((access & ACC_FINAL)    !=0) v.addElement("final");
	if ((access & ACC_ABSTRACT) !=0) v.addElement("abstract");
	String[] accflags = new String[v.size()];
	v.copyInto(accflags);
	return accflags;
    }
    
} // end InnerClassData

