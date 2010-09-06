/**
 * @(#)Flags.java	1.38 04/06/17
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 * Use and Distribution is subject to the Java Research License available
 * at <http://wwws.sun.com/software/communitysource/jrl.html>.
 */

package com.sun.tools.javac.code;

/** Access flags and other modifiers for Java classes and members.
 *
 *  <p><b>This is NOT part of any API suppored by Sun Microsystems.  If
 *  you write code that depends on this, you do so at your own risk.
 *  This code and its internal interfaces are subject to change or
 *  deletion without notice.</b>
 */
public class Flags {

    private Flags() {} // uninstantiable

    public static String toString(long flags) {
	StringBuffer buf = new StringBuffer();
	if ((flags&PUBLIC) != 0) buf.append("public ");
	if ((flags&PRIVATE) != 0) buf.append("private ");
	if ((flags&PROTECTED) != 0) buf.append("protected ");
	if ((flags&STATIC) != 0) buf.append("static ");
	if ((flags&FINAL) != 0) buf.append("final ");
	if ((flags&SYNCHRONIZED) != 0) buf.append("synchronized ");
	if ((flags&VOLATILE) != 0) buf.append("volatile ");
	if ((flags&TRANSIENT) != 0) buf.append("transient ");
	if ((flags&NATIVE) != 0) buf.append("native ");
	if ((flags&INTERFACE) != 0) buf.append("interface ");
	if ((flags&ABSTRACT) != 0) buf.append("abstract ");
	if ((flags&STRICTFP) != 0) buf.append("strictfp ");
	if ((flags&BRIDGE) != 0) buf.append("bridge ");
	if ((flags&SYNTHETIC) != 0) buf.append("synthetic ");
	if ((flags&DEPRECATED) != 0) buf.append("deprecated ");
	if ((flags&HASINIT) != 0) buf.append("hasinit ");
	if ((flags&ENUM) != 0) buf.append("enum ");
	if ((flags&IPROXY) != 0) buf.append("iproxy ");
	if ((flags&NOOUTERTHIS) != 0) buf.append("noouterthis ");
	if ((flags&EXISTS) != 0) buf.append("exists ");
	if ((flags&COMPOUND) != 0) buf.append("compound ");
	if ((flags&CLASS_SEEN) != 0) buf.append("class_seen ");
	if ((flags&SOURCE_SEEN) != 0) buf.append("source_seen ");
	if ((flags&LOCKED) != 0) buf.append("locked ");
	if ((flags&UNATTRIBUTED) != 0) buf.append("unattributed ");
	if ((flags&ANONCONSTR) != 0) buf.append("anonconstr ");
	if ((flags&ACYCLIC) != 0) buf.append("acyclic ");
	if ((flags&PARAMETER) != 0) buf.append("parameter ");
	if ((flags&VARARGS) != 0) buf.append("varargs ");
	return buf.toString();
    }

    /* Standard Java flags.
     */
    public static final int PUBLIC	 = 1<<0;
    public static final int PRIVATE	 = 1<<1;
    public static final int PROTECTED    = 1<<2;
    public static final int STATIC	 = 1<<3;
    public static final int FINAL	 = 1<<4;
    public static final int SYNCHRONIZED = 1<<5;
    public static final int VOLATILE     = 1<<6;
    public static final int TRANSIENT    = 1<<7;
    public static final int NATIVE	 = 1<<8;
    public static final int INTERFACE    = 1<<9;
    public static final int ABSTRACT     = 1<<10;
    public static final int STRICTFP     = 1<<11;

    /* Flag that marks a symbol synthetic, added in classfile v49.0. */
    public static final int SYNTHETIC    = 1<<12;

    /** Flag that marks attribute interfaces, added in classfile v49.0. */
    public static final int ANNOTATION	 = 1<<13;

    /** An enumeration type or an enumeration constant, added in
     *  classfile v49.0. */
    public static final int ENUM	 = 1<<14;

    public static final int StandardFlags = 0x0fff;

    // Because the following access flags are overloaded with other
    // bit positions, we translate them when reading and writing class
    // files into unique bits positions: ACC_SYNTHETIC <-> SYNTHETIC,
    // for example.
    public static final int ACC_SUPER    = 0x0020;
    public static final int ACC_BRIDGE	 = 0x0040;
    public static final int ACC_VARARGS	 = 0x0080;

