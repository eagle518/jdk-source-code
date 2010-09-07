/*
 * @(#)JVMParameters.java	1.35 10/05/20
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.util;

import java.io.*;
import java.util.*;

import com.sun.deploy.Environment;
import com.sun.deploy.config.Config;

/** Represents a set of JVM command-line arguments and parameters such
    as maximum heap size, system property settings, Java platform
    version, etc. May be parsed from a command line or elements may be
    set individually. <P>

    FIXME: needs more thought. Part of the intent is to capture the
    essential JVM configuration (max heap, etc.) so that when a given
    applet requests a certain set of JVM parameters we can check the
    currently-attached JVMs and see whether any one satisfies the
    request so that we can put the applet in that JVM instance.
    However this class now also encapsulates the notion of the entire
    command line. May want to split that out into a separate class. <P>
*/

public class JVMParameters {
    private static final boolean DEBUG   = false;

    // We need to support three basic classes of command-line arguments:
    // 1. Internal command-line arguments. These are for example the
    //    -Xbootclasspath entries we need to add in order to bootstrap
    //    the new Java Plug-In.
    //
    // 2. Trusted command-line arguments. These come from
    //    deployment.properties and are specified by the end user via the
    //    Java Control Panel. It is important that such command-line
    //    arguments do not affect the "secure" state of the target JVM.
    //    For example, the user needs to be able to specify -Xdebug
    //    -Xrunjdwp[...] via the Java Control Panel to enable debugging
    //    of applets on the client machine without affecting the ability
    //    of the target JVM to run unsigned applets. We consider trusted
    //    command-line arguments in the satisfies() computation.
    //
    // 3. Normal (untrusted) command-line arguments. These are specified
    //    via the java_arguments parameter in the HTML. If a non-secure
    //    command-line argument is specified via this mechanism, then
    //    the target JVM must only run trusted code (signed, with the
    //    security dialog accepted), because the security sandbox might
    //    be compromised.

    // All internal arguments we add, e.g -Xbootclasspath
    private List/*<String>*/ internalArguments = new ArrayList/*<String>*/();

    // Conceptually, the next two ArgumentSets are concatenated.
    // Several methods below which deal with arguments have been
    // changed to fall off the end of one argument set and resume at
    // the beginning of the next one.

    // The trusted command-line arguments coming from deployment.properties
    private ArgumentSet trustedArguments = new ArgumentSet();
    // The normal command-line arguments coming from the HTML
    private ArgumentSet arguments = new ArgumentSet();
    // Maximum heap size in bytes. Value of zero indicates the max
    // heap size is not specified by -Xmx vm argument. Default max
    // heap size will be used.
    private long maxHeap = 0;
    // An indication of whether these JVM parameters should be
    // considered the defaults. This is used to understand, for
    // example, that deployment.properties may contain JVM arguments
    // for a given JVM version, and that applets that don't specify
    // command line arguments should by default execute in a JVM with
    // those parameters.
    private boolean isDefault;
    // It seems to be necessary for us to know the default maximum
    // heap size of the JVM to make intelligent decisions about when
    // to launch applets in the same JVM if an applet explicitly
    // requests a smaller-than-default heap size. (FIXME: would be
    // better not to know this, especially in the face of ergonomics,
    // where this number is wrong.)
    private static final long DEFAULT_HEAP = 64 * BufferUtil.MB;

    // Note that this argument is also known to the jp2launcher used
    // on Windows Vista; see JP2Launcher.cpp
    private static final String INTERNAL_SEPARATOR_ARG = "---";
    private static final String TRUSTED_SEPARATOR_ARG = "--";

    // Contains the jvm parameter of this running JVM
    private static JVMParameters runningJVMParameters = null;

    // The dependent jars for the new Java Plug-In
    // NOTE: SharedWindow.jar is temporary for the Mac OS X port
    private static final String[] PLUGIN_DEPENDENT_JARS = { "deploy.jar", "javaws.jar", "plugin.jar", "SharedWindow.jar" };

    static class ArgumentSet {
        // All -X options aside from -Xmx, individually tokenized
        private OrderedHashSet/*<String>*/ dashXOptions = new OrderedHashSet/*<String>*/();
        // All system property settings, individually tokenized
        private OrderedHashSet/*<Property>*/ systemProperties = new OrderedHashSet/*<Property>*/();
        // All other command-line arguments
        private OrderedHashSet/*<String>*/ otherArguments = new OrderedHashSet/*<String>*/();

        public void clear() {
            dashXOptions.clear();
            systemProperties.clear();
            otherArguments.clear();
        }

        public static final int NUM_ARGUMENT_LISTS = 3;

        private String[] systemProperties2StringArray() {
            String[] propStrs = new String[systemProperties.size()];
            int i=0;
            for (Iterator iter = systemProperties.iterator(); iter.hasNext(); ) {
                propStrs[i++] = ((Property)iter.next()).toString();
            }
            return propStrs;
        }

        /** For sending down command-line arguments over the wire */
        public void copyToStringArrays(String[][] argArrays,
                                       int startIndex) {
            argArrays[startIndex + 0] = (String[]) dashXOptions.toArray(new String[0]);
            argArrays[startIndex + 1] = systemProperties2StringArray();
            argArrays[startIndex + 2] = (String[]) otherArguments.toArray(new String[0]);
        }

        /** For reading command-line arguments sent down over the wire */
        public void getFromStringArrays(String[][] argArrays,
                                        int startIndex) {
            for (int i = 0; i < argArrays[startIndex + 0].length; i++) {
                dashXOptions.add(argArrays[startIndex + 0][i]);
            }

            for (int i = 0; i < argArrays[startIndex + 1].length; i++) {
                systemProperties.add(new Property(argArrays[startIndex + 1][i]));
            }

            for (int i = 0; i < argArrays[startIndex + 2].length; i++) {
                otherArguments.add(argArrays[startIndex + 2][i]);
            }
        }

        /** Adds all of the arguments from the other ArgumentSet to
            this one */
        public void addAll(ArgumentSet other) {
            dashXOptions.addAll(other.dashXOptions);
            systemProperties.addAll(other.systemProperties);
            otherArguments.addAll(other.otherArguments);
        }

        /** Adds all given properties to this set.*/
        public void addAll(Properties props)
        {
            for (Enumeration ep = props.propertyNames(); ep.hasMoreElements(); ) {
                String key = (String) ep.nextElement();
                String val = props.getProperty(key);
                systemProperties.add(new Property(key,val));
            }
        }

