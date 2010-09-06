/*
 * @(#)JConsoleResources.java	1.34 04/06/28
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 * NOTE: Anyone changing this file should make the equivalent
 *       change to the regression test:
 *           test/sun/tools/jconsole/ResourceCheckTest.java
 */

package sun.tools.jconsole.resources;

/**
 * <p> This class represents the <code>ResourceBundle</code>
 * for the following package(s):
 *
 * <ol>
 * <li> sun.tools.jconsole
 * </ol>
 *
 * @version @(#)JConsoleResources.java	1.34 04/06/28
 */
public class JConsoleResources extends java.util.ListResourceBundle {

    private static final String cr = System.getProperty("line.separator");

    /**
     *
     */
    private static final Object[][] contents = {
        // NOTE 1: The value strings in this file containing "{0}" are
        //         processed by the java.text.MessageFormat class.  Any
        //         single quotes appearing in these strings need to be
        //         doubled up.
        //
        // NOTE 2: To make working with this file a bit easier, please
        //         maintain these messages in ASCII sorted order by
        //         message key.
        //
        // LOCALIZE THIS
        {" 1 day"," 1 day"},
        {" 1 hour"," 1 hour"},
        {" 1 min"," 1 min"},
        {" 1 month"," 1 month"},
        {" 1 year"," 1 year"},
        {" 2 hours"," 2 hours"},
        {" 3 hours"," 3 hours"},
        {" 3 months"," 3 months"},
        {" 5 min"," 5 min"},
        {" 6 hours"," 6 hours"},
        {" 6 months"," 6 months"},
        {" 7 days"," 7 days"},
        {" kbytes"," kbytes"},
        {" version "," version "},
        {"10 min","10 min"},
        {"12 hours","12 hours"},
        {"30 min","30 min"},
        {"<","<"},
        {"<<","<<"},
        {">",">"},
	{"Advanced", "Advanced"},
        {"All","All"},
        {"Apply","Apply"},
        {"Architecture","Architecture"},
        {"Array, OpenType", "Array, OpenType"},
        {"Array, OpenType, Numeric value viewer","Array, OpenType, Numeric value viewer"},
        {"Attributes","Attributes"},
        {"BlockedCount WaitedCount",
             "Total blocked: {0}  Total waited: {1}" + cr},
        {"Boot class path","Boot class path"},
        {"Cancel","Cancel"},
        {"Cascade","Cascade"},
        {"Chart", "Chart"},
        {"Chart:", "Chart:"},
        {"Class path","Class path"},
	{"Class","Class"},
        {"Classes","Classes"},
        {"Clear","Clear"},
	{"Column.Class and Arguments", "Class and Arguments"},
	{"Column.PID", "PID"},
        {"Committed memory","Committed memory"},
        {"Committed virtual memory","Committed virtual memory"},
        {"Committed", "Committed"},
        {"Compiler","Compiler"},
        {"Composite Navigation", "Composite Navigation"},
        {"CompositeData","CompositeData"},
        {"Config","Config"},
        {"Connect","Connect"},
        {"Connect...","Connect..."},
        {"Connection failed","Connection failed"},
        {"Connection","Connection"},
        {"ConnectionName (disconnected)","{0}@{1} (disconnected)"},
        {"ConnectionName","{0}@{1}"},
        {"Current classes loaded", "Current classes loaded"},
        {"Current heap size","Current heap size"},
        {"Current value","Current value: {0}"},
	{"Create", "Create"},
        {"Daemon threads","Daemon threads"},
        {"Double click to expand/collapse","Double click to expand/collapse"},
        {"Double click to visualize", "Double click to visualize"},
        {"Description: ", "Description: "},
        {"Details", "Details"},
        {"Dimension is not supported:","Dimension is not supported:"},
        {"Discard chart", "Discard chart"},
        {"DurationDaysHoursMinutes","{0,choice,1#{0,number,integer} day |1.0<{0,number,integer} days }" +
                                    "{1,choice,0<{1,number,integer} hours |1#{1,number,integer} hour |1<{1,number,integer} hours }" +
                                    "{2,choice,0<{2,number,integer} minutes|1#{2,number,integer} minute|1.0<{2,number,integer} minutes}"},

        {"DurationHoursMinutes","{0,choice,1#{0,number,integer} hour |1<{0,number,integer} hours }" +
                                "{1,choice,0<{1,number,integer} minutes|1#{1,number,integer} minute|1.0<{1,number,integer} minutes}"},

        {"DurationMinutes","{0,choice,1#{0,number,integer} minute|1.0<{0,number,integer} minutes}"},
        {"DurationSeconds","{0} seconds"},
        {"Empty array", "Empty array"},
        {"Empty opentype viewer", "Empty opentype viewer"},
        {"Error","Error"},
	{"Error: MBeans already exist","Error: MBeans already exist"},
	{"Error: MBeans do not exist","Error: MBeans do not exist"},
        {"Error:","Error:"},
        {"Event","Event"},
        {"Exit","Exit"},
        {"Failed loading L&F: ","Failed loading L&F: {0}"},
        {"Filter: ","Filter: "},
        {"Free physical memory","Free physical memory"},
        {"Free swap space","Free swap space"},
        {"Garbage collector","Garbage collector"},
        {"GTK","GTK"},
        {"GcInfo","Name = ''{0}'', Collections = {1,choice,-1#Unavailable|0#{1,number,integer}}, Total time spent = {2}"},
        {"GC time","GC time"},
        {"GC time details","{0} seconds on {1} ({2} collections)"},
        {"Heap Memory Usage","Heap Memory Usage"},
        {"Heap", "Heap"},
        {"Host or IP: ","Host or IP: "},
        {"Info","Info"},
	{"Invalid URL", "Invalid URL"},
        {"J2SE 5.0 Monitoring & Management Console", "J2SE 5.0 Monitoring & Management Console"},
        {"JConsole version","JConsole version \"{0}\""},
        {"JConsole: Connect to Agent","JConsole: Connect to Agent"},
        {"JIT compiler","JIT compiler"},
	{"JMX URL: ", "JMX URL: "},
        {"Java Virtual Machine","Java Virtual Machine"},
        {"Java","Java"},
        {"Library path","Library path"},
        {"Listeners","Listeners"},
	{"Live Threads","Live Threads"},
	{"Loaded", "Loaded"},
	{"Local", "Local"},
        {"Look and Feel","Look and Feel"},
        {"MBean Java Class","MBean Java Class"},
        {"MBean Name","MBean Name"},
	{"MBean Notification","MBean Notification"},
        {"MBeans","MBeans"},
        {"Manage Hotspot MBeans in: ", "Manage Hotspot MBeans in: "},
        {"Max","Max"},
        {"Maximum heap size","Maximum heap size"},
        {"Memory","Memory"},
        {"MemoryPoolLabel", "Memory Pool \"{0}\""},
        {"Message","Message"},
        {"Method successfully invoked", "Method successfully invoked"},
        {"Minimize All","Minimize All"},
        {"Minus Version", "This is {0} version {1}"},
        {"Monitoring Self","{0} (Monitoring Self)"},
        {"Motif","Motif"},
        {"Name Build and Mode","{0} (build {1}, {2})"},
        {"Name and Build","{0} (build {1})"},
        {"Name","Name"},
        {"Name: ","Name: "},
        {"Name State",
             "Name: {0}" + cr +
             "State: {1}" + cr},
        {"Name State LockName",
             "Name: {0}" + cr +
             "State: {1} on {2}" + cr},
        {"Name State LockName LockOwner",
             "Name: {0}" + cr +
             "State: {1} on {2} owned by: {3}" + cr},
        {"New Connection...","New Connection..."},
        {"New value applied","New value applied"},
        {"No attribute selected","No attribute selected"},
        {"No value selected","No value selected"},
        {"Non-Heap Memory Usage","Non-Heap Memory Usage"},
        {"Non-Heap", "Non-Heap"},
        {"Not Yet Implemented","Not Yet Implemented"},
        {"Not a valid event broadcaster", "Not a valid event broadcaster"},
        {"Notifications","Notifications"},
	{"Number of Threads","Number of Threads"},
        {"Number of Loaded Classes","Number of Loaded Classes"},
        {"Number of processors","Number of processors"},
        {"Objects pending for finalization","Objects pending for finalization"},
        {"Operating System","Operating System"},
        {"Operation return value", "Operation return value"},
        {"Operations","Operations"},
        {"Password: ", "Password: "},
        {"Peak","Peak"},
        {"Perform GC","Perform GC"},
        {"Port: ","Port: "},
        {"Problem adding listener","Problem adding listener"},
        {"Problem displaying MBean", "Problem displaying MBean"},
        {"Problem dropping object","Problem adding listener"},
        {"Problem invoking", "Problem invoking"},
        {"Problem setting attribute","Problem setting attribute"},
        {"Process CPU time","Process CPU time"},
        {"R/W","R/W"},
        {"Received","Received"},
        {"Refresh","Refresh"},
	{"Remote","Remote"},
        {"Remove","Remove"},
        {"Restore All","Restore All"},
        {"Return value", "Return value"},
	{"SeqNum","SeqNum"},
        {"Size Bytes", "{0,number,integer} bytes"},
        {"Size Gb","{0} Gb"},
        {"Size Kb","{0} Kb"},
        {"Size Mb","{0} Mb"},
        {"Source","Source"},
        {"Stack trace",
             cr + "Stack trace: " + cr},
        {"Subscribe","Subscribe"},
        {"Success:","Success:"},
        {"Summary","Summary"},
        {"Tabular Navigation","Tabular Navigation"},
        {"TabularData are not supported", "TabularData are not supported"},
        {"Threads","Threads"},
	{"Threshold","Threshold"},
        {"Tile","Tile"},
        {"TipDescrTypeModifier","Descr : {0}, Type : {1}, Modifier : {2}"},
        {"Time Range:", "Time Range:"},
        {"Time", "Time"},
        {"TimeStamp","TimeStamp"},
	{"Total Loaded", "Total Loaded"},
        {"Total classes loaded","Total classes loaded"},
        {"Total classes unloaded","Total classes unloaded"},
        {"Total compile time","Total compile time"},
        {"Total physical memory","Total physical memory"},
	{"Total Started","Total Started"},
        {"Total started","Total started"},
        {"Total swap space","Total swap space"},
        {"Tree","Tree"},
        {"Type","Type"},
        {"Unavailable","Unavailable"},
        {"Unsubscribe","Unsubscribe"},
	{"Unregister", "Unregister"},
        {"Uptime","Uptime"},
        {"Uptime: ","Uptime: "},
        {"Usage Threshold","Usage Threshold"},
        {"Used","Used"},
        {"User Name: ","User Name: "},
        {"UserData","UserData"},
        {"VM Information","VM Information"},
        {"VM arguments","VM arguments"},
        {"VM","VM"},
        {"Value","Value"},
        {"Vendor", "Vendor"},
        {"Verbose Output","Verbose Output"},
        {"View value", "View value"},
        {"View","View"},
        {"Window","Window"},
        {"Windows","Windows"},
        {"You cannot drop a class here", "You cannot drop a class here"},
        {"collapse", "collapse"},
        {"expand", "expand"},
        {"kbytes","{0} kbytes"},
        {"operation","operation"},
        {"plot", "plot"},
        {"visualize","visualize"},
        {"zz usage text",
             "Usage: {0} [ -interval=n ] [ -notile ] [ -version ] [ pid | [connection ... ]]" + cr +
             cr +
             "  -interval Set the update interval to n seconds (default is 4 seconds)" + cr +
             "  -notile   Do not tile windows initially (for two or more connections)" + cr +
             "  -version  Print program version" + cr +
             cr +
             "  pid       The process id of a target process" + cr +
             cr +
	     "  connection = host:port || JMX URL (service:jmx:<protocol>://...)" + cr +
             "  host      A remote host name or IP address" + cr +
             "  port      The port number for the remote connection" },
        // END OF MATERIAL TO LOCALIZE
    };

    /**
     * Returns the contents of this <code>ResourceBundle</code>.
     *
     * <p>
     *
     * @return the contents of this <code>ResourceBundle</code>.
     */
    public Object[][] getContents() {
        return contents;
    }
}
