/**
 * @(#)PrintAP.java	1.8 04/06/11
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.apt.comp;

import com.sun.mirror.declaration.*;
import com.sun.mirror.type.*;
import com.sun.mirror.apt.*;

import java.util.*;
import com.sun.mirror.util.*;

/**
 * Class used to implement "-print" option.
 */
public class PrintAP implements AnnotationProcessor {


    static class PrintingVisitors {
	int indentation = 0; // Indentation level;
	AnnotationProcessorEnvironment env;
	Messager out;
	Declaration java_lang_Object;
	Declaration java_lang_annotation_Annotation;

	PrintingVisitors(AnnotationProcessorEnvironment env) {
	    this.env = env;
	    this.out = env.getMessager();
	    this.java_lang_Object = env.getTypeDeclaration("java.lang.Object");
	    this.java_lang_annotation_Annotation = env.getTypeDeclaration("java.lang.annotation.Annotation");
	}
	

	static String [] spaces = {
	    "",
	    "  ",
	    "    ",
	    "      ",
	    "        ",
	    "          ",
	    "            ",
	    "              ",
	    "                ",
	    "                  ",
	    "                    "
	};
	

	String indent(){
	    int indentation = this.indentation;
	    if (indentation < 0)
		return "";
	    else if (indentation <= 10)
		return spaces[indentation];
	    else {
		StringBuilder sb = new StringBuilder();
		while (indentation > 10) {
		    sb.append(spaces[indentation]);
		    indentation -= 10;
		}
		sb.append(spaces[indentation]);
	    return sb.toString();
	    }
	}


	class PrePrinting extends SimpleDeclarationVisitor {
	    Map<EnumDeclaration, Integer> enumCardinality  = new HashMap<EnumDeclaration, Integer>();
	    Map<EnumDeclaration, Integer> enumConstVisited = new HashMap<EnumDeclaration, Integer>();

	    PrePrinting(){}

	    public void visitClassDeclaration(ClassDeclaration d) {
		System.out.println();
		printDocComment(d);
		printModifiers(d);
		System.out.print("class " + d.getSimpleName());
		printFormalTypeParameters(d);

		// Elide "extends Object"
		ClassType Super = d.getSuperclass();
		if (Super != null && !java_lang_Object.equals(Super.getDeclaration()) )
		    System.out.print(" extends " + Super.toString());

		printInterfaces(d);

		System.out.println(" {");

		PrintingVisitors.this.indentation++;
	    }	    

	    public void visitEnumDeclaration(EnumDeclaration d) {
		enumCardinality.put(d, d.getEnumConstants().size());
		enumConstVisited.put(d, 1);

		System.out.println();
		printDocComment(d);
		printModifiers(d);

		System.out.print("enum " + d.getSimpleName());
		printFormalTypeParameters(d);
		printInterfaces(d);

		System.out.println(" {");

		PrintingVisitors.this.indentation++;
	    }	    


	    public void visitInterfaceDeclaration(InterfaceDeclaration d) {
		System.out.println();
		printDocComment(d);
		printModifiers(d);
		System.out.print("interface " + d.getSimpleName());

		printFormalTypeParameters(d);
		printInterfaces(d);

		System.out.println(" {");

		PrintingVisitors.this.indentation++;
	    }	    

	    public void visitAnnotationTypeDeclaration(AnnotationTypeDeclaration d) {
		System.out.println();
		printDocComment(d);
		printModifiers(d);
		System.out.print("@interface " + d.getSimpleName());
		printFormalTypeParameters(d);

		printInterfaces(d); 

		System.out.println(" {");

		PrintingVisitors.this.indentation++;
	    }	    

	    public void visitFieldDeclaration(FieldDeclaration d) {
		System.out.println();
		printDocComment(d);
		printModifiers(d);
		System.out.print(d.getType().toString() + " " + 
				   d.getSimpleName() );
		String constantExpr = d.getConstantExpression();
		if (constantExpr != null) {
		    System.out.print(" = " + constantExpr);
		}
		System.out.println(";" );
	    }

