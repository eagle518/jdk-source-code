/*
 * @(#)AbstractPlugin.java	1.13 10/03/24
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin2.main.server;

import java.io.*;
import netscape.javascript.*;
import sun.plugin2.liveconnect.*;
import sun.plugin2.message.*;
import sun.plugin2.util.SystemUtil;

/** This abstract base class provides a significant portion of the
    browser-side framework for supporting LiveConnect (bi-directional
    Java/JavaScript communication). It turns out that there are common
    concepts between the scripting engines for Internet Explorer and
    the Mozilla browsers, and large portions of code can be shared
    between them, with only small native methods that need to be
    customized for argument marshaling. <P>

    Any browser which has many of the same concepts (such as a
    "variant" or other boxing type for JavaScript values, passing
    argument lists in the form of variant arrays, the use of stable
    C-like pointers to refer to JavaScript objects, etc.) and which
    can conform to the method signatures of this class can derive its
    Java-side plugin class from this one and take advantage of the
    associated native code to hide the necessary JNI work. The
    signatures of the methods in this class can also, of course, be
    expanded to provide support for more browsers if necessary.
*/

public abstract class AbstractPlugin implements Plugin {
    protected static final boolean DEBUG = (SystemUtil.getenv("JPI_PLUGIN2_DEBUG") != null);
    protected static final boolean VERBOSE = (SystemUtil.getenv("JPI_PLUGIN2_VERBOSE") != null);

    // The applet ID this plugin is running
    private AppletID appletID;
    
    // The platform-dependent ResultHandler needed to properly wait
    // for LiveConnect results
    private ResultHandler resultHandler;

    // number of active Java Scripts calls on the stack
    private int activeJScripts=0; 

    // Mark the thread that stopApplet() call is on the stack
    protected static final ThreadLocal appletStopMark = new ThreadLocal();

    /** Initializes this AbstractPlugin with the applet's ID and a
        ResultHandler to be able to wait for LiveConnect results
        without blocking the browser's ability to repaint itself. This
        must be called by subclasses once an AppletID is available. */
    protected void appletStarted(AppletID appletID,
                                 ResultHandler resultHandler) {
        this.appletID = appletID;
        this.resultHandler = resultHandler;
    }

    // The amount of time we wait for an acknowledgment of a request
    // to stop an applet, in milliseconds
    protected static final long STOP_ACK_DELAY = 1100; // ms

    // The amount of time to reduce for wait once we find recursive stop
    protected static final long STOP_ACK_DELAY_REDUCTION = 900; // ms
    
    /** Indicates to this AbstractPlugin that the applet has been
        stopped. This should be called by subclasses once the applet
        has been stopped. */
    protected void appletStopped() {
        appletID = null;
        resultHandler = null;
    }

    /** Returns a number of active Java Script calls on the stack. */
    public int getActiveJSCounter(){
        return activeJScripts;
    }

    /** Increments a number of active Java Script calls on the stack. */
    public void incrementActiveJSCounter(){
        activeJScripts++;
    }

    /** Decrements a number of active Java Script calls on the stack. */
    public void decrementActiveJSCounter(){
        activeJScripts--;
    }
    

    //----------------------------------------------------------------------
    //                                                              
    // The following abstract methods must be overridden by subclasses
    // and largely provide argument marshaling and other functionality
    //

    /** Indicates whether argument lists (in the form of variant
        arrays) coming back and forth from the web browser are in
        reverse order from Java's convention. This is true on IE and
        false on Mozilla, for example. */
    protected abstract boolean scriptingObjectArgumentListsAreReversed();

    /** Allocates a "variant array" of the specified size,
        initializing the elements to the empty value. On IE, this
        allocates a VARIANTARG array and initializes each element to
        VT_EMPTY. On Mozilla, it allocates an NPVariant array and
        initializes each element to NPVariantType_Void. */
    protected abstract long allocateVariantArray(int size);

    /** Frees the given variant array of the specified size. */
    protected abstract void freeVariantArray(long array, int size);

