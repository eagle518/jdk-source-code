/*
 * @(#)TransformerManager.java	1.3 03/09/10
 *
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms. 
 */

package sun.instrument;


import java.lang.instrument.Instrumentation;
import java.lang.instrument.ClassFileTransformer;
import java.security.ProtectionDomain;

/*
 * Copyright 2003 Wily Technology, Inc.
 */

/**
 * Support class for the InstrumentationImpl. Manages the list of registered transformers.
 * Keeps everything in the right order, deals with sync of the list,
 * and actually does the calling of the transformers.
 */
public class TransformerManager
{
    /**
     * a given instance of this list is treated as immutable to simplify sync;
     * we pay copying overhead whenever the list is changed rather than every time 
     * the list is referenced.
     * The array is kept in the order the transformers are added via addTransformer
     * (first added is 0, last added is length-1)
     * Use an array, not a List or other Collection. This keeps the set of classes
     * used by this code to a minimum. We want as few dependencies as possible in this
     * code, since it is used inside the class definition system. Any class referenced here
     * cannot be transformed by Java code.
     */
    private ClassFileTransformer[]  mTransformerList;
    
    public
    TransformerManager() {
        mTransformerList    = new ClassFileTransformer[0];
    }
    
    public synchronized void
    addTransformer( ClassFileTransformer    transformer) {
        ClassFileTransformer[] oldList = mTransformerList;
        ClassFileTransformer[] newList = new ClassFileTransformer[oldList.length + 1];
        System.arraycopy(   oldList,
                            0,
                            newList,
                            0,
                            oldList.length);    
        newList[oldList.length] = transformer;
        mTransformerList = newList;
    }

    public synchronized boolean
    removeTransformer(ClassFileTransformer  transformer) {
        boolean                 found           = false;
        ClassFileTransformer[]  oldList         = mTransformerList;
        int                     oldLength       = oldList.length;
        int                     newLength       = oldLength - 1;
            
        // look for it in the list, starting at the last added, and remember where it was if we found it
        int matchingIndex   = 0;
        for ( int x = oldLength - 1; x >= 0; x-- ) {
            if ( oldList[x] == transformer ) {
                found           = true;
                matchingIndex   = x;
                break;
            }
        }
                
        // make a copy of the array without the matching element    
        if ( found ) {
            ClassFileTransformer[]  newList = new ClassFileTransformer[newLength];
                
            // copy up to but not including the match
            if ( matchingIndex > 0 ) {
                System.arraycopy(   oldList,
                                    0,
                                    newList,
                                    0,
                                    matchingIndex);
            }
            
            // if there is anything after the match, copy it as well
            if ( matchingIndex < (newLength) ) {
                System.arraycopy(   oldList,
                                    matchingIndex + 1,
                                    newList,
                                    matchingIndex,
                                    (newLength) - matchingIndex);
            }
            mTransformerList = newList;
        }
        return found;
    }

    private ClassFileTransformer[]
    getSnapshotTransformerList() {
        return mTransformerList;
    }
        
    public byte[]
    transform(  ClassLoader         loader,
                String              classname,
                Class               classBeingRedefined,
                ProtectionDomain    protectionDomain,
                byte[]              classfileBuffer) {
        boolean someoneTouchedTheBytecode = false;

        ClassFileTransformer[]  transformerList = getSnapshotTransformerList();
        
        byte[]  bufferToUse = classfileBuffer;
        
        // order matters, gotta run 'em in the order they were added
        for ( int x = 0; x < transformerList.length; x++ ) {
            ClassFileTransformer    transformer = transformerList[x];
            byte[]                  transformedBytes = null;
            
            try {
                transformedBytes = transformer.transform(   loader,
                                                            classname,
                                                            classBeingRedefined,
                                                            protectionDomain,
                                                            bufferToUse);
            }
            catch (Throwable t) {
                // don't let any one transformer mess it up for the others.
                // This is where we need to put some logging. What should go here? FIXME
            }
            
            if ( transformedBytes != null ) {
                someoneTouchedTheBytecode = true;
                bufferToUse = transformedBytes;
            }
        }
        
        // if someone modified it, return the modified buffer. 
        // otherwise return null to mean "no transforms occurred"
        byte [] result;
        if ( someoneTouchedTheBytecode ) {
            result = bufferToUse;
        }
        else {
            result = null;
        }

        return result;
    }
    
}
