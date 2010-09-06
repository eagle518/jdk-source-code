/*
 * @(#)AbstractTreeWriter.java	1.31 04/01/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.formats.html;

import com.sun.tools.doclets.internal.toolkit.util.*;

import com.sun.javadoc.*;
import java.io.*;
import java.util.*;

/**
 * Abstract class to print the class hierarchy page for all the Classes. This
 * is sub-classed by {@link PackageTreeWriter} and {@link TreeWriter} to
 * generate the Package Tree and global Tree(for all the classes and packages)
 * pages.
 *
 * @author Atul M Dambalkar
 */
public abstract class AbstractTreeWriter extends HtmlDocletWriter {

    /**
     * The class and interface tree built by using {@link ClassTree}
     */
    protected final ClassTree classtree;

    /**
     * Constructor initilises classtree variable. This constructor will be used
     * while generating global tree file "overview-tree.html".
     *
     * @param filename   File to be generated.
     * @param classtree  Tree built by {@link ClassTree}.
     * @throws IOException
     * @throws DocletAbortException
     */
    protected AbstractTreeWriter(ConfigurationImpl configuration,
                                 String filename, ClassTree classtree)
                                 throws IOException {
        super(configuration, filename);
        this.classtree = classtree;
    }
    
    /**
     * Create appropriate directory for the package and also initilise the
     * relative path from this generated file to the current or
     * the destination directory. This constructor will be used while
     * generating "package tree" file.
     *
     * @param path Directories in this path will be created if they are not
     * already there.
     * @param filename Name of the package tree file to be generated.
     * @param classtree The tree built using {@link ClassTree}.
     * for the package pkg.
     * @param pkg PackageDoc for which tree file will be generated.
     * @throws IOException
     * @throws DocletAbortException
     */
    protected AbstractTreeWriter(ConfigurationImpl configuration,
                                 String path, String filename,
                                 ClassTree classtree, PackageDoc pkg)
                                 throws IOException {
        super(configuration,
              path, filename, DirectoryManager.getRelativePath(pkg.name()));
        this.classtree = classtree;
    }

    /**
     * Generate each level of the class tree. For each sub-class or
     * sub-interface indents the next level information.
     * Recurses itself to generate subclasses info.
     * To iterate is human, to recurse is divine - L. Peter Deutsch.
     *
     * @param parent the superclass or superinterface of the list.
     * @param list list of the sub-classes at this level.
     * @param isEnum true if we are generating a tree for enums.
     */
    protected void generateLevelInfo(ClassDoc parent, List list, 
            boolean isEnum) {
        if (list.size() > 0) {
            ul();
            for (int i = 0; i < list.size(); i++) {
                ClassDoc local = (ClassDoc)list.get(i);
                printPartialInfo(local);
                printExtendsImplements(parent, local);
                generateLevelInfo(local, classtree.subs(local, isEnum), 
                    isEnum);   // Recurse
            }
            ulEnd();
        }
    }

    /**
     * Generate the heading for the tree depending upon tree type if it's a
     * Class Tree or Interface tree and also print the tree.
     *
     * @param list List of classes which are at the most base level, all the
     * other classes in this run will derive from these classes.
     * @param heading Heading for the tree.
     */
    protected void generateTree(List list, String heading) {
        if (list.size() > 0) {
            ClassDoc firstClassDoc = (ClassDoc)list.get(0);
            printTreeHeading(heading);
            generateLevelInfo(!firstClassDoc.isInterface()? firstClassDoc : null, 
                list,
                list == classtree.baseEnums());
        }
    }

    /**
     * Print the information regarding the classes which this class extends or
     * implements.
     *
     * @param cd The classdoc under consideration.
     */
    protected void printExtendsImplements(ClassDoc parent, ClassDoc cd) {
        ClassDoc[] interfaces = cd.interfaces();
        if (interfaces.length > (cd.isInterface()? 1 : 0)) {
            Arrays.sort(interfaces);            
            int counter = 0;
            for (int i = 0; i < interfaces.length; i++) {
                if (parent != interfaces[i]) {
                    if (! (interfaces[i].isPublic() ||
                            Util.isLinkable(interfaces[i], configuration()))) {
                        continue;
                    }
                    if (counter == 0) {
                        if (cd.isInterface()) {
                            print(" (" + configuration.getText("doclet.also") + " extends ");
                        } else {
                            print(" (implements ");
                        }
                    } else {
                        print(", ");
                    }
                    printPreQualifiedClassLink(LinkInfoImpl.CONTEXT_TREE, 
                        interfaces[i]);
                    counter++;
                }
            }
            if (counter > 0) {
                println(")");
            }
        }
    }

    /**
     * Print information about the class kind, if it's a "class" or "interface".
     *
     * @param cd classdoc.
     */
    protected void printPartialInfo(ClassDoc cd) {
        li("circle");
        printPreQualifiedBoldClassLink(LinkInfoImpl.CONTEXT_TREE, cd);
    }

    /**
     * Print the heading for the tree.
     *
     * @param heading Heading for the tree.
     */
    protected void printTreeHeading(String heading) {
        h2();
        println(configuration.getText(heading));
        h2End();
    }

    /**
     * Highlight "Tree" word in the navigation bar, since this is the tree page.
     */
    protected void navLinkTree() {
        navCellRevStart();
        fontStyle("NavBarFont1Rev");
        boldText("doclet.Tree");
        fontEnd();
        navCellEnd();
    }
}
