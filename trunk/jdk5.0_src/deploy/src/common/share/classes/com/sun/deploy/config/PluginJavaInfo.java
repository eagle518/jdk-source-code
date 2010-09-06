/*
 * @(#)PluginJavaInfo.java	1.10 04/02/03
 * 
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.config;

import java.io.File;
import java.util.Vector;
import java.util.StringTokenizer;
import java.util.Properties;
import java.util.Enumeration;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import com.sun.deploy.panel.PlatformSpecificUtils;

/**
 *
 * @author  mfisher
 * @version 
 */
public class PluginJavaInfo extends JREInfo {

    private static ArrayList pluginJres = new ArrayList(); 
    private static String JRENAME = getMessage("jpi.jre.string");
    private static String JDKNAME = getMessage("jpi.jdk.string");

    private String vm_args;    
    private String productName = JRENAME;   
    
    /** Creates new PluginJavaInfo */
    public PluginJavaInfo(String version, String path, boolean system, String args) {
                        
        super((String) null, version, (String)null, path, Config.getOSName(), Config.getOSArch(), false, false);
        setSystemJRE(system);
        vm_args = args;        
    }
    
    public PluginJavaInfo(PluginJavaInfo toCopy) {
        super(toCopy.getPlatform(),
              toCopy.getProduct(),
              toCopy.getLocation(),
              toCopy.getPath(),
              toCopy.getOSName(),
              toCopy.getOSArch(),
              toCopy.isEnabled(),
              toCopy.isRegistered());
        
        setSystemJRE(toCopy.isSystemJRE());
        
        vm_args = toCopy.getVmArgs();  
        productName = toCopy.getProductName();
    }
    
    public String getVmArgs (){ return vm_args; }
    public String getProductName(){ return productName; }
    
    public void setVmArgs(String vmArgs){ vm_args = vmArgs; }
    public void setProductName( String name){ productName = name; }
    
    /*
     * Override addJRE() in the super class, so that we would be adding
     * PluginJavaInfo to array of plugin jres.
     */
    public static void addJRE(PluginJavaInfo je) {
	pluginJres.add(je);
    }

    /*
     * Override removeJRE in the super class, so that we would be
     * removing PluginJavaInfo objects from array of plugin jres.
     */    
    public static void removeJRE(int index) {
	pluginJres.remove(index);
    }

    /*
     * Override setJREInfo of the super class.
     */
    public static void setJREInfo(int index, PluginJavaInfo je) {
	pluginJres.set(index, je);
    }

    /*
     * Override clear() of the super class to clear only
     * plugin jres.
     */
    public static void clear() {
	pluginJres.clear();
    }

    /*
     * return all plugin jres.
     */
    public static PluginJavaInfo[] getAll() {
        return (PluginJavaInfo []) pluginJres.toArray(new PluginJavaInfo[0]);
    }    
    
    /*
     * Get public JREs and JREs from the properties file.
     * Set vm arguments for JREs.
     */
    public static void initialize(Properties jreProps, Properties jdkProps){

	clear();

        /*
         * First get system-wide JREs
         */
        Vector vec =  PlatformSpecificUtils.getPublicJres();
        setInstalledJREList(vec);
        
        //
        // now get jdks
        // Don't get JDKs.  Decided not to display JDKs starting with 1.5
        //vec = PlatformSpecificUtils.getPublicJdks();
        //setInstalledJDKList(vec);
                
        /*
         * Then see if the JREs/JDKs from properties file have arguments for
         * system-wide jres/jdks.
         */
        addToList(jreProps);
        addToList(jdkProps);                       
    }
    