        /** Adds all given properties to this set.*/
        public void addAll(List/*Property*/ propList)
        {
            for (Iterator iter = propList.iterator(); iter.hasNext(); ) {
                Object o = iter.next();
                if(o instanceof Property) {
                    systemProperties.add(o);
                }
            }
        }

        /** Adds all of the arguments from this ArgumentSet to the
            given list 
            @return string length of added arguments
          */
        public int addTo(List/*<String>*/ args, boolean osConform, boolean includeInsecureArgs, 
                         int maxCmdLineLength) {
            Iterator iter;
            String stmp;
            int argsLen=0;
            int ilen;

            // process otherArgs first, since these arguments have high priority 
            // in respect to the maxCmdLineLength. So process them to a intermediate List
            List/*<String>*/ otherArgs = new ArrayList/*<String>*/();
            for (iter = otherArguments.iterator(); argsLen<maxCmdLineLength && iter.hasNext(); ) {
                stmp = (String)iter.next();
                ilen = stmp.length()+1; // incl seperator
                if (includeInsecureArgs || Config.isSecureVmArg(stmp)) {
                    if((argsLen+ilen)<maxCmdLineLength) {
                        otherArgs.add(stmp);
                        argsLen+=ilen;
                    }
                }
            }

            for (iter = dashXOptions.iterator(); argsLen<maxCmdLineLength && iter.hasNext(); ) {
                stmp = (String)iter.next();
                ilen = stmp.length()+1; // incl seperator
                if (includeInsecureArgs || Config.isSecureVmArg(stmp)) {
                    if((argsLen+ilen)<maxCmdLineLength) {
                        args.add(stmp);
                        argsLen+=ilen;
                    }
                }
            }
            for (iter = systemProperties.iterator(); argsLen<maxCmdLineLength && iter.hasNext(); ) {
                Property prop = (Property)iter.next();
                if (includeInsecureArgs || prop.isSecure()) {
                    stmp = prop.toString(osConform);
                    ilen = stmp.length()+1; // incl seperator
                    if((argsLen+ilen)<maxCmdLineLength) {
                        args.add(stmp);
                        argsLen+=ilen;
                    }
                }
            }
            // append otherArg to the end of this list
            args.addAll(otherArgs);

            return argsLen;
        }

        public void addTo(Properties props) {
            for (Iterator iter = systemProperties.iterator(); iter.hasNext(); ) {
                ((Property)iter.next()).addTo(props);
            }
        }

        public boolean addArgument(String arg, boolean checkForJVMArg) {
            boolean res = false;
    	    String argValue = null;

            // We uniformly forbid the specification of
            // -Dsun.plugin2.jvm.args for security reasons; at this point
            // in the code these command line arguments are coming from an
            // untrusted / unknown source. We might consider reusing the
            // system property mechanism on non-Vista platforms, but its
            // use would have to be carefully injected so specification of
            // the value in deployment.properties would not be allowed
            if (arg.startsWith("-Dsun.plugin2.jvm.args")) {
                throw new IllegalArgumentException("May not specify the sun.plugin2.jvm.args system property");
            }

            // Try Property (includes removing quotes in system properties value)
            Property prop = Property.createProperty(arg);

            if(null!=prop) {
                // property key
                arg = prop.getKey();
                argValue = prop.getValue();
            }
        
            if ( (arg != null && containsUnsupportedCharacter(arg)) ||
                 (argValue != null && containsUnsupportedCharacter(argValue)) ) {
                // ignore illegal characters, ", % and any not in [ -~]
            
                if (DEBUG) {
                    System.out.println("JVMParameters: Illegal character in VM argument, ignore: "
					+arg + " value: " + argValue);
                }
                return res;
            }
        
            if (checkForJVMArg && null!=prop) {
                systemProperties.add(prop);
                res = true;
            } else if (checkForJVMArg && arg.startsWith("-X")) {
                if (arg.startsWith("-Xbootclasspath:") || arg.startsWith("-Xbootclasspath/p:")) {
                    // for security reason, we disallow setting and prepending the bootclasspath from 
                    // html java_arguments. Please note the trusted bootclasspath is added to a different
                    // internal argument list
                    return true;
                }
                // Note that -Xmx is parsed at a higher level
                dashXOptions.add(arg);
                res = true;
            } else {
                // FIXME: could be (and may need to be) more clever;
                // for example, comprehending the classpath,
                // bootclasspath, and sun.boot.library.path
                otherArguments.add(arg);
                res = arg.startsWith("-"); // still a JVM argument?
            }
            return res;
        }

        // Returns true if the argument was found and removed, false otherwise
        public boolean removeArgument(String argument) {
            if (argument.startsWith("-X")) {
                return removeArgumentHelper(dashXOptions, argument);
            } else if (argument.startsWith("-D")) {
                return removeArgumentHelper(systemProperties, new Property(argument));
            } else {
                return removeArgumentHelper(otherArguments, argument);
            }
        }

        private boolean removeArgumentHelper(OrderedHashSet/*<Property>*/ props,
                                             Property prop) {
            return props.remove(prop);
        }

        private boolean removeArgumentHelper(OrderedHashSet/*<String>*/ args,
                                             String arg) {
            for (Iterator iter = args.iterator(); iter.hasNext(); ) {
                String str = (String) iter.next();
                if (str.startsWith(arg)) {
                    args.remove(str);
                    return true;
                }
            }
            return false;
        }

        public boolean isDefault() {
            return (dashXOptions.isEmpty() &&
                    systemProperties.isEmpty() &&
                    otherArguments.isEmpty());
        }

        // @return true if all arguments are secure ones,
        //         i.e. don't need the JVM to be started by a signed source
        public boolean isSecure() {
            // Check dashXOptions
            Iterator i = dashXOptions.iterator();
            while (i.hasNext()) {
                if (!Config.isSecureVmArg((String)i.next())) {
                    return false;
                }
            }

            // Check otherArguments
            i = otherArguments.iterator();
            while (i.hasNext()) {
                if (!Config.isSecureVmArg((String)i.next())) {
                    return false;
                }
            }

            // Check system properties
            i = systemProperties.iterator();
            while (i.hasNext()) {
                if ( !Config.isSecureSystemPropertyKey( ((Property)i.next()).getKey() ) ) {
                    return false;
                }
            }

            return true;
        }

        /** Supports addressing all of the entries in this ArgumentSet */
        public int size() {
            return (dashXOptions.size() +
                    systemProperties.size() +
                    otherArguments.size());
        }