    /** Sets the given browser-specific variant array element to the given boolean value. */
    protected abstract void setVariantArrayElement(long variantArray, int index, boolean value);
    /** Sets the given browser-specific variant array element to the given byte value. */
    protected abstract void setVariantArrayElement(long variantArray, int index, byte value);
    /** Sets the given browser-specific variant array element to the given char value. */
    protected abstract void setVariantArrayElement(long variantArray, int index, char value);
    /** Sets the given browser-specific variant array element to the given short value. */
    protected abstract void setVariantArrayElement(long variantArray, int index, short value);
    /** Sets the given browser-specific variant array element to the given int value. */
    protected abstract void setVariantArrayElement(long variantArray, int index, int value);
    /** Sets the given browser-specific variant array element to the given long value. */
    protected abstract void setVariantArrayElement(long variantArray, int index, long value);
    /** Sets the given browser-specific variant array element to the given float value. */
    protected abstract void setVariantArrayElement(long variantArray, int index, float value);
    /** Sets the given browser-specific variant array element to the given double value. */
    protected abstract void setVariantArrayElement(long variantArray, int index, double value);
    /** Sets the given browser-specific variant array element to the given String value. */
    protected abstract void setVariantArrayElement(long variantArray, int index, String value);
    /** Sets the given browser-specific variant array element to the
        given browser-specific scripting object. On IE the passed
        value is an IDispatch*; on Mozilla it is an NPObject*. A 0
        value indicates that the value should be set to "null", if the
        browser's scripting engine has a separate notion of "null" as
        opposed to the "empty" or "void" value. */
    protected abstract void setVariantArrayElementToScriptingObject(long variantArray, int index, long value);
    /** Sets the given browser-specific variant array element to the
        "empty" or "void" value, for browsers which distinguish
        between this value and the "null" value. */
    protected abstract void setVariantArrayElementToVoid(long variantArray, int index);

    /** This method converts a single browser-side "variant" value to
        a Java object. The variantArray is an array of VARIANTs on IE
        and an array of NPVariants on Mozilla. */
    protected abstract Object variantArrayElementToObject(long variantArray, int index);

    /** This method should look up or allocate a new browser-side
        scripting object for the given RemoteJavaObject, which is a
        reference to a real Java object in an attached JVM. The return
        value is browser-dependent; on IE it is an IDispatch*, and on
        Mozilla an NPObject*. The objectIsApplet argument is a
        concession to IE for short-circuiting certain IE-specific
        scripting requests and can be ignored for other browsers. */
    protected abstract long lookupScriptingObject(RemoteJavaObject object,
                                                  boolean objectIsApplet);

    /** This method takes in a scripting object from the browser
        (IDispatch*, NPObject*, etc.) and is expected to produce a
        Java object from it. If the incoming object is a native one to
        the browser, then it should be wrapped in a BrowserSideObject
        and registered with the LiveConnect support (to appropriately
        increment its reference count). If it is one that we created
        and passed to the browser to wrap a remote Java object, then
        we should unwrap it and return the underlying
        RemoteJavaObject. Note that some implementations may make the
        determination of whether the scripting object was created by
        us or not down in native code rather than in the Java
        implementation of this method. */
    protected abstract Object wrapOrUnwrapScriptingObject(long scriptingObject);

    /** Converts the given method identifier to a String. On IE the
        method identifier is a DISPID ordinarily represented as a Java
        int, and is managed by the IExplorerPlugin class. On Mozilla
        it is an NPIdentifier. */
    protected abstract String identifierToString(long methodIdentifier);

    /** This method should use the given browser-side "exception info"
        object to propagate the exception message to the JavaScript
        engine. */
    protected abstract void fillInExceptionInfo(long exceptionInfo, String message);

    /** This method should use the given browser-side "exception info"
        object to propagate the exception in the best form possible to
        the JavaScript engine. */
    protected abstract void fillInExceptionInfo(long exceptionInfo, Exception exc);
    
    //----------------------------------------------------------------------
    //
    // The following methods are called both by Java and native code
    // and may be optionally overridden
    //

    // Simple helper method -- kept private for now
    private void setVariantArrayElement(long variantArray, int index, BrowserSideObject value) {
        setVariantArrayElementToScriptingObject(variantArray, index, value.getNativeObjectReference());
    }

    // Simple helper method -- kept private for now
    private void setVariantArrayElement(long variantArray, int index, RemoteJavaObject value) {
        long scriptingObject = lookupScriptingObject(value, false);
        setVariantArrayElementToScriptingObject(variantArray, index, scriptingObject);
    }

