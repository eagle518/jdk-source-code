/*
 * @(#)HTMLConverter.java	1.27 02/04/08
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

import sun.plugin.converter.gui.ConverterGUI;
import sun.plugin.converter.engine.*;
import sun.plugin.converter.util.*;
import sun.plugin.converter.*;
import java.io.*;
import java.util.*;

public class HTMLConverter{
    
    /**
     * this object will hold all command line data entered by user
     */
    private CommandLine cmdLine;

    private FlexFilter fileFilter = new FlexFilter();
    private PluginConverter converter = null;
    private DefaultSet set = null;
    private File logFile = null;
    private boolean stdin = false;
    private boolean stdout = false;

    private String sourceStr = null;
    private String backupStr = null;
    private String destStr = null;
    private String tempStr = null;
    private String logStr = null;

    private String sep = System.getProperty("file.separator");

    /**
     * Handles reading the command line and determining to go with or
     * without GUI
     * @param s[] holds the value of the command line entered by user
     */
     public HTMLConverter(String s[]){
	//this will get all the info we need from the command line
        try { 
            cmdLine = new CommandLine(s);
            if(cmdLine.justHelp()){
	        showHelp();
		System.exit(-1);
	    }
	    else if(cmdLine.doGUI())
	        processWithGUI();
	    else
	        processCommandLine();
	
         }
         catch (CommandLineException e) {
             showHelp();
	     System.exit(-1);
         }   	
         catch (IndexOutOfBoundsException e) {
             showHelp();
	     System.exit(-1);
         }   	
         catch (NotDirectoryException e) {
	     System.out.println(e.getMessage());
	     System.exit(-1);
         }   	
         catch (FileAccessException e) {
             System.out.println(e.getMessage());
	     System.exit(-1);
         }   	
	
    }

    /**
     * Initiates and sets GUI version properly
     */
    private void processWithGUI(){
	
	ConverterGUI userInterface = new ConverterGUI();
	
	//this is where we will need to set the initial settings for GUI 
	
	userInterface.pack();
	userInterface.setVisible(true);
	
    }
    
    /**
     * Initiates and sets Command Line version properly
     */
    private void processCommandLine() throws NotDirectoryException, FileAccessException {

        Vector files;

        try {
	    converter = new PluginConverter(cmdLine.isLogFile(), cmdLine.showProgress());
            converter.setStaticVersioning(cmdLine.getStaticVersioning());
	    converter.setOverwrite(cmdLine.isOverwrite());
	    converter.setConvertSet(set = new DefaultSet());
	    converter.setCommandLine(true);
	}
	catch(FileNotFoundException e){
	    System.out.println(ResourceHandler.getMessage("notemplate_dialog.info0") + converter.getDefaultTemplateFileName() + ResourceHandler.getMessage("notemplate_dialog.info1"));
	    System.exit(-2);
	}
	
        if(cmdLine.isStdIn()) {
                stdin = true; 
                files = cmdLine.getStandardInput();
    	        if (files != null) {
                   for (Enumeration e = files.elements(); e.hasMoreElements() ;) {
	              fileFilter.addDescriptor((String)e.nextElement());	
	           }   
                } 
        }
        else { 
	        files = cmdLine.getFileSpecs();
                for (Enumeration e = files.elements(); e.hasMoreElements() ;) {
	            fileFilter.addDescriptor((String)e.nextElement());	
	        }   
       }	    
	    
      if(cmdLine.doSubDirs()){	//  -subdirs
		fileFilter.setFilesOnly(false);
		converter.setRecurse(true);
      }
	    
      if(cmdLine.showProgress()){	//  -progress
        	converter.setShowProgressStdOut(true);
      }	   

      if(cmdLine.isStdOut()) {
                stdout = true;
      } 
	  
	    //set the values in the defaultSet object
      setSource();
      setDest();
      setBack();
      setTemp();
      setLog();	    
   
      Enumeration filesList = set.getFilesEnumeration();
      if(!stdin && set.getFileCount()==0){
          System.out.println(ResourceHandler.getMessage("plugin_converter.nofiles"));
	  System.exit(-2); 
      } 
      else{
          if(cmdLine.justSimulate()){
	      simulate(filesList);
          }
	  else{
              converter.setSourceType(stdin);
              converter.setDestType(stdout);
       	      converter.startConversion();
	  }
      }							
    }					
    
    private void showHelp(){
	System.out.println(ResourceHandler.getMessage("plugin_converter.help_message"));
    }
    
    public static boolean checkTrue(String t){
	t = t.trim().toUpperCase();
	return (
		t.equals("TRUE") ||
		t.equals("T")	||
		t.equals("YES")	||
		t.equals("Y"));
    }
    
   /**
     * This helps eliminate bulkyness of the process command line by
     * grouping source file processes together
     */
    private void setSource() throws NotDirectoryException {
             sourceStr = cmdLine.getSourceDir();
  	     if(sourceStr != null){
		  File f = new File(sourceStr);
      		  set.setSourcePath(f,fileFilter);
     	     }
	     else{
	        ((DefaultSet)set).setFiles(fileFilter);
	     }
     }
    
     /**
     * This helps eliminate bulkyness of the process command line by
     * grouping destination file processes together
     */
    private void setDest(){
	destStr = cmdLine.getDestDir();
	if(destStr != null){
	    File f = new File(destStr);
	    set.setDestinationPath(f);	
	}
	set.setDestinationPath(((DefaultSet)set).getDefaultDestPath());
    }
    
     /**
     * This helps eliminate bulkyness of the process command line by
     * grouping backup file processes together
     */
    private void setBack(){
	backupStr = cmdLine.getBackupDir();
	if(backupStr != null){
	    File f = new File(backupStr);		
	    set.setBackupPath(f);	
	}
	else if (!stdin)
	    set.setBackupPath(((DefaultSet)set).getDefaultBackupPath());
    }
    
     /**
     * This helps eliminate bulkyness of the process command line by
     * grouping template file processes together
     */
    private void setTemp(){
	tempStr = cmdLine.getTemplateDir();
	if(tempStr != null){
	    try{			    
		InputStream is = HTMLConverter.class.getClassLoader().getResourceAsStream(tempStr);
		converter.setTemplateFilePath(tempStr);
		
	    }
	    catch(FileNotFoundException e){
		System.out.println(ResourceHandler.getMessage("nottemplatefile_dialog.info2") +  ":  " + tempStr);
		System.exit(-2);
	    }
	}   
    }

    /**
     * This helps eliminate bulkyness of the process command line by
     * grouping log file processes together
     */
    private void setLog(){
	logStr = cmdLine.getLogFile();
	if(logStr != null){
	    logFile = new File(logStr);
	    converter.setLogFile(logFile);
	}
    }

    //this is the output from a simulation
    private void simulate(Enumeration filesList){
	System.out.println(ResourceHandler.getMessage("plugin_converter.files"));
	while(filesList.hasMoreElements()){					
	    System.out.println(set.getSourcePath()+sep+(String)filesList.nextElement());
	}
	System.out.println("\n" + ResourceHandler.getMessage("plugin_converter.backup_path") + 
			   ":\t"+set.getBackupPath());
	if(logFile!=null)
	    System.out.println(ResourceHandler.getMessage("plugin_converter.log_path") + 
			       ":\t"+((cmdLine.getLogFile()==null)?(converter.getDefaultLogFile()).
				      getAbsolutePath():logFile.getAbsolutePath()));
	
	System.out.println("\n" + ResourceHandler.getMessage("plugin_converter.template_file") +
			   ":\t"+converter.getTemplateFilePath());						
	System.out.println(ResourceHandler.getMessage("plugin_converter.process_subdirs") +
			   ":\t"+converter.isRecurse());
	System.out.println(ResourceHandler.getMessage("plugin_converter.show_progress") +
			   ":\t"+converter.isShowProgressStdOut());
	System.out.println("");				    
    }

    public static void main(String s[]){
	
	HTMLConverter htmlConverter = new HTMLConverter(s);
	
    }

}
