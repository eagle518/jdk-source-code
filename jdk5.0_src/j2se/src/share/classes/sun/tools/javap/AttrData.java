/*
 * @(#)AttrData.java	1.11 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */



package sun.tools.javap;

import java.io.*;

/**
 * Reads and stores attribute information.
 *
 * @author  Sucheta Dambalkar (Adopted code from jdis)
 */
class AttrData {
    ClassData cls;
    int name_cpx;
    int datalen;
    byte data[];
    
    public AttrData (ClassData cls) {
	this.cls=cls;
    }
    
    /**
     * Reads unknown attribute.
     */ 
    public void read(int name_cpx, DataInputStream in) throws IOException {
	this.name_cpx=name_cpx;
	datalen=in.readInt();
	data=new byte[datalen];
       	in.readFully(data);
    }
    
    /**
     * Reads just the name of known attribute.
     */
    public void read(int name_cpx){
	this.name_cpx=name_cpx;
    }
    
    /**
     * Returns attribute name.
     */
    public String getAttrName(){
	return cls.getString(name_cpx);
    }

    /**
     * Returns attribute data.
     */
    public byte[] getData(){
	return data;
    }
}