        /** Supports addressing all of the entries in this ArgumentSet */
        public String get(int index) {
            if (index < dashXOptions.size()) {
                return (String) dashXOptions.get(index);
            }
            index -= dashXOptions.size();

            if (index < systemProperties.size()) {
                return ((Property)systemProperties.get(index)).toString();
            }
            index -= systemProperties.size();

            return (String) otherArguments.get(index);
        }
    }

    public JVMParameters() {
        clear();
    }

    /** Clears out this set of parameters, returning it to the default
        state. */
    public void clear() {
        internalArguments.clear();
        clearUserArguments();
    }

    /** Clears out only the user-specified arguments (both trusted and
        untrusted), leaving the internal arguments alone. */
    public void clearUserArguments() {
        trustedArguments.clear();
        arguments.clear();
        maxHeap = 0;
        isDefault = true;
    }

    public JVMParameters copy() {
        JVMParameters params = new JVMParameters();
        params.addArguments(this);
        return params;
    }

    /** Copy the parameters to a two dimensional String array.
        This is used to transport via the wire. */
    public String[][] copyToStringArrays() {
        String[][] argArrays = new String[2 * ArgumentSet.NUM_ARGUMENT_LISTS + 2][];
        argArrays[0] = (String[]) internalArguments.toArray(new String[0]);
        trustedArguments.copyToStringArrays(argArrays, 1);
        String[] xmxArray = new String[1];
        xmxArray[0] = getXmx();
        argArrays[1 + ArgumentSet.NUM_ARGUMENT_LISTS] = xmxArray;
        arguments.copyToStringArrays(argArrays, 2 + ArgumentSet.NUM_ARGUMENT_LISTS);
        return argArrays;
    }

    /** Get parameters from a two dimensional String array */
    public void getFromStringArrays(String[][] argArrays) {
        for (int i = 0; i < argArrays[0].length; i++) {
            internalArguments.add(argArrays[0][i]);
        }
        trustedArguments.getFromStringArrays(argArrays, 1);
        String xmx = argArrays[1 + ArgumentSet.NUM_ARGUMENT_LISTS][0];
        if (xmx != null) {
            addArgument(xmx);
        }
        arguments.getFromStringArrays(argArrays, 2 + ArgumentSet.NUM_ARGUMENT_LISTS);
    }

    /** Parses the given command line, adding all options encountered. 
     *  Each argument will be unquoted.
     */
    public void parse(String commandLine) {
        parseImpl(commandLine, false, false);
    }

    /** Parses the given command line, adding only the secure ones
     *  if secureArgsOnly is set to true. 
     *  Each argument will be unquoted.
     */
    public void parse(String commandLine, boolean secureArgsOnly) {
	parseImpl(commandLine, false, secureArgsOnly);
    }

    /** Parses the given command line, treating all options
        encountered as trusted options. In other words, they do not
        affect the result of isSecure(), even though they might
        contain non-secure JVM arguments or system properties. */
    public void parseTrustedOptions(String commandLine) {
        parseImpl(commandLine, true, false);
    }

    private void parseImpl(String commandLine, boolean trusted, boolean secureArgsOnly) {
        if (commandLine == null)
            return;

        if(DEBUG) {
            System.out.println("parseImpl commandline: <"+commandLine+">");
        }
        List/*String*/ cmdlnArgs;
        try {
            cmdlnArgs = StringQuoteUtil.getCommandLineArgs(commandLine);
        } catch (IllegalArgumentException iae) {
            if (DEBUG) {
                iae.printStackTrace();
            } else {
                System.out.println(iae.getMessage());
            }
            return; // bail out
        }

        // Once we get to the first non-special command-line argument,
        // stop the special treatment
        boolean checkForJVMArg = true;

        for(int i=0; i<cmdlnArgs.size(); i++) {
            String tmp = (String)cmdlnArgs.get(i);
            if(DEBUG) {
                System.out.println("parseImpl token: <"+tmp+">");
            }
            checkForJVMArg = addArgumentImpl(tmp, checkForJVMArg, trusted, secureArgsOnly);
        }
    }
  

    /** Adds all given properties to this set.
        Equivalent to: <CODE> for(i in props) addArgument(props[i].toString())</CODE>. */
    public void addProperties(Properties props)
    {
        arguments.addAll(props);
        recomputeIsDefault();
    }

    /** Adds all given properties to this set.
        Equivalent to: <CODE> for(i in props) addArgument(props[i].toString())</CODE>. */
    public void addProperties(List/*Property*/ propList)
    {
        arguments.addAll(propList);
        recomputeIsDefault();
    }

    /** Adds a given command line argument to this set, checking to
        see whether it is a special JVM command-line argument.
        Equivalent to <CODE>addArgument(arg, true)</CODE>. */
    public void addArgument(String arg) {
        addArgument(arg, true);
    }

    /** Adds a given command line argument to this set. The parameter
        "checkForJVMArg" indicates whether to test the argument to see
        whether it is a special JVM command line argument (starting
        with -X, -D, etc.) and to place it in the appropriate place on
        the argument list, or just to put it at the end of the command
        line. 
     *  The argument will be unquoted, if quoted as single entity.
     */
    public void addArgument(String arg, boolean checkForJVMArg) {
        addArgumentImpl(StringQuoteUtil.unquoteIfEnclosedInQuotes(arg), checkForJVMArg, false, false);
    }

    /** Adds an argument to the internal arguments list -- i.e., one
        which is used for the inner workings of the new Java Plug-In. */
    public void addInternalArgument(String arg) {
        internalArguments.add(arg);
    }

    /** Adds all of the parameters from the other JVMParameters into
        this one. They are placed after the existing ones and
        therefore tend to override them. */
    public void addArguments(JVMParameters other) {
        internalArguments.addAll(other.internalArguments);
        trustedArguments.addAll(other.trustedArguments);
        arguments.addAll(other.arguments);
        // FIXME: the propagation of the maximum heap size needs more thought.

	// Use other's max heap only if it is specified in -Xmx
        if (!other.isDefault && other.maxHeap > 0) {
            if (DEBUG) {
                System.out.println("  JVMParameters.addArguments: Propagating max heap size of " + other.getMaxHeapSize() );
            }
            maxHeap = other.maxHeap;
        } else if (isDefault && other.isDefault) {
            // FIXME: this is somewhat of a hack
	    if (maxHeap != 0 || other.maxHeap != 0) {
		// if one has specified -Xmx
		maxHeap = Math.max(getMaxHeapSize(), other.getMaxHeapSize());
	    } else {
		// use default max heap size
		maxHeap = 0;
	    }
            if (DEBUG) {
                System.out.println("  JVMParameters.addArguments: Chose heap size of Math.max(" + getMaxHeapSize() + ", " + other.getMaxHeapSize() + ")");
            }
        }
        isDefault = (isDefault && other.isDefault);
    }