    /** Helper method used to convert Java objects to browser-specific
        "variant" objects -- for example, VARIANTs on IE, and
        NPVariants on Mozilla. Note that for "void" return results
        from Java method invocations, the incoming Java object will
        not be null, but instead the value Void.TYPE. */
    protected void objectToVariantArrayElement(Object object,
                                               long variantArray,
                                               int index) {
        // Based on experience with Internet Explorer, don't assume
        // the JavaScript engine provides us an NPVariant* if the result
        // is just being discarded
        if (variantArray != 0) {
            if (object != null) {
                if (object instanceof Boolean) {
                    setVariantArrayElement(variantArray, index, ((Boolean) object).booleanValue());
                } else if (object instanceof Byte) {
                    setVariantArrayElement(variantArray, index, ((Byte) object).byteValue());
                } else if (object instanceof Character) {
                    setVariantArrayElement(variantArray, index, ((Character) object).charValue());
                } else if (object instanceof Short) {
                    setVariantArrayElement(variantArray, index, ((Short) object).shortValue());
                } else if (object instanceof Integer) {
                    setVariantArrayElement(variantArray, index, ((Integer) object).intValue());
                } else if (object instanceof Long) {
                    setVariantArrayElement(variantArray, index, ((Long) object).longValue());
                } else if (object instanceof Float) {
                    setVariantArrayElement(variantArray, index, ((Float) object).floatValue());
                } else if (object instanceof Double) {
                    setVariantArrayElement(variantArray, index, ((Double) object).doubleValue());
                } else if (object instanceof String) {
                    setVariantArrayElement(variantArray, index, (String) object);
                } else if (object instanceof BrowserSideObject) {
                    setVariantArrayElement(variantArray, index, (BrowserSideObject) object);
                } else if (object instanceof RemoteJavaObject) {
                    setVariantArrayElement(variantArray, index, (RemoteJavaObject) object);
                } else if (object == Void.TYPE) {
                    setVariantArrayElementToVoid(variantArray, index);
                } else {
                    throw new JSException("Inconvertible argument type to LiveConnect: " + object.getClass().getName());
                }
            } else {
                // Set this element to null instead of void
                setVariantArrayElementToScriptingObject(variantArray, index, 0);
            }
        }
    }
    
    /** Allocates a new BrowserSideObject wrapping a native scripting
        object from the browser, registering it with the
        LiveConnectSupport so its reference count is properly
        maintained. */
    protected BrowserSideObject newBrowserSideObject(long scriptingObjectPointer) {
        return newBrowserSideObject(scriptingObjectPointer, true);
    }

    /** Allocates a new BrowserSideObject wrapping a native scripting
        object from the browser, optionally registering it with the
        LiveConnectSupport. If it is registered, then its reference
        count will be incremented via javaScriptRetainObject. */
    protected BrowserSideObject newBrowserSideObject(long scriptingObjectPointer,
                                                     boolean registerWithLiveConnectSupport) {
        if (registerWithLiveConnectSupport && appletID == null)
            return null;

        BrowserSideObject obj = new BrowserSideObject(scriptingObjectPointer);
        // When "registerWithLiveConnectSupport" is false, the underlying reference count
        // of the scripting object won't be bumped by one.
        if (registerWithLiveConnectSupport) {
            LiveConnectSupport.registerObject(appletID.getID(), obj);
        }
        return obj;
    }

    /** Returns a browser-side scripting object corresponding to the
        applet. On the Internet Explorer browser the return value
        corresponds to an IDispatch* (actually, an IDispatchEx* in our
        implementation). On the Mozilla browsers the return value
        corresponds to an NPObject*. Other browsers will likely use
        different return types. The exceptionInfo argument is a native
        browser-side object which on some browsers may help provide
        more detailed information in case of an error. On IE it is an
        EXCEPINFO*; on Mozilla there is no analogue and 0 is passed. */
    protected long getScriptingObjectForApplet(long exceptionInfo) {
        long res = 0;
        String cause = null;
        if (appletID == null) {
            fillInExceptionInfo(exceptionInfo, "Applet is not running");
            if (DEBUG) {
                cause = " because applet is not running";
            }
        } else {
            // Send out the message
            try {
                ResultID id = LiveConnectSupport.sendGetApplet(this, appletID);
                // FIXME: should have more failsafes in this loop in case
                // there's a bug on the other side where it doesn't send a
                // reply message back
                if (DEBUG) {
                    System.out.println("AbstractPlugin.getScriptingObjectForApplet starting to wait for result ID " + id.getID());
                }

                // This is the platform-dependent part of this code
                resultHandler.waitForResult(id, appletID);

                if (DEBUG) {
                    System.out.println("AbstractPlugin.getScriptingObjectForApplet ending wait for result ID " + id.getID());
                }
                // We just consumed a signaling of the event; signal it
                // again to give any frames higher up the stack a chance
                // to wake up
                notifyMainThread();
                if (LiveConnectSupport.resultAvailable(id)) {
                    try {
                        // Wrap in a browser-side scripting object
                        res = lookupScriptingObject((RemoteJavaObject) LiveConnectSupport.getResult(id), true);
                    } catch (RuntimeException e) {
                        fillInExceptionInfo(exceptionInfo, e);
                    }
                } else {
                    fillInExceptionInfo(exceptionInfo, "Target applet or JVM process exited abruptly");
                }
            } catch (IOException e) {
                fillInExceptionInfo(exceptionInfo, "Target JVM seems to have already exited");
            }
        }
        if (DEBUG) {
            System.out.println("AbstractPlugin.getScriptingObjectForApplet(" +
                               appletID +
                               ") returning 0x" +
                               Long.toHexString(res) +
                               ((cause != null) ? cause : ""));
        }

        return res;
    }

