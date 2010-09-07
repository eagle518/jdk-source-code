 /* @(#)XMLDocumentHandler.java	1.15 10/03/24
 * 
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.panel;


import javax.swing.*;
import javax.swing.table.*;
import javax.swing.tree.*;
import javax.xml.parsers.*;
import org.xml.sax.helpers.*;
import org.xml.sax.*;
import java.awt.Font;
import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.StringTokenizer;
import java.util.Vector;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.config.Config;

/** 
 * XMLDocumentHandler is a XML handler for parsing Control Panel XML document.
 *
 * @author  Stanley Man-Kit Ho
 */
final class XMLDocumentHandler extends DefaultHandler
{    
    public void startDocument() throws SAXException        
    {
        // Populate stack with root node
        ITreeNode root = new SimpleTreeNode( getMessage("common.settings") );
        elementStack.add(0, root);
    }

    public void startElement(String uri, String localName, String qName,
                             Attributes attributes)
                             throws SAXException
    {
        if (qName.equalsIgnoreCase("label"))
        {
            /*
             * Check if node should be added.
             */
            if (getShouldAdd()){
                SimpleTreeNode stn = new SimpleTreeNode( attributes.getValue("text") );

                // Obtain top of the stack
                ITreeNode itn = (ITreeNode) elementStack.get(0);

                // Setup parent/child node relationship
                itn.addChildNode(stn);

                // Push on top of the stack
                elementStack.add(0, stn);
            }
        }
        else if (qName.equalsIgnoreCase("checkbox"))
        {       
            /*
             * Check if node should be added.
             */
            if ( getShouldAdd() ){
                // Creat Toggle property
                ToggleProperty prop = new ToggleProperty( attributes.getValue("propertyName"),
                                                      attributes.getValue("checked"));
                
                // Obtain parent node from the top of the stack
                ITreeNode node = (ITreeNode) elementStack.get(0);
                node.addProperty( prop );     
            }
        }
        else if (qName.equalsIgnoreCase("radiogroup"))
        {
            /*
             * Check if node should be added.
             */
            if ( getShouldAdd() ){
                String propName = attributes.getValue("propertyName");
                String selectedValue = attributes.getValue("checked");
                        
                // Creat RadioPropertyGroup
                RadioPropertyGroup group = new RadioPropertyGroup (propName, selectedValue);
               
                // Push on top of the stack
                elementStack.add(0, group);
            }
        }
        else if ( qName.equalsIgnoreCase("rButton") )
        {                        
            /*
             * Check if node should be added.
             */
            if ( getShouldAdd() ){
                String buttonName = attributes.getValue("text");
                
                RadioPropertyGroup group = (RadioPropertyGroup)elementStack.remove(0);
                String propertyName = group.getPropertyName();
                
                // Create RadioProperty
                RadioProperty rp = new RadioProperty( propertyName, buttonName);
                rp.setGroup(group);
                    
                // Obtain parent node from the top of the stack
                ITreeNode node = (ITreeNode) elementStack.get(0);
                node.addProperty( rp ); 
                
                // Push the RadioPropertyGroup back on top of the stack:
                elementStack.add(0, group);
            }
        }
        else if ( qName.equalsIgnoreCase("TextField") ){
                        
            /*
             * Check if node should be added.
             */
            if ( getShouldAdd() ){
                String propName = attributes.getValue("propertyName");
                TextFieldProperty tfp = new TextFieldProperty (propName, "");            
                ITreeNode itn = (ITreeNode) elementStack.get(0);
                itn.addProperty(tfp);
            }
        }
        else if ( qName.equalsIgnoreCase("platform") ){
            String platform = attributes.getValue("text");
            
            Vector allPlatforms = new Vector();
            // See if there is more then one platform specified in the attribute
            if (platform.indexOf(",") != -1){
                // Tokenize the string with "," as delimiter and save tokens in vector.
                StringTokenizer strTok = new StringTokenizer(platform, ",");
                while (strTok.hasMoreTokens()){
                    String tok = strTok.nextToken();
                    allPlatforms.add(tok.trim());
                }
            }else{
                // just add platform to the vector
                allPlatforms.add(platform.trim());
            }
            
            if ( !setShouldAdd(allPlatforms) ){  
                // Push platform name on top of the stack if nodes
                // within <platform>...</platform> should NOT be added
                // to the tree.            
                elementStack.add(0, platform);
            }
        }
        else if ( qName.equalsIgnoreCase("permission") ){
            String permission = attributes.getValue("text");
            
            if (permission.indexOf("admin") != -1){
                // Nodes within "permission" tag should be added only
                // if user has administrative privileges
                if (!Config.getHasAdminPrivileges()){
                    // Only users on windows with admin priviledges 
                    // should see this option.  If NON-ADMIN, 
                    // do not add nodes within permission tag to tree.
                    addNode = false;                    
                }
            }
            if (!addNode){
                // Push permission value on top of the stack if nodes within
                // <permission"...</permission> should NOT be added to the tree.
                elementStack.add(0, permission);
            }
        }
    }

