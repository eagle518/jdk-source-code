/*
 * @(#)XMBeanOperations.java	1.5 04/04/13
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
package sun.tools.jconsole.inspector;

import javax.management.*;
import java.util.ArrayList;

import sun.tools.jconsole.MBeansTab;

public class XMBeanOperations extends XOperations {

    public XMBeanOperations(MBeansTab mbeansTab) {
	super(mbeansTab);
    }

    protected MBeanOperationInfo[] updateOperations(MBeanOperationInfo[] operations) {
	//remove get,set and is
	ArrayList<MBeanOperationInfo> mbeanOperations = 
	new ArrayList<MBeanOperationInfo>(operations.length);
	for(MBeanOperationInfo operation : operations) {
	    if (!( (operation.getSignature().length == 0 && 
		    operation.getName().startsWith("get") &&
		    !operation.getReturnType().equals("void"))  || 
		   (operation.getSignature().length == 1 && 
		    operation.getName().startsWith("set") &&
		    operation.getReturnType().equals("void")) ||
		   (operation.getName().startsWith("is") && 
		    operation.getReturnType().equals("boolean"))
		   ) ) {
		mbeanOperations.add(operation);
	    }
	}
	return  (MBeanOperationInfo[]) 
	mbeanOperations.toArray(new MBeanOperationInfo[0]);
    }

}
		 
		
							    
	    
       
