/*
 * @(#)InstrumentationImpl.java	1.6 04/06/08
 *
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms. 
 */

package sun.instrument;

import  java.lang.reflect.Method;

import java.lang.instrument.ClassFileTransformer;
import java.lang.instrument.ClassDefinition;
import java.lang.instrument.Instrumentation;
import java.security.ProtectionDomain;

/*
 * Copyright 2003 Wily Technology, Inc.
 */

/**
 * The Java side of the JPLIS implementation. Works in concert with a native JVMTI agent
 * to implement the JPLIS API set. Provides both the Java API implementation of
 * the Instrumentation interface and utility Java routines to support the native code.
 * Keeps a pointer to the native data structure in a scalar field to allow native
 * processing behind native methods.
 */
public class InstrumentationImpl implements Instrumentation {
    private final   TransformerManager      mTransformerManager;
    private final   long                    mNativeAgent;       // needs to store a native pointer, so use 64 bits
    private final   boolean                 mEnvironmentSupportsRedefineClasses;

    private
    InstrumentationImpl(long    nativeAgent,
                        boolean environmentSupportsRedefineClasses) {
        mTransformerManager                 = new TransformerManager();
        mNativeAgent                        = nativeAgent;
        mEnvironmentSupportsRedefineClasses = environmentSupportsRedefineClasses;
    }

    public void
    addTransformer(ClassFileTransformer transformer) {
        if (transformer == null) {
            throw new NullPointerException("null passed as 'transformer' in addTransformer");
        }
        mTransformerManager.addTransformer(transformer);
    }

    public boolean
    removeTransformer(ClassFileTransformer transformer) {
        if (transformer == null) {
            throw new NullPointerException("null passed as 'transformer' in removeTransformer");
        }
        return mTransformerManager.removeTransformer(transformer);
    }

    public boolean
    isRedefineClassesSupported() {
        return mEnvironmentSupportsRedefineClasses;
    }
        
    public void
    redefineClasses(ClassDefinition[]   definitions)
            throws  ClassNotFoundException {
        if (!isRedefineClassesSupported()) {
            throw new UnsupportedOperationException("redefineClasses is not supported in this environment");
        }
        if (definitions == null) {
            throw new NullPointerException("null passed as 'definitions' in redefineClasses");
        }
        for (int i = 0; i < definitions.length; ++i) {
          if (definitions[i] == null) {
            throw new NullPointerException("element of 'definitions' is null in redefineClasses");
          }
        }
        if (definitions.length == 0) {
            return; // short-circuit if there are no changes requested
        }
            
        redefineClasses0(definitions);
    }
    
    public Class[]
    getAllLoadedClasses() {
        return getAllLoadedClasses0();
    }

    public Class[]
    getInitiatedClasses(ClassLoader loader) {
        return getInitiatedClasses0(loader);
    }
    
    public long
    getObjectSize(Object objectToSize) {
        if (objectToSize == null) {
            throw new NullPointerException("null passed as 'objectToSize' in getObjectSize");
        }
        return getObjectSize0(objectToSize);
    }

    /*
     *  Natives
     */ 
    private native void
    redefineClasses0(ClassDefinition[]  definitions)
        throws  ClassNotFoundException;

    public native Class[]
    getAllLoadedClasses0();

    public native Class[]
    getInitiatedClasses0(ClassLoader loader);

    public native long
    getObjectSize0(Object objectToSize);

    /*
     *  Internals
     */
     
    // WARNING: the native code knows the name & signature of this method
    private long
    getNativeAgent() {
        return mNativeAgent;
    }

    // WARNING: the native code knows the name & signature of this method
    private void
    loadClassAndCallPremain(    String  classname,
                                String  optionsString)
            throws Throwable {

        ClassLoader mainAppLoader   = ClassLoader.getSystemClassLoader();
        Class       javaAgentClass  = mainAppLoader.loadClass(classname);
        
        Method  premainMethod       = javaAgentClass.getMethod( 
                                            "premain",
                                            new Class[] {
                                                String.class,
                                                java.lang.instrument.Instrumentation.class
                                                });
        premainMethod.invoke(   null, /* static */
                                new Object[] {
                                    optionsString,
                                    this
                                    });
    }
    
    // WARNING: the native code knows the name & signature of this method
    private byte[]
    transform(  ClassLoader         loader,
                String              classname,
                Class               classBeingRedefined,
                ProtectionDomain    protectionDomain,
                byte[]              classfileBuffer) {
        return mTransformerManager.transform(   loader,
                                                classname,
                                                classBeingRedefined,
                                                protectionDomain,
                                                classfileBuffer);
    }


}
