/*
 * @(#)CNBindingEnumeration.java	1.9 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.jndi.cosnaming;

import javax.naming.*;
import javax.naming.spi.NamingManager;

import java.util.NoSuchElementException;
import java.util.Hashtable;

import org.omg.CosNaming.*;
import org.omg.CosNaming.NamingContextPackage.*;
import org.omg.CORBA.*;

/**
  * Implements the JNDI NamingEnumeration interface for COS
  * Naming. Gets hold of a list of bindings from the COS Naming Server 
  * and allows the client to iterate through them.
  *
  * @author Raj Krishnamurthy
  * @author Rosanna Lee
  * @version 1.9 03/12/19
  */

final class CNBindingEnumeration implements NamingEnumeration {

    private static final int DEFAULT_BATCHSIZE = 100;
    private BindingListHolder _bindingList; // list of bindings
    private BindingIterator _bindingIter;   // iterator for getting list of bindings
    private int counter;		    // pointer in _bindingList
    private int batchsize = DEFAULT_BATCHSIZE;  // how many to ask for each time
    private CNCtx _ctx;			    // ctx to list
    private Hashtable _env;		    // environment for getObjectInstance
    private boolean more = false;	    // iterator done?

  /**
    * Creates a CNBindingEnumeration object.
    * @param ctx Context to enumerate
    */

  CNBindingEnumeration(CNCtx ctx, Hashtable env) {
    // Get batch size to use
    String batch = (env != null ? 
	(String)env.get(javax.naming.Context.BATCHSIZE) : null);
    if (batch != null) {
	try {
	    batchsize = Integer.parseInt(batch);
	} catch (NumberFormatException e) {
	    throw new IllegalArgumentException("Batch size not numeric: " + batch);
	}
    }

    _ctx = ctx;
    _env = env;
    _bindingList = new BindingListHolder();
    BindingIteratorHolder _bindingIterH = new BindingIteratorHolder();

    // Perform listing and request that bindings be returned in _bindingIter
    // Upon return,_bindingList returns a zero length list
    _ctx._nc.list(0, _bindingList, _bindingIterH);

    _bindingIter = _bindingIterH.value;

    // Get first batch using _bindingIter
    if (_bindingIter != null) {
	more = _bindingIter.next_n(batchsize, _bindingList);
    } else {
	more = false;
    }
    counter = 0;
  }

  /**
    * Returns the next binding in the list.
    * @exception NamingException any naming exception.
    */

  public java.lang.Object next() throws NamingException {
      if (more && counter >= _bindingList.value.length) {
	  getMore();
      }
      if (more && counter < _bindingList.value.length) { 
	  org.omg.CosNaming.Binding bndg = _bindingList.value[counter];
	  counter++;
	  return mapBinding(bndg);
      } else {
	  throw new NoSuchElementException();
      }
  }


  /**
    * Returns true or false depending on whether there are more bindings.
    * @return boolean value
    */

  public boolean hasMore() throws NamingException {
      // If there's more, check whether current bindingList has been exhausted,
      // and if so, try to get more.
      // If no more, just say so.
      return more ? (counter < _bindingList.value.length || getMore()) : false;
  }

  /**
    * Returns true or false depending on whether there are more bindings.
    * Need to define this to satisfy the Enumeration api requirement.
    * @return boolean value
    */

  public boolean hasMoreElements() {
      try {
	  return hasMore();
      } catch (NamingException e) {
	  return false;
      }
  }

  /**
    * Returns the next binding in the list.
    * @exception NoSuchElementException Thrown when the end of the
    * list is reached.
    */

    public java.lang.Object nextElement() {
	try {
	    return next();
	} catch (NamingException ne) {
	    throw new NoSuchElementException();
	}
  }

    public void close() {
	more = false;
	if (_bindingIter != null) {
	    _bindingIter.destroy();
	    _bindingIter = null;
	}
    }

    protected void finalize() {
	close();
    }

    /**
     * Get the next batch using _bindingIter. Update the 'more' field.
     */
    private boolean getMore() throws NamingException {
	try {
	    more = _bindingIter.next_n(batchsize, _bindingList);
	    counter = 0; // reset
	} catch (Exception e) {
	    more = false;
	    NamingException ne = new NamingException(
		"Problem getting binding list");
	    ne.setRootCause(e);
	    throw ne;
	}
	return more;
    }

  /**
    * Constructs a JNDI Binding object from the COS Naming binding
    * object. 
    * @exception NameNotFound No objects under the name.
    * @exception CannotProceed Unable to obtain a continuation context
    * @exception InvalidName Name not understood.
    * @exception NamingException One of the above.
    */

    private javax.naming.Binding mapBinding(org.omg.CosNaming.Binding bndg) 
		throws NamingException {
	java.lang.Object obj = _ctx.callResolve(bndg.binding_name);

	Name cname = CNNameParser.cosNameToName(bndg.binding_name);

	try {
	    obj = NamingManager.getObjectInstance(obj, cname, _ctx, _env);
	} catch (NamingException e) {
	    throw e;
	} catch (Exception e) {
	    NamingException ne = new NamingException(
			"problem generating object using object factory");
	    ne.setRootCause(e);
	    throw ne;
	}

	// Use cname.toString() instead of bindingName because the name
	// in the binding should be a composite name
	String cnameStr = cname.toString();
	javax.naming.Binding jbndg = new javax.naming.Binding(cnameStr, obj);

	NameComponent[] comps = _ctx.makeFullName(bndg.binding_name);
	String fullName = CNNameParser.cosNameToInsString(comps);	
	jbndg.setNameInNamespace(fullName);
	return jbndg;
  }
}