    private static void addToList(Properties props){
        Enumeration en = props.keys();
        HashMap paths = new HashMap();
        HashMap args = new HashMap();
        boolean jresProps = true;
        
	while(en.hasMoreElements()) {
	    String key = (String) en.nextElement();
                          
            // Extract version information.                
            int begin, end;
            String version;
            String pathStr;
            String argStr;
            
            if (key.startsWith(Config.JPI_JRE_KEY)){
                // These are JRE properties                            
                begin = Config.JPI_JRE_KEY.length(); 
            }else{
                // These are JDK properties
                begin = Config.JPI_JDK_KEY.length();
                jresProps = false;
            }
            
	    if (key.endsWith(Config.JPI_JAVA_PATH)) {
                // This is deployment.javapi.jre.1.1.1_01.path property
                end = key.indexOf(Config.JPI_JAVA_PATH); 
                version = key.substring(begin, end);
                pathStr = (String)props.get(key);
                paths.put(version, pathStr);
            }else{
                // This is deployment.javapi.jre.1.1.1_01.args property
                end = key.indexOf(Config.JPI_JAVA_ARGS);     
		if (end != -1) {
		    version = key.substring(begin, end);                
		    argStr = (String)props.get(key);
		    args.put(version, argStr);
		}
            }

        }// end while...
        
        /*
         * Now, go through the list of existing jres/jdks for plugin and see if  
         * version of JRE/JDK in HashMap(s) is already in the list of Plugin' jres.
         * If it is, then set path and args fields for it.  
         * If it is not, then add new PluginJavaInfo to the list.
         *
         * We are searching in paths HashMap, since version and path are required
         * for jre entry to be saved in properties file.
         */
        PluginJavaInfo[] allJres = PluginJavaInfo.getAll();
        for(int i = 0; i < allJres.length; i++) {	 
	    // only compare if it is the right OSName and OSArch
	    if (allJres[i].getOSName().equals(Config.getOSName()) &&
		allJres[i].getOSArch().equals(Config.getOSArch())) {
		
		String version = allJres[i].getProduct();
		if (paths.containsKey(version)){
		    // delete this entry from both hash maps
		    paths.remove(version);
		    if (args.containsKey(version)){
			// See if there are arguments for this JRE/JDK first.
			String vm_args = (String)args.get(version);
			allJres[i].setVmArgs(vm_args);
			args.remove(version);
		    }
		}
	    }
        }  
        
        /*
         * Now we have only non-duplicate entries in the HashMap(s).
         * Add them to the PluginJavaInfo list.
         */
        Iterator iter = paths.keySet().iterator();
        while(iter.hasNext()) {
            String key = (String) iter.next();
            String _version = key;
            String _path = (String)paths.get(key);
            String _args = (String)args.get(key);
            PluginJavaInfo newInfo = new PluginJavaInfo( _version,
                                                         _path,
                                                         false, 
                                                         _args);
            if ( !jresProps ){
                newInfo.setProductName(JDKNAME);
            }
            PluginJavaInfo.addJRE(newInfo);                                                              
        }        
    }
    
    /*
     * Add system-wide JREs to the list
     */
    public static void setInstalledJREList(Vector vec){
        // Vector contains: version, path pairs.                
        for (int i = 0; i < vec.size(); i+=2){
            String version = (String)vec.get(i);
            String path = (String)vec.get(i + 1);

            /*
             * Do not add x.x to the list, because it will point to the latest
             * installed JRE and we'll have duplicate path entries.
             * Example:
             * Do not add 1.5 to the list, because if you install 1.5.0_01, they will
             * be pointing to the same JRE.
             */
            if ( version.lastIndexOf(".") > 2 ){
                // These are system-wide JREs.
                PluginJavaInfo jre = new PluginJavaInfo(version, path, true, "");
                PluginJavaInfo.addJRE(jre);
            }
        }
    }
    
    /*
     * Add system-wide JDKs to the list
     * This code is not used, since we don't show JDKs in the list.
     */
    public static void setInstalledJDKList(Vector vec){
        // Vector contains: version, path pairs. 
        for (int i = 0; i < vec.size(); i+=2){
            String version = (String)vec.get(i);
            String path = (String)vec.get(i + 1);
            
            // These are system-wide JDKs.
            PluginJavaInfo jdk = new PluginJavaInfo(version, path, true, "");
            jdk.setProductName(JDKNAME);
            PluginJavaInfo.addJRE(jdk);            
        }
    }
        
    private static String getMessage(String id)
    {
	return com.sun.deploy.resources.ResourceManager.getMessage(id);
    } 
}
