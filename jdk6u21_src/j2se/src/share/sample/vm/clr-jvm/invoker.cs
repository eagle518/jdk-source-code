/*
* @(#)invoker.cs	1.2 10/03/23
*
* Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* -Redistribution of source code must retain the above copyright notice, this
*  list of conditions and the following disclaimer.
*
* -Redistribution in binary form must reproduce the above copyright notice,
*  this list of conditions and the following disclaimer in the documentation
*  and/or other materials provided with the distribution.
*
* Neither the name of Oracle or the names of contributors may
* be used to endorse or promote products derived from this software without
* specific prior written permission.
*
* This software is provided "AS IS," without a warranty of any kind. ALL
* EXPRESS OR IMPLIED CONDITIONS, REPRESENTATIONS AND WARRANTIES, INCLUDING
* ANY IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
* OR NON-INFRINGEMENT, ARE HEREBY EXCLUDED. SUN MICROSYSTEMS, INC. ("SUN")
* AND ITS LICENSORS SHALL NOT BE LIABLE FOR ANY DAMAGES SUFFERED BY LICENSEE
* AS A RESULT OF USING, MODIFYING OR DISTRIBUTING THIS SOFTWARE OR ITS
* DERIVATIVES. IN NO EVENT WILL SUN OR ITS LICENSORS BE LIABLE FOR ANY LOST
* REVENUE, PROFIT OR DATA, OR FOR DIRECT, INDIRECT, SPECIAL, CONSEQUENTIAL,
* INCIDENTAL OR PUNITIVE DAMAGES, HOWEVER CAUSED AND REGARDLESS OF THE THEORY
* OF LIABILITY, ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE,
* EVEN IF SUN HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
*
* You acknowledge that this software is not designed, licensed or intended
* for use in the design, construction, operation or maintenance of any
* nuclear facility.
*/

using System;
using System.Runtime.InteropServices;

class jinvoker{

    public static int Main(string[] aArgs){
        
        // Print Hello to show we are in CLR
        Console.WriteLine("Hello from C#");
        if(aArgs.Length > 0)
            // invoke JVM
            return InvokeMain(aArgs[0]);
        else
            return -1;
    }

    // Link the JVM API functions and the wrappers

    [DllImport("jvm.dll")]      public unsafe static extern int  JNI_CreateJavaVM(void** ppVm, void** ppEnv, void* pArgs);
    [DllImport("jinvoker.dll")] public unsafe static extern int  MakeJavaVMInitArgs( void** ppArgs );
    [DllImport("jinvoker.dll")] public unsafe static extern void FreeJavaVMInitArgs( void* pArgs );
    [DllImport("jinvoker.dll")] public unsafe static extern int  FindClass( void* pEnv, String sClass, void** ppClass );
    [DllImport("jinvoker.dll")] public unsafe static extern int  GetStaticMethodID( void*  pEnv,
                                                                                    void*  pClass, 
                                                                                    String szName, 
                                                                                    String szArgs, 
                                                                                    void** ppMid);
	
    [DllImport("jinvoker.dll")] public unsafe static extern int NewObjectArray( void*  pEnv,
                                                                                int    nDimension,
                                                                                String sType,
                                                                                void** ppArray );

    [DllImport("jinvoker.dll")] public unsafe static extern int CallStaticVoidMethod( void* pEnv,
                                                                                      void* pClass,
                                                                                      void* pMid,
                                                                                      void* pArgs);

    [DllImport("jinvoker.dll")] public unsafe static extern int DestroyJavaVM( void* pJVM );

	public unsafe static int InvokeMain( String sClass ){
	    
        void*  pJVM;    // JVM struct
        void*  pEnv;    // JVM environment
        void*  pVMArgs; // VM args
        void*  pClass;  // Class struct of the executed method
        void*  pMethod; // The executed method struct
        void*  pArgs;   // The executed method arguments struct

        // Fill the pVMArgs structs
        MakeJavaVMInitArgs( &pVMArgs );

        // Create JVM
        int nRes = JNI_CreateJavaVM( &pJVM, &pEnv, pVMArgs );
        if( nRes == 0 ){
			
            // Find the executed method class 
            if(FindClass( pEnv, sClass, &pClass) == 0 )
				
                // Find the executed method
                if( GetStaticMethodID( pEnv, pClass, "main", "([Ljava/lang/String;)V", &pMethod ) == 0 )
					
                    // Create empty String[] array to pass to the main()
                    if( NewObjectArray( pEnv, 0, "java/lang/String", &pArgs ) == 0 ){

                        // Call main()
                        nRes = CallStaticVoidMethod( pEnv, pClass, pMethod, pArgs );
                        if( nRes != -1 )
                            Console.WriteLine("Result:"+nRes);
                        else
                            Console.WriteLine("Exception");
                        
                    }else{
                        Console.WriteLine("Error while making args array");
                        nRes = -100;
                    }
                else{
                    Console.WriteLine("can not find method main(String[])");
                    nRes = -101;
                }
            else{
                Console.WriteLine("can not find class:"+sClass);
                nRes = -102;
            }
			
            // Destroy the JVM
            DestroyJavaVM( pJVM );

        }else
            Console.WriteLine("Can not create Java VM");

        // Free the JVM args structs
        FreeJavaVMInitArgs(pVMArgs);
		
        return nRes;
    }
}