	    public void visitEnumConstantDeclaration(EnumConstantDeclaration d) {
		EnumDeclaration ed = d.getDeclaringType();
		int enumCard = enumCardinality.get(ed); 
		int enumVisit = enumConstVisited.get(ed); 

		System.out.println();
		printDocComment(d);
		System.out.print(PrintingVisitors.this.indent());
		System.out.print(d.getSimpleName() );
		System.out.println((enumVisit < enumCard )? ",":";" );
		
		enumConstVisited.put(ed, enumVisit+1);
	    }

	    public void visitMethodDeclaration(MethodDeclaration d) {
		System.out.println();
		printDocComment(d);
		printModifiers(d);
		printFormalTypeParameters(d);
		System.out.print(d.getReturnType().toString() + " ");
		System.out.print(d.getSimpleName() + "(");
		printParameters(d);
		System.out.print(")");
		printThrows(d);
		System.out.println(";");
	    }

	    public void visitConstructorDeclaration(ConstructorDeclaration d) {
		System.out.println();
		printDocComment(d);
		printModifiers(d);
		printFormalTypeParameters(d);
		System.out.print(d.getSimpleName() + "(");
		printParameters(d);
		System.out.print(")");
		printThrows(d);
		System.out.println(";");
	    }


	}

	class PostPrinting extends SimpleDeclarationVisitor {
	    PostPrinting(){}

	    public void visitTypeDeclaration(TypeDeclaration d) {
		PrintingVisitors.this.indentation--;

		System.out.print(PrintingVisitors.this.indent());
		System.out.println("}");
	    }	    
	}

	private void printAnnotations(Collection<AnnotationMirror> annots) {

	    for(AnnotationMirror annot: annots) {
		System.out.print(this.indent());
		System.out.print(annot.toString());
		System.out.println();
	    }
	}

	private void printAnnotationsInline(Collection<AnnotationMirror> annots) {

	    for(AnnotationMirror annot: annots) {
		System.out.print(annot);
		System.out.print(" ");
	    }
	}


	private void printParameters(ExecutableDeclaration ex) {

	    Collection<ParameterDeclaration> parameters = ex.getParameters();
	    int size = parameters.size();

	    switch (size) {
	    case 0:
		break;
		
	    case 1:
		for(ParameterDeclaration parameter: parameters) {
		    printModifiers(parameter);

		    if (ex.isVarArgs() ) {
			System.out.print(((ArrayType)parameter.getType()).getComponentType() );
			System.out.print("...");
		    } else
			System.out.print(parameter.getType());
		    System.out.print(" " + parameter.getSimpleName());
		}
		break;

	    default:
		{
		    int i = 1;
		    for(ParameterDeclaration parameter: parameters) {
			if (i == 2)
			    PrintingVisitors.this.indentation++;

			if (i > 1)
			    System.out.print(PrintingVisitors.this.indent());

			printModifiers(parameter);

			if (i == size && ex.isVarArgs() ) {
			    System.out.print(((ArrayType)parameter.getType()).getComponentType() );
			    System.out.print("...");
			} else
			    System.out.print(parameter.getType());
			System.out.print(" " + parameter.getSimpleName());

			if (i < size)
			    System.out.println(",");
			    
			i++;
		    }
		    
		    if (parameters.size() >= 2) 
			PrintingVisitors.this.indentation--;
		}
		break;
	    }
	}
	
	private void printDocComment(Declaration d) {
	    String docComment = d.getDocComment();

	    if (docComment != null) {
		// Break comment into lines 
		java.util.StringTokenizer st = new StringTokenizer(docComment,
								  "\n\r");
		System.out.print(PrintingVisitors.this.indent());
		System.out.println("/**");
		
		while(st.hasMoreTokens()) {
		    System.out.print(PrintingVisitors.this.indent());
		    System.out.print(" *");
		    System.out.println(st.nextToken());
		}

		System.out.print(PrintingVisitors.this.indent());
		System.out.println(" */");
	    }
	}