    /** Removes a given command line argument (starting with the
        supplied string) from the set. */
    public void removeArgument(String argument) {
        if (argument.startsWith("-Xmx")) {
            maxHeap = 0;
        } else {
            // Remove the first instance of the argument from both the
            // trusted and normal arguments
            if (!trustedArguments.removeArgument(argument)) {
                arguments.removeArgument(argument);
            }
        }
        recomputeIsDefault();
    }

    /** Parses the value of the sun.boot.class.path system property
        and memorizes the locations of the given jar names, for
        passing down to sub-processes. Equivalent to
        <CODE>parseBootClassPath(System.getProperty("sun.boot.class.path"), targetJarNames);</CODE>. */
    public void parseBootClassPath(String[] targetJarNames) {
        parseBootClassPath(Config.getSystemProperty("sun.boot.class.path"), targetJarNames);
    }

    /** Parses the given sun.boot.class.path and locates the given
        target jar names, saving them for later appending via
        <CODE>-Xbootclasspath/a:</CODE>. */
    public void parseBootClassPath(String bootClassPath, String[] targetJarNames) {
        if (targetJarNames != null && targetJarNames.length > 0) {
            String[] entries = null;
            String appendList = null;

	    // Calling own split method for old JRE 1.3.1
	    try {
	        entries = bootClassPath.split(File.pathSeparator);
	    }
	    catch (NoSuchMethodError nsme) {
	        entries = splitString(bootClassPath, File.pathSeparator);
	    }
	    
            for (int i = 0; i < entries.length; i++) {
                String curEntry = entries[i];
                for (int j = 0; j < targetJarNames.length; j++) {
                    if (curEntry.endsWith(targetJarNames[j])) {
                        if (appendList == null) {
                            appendList = curEntry;
                        } else {
                            appendList = appendList + File.pathSeparator + curEntry;
                        }
                        break;
                    }
                }
            }
            if (appendList != null) {
                addInternalArgument("-Xbootclasspath/a:" + appendList);
            }
        }
    }

    /** Returns the dependent jars of the new Java Plug-In for passing
        in to <CODE>parseBootClassPath</CODE>. */
    public static String[] getPlugInDependentJars() {
        return (String[]) PLUGIN_DEPENDENT_JARS.clone();
    }

    /** Fetches all of the JVM command line arguments this object
        contains as one large list of Strings. Equivalent to
        <CODE>getCommandLineArguments(includeSeparatorArgument, true, osConform, true, -1)<CODE>. 
        Each argument will be quoted if needed.
       */
    public List/*<String>*/ getCommandLineArguments(boolean includeSeparatorArgument, boolean osConform) {
        return getCommandLineArguments(includeSeparatorArgument, true, osConform, true, -1);
    }

    /** Fetches all of the JVM command line arguments this object
        contains as one large list of Strings. A boolean argument of
        "true" for includeSeparatorArgument will cause a separator
        argument ("---") to be added between the internal and the
	trusted arguments and another separator ("--") to be added 
	between the trusted and non-trusted arguments. 
	This is used on the Windows Vista platform as the arguments 
	are passed down through the jp2launcher and from there to 
	PluginMain. The internal arguments may be excluded by passing 
	false for includeInternalArguments.
        Each argument will be quoted if needed.
       */
    public List/*<String>*/ getCommandLineArguments(boolean includeSeparatorArgument,
                                                    boolean includeInternalArguments, 
                                                    boolean osConform,
                                                    boolean includeInsecureArgs, 
                                                    int maxCmdLineLength) {
	int ilen;
        int argsLen = 0;
        if(maxCmdLineLength<0) {
            maxCmdLineLength=Integer.MAX_VALUE;
        }
        List/*<String>*/ args = new ArrayList/*<String>*/();
        if (includeInternalArguments) {
            Iterator iter;
            String stmp;
            for (iter = internalArguments.iterator(); argsLen<maxCmdLineLength && iter.hasNext(); ) {
                stmp = (String)iter.next();
                ilen = stmp.length()+1; // incl seperator
                if((argsLen+ilen)<maxCmdLineLength) {
                    args.add(stmp);
                    argsLen+=ilen;
                }
            }
        }
	if (includeSeparatorArgument) {
	    args.add(INTERNAL_SEPARATOR_ARG);
	}
        if(argsLen<maxCmdLineLength) {
            argsLen += trustedArguments.addTo(args, osConform, includeInsecureArgs, maxCmdLineLength-argsLen);
        }
        if (includeSeparatorArgument) {
            args.add(TRUSTED_SEPARATOR_ARG);
        }
        String xmx = getXmx();
        if (xmx != null) {
            ilen = xmx.length()+1; // incl seperator
            if((argsLen+ilen)<maxCmdLineLength) {
                args.add(xmx);
                if (DEBUG) {
                    System.out.println("  JVMParameters.getCommandLineArguments: added " + xmx);
                }
                argsLen+=ilen;
            }
        }
        if(argsLen<maxCmdLineLength) {
            argsLen += arguments.addTo(args, osConform, includeInsecureArgs, maxCmdLineLength-argsLen);
        }

        if(argsLen>=maxCmdLineLength) {
            // Ooops, the above algorythm failed.
            Exception e = new Exception("Internal Error: JVMParameters.getCommandLineArguments: string size: " + 
                                    argsLen + " >= "+maxCmdLineLength);
            e.printStackTrace();
        } else {
            if(DEBUG) {
                System.out.println("JVMParameters.getCommandLineArguments: string size: " + argsLen + " < "+maxCmdLineLength);
            }
        }

        return args;
    }

    /** Fetches all of the JVM command-line arguments this object
        contains as one long string. The internal arguments may be
        excluded by passing false for the includeInternalArguments
        flag.
        <CODE>getCommandLineArguments(false, includeInternalArguments, false, true, -1)<CODE>. 
      *
      * Each single argument will be quoted, if necessary
      */
    public String getCommandLineArgumentsAsString(boolean includeInternalArguments) {
        return StringQuoteUtil.getStringByCommandList(
            getCommandLineArguments(false, includeInternalArguments, false, true, -1) );
    }

