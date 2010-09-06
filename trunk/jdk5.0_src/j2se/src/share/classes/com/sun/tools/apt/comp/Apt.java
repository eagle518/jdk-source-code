/**
 * @(#)Apt.java	1.15 04/06/25
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.apt.comp;

import com.sun.tools.javac.comp.*;
import com.sun.tools.javac.tree.*;
import com.sun.tools.javac.util.*;
import com.sun.tools.javac.tree.TreeScanner;
import com.sun.tools.javac.util.Context;
import com.sun.tools.apt.util.Bark;
import com.sun.tools.javac.util.Position;

import java.util.*;
import java.util.regex.*;
import java.lang.reflect.*;
import java.lang.reflect.InvocationTargetException;
import java.io.IOException;

import com.sun.tools.apt.*;
import com.sun.tools.apt.comp.*;
import com.sun.tools.javac.code.Symbol.*;

import com.sun.mirror.declaration.TypeDeclaration;
import com.sun.mirror.declaration.AnnotationTypeDeclaration;
import com.sun.mirror.apt.*;
// import com.sun.mirror.apt.AnnotationProcessorFactory;
import com.sun.mirror.apt.AnnotationProcessors;

import com.sun.tools.apt.mirror.AptEnv;
import com.sun.tools.apt.mirror.apt.FilerImpl;
import com.sun.tools.apt.mirror.apt.AnnotationProcessorEnvironmentImpl;


import static com.sun.tools.apt.mirror.declaration.DeclarationMaker.isJavaIdentifier;

/** 
 * Apt compiler phase.
 *
 *  <p><b>This is NOT part of any API suppored by Sun Microsystems.  If
 *  you write code that depends on this, you do so at your own risk.
 *  This code and its internal interfaces are subject to change or
 *  deletion without notice.</b>
 */
public class Apt extends ListBuffer<Env<AttrContext>> {
    java.util.Set<String> genSourceFileNames = new java.util.LinkedHashSet<String>();
    public java.util.Set<String> getSourceFileNames() {
	return genSourceFileNames;
    }

    /** List of names of generated class files.
     */
    java.util.Set<String> genClassFileNames  = new java.util.LinkedHashSet<String>();
    public java.util.Set<String> getClassFileNames() {
	return genClassFileNames;
    }

    /* AptEnvironment */
    AptEnv aptenv;

    private Context context;

    /** The context key for the todo list. */

    protected static final Context.Key<Apt> aptKey =
        new Context.Key<Apt>();

    /** Get the Apt instance for this context. */
    public static Apt instance(Context context) {
        Apt instance = context.get(aptKey);
        if (instance == null) 
            instance = new Apt(context);
        return instance;
    }

    /** Create a new apt list. */
    protected Apt(Context context) {
	this.context = context;

        context.put(aptKey, this);
	aptenv = AptEnv.instance(context);
    }

    /**
     * Used to scan javac trees to build data structures needed for
     * bootstrapping the apt environment.  In particular:
     *
     * <ul>
     *
     * <li> Generate list of canonical names of annotation types that
     * appear in source files given on the command line
     *
     * <li> Collect list of javac symbols representing source files
     * given on the command line
     *
     * </ul>
     */
    static class AptTreeScanner extends TreeScanner {
	
	// Set of fully qualified names of annotation types present in
	// examined source
	private Set<String> annotationSet;

	// Symbols to build bootstrapping declaration list
	private Collection<ClassSymbol> specifiedDeclCollection;
	private Collection<ClassSymbol> declCollection;

	public Set<String> getAnnotationSet() {
	    return annotationSet;
	}

	public AptTreeScanner(){
	    annotationSet = new  LinkedHashSet<String>();
	    specifiedDeclCollection = new LinkedHashSet<ClassSymbol>(); 
	    declCollection = new LinkedHashSet<ClassSymbol>(); 

	}

	public void visitTopLevel(Tree.TopLevel tree) {
	    super.visitTopLevel(tree);
	    // Print out contents -- what are we dealing with?

	    for(Tree d: tree.defs) {
		if (d instanceof Tree.ClassDef)
		    specifiedDeclCollection.add(((Tree.ClassDef) d).sym);
	    }

	}

