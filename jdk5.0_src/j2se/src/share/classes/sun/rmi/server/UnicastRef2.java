/*
 * @(#)UnicastRef2.java	1.12 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.rmi.server;

import java.io.IOException;
import java.io.ObjectInput;
import java.io.ObjectOutput;
import sun.rmi.transport.LiveRef;

public class UnicastRef2 extends UnicastRef {

    /**
     * Create a new (empty) Unicast remote reference.
     */
    public UnicastRef2()
    {}
    
    /** 
     * Create a new Unicast RemoteRef.
     */
    public UnicastRef2(LiveRef liveRef) {
	super(liveRef);
    }

    /**
     * Returns the class of the ref type to be serialized
     */
    public String getRefClass(ObjectOutput out)
    {
	return "UnicastRef2";
    }

    /**
     * Write out external representation for remote ref.
     */
    public void writeExternal(ObjectOutput out) throws IOException 
    {
	ref.write(out, true);
    }

    /**
     * Read in external representation for remote ref.
     * @exception ClassNotFoundException If the class for an object
     * being restored cannot be found.
     */
    public void readExternal(ObjectInput in)
	throws IOException, ClassNotFoundException
    {
	ref = LiveRef.read(in, true);
    }
}
