/*
 * Copyright (c) 2002, 2003, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

package sun.jvm.hotspot.jdi;

import sun.jvm.hotspot.oops.Oop;
import sun.jvm.hotspot.oops.Instance;
import sun.jvm.hotspot.oops.Klass;
import sun.jvm.hotspot.memory.SystemDictionary;
import sun.jvm.hotspot.memory.Universe;
import sun.jvm.hotspot.runtime.VM;

import com.sun.jdi.*;
import java.util.*;

public class ClassLoaderReferenceImpl
    extends ObjectReferenceImpl
    implements ClassLoaderReference
{
     // because we work on process snapshot or core we can
     // cache visibleClasses & definedClasses always (i.e., no suspension)
     private List visibleClassesCache;
     private List definedClassesCache;

     ClassLoaderReferenceImpl(VirtualMachine aVm, Instance oRef) {
         super(aVm, oRef);
     }

     protected String description() {
         return "ClassLoaderReference " + uniqueID();
     }

     public List definedClasses() {
         if (definedClassesCache == null) {
             definedClassesCache = new ArrayList();
             Iterator iter = vm.allClasses().iterator();
             while (iter.hasNext()) {
                 ReferenceType type = (ReferenceType)iter.next();
                 if (equals(type.classLoader())) {  /* thanks OTI */
                     definedClassesCache.add(type);
                 }
             }
         }
         return definedClassesCache;
     }

     private SystemDictionary getSystemDictionary() {
         return vm.saSystemDictionary();
     }

     private Universe getUniverse() {
         return vm.saUniverse();
     }

     public List visibleClasses() {
         if (visibleClassesCache != null)
            return visibleClassesCache;

         visibleClassesCache = new ArrayList();

         // refer to getClassLoaderClasses in jvmtiGetLoadedClasses.cpp
         //  a. SystemDictionary::classes_do doesn't include arrays of primitive types (any dimensions)
         SystemDictionary sysDict = getSystemDictionary();
         sysDict.classesDo(
                           new SystemDictionary.ClassAndLoaderVisitor() {
                                public void visit(Klass k, Oop loader) {
                                    if (ref().equals(loader)) {
                                        for (Klass l = k; l != null; l = l.arrayKlassOrNull()) {
                                            visibleClassesCache.add(vm.referenceType(l));
                                        }
                                    }
                                }
                           }
                           );

         // b. multi dimensional arrays of primitive types
         sysDict.primArrayClassesDo(
                                    new SystemDictionary.ClassAndLoaderVisitor() {
                                         public void visit(Klass k, Oop loader) {
                                             if (ref().equals(loader)) {
                                                 visibleClassesCache.add(vm.referenceType(k));
                                             }
                                         }
                                     }
                                     );

         // c. single dimensional primitive array klasses from Universe
         // these are not added to SystemDictionary
         getUniverse().basicTypeClassesDo(
                            new SystemDictionary.ClassVisitor() {
                                public void visit(Klass k) {
                                    visibleClassesCache.add(vm.referenceType(k));
                                }
                            }
                            );

         return visibleClassesCache;
     }

     Type findType(String signature) throws ClassNotLoadedException {
         List types = visibleClasses();
         Iterator iter = types.iterator();
         while (iter.hasNext()) {
             ReferenceType type = (ReferenceType)iter.next();
             if (type.signature().equals(signature)) {
                 return type;
             }
         }
         JNITypeParser parser = new JNITypeParser(signature);
         throw new ClassNotLoadedException(parser.typeName(),
                                          "Class " + parser.typeName() + " not loaded");
     }
}
