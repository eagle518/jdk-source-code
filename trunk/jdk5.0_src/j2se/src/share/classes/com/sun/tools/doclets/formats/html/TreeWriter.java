/*
 * @(#)TreeWriter.java	1.39 04/01/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.formats.html;

import com.sun.tools.doclets.internal.toolkit.util.*;
import com.sun.javadoc.*;
import java.io.*;
/**
 * Generate Class Hierarchy page for all the Classes in this run.  Use
 * ClassTree for building the Tree. The name of
 * the generated file is "overview-tree.html" and it is generated in the
 * current or the destination directory.
 *
 * @author Atul M Dambalkar
 */
public class TreeWriter extends AbstractTreeWriter {

    /**
     * Packages in this run.
     */
    private PackageDoc[] packages;

    /**
     * True if there are no packages specified on the command line,
     * False otherwise.
     */
    private boolean classesonly;

    /**
     * Constructor to construct TreeWriter object.
     *
     * @param configuration the current configuration of the doclet.
     * @param filename String filename
     * @param classtree the tree being built.
     */
    public TreeWriter(ConfigurationImpl configuration,
            String filename, ClassTree classtree)
    throws IOException {
        super(configuration, filename, classtree);
        packages = configuration.packages;
    classesonly = packages.length == 0;
    }

    /**
     * Create a TreeWriter object and use it to generate the
     * "overview-tree.html" file.
     *
     * @param classtree the class tree being documented.
     * @throws  DocletAbortException
     */
    public static void generate(ConfigurationImpl configuration,
                                ClassTree classtree) {
        TreeWriter treegen;
        String filename = "overview-tree.html";
        try {
            treegen = new TreeWriter(configuration, filename, classtree);
            treegen.generateTreeFile();
            treegen.close();
        } catch (IOException exc) {
            configuration.standardmessage.error(
                        "doclet.exception_encountered",
                        exc.toString(), filename);
            throw new DocletAbortException();
        }
    }

    /**
     * Print the interface hierarchy and class hierarchy in the file.
     */
    public void generateTreeFile() throws IOException {
        printHtmlHeader(configuration.getText("doclet.Window_Class_Hierarchy"),
            null, true);

        printTreeHeader();

        printPageHeading();
    
        printPackageTreeLinks();

        generateTree(classtree.baseclasses(), "doclet.Class_Hierarchy");
        generateTree(classtree.baseinterfaces(), "doclet.Interface_Hierarchy");
        generateTree(classtree.baseAnnotationTypes(), "doclet.Annotation_Type_Hierarchy");
        generateTree(classtree.baseEnums(), "doclet.Enum_Hierarchy");

        printTreeFooter();
    }

    /**
     * Generate the links to all the package tree files.
     */
    protected void printPackageTreeLinks() {
        //Do nothing if only unnamed package is used
        if (packages.length == 1 && packages[0].name().length() == 0) {
            return;
        }
        if (!classesonly) {
            dl();
            dt();
            boldText("doclet.Package_Hierarchies");
            dd();
            for (int i = 0; i < packages.length; i++) {
                if (packages[i].name().length() == 0) {
                    continue;
                }
                String filename = pathString(packages[i], "package-tree.html");
                printHyperLink(filename, "", packages[i].name());
                if (i < packages.length - 1) {
                    print(", ");
                }
            }
            dlEnd();
            hr();
        }
    }

    /**
     * Print the navigation bar links at the top.
     */
    protected void printTreeHeader() {
        navLinks(true);
        hr();
    }

    /**
     * Print the navigation bar links at the bottom.
     */
    protected void printTreeFooter() {
        hr();
        navLinks(false);
        printBottom();
        printBodyHtmlEnd();
    }

    /**
     * Print the page title "Hierarchy For All Packages" at the top of the tree
     * page.
     */
    protected void printPageHeading() {
        center();
        h2();
        printText("doclet.Hierarchy_For_All_Packages");
        h2End();
        centerEnd();
    }
}