    public void addTo(Properties props) {
        trustedArguments.addTo(props);
        arguments.addTo(props);
    }

    public Properties getProperties()
    {
        Properties props = new Properties();
        addTo(props);
        return props;
    }

    /** Sets whether this JVMParameters instance should consider
        itself to contain the default arguments for a given platform
        version. When we parse the JVM command line arguments out of
        deployment.properties, we want applets that don't specify any
        command-line arguments to consider themselves satisfied by
        running JVM instances with those command line arguments. */
    public void setDefault(boolean isDefault) {
        this.isDefault = isDefault;
    }

    /** Indicates whether this JVMParameters object is in the default
        state. Adding additional command line parameters automatically
        invalidates this. */
    public boolean isDefault() {
        return isDefault;
    }

    /** Indicates whether this set of JVM parameters satisfies the
        requirements of the other set. Currently this indicates
        whether the requested maximum heap size is greater than or
        equal to the other one, and whether key JVM command-line
        arguments and system properties are the same across the two
        parameter sets. */
    public boolean satisfies(JVMParameters other) {
        if (other == null) {
            return false;
        }

        if (getMaxHeapSize() < other.getMaxHeapSize() ) {
            if (DEBUG) {
                System.out.println("  JVMParameters.satisfies() returning false because maxHeap " +
                                   getMaxHeapSize() + " < other.maxHeap" + other.getMaxHeapSize() );
            }
            return false;
        }

        if (isDefault && other.isDefault) {
            if (DEBUG) {
                System.out.println("  JVMParameters.satisfies() returning true because isDefault && other.isDefault");
            }
            return true;
        }

        if (isSecure() != other.isSecure()) {
            return false;
        }

        // Assume that JVM instances with non-default sets of
        // command-line arguments shouldn't match those which expect
        // the defaults
        if (!isDefault && other.isDefault) {
            if (DEBUG) {
                System.out.println("  JVMParameters.satisfies() returning false because !isDefault && other.isDefault");
            }
            return false;
        }

        if (DEBUG) {
            System.out.println("  JVMParameters.satisfies() considering subset of options");
        }

        // For now let's say at this point that we can reuse a JVM
        // instance if the requested -X options and system property
        // specifications are a subset of ours; in other words, if we
        // contain all of the arguments in the other one
        int otherSz = other.size();
        for (int i = 0; i < otherSz; i++) {
            String arg = other.get(i);
            if (!isExcluded(arg) && !contains(arg)) {
                if (DEBUG) {
                    System.out.println("  JVMParameters.satisfies() returning false because we don't contain " + other.get(i));
                }
                return false;
            }
        }

        if (DEBUG) {
            System.out.println("  JVMParameters.satisfies() returning true");
        }
        return true;
    }

    /** Indicates whether this set of JVM parameters satisfies the
        requirements of the secure subset of the other set.  */
    public boolean satisfiesSecure(JVMParameters other) {
        if (other == null) {
            return false;
        }

        if (getMaxHeapSize() < other.getMaxHeapSize() ) {
            return false;
        }

        if (isDefault && other.isDefault) {
            return true;
        }

        // Assume that JVM instances with non-default sets of
        // command-line arguments shouldn't match those which expect
        // the defaults
        if (!isDefault && other.isDefault) {
            return false;
        }

        // For now let's say at this point that we can reuse a JVM
        // instance if the requested -X options and system property
        // specifications are a subset of ours; in other words, if we
        // contain all of the arguments in the other one
        int otherSz = other.size();
        for (int i = 0; i < otherSz; i++) {
            String arg = other.get(i);
            if (!isExcluded(arg) && isSecureArgument(arg) && !contains(arg)) {
                return false;
            }
        }

        return true;
    }

    private static boolean isSecureArgument(String arg) {

        Property prop = Property.createProperty(arg);
        if (prop != null) {
            return prop.isSecure();
        }
        return Config.isSecureVmArg(arg);
    }
                
    public static boolean isJVMCommandLineArgument(String arg) {
        if (arg == null) return false;
        arg = StringQuoteUtil.unquoteIfEnclosedInQuotes(arg);
        return (arg.charAt(0) == '-');
    }

    // return the size of max heap in bytes
    public long getMaxHeapSize() { return maxHeap>0? maxHeap : DEFAULT_HEAP; }

    public void setMaxHeapSize(long v) { maxHeap=(v>0)?v:0; }

    public static final long getDefaultHeapSize() { return DEFAULT_HEAP; }

    public static JVMParameters getRunningJVMParameters() {
        return runningJVMParameters;
    }

    /* set the JVMParameters for the running JVM based upon the given JVMParameters.
     * <UL>
     * <LI> a new instance is created
     * <LI> add all set secure system properties
     * <LI> add the given JVMParameters
     * </UL>
     */
    public static void setRunningJVMParameters(JVMParameters p) {
        runningJVMParameters = new JVMParameters();

        Properties sysProps = new Properties();
        // or shall we use addAllSystemPropertiesTo, which is huge ?
        //      Config.addAllSystemPropertiesTo(sysProps); 
        Config.addSecureSystemPropertiesTo(sysProps); 
        runningJVMParameters.addProperties(sysProps);

        runningJVMParameters.addArguments(p);

        if(DEBUG) {
            System.out.println("JVMParameters: setRunningJVMParameters: "+runningJVMParameters);
        }
    }

    public String toString() {
        StringBuffer buf = new StringBuffer("[JVMParameters: isSecure: "+isSecure()+", args: ");
        buf.append(getCommandLineArgumentsAsString(true));
        buf.append("]");
        return buf.toString();
    }

    //----------------------------------------------------------------------
    // Reconstitution of arguments on the Windows Vista platform
    //

    /** Parses the jvm args from system property, e.g. 'sun.plugin2.jvm.args', 
        if specified, into a newly-created JVMParameters object. 
        Returns null if the system property was not specified, e.g, on non-Vista
        platforms. 

        We filter out the first -Xbootclasspath/a as this is
        something we know we specified.
        Arguments up to the separator argument are trusted,
        those specified via deployment.properties.
       */
    public static JVMParameters parseFromSystemProperty(String propertykey) {
        JVMParameters res = null;
        String value = System.getProperty(propertykey);
        if(DEBUG) {
            System.out.println("JVMParameters.parseFromSystemProperty: <"+value+">");
        }
        if(null!=value) {
            res = new JVMParameters();
            res.parseFromSystemPropertyImpl(value);
        }
        return res;
    }