	private void printModifiers(Declaration d) {
	    printAnnotations(d.getAnnotationMirrors());

	    System.out.print(PrintingVisitors.this.indent());
	    Collection<Modifier> mods = d.getModifiers();
	    for(Modifier m: mods) {
		System.out.print(m.toString() + " ");
	    }
	}

	private void printModifiers(ParameterDeclaration d) {
	    printAnnotationsInline(d.getAnnotationMirrors());

	    Collection<Modifier> mods = d.getModifiers();
	    for(Modifier m: mods) {
		System.out.print(m.toString() + " ");
	    }
	}

	private void printFormalTypeParameters(ExecutableDeclaration e) {
	    printFormalTypeParameterSet(e.getFormalTypeParameters());
	    System.out.print(" ");
	}

	private void printFormalTypeParameters(TypeDeclaration d) {
	    printFormalTypeParameterSet(d.getFormalTypeParameters());
	}

	private void printFormalTypeParameterSet(Collection<TypeParameterDeclaration> typeParams) {
	    if (typeParams.size() != 0) {
		System.out.print("<");

		boolean first = true;
		for(TypeParameterDeclaration tpd: typeParams) {
		    if (!first)
			System.out.print(", ");
		    System.out.print(tpd.toString());
		}

		System.out.print(">");
	    }
	}

	private void printInterfaceSet(Collection<InterfaceType> interfaces,
				       boolean classNotInterface) {
	    if (interfaces.size() != 0) {
		System.out.print((classNotInterface?" implements" : " extends"));
		
		boolean first = true;
		for(InterfaceType interType: interfaces) {
		    if (!first)
			System.out.print(",");
		    System.out.print(" ");
		    System.out.print(interType.toString());
		    first = false;
		}
	    }
	}

	private void printInterfaces(TypeDeclaration d) {
	    printInterfaceSet(d.getSuperinterfaces(), d instanceof ClassDeclaration);
	}

	private void printInterfaces(AnnotationTypeDeclaration d) {
	    Collection<InterfaceType> interfaces = new HashSet<InterfaceType>(d.getSuperinterfaces());

	    for(InterfaceType interType: interfaces) {
		if (java_lang_annotation_Annotation.equals(interType.getDeclaration()) )
		    interfaces.remove(interType);
	    }

	    printInterfaceSet(interfaces, d instanceof ClassDeclaration);
	}

	private void printThrows(ExecutableDeclaration d) {
	    Collection<ReferenceType> thrownTypes = d.getThrownTypes();
	    final int size = thrownTypes.size();
	    if (size != 0) {
		System.out.print(" throws");

		int i = 1;
		for(ReferenceType thrownType: thrownTypes) {
		    if (i == 1) {
			System.out.print(" ");
		    }

		    if (i == 2)
			PrintingVisitors.this.indentation++;

		    if (i >= 2)
			System.out.print(PrintingVisitors.this.indent());			

		    System.out.print(thrownType.toString());
		    

		    if (i != size) {
			System.out.println(", ");
		    }
		    i++;
		}

		if (size >= 2)
		    PrintingVisitors.this.indentation--;
	    }
	}

	DeclarationVisitor getPrintingVisitor() {
	    return DeclarationVisitors.getSourceOrderDeclarationScanner(new PrePrinting(),
									new PostPrinting());
	}
    }

    AnnotationProcessorEnvironment env;
    PrintAP(AnnotationProcessorEnvironment env) {
	this.env = env;
    }


    public void process() {
	Collection<TypeDeclaration> typedecls = env.getSpecifiedTypeDeclarations();
	
	for (TypeDeclaration td: typedecls)
	    td.accept((new PrintingVisitors(env)).getPrintingVisitor());
    }
}