    public void endElement(String uri, String localName, String qName)
                throws SAXException
    {
        if (qName.equalsIgnoreCase("label"))
        {
            // Pop the stack if we were adding nodes
            if ( getShouldAdd() ){
                elementStack.remove(0);
            }
        }
        else if (qName.equalsIgnoreCase("radiogroup"))
        {
            // Pop the stack if we were adding nodes.
            if ( getShouldAdd() ){                
                elementStack.remove(0);
            }
        }
        else if (qName.equalsIgnoreCase("platform")){
            // Pop the stack if we were NOT adding nodes
            if ( !getShouldAdd() ){
                elementStack.remove(0);
            }
            
            // Change addNode back to true.  Everything from now on should
            // be added to the tree.
            addNode = true;
        }
        else if (qName.equalsIgnoreCase("permission")){
            // Pop the stack if we were NOT adding nodes
            if ( !getShouldAdd() ){
                elementStack.remove(0);
            }
            
            // Change addNode back to true.  Everything from now on should
            // be added to the tree.
            addNode = true;
        }
    }
    
    /*
     * This functions returns a boolean value indicating if an 
     * element should be added to the tree.
     */
    private boolean getShouldAdd(){
        return addNode;
    }
    
    /*
     * This function sets the value for addNode when platform
     * specified in the xml does not match the platform we are 
     * running on.
     */
    private boolean setShouldAdd(Vector platforms){
        /*
         * Check if platform specified in the "platform" xml tag does not match
         * the platform we are running on.  If that's the case, then
         * the set addNode to false.
         */
        String osName = System.getProperty("os.name").toLowerCase();
        
        for (int i = 0; i < platforms.size(); i++){
	    String platform = ((String)platforms.get(i)).toLowerCase();
	 
	    if (platform.indexOf("gnome") != -1 && Config.getInstance().isLocalInstallSupported()) {
		addNode = true;
		break;
	    }

            if ( osName.indexOf( platform ) != -1 ){
                addNode = true;
                break;
            }
            addNode = false;
        }

        return addNode;
    }

    public void endDocument() throws SAXException
    {
        ITreeNode root = (ITreeNode) elementStack.get(0);

        try
        {
        // Dump property/value pairs
        //Properties props = new Properties();
        //root.storePropertyValue(props);
        //props.list(System.out);

        }
        catch (Throwable e)
        {
            e.printStackTrace();
        }
    }

    public void error(SAXParseException exception) 
    {
        System.err.println("Error: " + exception);
    }

    public void fatalError(SAXParseException exception) 
    {
        System.err.println("FatalError: " + exception);
    }

    public void warning(SAXParseException exception) 
    {
        System.err.println("Warning: " + exception);
    }

    /**
     * Construct JTree after XML parsing.
     */
    JTree getJTree()
    {
        // Root node should be at the top of the stack
        ITreeNode root = (ITreeNode) elementStack.get(0);

        final JTree tree = TreeBuilder.createTree( new PropertyTreeModel( root ) );
	tree.setFont(ResourceManager.getUIFont());
        return tree;
    }
    
    private String getMessage(String id)
    {
	return com.sun.deploy.resources.ResourceManager.getMessage(id);
    }     

    // XML element stack
    private ArrayList elementStack = new ArrayList();
    
    /*
     * This variable indicates if node should be added to the tree.  By default,
     * all nodes should be added, unless we find nodes within <platform> tag which
     * do not match with the current platform.
     */
    private boolean addNode = true;
}