    private void parseFromSystemPropertyImpl(String commandLine) {
        if (commandLine == null) {
            return;
        }
        // We're running on Windows Vista and our custom launcher
        // passed these arguments up to us -- this is the only
        // secure communications channel we are guaranteed for the
        // purposes of communicating this information
        List/*String*/ args;
        try {
            args = StringQuoteUtil.getCommandLineArgs(commandLine);
        } catch (IllegalArgumentException iae) {
            if (DEBUG) {
                iae.printStackTrace();
            } else {
                System.out.println(iae.getMessage());
            }
            return; // bail out
        }


	// The arguments up to the internal separator argument are internal
	boolean internal = true;

        // The arguments up to the trusted separator argument are trusted
        // (those specified via deployment.properties)
        boolean trusted = true;
        for (Iterator iter = args.iterator(); iter.hasNext(); ) {
            String arg = (String) iter.next();
	    // the list of arguments consist of internal,
	    // followed by trusted and then followed by other
	    // we don't need to add internal ones
	    if (arg.equals(INTERNAL_SEPARATOR_ARG)) {
		internal = false;
	    } else if (arg.equals(TRUSTED_SEPARATOR_ARG)) {
		trusted = false;
	    } else {
		if (!internal) {
                    addArgumentImpl(arg, true, trusted, false);
		}
	    }
        }
    }

    /** Indicates whether this JVMParameters object contains the given
        command-line argument */
    public boolean contains(String argument) {
        int sz = size();
        for (int i = 0; i < sz; i++) {
            if (argument.equals(get(i))) {
                return true;
            }
        }
        return false;
    }

    /** Indicates if this is a property that is excluded from the check **/
    public boolean isExcluded(String argument) {
        if (argument.startsWith("-Djnlp.") || argument.startsWith("-Djavaws.")) {
	    // Don't need to do double launcher for Java Webstart application
	    // with these secure properties, they will be set in AppPolicy class
	    // before any application code is called 
	    if (Environment.isJavaWebStart()) {
		return true;
            }

            // exclude jnlp.packEnabled and jnlp.versionEnabled
            // from the check
            if (argument.startsWith("-Djnlp.packEnabled") ||
                    (argument.startsWith("-Djnlp.versionEnabled") ||
                    (argument.startsWith("-Djnlp.concurrentDownloads")))) {
                return true;
            }
        }
        return false;
    }

    /** Indicates whether this JVMParameters object contains a argument that 
        starts with the given argument */
    public boolean containsPrefix(String argument) {
        int sz = size();
        for (int i = 0; i < sz; i++) {
            if (get(i).startsWith(argument)) {
                return true;
            }
        }
        return false;
    }

    /** Check whether this JVMParameters is secure. 
        Does not check the internalArguments or trusted arguments. 

        @return true if the untrusted arguments are secure ones,
                i.e. don't need the JVM to be started by a signed source
      */
    public boolean isSecure() {
        return arguments.isSecure();
    }

    //----------------------------------------------------------------------
    // Utility to parse memory specification
    //

    // Helper to parse memory size specifications ("256m", which is
    // the tail of "-Xmx256m")
    public static long parseMemorySpec(String spec) throws IllegalArgumentException {
        char sizeSpecifier = (char) 0;
        String origSpec = spec;
        for (int i = 0; i < spec.length(); i++) {
            if (!Character.isDigit(spec.charAt(i))) {
                if (i != spec.length() - 1) {
                    throw new IllegalArgumentException("Too many characters after heap size specifier");
                } else {
                    sizeSpecifier = spec.charAt(i);
                    spec = spec.substring(0, spec.length() - 1);
                    break;
                }
            }
        }
        try {
            long val = Long.parseLong(spec);
            if (sizeSpecifier != (char) 0) {
                switch (sizeSpecifier) {
                    case 'T': case 't':
                        val = val * BufferUtil.TB;
                        break;
                    case 'G': case 'g':
                        val = val * BufferUtil.GB;
                        break;
                    case 'M': case 'm':
                        val = val * BufferUtil.MB;
                        break;
                    case 'K': case 'k':
                        val = val * BufferUtil.KB;
                        break;
                    default:
                        throw new IllegalArgumentException("Illegal heap size specifier " + sizeSpecifier + " in " + origSpec);
                }
            }
            return val;
        } catch (NumberFormatException e) {
            throw (IllegalArgumentException)
                new IllegalArgumentException().initCause(e);
        }
    }

    public static String unparseMemorySpec(long val) {
        // FIXME: seeing division by zero errors with this test
        // if (val % BufferUtil.TB == 0) {
        //     return "" + (val / BufferUtil.TB) + "t";
        // }

        if (val % BufferUtil.GB == 0) {
            return "" + (val / BufferUtil.GB) + "g";
        }

        if (val % BufferUtil.MB == 0) {
            return "" + (val / BufferUtil.MB) + "m";
        }

        if (val % BufferUtil.KB == 0) {
            return "" + (val / BufferUtil.KB) + "k";
        }

        return "" + val;
    }
    //----------------------------------------------------------------------
    // Internals only below this point
    //

    // Helper methods for quoting command line arguments

    private static String tail(String str, String tail) {
        return str.substring(tail.length());
    }

    private boolean addArgumentImpl(String arg, boolean checkForJVMArg, boolean trusted, boolean secureArgsOnly) {
        boolean res;
        if (checkForJVMArg && arg.startsWith("-Xmx")) {
            try {
                maxHeap = parseMemorySpec(tail(arg, "-Xmx"));
            } catch (IllegalArgumentException e) {
                // This can happen because of illegal user-defined
                // command-line arguments in the HTML and should not
                // result in a failure to launch the JVM
                if (DEBUG) {
                    e.printStackTrace();
                }
            }
            res = true;
        } else {
            if (trusted) {
                res = trustedArguments.addArgument(arg, checkForJVMArg);
            } else {
		if (secureArgsOnly) {
		    if (Config.isSecureVmArg(arg) || Config.isSecureSystemProperty(arg)){
			res = arguments.addArgument(arg, checkForJVMArg);
		    } else {
			// don't add insecure arg
			res = true;
		    }
		} else {
		    res = arguments.addArgument(arg, checkForJVMArg);
		}
            }
        }
        recomputeIsDefault();
        return res;
    }

    // Gets the "-Xmx" specifier we should use, or null if the default
    // heap size is in use
    private String getXmx() {
        if (getMaxHeapSize() != DEFAULT_HEAP) {
            return "-Xmx" + unparseMemorySpec(maxHeap);
        }
        return null;
    }

