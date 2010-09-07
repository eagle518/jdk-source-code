/*
 * @(#)JSObjectFactory.java	1.10 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.javascript.navig;

import netscape.javascript.JSException;
import sun.plugin.javascript.navig.JSObject;

/** 
 * <p> JSObjectFactory is a factory class for JSObject in Navigator 3.x.
 * </p>
 */
public class JSObjectFactory implements JSObjectResolver {
    
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

	    if (type.indexOf("Array]") == -1)  {
		// The result is not an array

		if (type.indexOf(JSType.Window) != -1)
		    return new Window(instance, context);
		else if (type.indexOf(JSType.Anchor) != -1) 
		    return new Anchor(instance, context);
		else if (type.indexOf(JSType.Document) != -1) 
		    return new Document(instance, context);
		else if (type.indexOf(JSType.Element) != -1)
		    return new Element(instance, context, (Form)custom);
		else if (type.indexOf(JSType.Form) != -1)
		    return new Form(instance, context);
		else if (type.indexOf(JSType.History) != -1)
		    return new History(instance, context);
		else if (type.indexOf(JSType.Image) != -1)
		    return new Image(instance, context);
		else if (type.indexOf(JSType.Link) != -1)
		    return new Link(instance, context);
		else if (type.indexOf(JSType.Location) != -1)
		    return new Location(instance, context);
		else if (type.indexOf(JSType.Navigator) != -1)
		    return new Navigator(instance);
		else if (type.indexOf(JSType.Option) != -1)
		    return new Option(instance, context);
		else if (type.indexOf(JSType.URL) != -1)
		    return new URL(instance, context);
		else
		    throw new JSException(type + " cannot be resolved as JSObject.");
	    }
	    else {
		// This is an array object.
		int length = 0;

		try {
		    Object val = js.eval(context + ".length");
		    length = Integer.parseInt(val.toString().trim());
		} catch (Throwable e)  {
		    throw new JSException("resolveObject does not support " + toString() + ".length");
		}

		if (type.indexOf(JSType.AnchorArray) != -1) 
		    return new AnchorArray(instance, context, length);
		else if (type.indexOf(JSType.ElementArray) != -1)
		    return new ElementArray(instance, context, length, (Form)custom);
		else if (type.indexOf(JSType.FormArray) != -1)
		    return new FormArray(instance, context, length);
		else if (type.indexOf(JSType.FrameArray) != -1)
		    return new FrameArray(instance, context, length);
		else if (type.indexOf(JSType.ImageArray) != -1)
		    return new ImageArray(instance, context, length);
		else if (type.indexOf(JSType.LinkArray) != -1)
		    return new LinkArray(instance, context, length);
		else if (type.indexOf(JSType.OptionArray) != -1)
		    return new OptionArray(instance, context, length);
		else if (type.indexOf(JSType.AppletArray) != -1)
		    return new AnchorArray(instance, context, length);
		else if (type.indexOf(JSType.EmbedArray) != -1)
		    return new AnchorArray(instance, context, length);
		else
		    throw new JSException(type + " cannot be resolved as JSObject.");
	    }
	}

	return result;
    }
}
