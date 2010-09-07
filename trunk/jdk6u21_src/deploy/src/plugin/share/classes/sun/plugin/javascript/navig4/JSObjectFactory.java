/*
 * @(#)JSObjectFactory.java	1.9 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.javascript.navig4;

import netscape.javascript.JSException;
import sun.plugin.javascript.navig.JSObject;
import sun.plugin.javascript.navig.JSType;


/** 
 * <p> JSObjectFactory is a factory class for JSObject in Navigator 4.x.
 * </p>
 */
public class JSObjectFactory extends sun.plugin.javascript.navig.JSObjectFactory {
    
    /** 
     * <p> Create a new JSObject.
     * </p>
     *
     * @param js JSObject instance.
     * @param type JSObject type.
     * @param instance Plugin instance.
     * @param context Evaluation context.
     * @param custom Custom object for JSObject creation.
     * @return New JSObject with type "type".
     */
    public Object resolveObject(JSObject js, String type, int instance, String context, Object custom) 
    throws JSException
    {
	Object result = type;

	if (type != null && type.indexOf("[") != -1)  {

	    if (type.indexOf(JSType.Window) != -1)
		return new Window(instance, context);
	    else if (type.indexOf(JSType.Anchor) != -1) 
		return new Anchor(instance, context);
	    else if (type.indexOf(JSType.Document) != -1) 
		return new Document(instance, context);
	    else if (type.indexOf(JSType.Link) != -1)
		return new Link(instance, context);
	    else if (type.indexOf(JSType.Layer) != -1)
		return new Layer(instance, context);
	    else if (type.indexOf(JSType.Navigator) != -1)
		return new Navigator(instance);
	    else if (type.indexOf(JSType.UIBar) != -1)
		return new UIBar(instance, context);
	    else if (type.indexOf(JSType.LayerArray) != -1)  {
		int length = 0;

		try {
		    Object val = js.eval(context + ".length");
		    length = Integer.parseInt(val.toString().trim());
		} catch (Throwable e)  {
		    throw new JSException("resolveObject does not support " + toString() + ".length");
		}

		return new LayerArray(instance, context, length);
	    }
	    else    {
	        return super.resolveObject(js, type, instance, context, custom);
	    }
	}

	return result;
    }
}