    // Recomputes the "isDefault" property based on the contents of
    // this JVMParameters object
    private void recomputeIsDefault() {
        isDefault = (getMaxHeapSize() == DEFAULT_HEAP &&
                     trustedArguments.isDefault() &&
                     arguments.isDefault());
    }

    /** Supports addressing all of the entries in this JVMParameters */
    private int size() {
        return trustedArguments.size() + arguments.size();
    }

    /** Supports addressing all of the entries in this JVMParameters */
    private String get(int index) {
        if (index < trustedArguments.size()) {
            return trustedArguments.get(index);
        }
        index -= trustedArguments.size();
        return arguments.get(index);
    }

    /* Copied from com.sun.javaws.jnl.XMLLFormat.java
     *
     * Disallow ", % and any character not in [ -~]
     * returns true if string contains unsupported character
     * returns false otherwise
     */
    private static boolean containsUnsupportedCharacter(String s) {
        for (int i = 0; i < s.length(); i++) {
            char a = s.charAt(i);
            if (a < ' ' || a > '~' || a == '%' || a == '"') {
                return true;
            }
         }
        return false;
    }

    /* This method is used by old JRE 1.3.1, which don't have String.split() */
    private static String[] splitString(String s, String delimiter) {
   	StringTokenizer tokenizer = new StringTokenizer(s, delimiter);
    	String[] result = new String[tokenizer.countTokens()];

	for (int i=0; i < result.length; i++) {
      	    result[i] = tokenizer.nextToken();
    	}
    	return result;
    }

    public static void main(String[] args) {
        if( ! StringQuoteUtil.test() ) {
            System.exit(1);
        }
        System.exit(test(args)?0:2);
    }

