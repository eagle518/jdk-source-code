/**
 * @(#)JavadocClassReader.java	1.10 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.javadoc;

import com.sun.tools.javac.code.Symbol.PackageSymbol;
import com.sun.tools.javac.jvm.ClassReader;
import com.sun.tools.javac.util.Context;

import java.io.File;


/** Javadoc uses an extended class reader that records package.html entries
 *  @author Neal Gafter
 */
class JavadocClassReader extends ClassReader {

    public static JavadocClassReader instance0(Context context) {
	ClassReader instance = context.get(classReaderKey);
	if (instance == null) 
	    instance = new JavadocClassReader(context);
	return (JavadocClassReader)instance;
    }

    public static void preRegister(final Context context) {
	context.put(classReaderKey, new Context.Factory<ClassReader>() {
	    public ClassReader make() {
		return new JavadocClassReader(context);
	    }
	});
    }

    private DocEnv docenv;

    private JavadocClassReader(Context context) {
	super(context, true);
	docenv = DocEnv.instance(context);
    }

    /** Override extraZipFileActions to check for package documentation
     */
    protected void extraZipFileActions(PackageSymbol pack,
				       String zipEntryName,
				       String classPathName,
				       String zipName) {
	if (docenv != null && zipEntryName.endsWith("package.html"))
	    docenv.getPackageDoc(pack).setDocPath(zipName, classPathName);
    }

    /** Override extraFileActions to check for package documentation
     */
    protected void extraFileActions(PackageSymbol pack, String fileName, File fileDir) {
	if (docenv != null && fileName.equals("package.html"))
	    docenv.getPackageDoc(pack).setDocPath(fileDir.getAbsolutePath());
    }
}