    /** Returns a reference to a JavaNameSpace object which we can use
        to call static Java methods and allocate Java objects from
        JavaScript. Due to the lazy nature of the scripting objects we
        construct to represent these namespace references, this method
        returns a Java object rather than a browser-side scripting
        object (which is assumed to already exist by the time this
        method is called). */
    protected Object getJavaNameSpace(String nameSpace) {
        if (appletID == null) {
            // Applet not yet started
            return null;
        }

        // Send out the message
        try {
            ResultID id = LiveConnectSupport.sendGetNameSpace(this, appletID, nameSpace);
            // FIXME: should have more failsafes in this loop in case
            // there's a bug on the other side where it doesn't send a
            // reply message back
            if (DEBUG) {
                System.out.println("AbstractPlugin.getJavaNameSpace starting to wait for result ID " + id.getID());
            }

            // This is the platform-dependent part of this code
            resultHandler.waitForResult(id, appletID);

            if (DEBUG) {
                System.out.println("AbstractPlugin.getJavaNameSpace ending wait for result ID " + id.getID());
            }
            // We just consumed a signaling of the event; signal it
            // again to give any frames higher up the stack a chance
            // to wake up
            notifyMainThread();
            if (LiveConnectSupport.resultAvailable(id)) {
                try {
                    return LiveConnectSupport.getResult(id);
                } catch (RuntimeException e) {
                    if (DEBUG) {
                        System.out.println("AbstractPlugin.getJavaNameSpace: exception occurred during fetch of namespace \"" +
                                           nameSpace + "\"");
                        e.printStackTrace();
                    }
                }
            } else {
                if (DEBUG) {
                    System.out.println("AbstractPlugin.getJavaNameSpace: target applet or JVM process exited abruptly");
                }
            }
        } catch (IOException e) {
            if (DEBUG) {
                System.out.println("AbstractPlugin.getJavaNameSpace: target JVM process seems to have already exited");
            }
        }
        return null;
    }

    /** Invokes the specified method against the given
        RemoteJavaObject. The "objectIsApplet" argument is a
        concession to Internet Explorer and may be ignored on other
        browsers. The methodIdentifier is browser-specific; see {@link
        #identifierToString identifierToString}. The exceptionInfo
        argument is a browser-specific object which is used in
        conjunction with fillInExceptionInfo to provide more detailed
        information to the user in the case of an error. On IE it is
        an EXCEPINFO*; on Mozilla it is the NPObject* associated with
        this invocation. */
    protected boolean javaObjectInvoke(RemoteJavaObject object,
                                       boolean objectIsApplet,
                                       long methodIdentifier,
                                       long variantArgs,
                                       int  argCount,
                                       long variantResult,
                                       long exceptionInfo) {
        return doJavaObjectOp(object, objectIsApplet, JavaObjectOpMessage.CALL_METHOD,
                              identifierToString(methodIdentifier), methodIdentifier, variantArgs, argCount, variantResult, exceptionInfo);
    }

    /** Calls a constructor of the given Java class, which is
        represented as a RemoteJavaObject pointing to a JavaNameSpace
        on the client side. See {@link #javaObjectInvoke
        javaObjectInvoke} for a description of the objectIsApplet and
        exceptionInfo arguments. */
    protected boolean javaObjectInvokeConstructor(RemoteJavaObject object,
                                                  boolean objectIsApplet,
                                                  long variantArgs,
                                                  int  argCount,
                                                  long variantResult,
                                                  long exceptionInfo) {
        return doJavaObjectOp(object, objectIsApplet, JavaObjectOpMessage.CALL_METHOD,
                              "<init>", -1, variantArgs, argCount, variantResult, exceptionInfo);
    }