	public void visitBlock(Tree.Block tree) {
	    ; // Do nothing.
	}


	// should add nested classes to packages, etc.
	public void visitClassDef(Tree.ClassDef tree) {
	    if (tree.sym == null) {
		// could be an anon class w/in an initializer
		return;
	    }

	    super.visitClassDef(tree);

	    declCollection.add(tree.sym);
	}

	public void visitMethodDef(Tree.MethodDef tree) {
	    super.visitMethodDef(tree);
	}

	public void visitVarDef(Tree.VarDef tree) {
	    super.visitVarDef(tree);
	}

	public void visitAnnotation(Tree.Annotation tree) {
	    super.visitAnnotation(tree);
	    annotationSet.add(tree.type.tsym.toString());
	}
    }

    public void main(com.sun.tools.javac.util.List<Tree> treeList,
		     Map<String, String> origOptions,
		     ClassLoader aptCL,
		     java.util.Set<Class<? extends AnnotationProcessorFactory> > productiveFactories) {
	Bark bark = Bark.instance(context);
	java.io.PrintWriter out = bark.warnWriter;

	AptTreeScanner ats = new AptTreeScanner();
	for(Tree t: treeList) {
	    t.accept(ats);
	}
	
	// Turn collection of ClassSymbols into Collection of apt decls

	Collection<TypeDeclaration> spectypedecls = new LinkedHashSet<TypeDeclaration>();

	for (ClassSymbol cs : ats.specifiedDeclCollection) {
	    TypeDeclaration decl = aptenv.declMaker.getTypeDeclaration(cs);
	    spectypedecls.add(decl);
	}

	Set<AnnotationTypeDeclaration> emptyATDS = Collections.emptySet();
	Set<Class<? extends AnnotationProcessorFactory> > currentRoundFactories = 
	    new LinkedHashSet<Class<? extends AnnotationProcessorFactory> >();
	Collection<TypeDeclaration> typedecls = new LinkedHashSet<TypeDeclaration>();
	Set<String> unmatchedAnnotations = (new LinkedHashSet<String>());
	unmatchedAnnotations.addAll(ats.getAnnotationSet());

	for (ClassSymbol cs : ats.declCollection) {
	    TypeDeclaration decl = aptenv.declMaker.getTypeDeclaration(cs);
	    typedecls.add(decl);
	}

	Options options = Options.instance(context);
	
	if (options.get("-XListAnnotationTypes") != null) {
	    out.println("Set of annotations found:" +
			(new TreeSet<String>(ats.getAnnotationSet())).toString());
	}

	AnnotationProcessorEnvironmentImpl trivAPE =
	    new AnnotationProcessorEnvironmentImpl(spectypedecls, typedecls, origOptions, context);

	if (options.get("-XListDeclarations") != null) {
	    out.println("Set of Specified Declarations:" + 
			spectypedecls);

	    out.println("Set of Included Declarations: " +
			   typedecls);
	}

	if (options.get("-print") != null) {
	    if (treeList.size() == 0 )
		throw new UsageMessageNeededException();

	    // Run the printing processor
	    AnnotationProcessor proc = (new BootstrapAPF()).getProcessorFor(new HashSet<AnnotationTypeDeclaration>(),
									    trivAPE);
	    proc.process();
	} else {
	    // Discovery process
	    
	    // List of annotation processory factory instances
	    java.util.Iterator providers = null;
	    {
		/*
		 * If the "-factory" option is used, seach the
		 * appropriate path for the named class.  Otherwise,
		 * use sun.misc.Service to implement the default
		 * discovery policy.
		 */
		String factoryName = options.get("-factory");
		if (factoryName != null) {
		    java.util.List<AnnotationProcessorFactory> list = 
			new LinkedList<AnnotationProcessorFactory>();
		    try {
			AnnotationProcessorFactory factory = 
			    (AnnotationProcessorFactory) (aptCL.loadClass(factoryName).newInstance());
			list.add(factory);
		    } catch (ClassNotFoundException cnfe) {
			bark.warning(Position.NOPOS, "FactoryNotFound", factoryName);
		    } catch (ClassCastException cce) {
			bark.warning(Position.NOPOS, "FactoryWrongType", factoryName);
		    } catch (Exception e ) {
			bark.warning(Position.NOPOS, "FactoryCantInstantiate", factoryName);
		    }
		    providers = list.iterator();
		} else
		    providers = sun.misc.Service.providers(AnnotationProcessorFactory.class, aptCL);
	    }

	    java.util.Map<AnnotationProcessorFactory, Set<AnnotationTypeDeclaration>> factoryToAnnotation = 
		new LinkedHashMap<AnnotationProcessorFactory, Set<AnnotationTypeDeclaration>>();
		
	    if (!providers.hasNext() && productiveFactories.size() == 0) {
		if (unmatchedAnnotations.size() > 0)
		    bark.warning(Position.NOPOS, "NoAnnotationProcessors");
		if (treeList.size() == 0)
		    throw new UsageMessageNeededException();
		return; // no processors; nothing else to do
	    } else {
		// If there are no annotations, still give
		// processors that match everything a chance to
		// run.
		    
		if(unmatchedAnnotations.size() == 0)
		    unmatchedAnnotations.add("");

		Set<String> emptyStringSet = new HashSet<String>();
		emptyStringSet.add("");
		emptyStringSet = Collections.unmodifiableSet(emptyStringSet);

		while (providers.hasNext() ) {
		    Object provider = providers.next();
		    try {
			Set<String> matchedStrings = new HashSet<String>();
			    
			AnnotationProcessorFactory apf = (AnnotationProcessorFactory) provider;
			Collection<String> supportedTypes = apf.supportedAnnotationTypes();
			    
			Collection<Pattern> supportedTypePatterns = new LinkedList<Pattern>();
			for(String s: supportedTypes)
			    supportedTypePatterns.add(importStringToPattern(s));

			for(String s: unmatchedAnnotations) {
			    for(Pattern p: supportedTypePatterns) {
				if (p.matcher(s).matches()) {
				    matchedStrings.add(s);
				    break;
				}
			    }
			}

			unmatchedAnnotations.removeAll(matchedStrings);

			if (options.get("-XPrintFactoryInfo") != null) {
			    out.println("Factory " + apf.getClass().getName() + 
					" matches " + 
					((matchedStrings.size() == 0)?
					 "nothing.": matchedStrings));
			}

			if (matchedStrings.size() > 0) {
			    // convert annotation names to annotation
			    // type decls
			    Set<AnnotationTypeDeclaration> atds = new HashSet<AnnotationTypeDeclaration>();
				
			    // If a "*" processor is called on the
			    // empty string, pass in an empty set of
			    // annotation type declarations.
			    if (!matchedStrings.equals(emptyStringSet)) {
				for(String s: matchedStrings) {
				    TypeDeclaration decl = aptenv.declMaker.getTypeDeclaration(s);
				    AnnotationTypeDeclaration annotdecl;
				    if (decl == null) {
					bark.error(Position.NOPOS, "DeclarationCreation", s);
				    } else {
					try {
					    annotdecl = (AnnotationTypeDeclaration)decl;
					    atds.add(annotdecl);
									    
					} catch (ClassCastException cce) {
					    bark.error(Position.NOPOS, "BadDeclaration", s);
					}
				    }
				}
			    }
			    
			    currentRoundFactories.add(apf.getClass());
			    productiveFactories.add(apf.getClass());
			    factoryToAnnotation.put(apf, atds);
			} else if (productiveFactories.contains(apf.getClass())) {
			    // If a factory provided a processor in a
			    // previous round but doesn't match any
			    // annotations this round, call it with an
			    // empty set of declarations.
			    currentRoundFactories.add(apf.getClass());
			    factoryToAnnotation.put(apf, emptyATDS );
			}

			if (unmatchedAnnotations.size() == 0)
			    break;

		    } catch (ClassCastException cce) {
			bark.warning(Position.NOPOS, "BadFactory", cce);
		    }
		}

		unmatchedAnnotations.remove("");
	    }

	    // If the set difference of productiveFactories and
	    // currentRoundFactories is non-empty, call the remaining
	    // productive factories with an empty set of declarations.
	    {
		java.util.Set<Class<? extends AnnotationProcessorFactory> > neglectedFactories = 
		    new LinkedHashSet<Class<? extends AnnotationProcessorFactory>>(productiveFactories);
		neglectedFactories.removeAll(currentRoundFactories);
		for(Class<? extends AnnotationProcessorFactory> working : neglectedFactories) {
		    try {
			AnnotationProcessorFactory factory = working.newInstance();
			factoryToAnnotation.put(factory, emptyATDS);
		    } catch (Exception e ) {
			bark.warning(Position.NOPOS, "FactoryCantInstantiate", working.getName());
		    }
		
		}
	    }

	    if (unmatchedAnnotations.size() > 0)
		bark.warning(Position.NOPOS, "AnnotationsWithoutProcessors", unmatchedAnnotations);
		
	    Set<AnnotationProcessor> processors = new LinkedHashSet<AnnotationProcessor>();

	    // If there were no source files AND no factory matching "*",
	    // make sure the usage message is printed
	    if (treeList.size() == 0 && 
		factoryToAnnotation.keySet().size() == 0 )
		throw new UsageMessageNeededException();

	    try {
		for(AnnotationProcessorFactory apFactory: factoryToAnnotation.keySet()) {
		    AnnotationProcessor processor = apFactory.getProcessorFor(factoryToAnnotation.get(apFactory),
									      trivAPE);
		    if (processor != null)
			processors.add(processor);
		    else 
			bark.warning(Position.NOPOS, "NullProcessor", apFactory.getClass().getName());
		}
	    } catch(Throwable t) {
		throw new AnnotationProcessingError(t);
	    }

	    LinkedList<AnnotationProcessor> temp = new LinkedList<AnnotationProcessor>();
	    temp.addAll(processors);

	    AnnotationProcessor proc = AnnotationProcessors.getCompositeAnnotationProcessor(temp);

	    try {
		proc.process();
	    } catch (Throwable t) {
		throw new AnnotationProcessingError(t);
	    }

	    // Invoke listener callback mechanism
	    trivAPE.roundComplete();

	    FilerImpl filerimpl = (FilerImpl)trivAPE.getFiler();
	    genSourceFileNames = filerimpl.getSourceFileNames();
	    genClassFileNames = filerimpl.getClassFileNames();
	    filerimpl.flush(); // Make sure new files are written out
	}
    }

