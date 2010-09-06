/*
 * @(#)PackageTreeWriter.java	1.31 04/03/28
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.doclets.formats.html;

import com.sun.tools.doclets.internal.toolkit.util.*;
import com.sun.javadoc.*;
import java.io.*;

/**
 * Class to generate Tree page for a package. The name of the file generated is
 * "package-tree.html" and it is generated in the respective package directory.
 *
 * @author Atul M Dambalkar
 */
public class PackageTreeWriter extends AbstractTreeWriter {

    /**
     * Package for which tree is to be generated.
     */
    protected PackageDoc packagedoc;

    /**
     * The previous package name in the alpha-order list.
     */
    protected PackageDoc prev;

    /**
     * The next package name in the alpha-order list.
     */
    protected PackageDoc next;

    /**
     * Constructor.
     * @throws IOException
     * @throws DocletAbortException
     */
    public PackageTreeWriter(ConfigurationImpl configuration,
                             String path, String filename,
                             PackageDoc packagedoc,
                             PackageDoc prev, PackageDoc next)
                      throws IOException {
        super(configuration, path, filename,
              new ClassTree(
              	configuration.classDocCatalog.allClasses(packagedoc),
                configuration), 
              packagedoc);
        this.packagedoc = packagedoc;
        this.prev = prev;
        this.next = next;
    }

    /**
     * Construct a PackageTreeWriter object and then use it to generate the
     * package tree page.
     *
     * @param pkg      Package for which tree file is to be generated.
     * @param prev     Previous package in the alpha-ordered list.
     * @param next     Next package in the alpha-ordered list.
     * @param noDeprecated  If true, do not generate any information for
     * deprecated classe or interfaces.
     * @throws DocletAbortException
     */
    public static void generate(ConfigurationImpl configuration,
                                PackageDoc pkg, PackageDoc prev,
                                PackageDoc next, boolean noDeprecated) {
        PackageTreeWriter packgen;
        String path = DirectoryManager.getDirectoryPath(pkg);
        String filename = "package-tree.html";
        try {
            packgen = new PackageTreeWriter(configuration, path, filename, pkg,
                prev, next);
            packgen.generatePackageTreeFile();
            packgen.close();
        } catch (IOException exc) {
            configuration.standardmessage.error(
                        "doclet.exception_encountered",
                        exc.toString(), filename);
            throw new DocletAbortException();
        }
    }

    /**
     * Generate a separate tree file for each package.
     */
    protected void generatePackageTreeFile() throws IOException {
        printHtmlHeader(packagedoc.name() + " " 
            + configuration.getText("doclet.Window_Class_Hierarchy"), null, true);

        printPackageTreeHeader();

        if (configuration.packages.length > 1) {
            printLinkToMainTree();
        }

        generateTree(classtree.baseclasses(), "doclet.Class_Hierarchy");
        generateTree(classtree.baseinterfaces(), "doclet.Interface_Hierarchy");
        generateTree(classtree.baseAnnotationTypes(), "doclet.Annotation_Type_Hierarchy");
        generateTree(classtree.baseEnums(), "doclet.Enum_Hierarchy");

        printPackageTreeFooter();
        printBottom();
        printBodyHtmlEnd();
    }

    /**
     * Print the navigation bar header for the package tree file.
     */
    protected void printPackageTreeHeader() {
        navLinks(true);
        hr();
        center();
        h2(configuration.getText("doclet.Hierarchy_For_Package", 
            Util.getPackageName(packagedoc)));
        centerEnd();
    }

    /**
     * Generate a link to the tree for all the packages.
     */
    protected void printLinkToMainTree() {
        dl();
        dt();
        boldText("doclet.Package_Hierarchies");
        dd();
        navLinkMainTree(configuration.getText("doclet.All_Packages"));
        dlEnd();
        hr();
    }

    /**
     * Print the navigation bar footer for the package tree file.
     */
    protected void printPackageTreeFooter() {
        hr();
        navLinks(false);
    }

    /**
     * Link for the previous package tree file.
     */
    protected void navLinkPrevious() {
        if (prev == null) {
            navLinkPrevious(null);
        } else {
            String path = DirectoryManager.getRelativePath(packagedoc.name(),
                                                           prev.name());
            navLinkPrevious(path + "package-tree.html");
        }
    }
    
    /**
     * Link for the next package tree file.
     */
    protected void navLinkNext() {
        if (next == null) {
            navLinkNext(null);
        } else {
            String path = DirectoryManager.getRelativePath(packagedoc.name(),
                                                           next.name());
            navLinkNext(path + "package-tree.html");
        }
    }

    /**
     * Link to the package summary page for the package of this tree.
     */
    protected void navLinkPackage() {
        navCellStart();
        printHyperLink("package-summary.html", "", configuration.getText("doclet.Package"),
                        true, "NavBarFont1");
        navCellEnd();
    }
}