    /** Gets the given field of the given Java object. This works for
        named Java fields (both static and non-static), array elements
        (by passing the Strings "0", "1", etc. as the field
        identifier), and descending into the Java package and class
        namespace via the JavaNameSpace concept on the client side.
        See {@link #javaObjectInvoke javaObjectInvoke} for a
        description of the objectIsApplet and exceptionInfo
        arguments. */
    protected boolean javaObjectGetField(RemoteJavaObject object,
                                         boolean objectIsApplet,
                                         long fieldIdentifier,
                                         long variantResult,
                                         long exceptionInfo) {
        return doJavaObjectOp(object, objectIsApplet, JavaObjectOpMessage.GET_FIELD,
                              identifierToString(fieldIdentifier), fieldIdentifier, 0, 0, variantResult, exceptionInfo);
    }

    /** Sets the given field of the given Java object. This works both
        for named Java fields (both static and non-static) and array
        elements (by passing the Strings "0", "1", etc. as the field
        identifier). See {@link #javaObjectInvoke javaObjectInvoke}
        for a description of the objectIsApplet and exceptionInfo
        arguments. */
    protected boolean javaObjectSetField(RemoteJavaObject object,
                                         boolean objectIsApplet,
                                         long fieldIdentifier,
                                         long variantValue,
                                         long exceptionInfo) {
        return doJavaObjectOp(object, objectIsApplet, JavaObjectOpMessage.SET_FIELD,
                              identifierToString(fieldIdentifier), fieldIdentifier, variantValue, 1, 0, exceptionInfo);
    }

    /** Indicates whether the given Java object has the given
        field. This is a concession to the Mozilla JavaScript engine;
        see {@link sun.plugin2.liveconnect.JavaClass#hasField
        JavaClass.hasField}. */
    protected boolean javaObjectHasField(RemoteJavaObject object,
                                         long fieldIdentifier) {
        return doJavaObjectOp(object, false, JavaObjectOpMessage.HAS_FIELD,
                              identifierToString(fieldIdentifier), fieldIdentifier, 0, 0, 0, 0);
    }

    /** Indicates whether the given Java object has the given
        method. This is a concession to the Mozilla JavaScript engine;
        see {@link sun.plugin2.liveconnect.JavaClass#hasMethod
        JavaClass.hasMethod}. */
    protected boolean javaObjectHasMethod(RemoteJavaObject object,
                                          long fieldIdentifier) {
        return doJavaObjectOp(object, false, JavaObjectOpMessage.HAS_METHOD,
                              identifierToString(fieldIdentifier), fieldIdentifier, 0, 0, 0, 0);
    }

    /** Indicates whether the given Java object has the given
	field or method. Some javascript engine uses this form of 
	queries instead of separate field and method;
	This also works better to fit Internet Explorer's GetIDsOfNames
	to provide more precise answers;
	see {@link sun.plugin2.liveconnect.JavaClass#hasFieldOrMethod
	JavaClass.hasFieldOrMethod}. */
    protected boolean javaObjectHasFieldOrMethod(RemoteJavaObject object,
						 long fieldIdentifier) {
	return doJavaObjectOp(object, false, JavaObjectOpMessage.HAS_FIELD_OR_METHOD,
			      identifierToString(fieldIdentifier), fieldIdentifier, 0, 0, 0, 0);
    }
    