    /** Small test harness. */
    protected static boolean test(String[] args) {
        boolean ok = true;
        String stmp;
        JVMParameters test = new JVMParameters();
        test.parseBootClassPath(System.getProperty("sun.boot.class.path"), new String[] { "deploy.jar" });
        List/*<String>*/ subordinateArgs = test.getCommandLineArguments(false, false);
        System.out.println("Subordinate JVM Arguments:");
        for (Iterator iter = subordinateArgs.iterator(); iter.hasNext(); ) {
            System.out.println("  " + (String) iter.next());
        }
        String jvmArgProperty0 = "-DwinPath="+StringQuoteUtil._testPropVal[2];
        String jvmArgProperty0Quoted;
        if(Property.getQuotesWholePropertySpec()) {
            jvmArgProperty0Quoted=StringQuoteUtil.quoteIfNeeded(jvmArgProperty0);
        } else {
            jvmArgProperty0Quoted="-DwinPath="+StringQuoteUtil.quoteIfNeeded(StringQuoteUtil._testPropVal[2]);
        }
        Property prop = new Property("winPath", StringQuoteUtil._testPropVal[2]);
        stmp = prop.toString();
        if(jvmArgProperty0Quoted.equals(stmp)) {
            System.out.println("Test 0.0 passed");
        } else {
            System.out.println("Test 0.0 FAILED");
            ok = false;
            System.out.println("\t orig     : <winPath>,<"+StringQuoteUtil._testPropVal[2]+">");
            System.out.println("\t expected : <"+jvmArgProperty0Quoted+">");
            System.out.println("\t result   : <"+stmp+">");
        }

        String jvmArgProperty = "-Djnlpx.vmargs="+StringQuoteUtil._testJvmArgs; // 1
        String jvmArgPropertyQuoted;
        if(Property.getQuotesWholePropertySpec()) {
            jvmArgPropertyQuoted=StringQuoteUtil.quoteIfNeeded(jvmArgProperty);
        } else {
            jvmArgPropertyQuoted="-Djnlpx.vmargs="+StringQuoteUtil.quoteIfNeeded(StringQuoteUtil._testJvmArgs);
        }
        prop = new Property("jnlpx.vmargs", StringQuoteUtil._testJvmArgs);
        stmp = prop.toString();
        if(jvmArgPropertyQuoted.equals(stmp)) {
            System.out.println("Test 0.1 passed");
        } else {
            System.out.println("Test 0.1 FAILED");
            ok = false;
            System.out.println("\t orig     : <jnlpx.vmargs>,<"+StringQuoteUtil._testJvmArgs+">");
            System.out.println("\t expected : <"+jvmArgPropertyQuoted+">");
            System.out.println("\t parsed   : <"+stmp+">");
        }
        prop = Property.createProperty(jvmArgPropertyQuoted);
        if(prop!=null && 
           prop.getKey().equals("jnlpx.vmargs") &&
           prop.getValue().equals(StringQuoteUtil._testJvmArgs))
        {
            System.out.println("Test 0.2 passed");
        } else {
            System.out.println("Test 0.2 FAILED");
            ok = false;
            System.out.println("\t orig           : <"+jvmArgProperty+">");
            System.out.println("\t expected       : <"+StringQuoteUtil._testJvmArgs+">");
            System.out.println("\t parsed         : <"+prop.toString()+">");
        }

        jvmArgPropertyQuoted = "-Djnlpx.vmargs="+StringQuoteUtil._testJvmArgsUnixQuoted; // 1
        prop = Property.createProperty(jvmArgPropertyQuoted);
        if(prop!=null && 
           prop.getKey().equals("jnlpx.vmargs") &&
           prop.getValue().equals(StringQuoteUtil._testJvmArgsUnix))
        {
            System.out.println("Test 0.3 passed");
        } else {
            System.out.println("Test 0.3 FAILED");
            ok = false;
            System.out.println("\t orig           : <"+jvmArgProperty+">");
            System.out.println("\t expected       : <"+StringQuoteUtil._testJvmArgsUnix+">");
            System.out.println("\t parsed         : <"+prop.toString()+">");
        }

        JVMParameters p1 = new JVMParameters();
        JVMParameters p2 = new JVMParameters();

        p2.clear();
        jvmArgProperty = "-Djnlpx.vmargs="+StringQuoteUtil._testJvmArgs; // 1
        String jvmArgs = StringQuoteUtil.quote(jvmArgProperty) + " " + StringQuoteUtil._testJvmArgs; // 12 + 1
        p2.parse(jvmArgs);
        Properties props = p2.getProperties();
        stmp = props.getProperty("jnlpx.vmargs");
        if(StringQuoteUtil._testJvmArgs.equals(stmp)) {
            System.out.println("Test 0.4 passed");
        } else {
            System.out.println("Test 0.4 FAILED");
            ok = false;
            System.out.println("\t jvmParams      : <"+p2+">");
            System.out.println("\t props          : <"+props+">");
            System.out.println("\t expected       : <"+StringQuoteUtil._testJvmArgs+">");
            System.out.println("\t parsed         : <"+stmp+">");
        }

        p2.clear();
        jvmArgProperty = "-Djnlpx.vmargs="+StringQuoteUtil._testJvmArgsUnixQuoted; // 1
        jvmArgs = jvmArgProperty + " " + StringQuoteUtil._testJvmArgsUnix; // 12 + 1
        p2.parse(jvmArgs);
        props = p2.getProperties();
        stmp = props.getProperty("jnlpx.vmargs");
        if(StringQuoteUtil._testJvmArgsUnix.equals(stmp)) {
            System.out.println("Test 0.5 passed");
        } else {
            System.out.println("Test 0.5 FAILED");
            ok = false;
            System.out.println("\t jvmParams      : <"+p2+">");
            System.out.println("\t props          : <"+props+">");
            System.out.println("\t expected       : <"+StringQuoteUtil._testJvmArgsUnix+">");
            System.out.println("\t parsed         : <"+stmp+">");
        }
        System.out.println("Test 0.5 result: "+p2);

        p2.clear();
        p2.parse(StringQuoteUtil.unquoteIfEnclosedInQuotes(StringQuoteUtil._testJvmArgsUnixQuoted));
        System.out.println("Test 0.6 result: "+p2);

        // Test satisfaction of one set of parameters by another
        p1.clear();
        p2.clear();
        p1.parse("-Xmx128m");
        p2.parse("-Xmx256m");
        p1.setDefault(true);
        p2.setDefault(true);
        if (p2.satisfies(p1)) {
            System.out.println("Test 1 passed");
        } else {
            System.out.println("TEST 1 FAILED");
            ok = false;
        }
        if (p1.satisfies(p2)) {
            System.out.println("TEST 2 FAILED");
            ok = false;
        } else {
            System.out.println("Test 2 passed");
        }
        p1.addArgument("-Dsun.java2d.opengl=true");
        if (p2.satisfies(p1)) {
            System.out.println("TEST 3 FAILED");
            ok = false;
        } else {
            System.out.println("Test 3 passed");
        }
        p1.removeArgument("-Dsun.java2d.opengl=true");
        p2.addArgument("-Dsun.java2d.noddraw=true");
        // Two custom JVM argument sets should obey the postfix argument list matching rules
        if (p2.satisfies(p1)) {
            System.out.println("Test 4 passed");
        } else {
            System.out.println("TEST 4 FAILED");
            ok = false;
        }
        p1.removeArgument("-Xmx");
        if (p1.isDefault()) {
            System.out.println("Test 5 passed");
        } else {
            System.out.println("TEST 5 FAILED");
            ok = false;
        }
        if (p2.satisfies(p1)) {
            System.out.println("TEST 6 FAILED");
            ok = false;
        } else {
            System.out.println("Test 6 passed");
        }
        p2.setDefault(true);
        if (p2.satisfies(p1)) {
            System.out.println("Test 7 passed");
        } else {
            System.out.println("TEST 7 FAILED !(p2>=p1)");
            System.out.println("p2: "+p2);
            System.out.println("p1: "+p1);
            ok = false;
        }
        p2.addArgument("-Djava.security.policy=c:/insecurepolicy");
        if (p2.isSecure()) {
            System.out.println("Test 8 FAILED");
            ok = false;
        } else {
            System.out.println("Test 8 passed");
        }
        if (p2.satisfies(p1)) {
            System.out.println("Test 9 FAILED");
            ok = false;
        } else {
            System.out.println("Test 9 passed");
        }
        p2.addArgument("-Djava.security.manager=null");
        p1.addArgument("-Djava.security.manager=null");
        if (p2.satisfies(p1)) {
            System.out.println("Test 10 passed");
        } else {
            System.out.println("Test 10 FAILED");
            ok = false;
        }
        p2.removeArgument("-Djava.security.manager=null");
        p2.removeArgument("-Djava.security.policy=c:/insecurepolicy");
        if(p2.isSecure()) {
            System.out.println("Test 11 passed");
        } else {
            System.out.println("Test 11 FAILED");
            ok = false;
        }

        p1.clear();
        p1.parse("-Dwhy=????");
        p1.parse("-Dabcde=\"1111 and one\"");
        p1.parse(StringQuoteUtil.quote(jvmArgProperty0));
        p1.parse("-Djust.a.key");
        p1.parse("-Dababa=2222");
        p1.parse("-Djust.a.key.2");
        p1.parse("-Dlala=33333");
        p1.parse("-Djust.a.key");
        p1.parse("-Dsoso=55555");
        p1.parse("-Djust.a.key.2=6666");
        p1.parse("-Dwhy=\"LAST ELEMENT\"");
        JVMParameters.setRunningJVMParameters(p1);
        System.out.println("Running JVMParams: "+p1+"\n\t-> "+JVMParameters.getRunningJVMParameters());
        p2.clear();
        props = new Properties();
        props.setProperty("abcde", "1111 and one");
        props.setProperty("winPath", StringQuoteUtil._testPropVal[2]);
        p2.addProperties(props);
        p2.parse("-Dababa=2222 -Dlala=33333 -Djust.a.key -Dsoso=55555 -Djust.a.key.2=6666 -Dwhy=\"LAST ELEMENT\"");
        if (JVMParameters.getRunningJVMParameters().satisfies(p2)) {
            System.out.println("Test 12.1 passed");
            System.out.println("\t running JVMParam: "+JVMParameters.getRunningJVMParameters());
            System.out.println("\t does contain: "+p2);
        } else {
            System.out.println("Test 12.1 FAILED");
            ok = false;
            System.out.println("\t running JVMParam: "+JVMParameters.getRunningJVMParameters());
            System.out.println("\t does not contain: "+p2);
        }
        p2.clear();
        p2.parse("-Dabcde=1111 -Dababa=2222 -Dlala=33333 -Djust.a.key -Dsoso=55555 -Djust.a.key.2");
        if (!JVMParameters.getRunningJVMParameters().satisfies(p2)) {
            System.out.println("Test 12.2 passed");
        } else {
            System.out.println("Test 12.2 FAILED");
            ok = false;
            System.out.println("\t running JVMParam: "+JVMParameters.getRunningJVMParameters());
            System.out.println("\t     does contain: "+p2);
        }
        return ok;
    }
}