    /**
     * Convert import-style string to regex matching that string.  If
     * the string is a valid import-style string, return a regex that
     * won't match anything.
     */
    Pattern importStringToPattern(String s) {
	if (s.equals("*")) {
	    return allMatches;
	} else {
	    String t = s;
	    boolean star = false;

	    /*
	     * Validate string from factory is legal.  If the string
	     * has more than one asterisks or the asterisks does not
	     * appear as the last character (preceded by a period),
	     * the string is not legal.
	     */
	    
	    boolean valid = true;
	    int index = t.indexOf('*');
	    if (index != -1) {
		// '*' must be last character...
		if (index == t.length() -1) {
		     // ... and preceeding character must be '.'
		    if ( index-1 >= 0 ) {
			valid = t.charAt(index-1) == '.';
			// Strip off ".*$" for identifier checks
			t = t.substring(0, t.length()-2);
		    }
		} else
		    valid = false;
	    }

	    // Verify string is off the form (javaId \.)+ or javaId 
	    if (valid) {
		String[] javaIds = t.split("\\.", t.length()+2);
		for(String javaId: javaIds)
		    valid &= isJavaIdentifier(javaId);
	    }

	    if (!valid) {
		Bark bark = Bark.instance(context);
		bark.warning(Position.NOPOS, "MalformedSupportedString", s);
		return noMatches; // won't match any valid identifier
	    }
		
	    String s_prime = s.replaceAll("\\.", "\\\\.");

	    if (s_prime.endsWith("*")) {
		s_prime =  s_prime.substring(0, s_prime.length() - 1) + ".+";
	    }

	    return Pattern.compile(s_prime);
	}
    }

    private static final Pattern allMatches = Pattern.compile(".*");
    private static final Pattern noMatches  = Pattern.compile("(\\P{all})+");
}
