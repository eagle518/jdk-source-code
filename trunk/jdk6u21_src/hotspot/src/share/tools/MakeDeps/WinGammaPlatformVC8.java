/*
 * Copyright (c) 2005, 2009, Oracle and/or its affiliates. All rights reserved.
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

import java.io.*;
import java.util.*;

public class WinGammaPlatformVC8 extends WinGammaPlatformVC7 {

    String projectVersion() {return "8.00";};

}

class CompilerInterfaceVC8 extends CompilerInterfaceVC7 {

    Vector getBaseCompilerFlags(Vector defines, Vector includes, String outDir) {
        Vector rv = new Vector();

        getBaseCompilerFlags_common(defines,includes, outDir, rv);
        // Set /Yu option. 2 is pchUseUsingSpecific
        addAttr(rv, "UsePrecompiledHeader", "2");
        // Set /EHsc- option. 0 is cppExceptionHandlingNo
        addAttr(rv, "ExceptionHandling", "0");

        return rv;
    }


    Vector getDebugCompilerFlags(String opt) {
        Vector rv = new Vector();

        getDebugCompilerFlags_common(opt,rv);

        return rv;
    }

    Vector getProductCompilerFlags() {
        Vector rv = new Vector();

        getProductCompilerFlags_common(rv);

        return rv;
    }


}