    /*****************************************
     * Internal compiler flags (no bits in the lower 16).
     *****************************************/

    /** Flag is set if symbol is deprecated.
     */
    public static final int DEPRECATED   = 1<<17;

    /** Flag is set for a variable symbol if the variable's definition
     *	has an initializer part.
     */
    public static final int HASINIT	     = 1<<18;

    /** Flag is set for compiler-generated anonymous method symbols
     *	that `own' an initializer block.
     */
    public static final int BLOCK	     = 1<<20;

    /** Flag is set for compiler-generated abstract methods that implement
     *	an interface method (Miranda methods).
     */
    public static final int IPROXY	     = 1<<21;

    /** Flag is set for nested classes that do not access instance members
     *	or `this' of an outer class and therefore don't need to be passed
     *	a this$n reference.  This flag is currently set only for anonymous
     *	classes in superclass constructor calls and only for pre 1.4 targets.
     *	todo: use this flag for optimizing away this$n parameters in
     *	other cases.
     */
    public static final int NOOUTERTHIS  = 1<<22;

    /** Flag is set for package symbols if a package has a member or
     *	directory and therefore exists.
     */
    public static final int EXISTS	     = 1<<23;

    /** Flag is set for compiler-generated compound classes
     *	representing multiple variable bounds
     */
    public static final int COMPOUND     = 1<<24;

    /** Flag is set for class symbols if a class file was found for this class.
     */
    public static final int CLASS_SEEN   = 1<<25;

    /** Flag is set for class symbols if a source file was found for this
     *  class.
     */
    public static final int SOURCE_SEEN  = 1<<26;

    /* State flags (are reset during compilation).
     */

    /** Flag for class symbols is set and later re-set as a lock in
     *	Enter to detect cycles in the superclass/superinterface
     *	relations.  Similarly for constructor call cycle detection in
     *	Attr.
     */
    public static final int LOCKED	     = 1<<27;

    /** Flag for class symbols is set and later re-set to indicate that a class
     *	has been entered but has not yet been attributed.
     */
    public static final int UNATTRIBUTED = 1<<28;

    /** Flag for synthesized default constructors of anonymous classes.
     */
    public static final int ANONCONSTR   = 1<<29;

    /** Flag for class symbols to indicate it has been checked and found
     *	acyclic.
     */
    public static final int ACYCLIC	     = 1<<30;

    /** Flag that marks bridge methods.
     */
    public static final long BRIDGE	     = 1L<<31;

    /** Flag that marks formal parameters.
     */
    public static final long PARAMETER   = 1L<<33;

    /** Flag that marks varargs methods.
     */
    public static final long VARARGS   = 1L<<34;

    /** Flag for annotation type symbols to indicate it has been
     *  checked and found acyclic.
     */
    public static final int ACYCLIC_ANN	     = 1<<35;

    /** Flag that marks a generated default constructor.
     */
    public static final long GENERATEDCONSTR   = 1L<<36;

    /** Flag that marks a hypothetical method that need not really be
     *  generated in the binary, but is present in the symbol table to
     *  simplify checking for erasure clashes.
     */
    public static final long HYPOTHETICAL   = 1L<<37;

    /** Modifier masks.
     */
    public static final int
	AccessFlags	      = PUBLIC | PROTECTED | PRIVATE,
	LocalClassFlags	      = FINAL | ABSTRACT | STRICTFP | ENUM,
	MemberClassFlags      = LocalClassFlags | INTERFACE | AccessFlags,
	ClassFlags	      = LocalClassFlags | INTERFACE | PUBLIC | ANNOTATION,
	InterfaceVarFlags     = FINAL | STATIC | PUBLIC,
	VarFlags	      = AccessFlags | FINAL | STATIC |
				VOLATILE | TRANSIENT | ENUM,
	ConstructorFlags      = AccessFlags,
	InterfaceMethodFlags  = ABSTRACT | PUBLIC,
	MethodFlags	      = AccessFlags | ABSTRACT | STATIC | NATIVE |
                                SYNCHRONIZED | FINAL | STRICTFP;
    public static final long
	LocalVarFlags	      = FINAL | PARAMETER;
}