    /** The internal method which does all of the work for operating
        on remote Java objects. Made protected (and therefore
        overridable) as a concession to Internet Explorer. */
    protected boolean doJavaObjectOp(RemoteJavaObject object,
                                     boolean objectIsApplet,
                                     int operationKind,
                                     String name,
                                     long nameIdentifier,
                                     long variantArgs,
                                     int  argCount,
                                     long variantResult,
                                     long exceptionInfo) {
        if (name == null) {
            String errorMessage = "Invalid (null) name -- bad field or method identifier " + nameIdentifier;
            if (DEBUG) {
                System.out.println("AbstractPlugin.doJavaObjectOp: " + errorMessage);
            }
            fillInExceptionInfo(exceptionInfo, errorMessage);
            return false;
        }

        if (object == null) {
            String errorMessage = "Invalid (null) object";
            if (DEBUG) {
                System.out.println("AbstractPlugin.doJavaObjectOp: " + errorMessage);
            }
            fillInExceptionInfo(exceptionInfo, errorMessage);
            return false;
        }

        Object[] args = null;
        // Convert args if necessary
        if (variantArgs != 0 && argCount > 0) {
            args = new Object[argCount];
            if (scriptingObjectArgumentListsAreReversed()) {
                // Arguments are passed in the COM DISPPARAMS structure in reverse order (not well documented)
                for (int i = 0; i < argCount; i++) {
                    args[i] = variantArrayElementToObject(variantArgs, argCount - i - 1);
                }
            } else {
                for (int i = 0; i < argCount; i++) {
                    args[i] = variantArrayElementToObject(variantArgs, i);
                }
            }
        }
        Object res = null;
        // Send out the message
        try {
            ResultID id = LiveConnectSupport.sendRemoteJavaObjectOp(this,
                                                                    object,
                                                                    name,
                                                                    operationKind,
                                                                    args);
            if (DEBUG) {
                System.out.println("AbstractPlugin.doJavaObjectOp starting to wait for result ID " + id.getID());
            }

            // This is the platform-dependent part of this code
            resultHandler.waitForResult(id, object.getJVMID(), new AppletID(object.getAppletID()));

            if (DEBUG) {
                System.out.println("AbstractPlugin.doJavaObjectOp ending wait for result ID " + id.getID());
            }

            // We just consumed a signaling of the event; signal it
            // again to give any frames higher up the stack a chance
            // to wake up
            notifyMainThread();

            if (LiveConnectSupport.resultAvailable(id)) {
                try {
                    Object result = LiveConnectSupport.getResult(id);
                    // If this is the HAS_FIELD or HAS_METHOD query, handle these specially
                    if (operationKind == JavaObjectOpMessage.HAS_FIELD ||
                        operationKind == JavaObjectOpMessage.HAS_METHOD ||
			operationKind == JavaObjectOpMessage.HAS_FIELD_OR_METHOD) {
                        if (result != null) {
                            return ((Boolean) result).booleanValue();
                        }
                        return false;
                    }

                    // Otherwise, convert to variant
                    objectToVariantArrayElement(result, variantResult, 0);
                    return true;
                } catch (RuntimeException e) {
                    if (DEBUG) {
                        e.printStackTrace();
                    }
                    fillInExceptionInfo(exceptionInfo, e.getMessage());
                }
            } else {
                fillInExceptionInfo(exceptionInfo, "Target applet or JVM process exited abruptly");
            }
        } catch (IOException e) {
            if (DEBUG) {
                e.printStackTrace();
            }
            fillInExceptionInfo(exceptionInfo, "Target JVM seems to have already exited");
        } catch (JSException e) {
            if (DEBUG) {
                e.printStackTrace();
            }
            fillInExceptionInfo(exceptionInfo, e.toString());
        }
        return false;
    }

    // This method only exists to keep more localization up in Java
    private void javaObjectRemoveField(RemoteJavaObject object,
                                       long fieldIdentifier,
                                       long exceptionInfo) {
        fillInExceptionInfo(exceptionInfo, "Removal of Java fields (\"" + identifierToString(fieldIdentifier) + "\") not supported");
    }

    /** This method sends a message to the JVM instance hosting the
        given object to release it, allowing it to be reclaimed by the
        garbage collector. It can be overridden by subclasses to
        perform other necessary cleanup. It should be called when the
        reference count of a scripting object corresponding to a
        RemoteJavaObject drops to zero. */
    protected void releaseRemoteJavaObject(RemoteJavaObject object) {
        JVMManager.getManager().releaseRemoteJavaObject(object);
    }

    //
    // These methods are called from the native code and help pass
    // arguments and return values from JavaScript to Java
    //
    private Boolean   newBoolean  (boolean value) { return (value ? Boolean.TRUE : Boolean.FALSE); }
    private Byte      newByte     (byte value)    { return new Byte(value);      }
    private Character newCharacter(char value)    { return new Character(value); }
    private Short     newShort    (short value)   { return new Short(value);     }
    private Integer   newInteger  (int value)     { return new Integer(value);   }
    private Long      newLong     (long value)    { return new Long(value);      }
    private Float     newFloat    (float value)   { return new Float(value);     }
    private Double    newDouble   (double value)  { return new Double(value);    }
}
