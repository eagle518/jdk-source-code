/*
 * @(#)JConsoleResources_ja.java	1.5 04/07/01
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
 * @version @(#)JConsoleResources_ja.java	1.5 04/07/01
 */
public class JConsoleResources_ja extends java.util.ListResourceBundle {

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
        {" 1 day"," 1 \u65e5"},
        {" 1 hour"," 1 \u6642\u9593"},
        {" 1 min"," 1 \u5206"},
        {" 1 month"," 1 \u304b\u6708"},
        {" 1 year"," 1 \u5e74"},
        {" 2 hours"," 2 \u6642\u9593"},
        {" 3 hours"," 3 \u6642\u9593"},
        {" 3 months"," 3 \u304b\u6708"},
        {" 5 min"," 5 \u5206"},
        {" 6 hours"," 6 \u6642\u9593"},
        {" 6 months"," 6 \u304b\u6708"},
        {" 7 days"," 7 \u65e5"},
        {" kbytes"," k \u30d0\u30a4\u30c8"},
        {" version "," \u30d0\u30fc\u30b8\u30e7\u30f3 "},
        {"10 min","10 \u5206"},
        {"12 hours","12 \u6642\u9593"},
        {"30 min","30 \u5206"},
        {"<","<"},
        {"<<","<<"},
        {">",">"},
	{"Advanced", "\u8a73\u7d30"},
        {"All","\u3059\u3079\u3066"},
        {"Apply","\u9069\u7528"},
        {"Architecture","\u30a2\u30fc\u30ad\u30c6\u30af\u30c1\u30e3"},
        {"Array, OpenType", "\u914d\u5217\u3001OpenType"},
        {"Array, OpenType, Numeric value viewer","\u914d\u5217\u3001OpenType\u3001\u6570\u5024\u306e\u30d3\u30e5\u30fc\u30a2"},
        {"Attributes","\u5c5e\u6027"},
        {"BlockedCount WaitedCount",
             "\u7dcf\u30d6\u30ed\u30c3\u30af\u6570 : {0}  \u7dcf\u5f85\u6a5f\u6570 : {1}" + cr},
        {"Boot class path","\u30d6\u30fc\u30c8\u30af\u30e9\u30b9\u30d1\u30b9"},
        {"Cancel","\u53d6\u6d88\u3057"},
        {"Cascade","\u91cd\u306d\u3066\u8868\u793a"},
	{"Chart", "\u56f3"},
	{"Chart:", "\u56f3:"},
        {"Class path","\u30af\u30e9\u30b9\u30d1\u30b9"},
	{"Class","\u30af\u30e9\u30b9"},
        {"Classes","\u30af\u30e9\u30b9"},
        {"Clear","\u6d88\u53bb"},
	{"Column.Class and Arguments", "\u30af\u30e9\u30b9\u3068\u5f15\u6570"},
	{"Column.PID", "PID"},
        {"Committed memory","\u78ba\u5b9a\u30e1\u30e2\u30ea"},
        {"Committed virtual memory","\u78ba\u5b9a\u4eee\u60f3\u30e1\u30e2\u30ea"},
        {"Committed", "\u78ba\u5b9a"},
        {"Compiler","\u30b3\u30f3\u30d1\u30a4\u30e9"},
        {"Composite Navigation", "\u8907\u5408\u30ca\u30d3\u30b2\u30fc\u30b7\u30e7\u30f3"},
        {"CompositeData","CompositeData"},
        {"Config","\u69cb\u6210"},
        {"Connect","\u63a5\u7d9a"},
        {"Connect...","\u63a5\u7d9a..."},
        {"Connection failed","\u63a5\u7d9a\u306b\u5931\u6557\u3057\u307e\u3057\u305f"},
        {"Connection","\u63a5\u7d9a"},
        {"ConnectionName (disconnected)","{0}@{1} (\u63a5\u7d9a\u89e3\u9664)"},
        {"ConnectionName","{0}@{1}"},
        {"Current classes loaded", "\u73fe\u5728\u30ed\u30fc\u30c9\u3055\u308c\u3066\u3044\u308b\u30af\u30e9\u30b9"},
        {"Current heap size","\u73fe\u5728\u306e\u30d2\u30fc\u30d7\u30b5\u30a4\u30ba"},
        {"Current value","\u73fe\u5728\u306e\u5024: {0}"},
	{"Create", "\u4f5c\u6210"},
        {"Daemon threads","\u30c7\u30fc\u30e2\u30f3\u30b9\u30ec\u30c3\u30c9"},
        {"Double click to expand/collapse","\u30c0\u30d6\u30eb\u30af\u30ea\u30c3\u30af\u3057\u3066\u5c55\u958b/\u6298\u308a\u305f\u305f\u307f"},
        {"Double click to visualize", "\u30c0\u30d6\u30eb\u30af\u30ea\u30c3\u30af\u3057\u3066\u8868\u793a"},
        {"Description: ","\u8aac\u660e: "},
        {"Details", "\u8a73\u7d30"},
        {"Dimension is not supported:", "\u5927\u304d\u3055\u306f\u30b5\u30dd\u30fc\u30c8\u3055\u308c\u3066\u3044\u307e\u305b\u3093:"},
        {"Discard chart", "\u56f3\u3092\u7834\u68c4\u3059\u308b"},
        {"DurationDaysHoursMinutes","{0,choice,1#{0,number,integer} \u65e5 |1.0<{0,number,integer} \u65e5 }" +
                                    "{1,choice,0<{1,number,integer} \u6642\u9593 |1#{1,number,integer} \u6642\u9593 |1<{1,number,integer} \u6642\u9593 }" +
                                    "{2,choice,0<{2,number,integer} \u5206 |1#{2,number,integer} \u5206 |1.0<{2,number,integer} \u5206}"},

        {"DurationHoursMinutes","{0,choice,1#{0,number,integer} \u6642\u9593 |1<{0,number,integer} \u6642\u9593 }" +
                                "{1,choice,0<{1,number,integer} \u5206 |1#{1,number,integer} \u5206 |1.0<{1,number,integer} \u5206}"},

        {"DurationMinutes","{0,choice,1#{0,number,integer} \u5206 |1.0<{0,number,integer} \u5206}"},
        {"DurationSeconds","{0} \u79d2"},
        {"Empty array", "\u914d\u5217\u3092\u7a7a\u306b\u3059\u308b"},
        {"Empty opentype viewer", "OpenType \u30d3\u30e5\u30fc\u30a2\u3092\u7a7a\u306b\u3059\u308b"},
        {"Error","\u30a8\u30e9\u30fc"},
	{"Error: MBeans already exist","\u30a8\u30e9\u30fc : MBean \u306f\u3059\u3067\u306b\u5b58\u5728\u3057\u307e\u3059"},
	{"Error: MBeans do not exist","\u30a8\u30e9\u30fc : MBean \u306f\u5b58\u5728\u3057\u307e\u305b\u3093"},
        {"Error:","\u30a8\u30e9\u30fc:"},
        {"Event","\u30a4\u30d9\u30f3\u30c8"},
        {"Exit","\u7d42\u4e86"},
        {"Failed loading L&F: ","Look & Feel \u306e\u30ed\u30fc\u30c9\u306b\u5931\u6557\u3057\u307e\u3057\u305f: {0}"},
        {"Filter: ","\u30d5\u30a3\u30eb\u30bf: "},
        {"Free physical memory","\u7a7a\u304d\u7269\u7406\u30e1\u30e2\u30ea"},
        {"Free swap space","\u7a7a\u304d\u30b9\u30ef\u30c3\u30d7\u30b9\u30da\u30fc\u30b9"},
        {"Garbage collector","\u30ac\u30d9\u30fc\u30b8\u30b3\u30ec\u30af\u30bf"},
        {"GTK","GTK"},
        {"GcInfo","\u540d\u524d = ''{0}'', \u30b3\u30ec\u30af\u30b7\u30e7\u30f3 = {1,choice,-1#\u5229\u7528\u4e0d\u53ef|0#{1,number,integer}}, \u7dcf\u7d4c\u904e\u6642\u9593 = {2}"},
        {"GC time","GC \u6642\u9593"},
        {"Heap Memory Usage","\u30d2\u30fc\u30d7\u30e1\u30e2\u30ea\u306e\u4f7f\u7528\u72b6\u6cc1"},
        {"Heap", "\u30d2\u30fc\u30d7"},
        {"Host or IP: ","\u30db\u30b9\u30c8\u307e\u305f\u306f IP: "},
        {"Info","\u60c5\u5831"},
	{"Invalid URL", "\u7121\u52b9\u306a URL"},
        {"J2SE 5.0 Monitoring & Management Console", "J2SE 5.0 Monitoring & Management Console"},
        {"JConsole version","JConsole \u30d0\u30fc\u30b8\u30e7\u30f3 \"{0}\""},
        {"JConsole: Connect to Agent","JConsole: \u30a8\u30fc\u30b8\u30a7\u30f3\u30c8\u306b\u63a5\u7d9a"},
        {"JIT compiler","JIT \u30b3\u30f3\u30d1\u30a4\u30e9"},
	{"JMX URL: ", "JMX URL: "},
        {"Java Virtual Machine","Java \u4eee\u60f3\u30de\u30b7\u30f3"},
        {"Java","Java"},
        {"Library path","\u30e9\u30a4\u30d6\u30e9\u30ea\u30d1\u30b9"},
        {"Listeners","\u30ea\u30b9\u30ca\u30fc"},
	{"Live Threads","\u30e9\u30a4\u30d6\u30b9\u30ec\u30c3\u30c9"},
        {"Live threads","\u30e9\u30a4\u30d6\u30b9\u30ec\u30c3\u30c9"},
	{"Loaded", "\u30ed\u30fc\u30c9\u6e08\u307f"},
	{"Local", "\u30ed\u30fc\u30ab\u30eb"},
        {"Look and Feel","Look & Feel"},
        {"MBean Java Class","MBean Java \u30af\u30e9\u30b9"},
        {"MBean Name","MBean \u540d"},
	{"MBean Notification","MBean \u901a\u77e5"},
        {"MBeans","MBean"},
        {"Manage Hotspot MBeans in: ", "Hotspot MBean \u3092\u7ba1\u7406: "},
        {"Max","\u6700\u5927"},
        {"Maximum heap size","\u6700\u5927\u30d2\u30fc\u30d7\u30b5\u30a4\u30ba"},
        {"Memory","\u30e1\u30e2\u30ea"},
        {"MemoryPoolLabel", "\u30e1\u30e2\u30ea\u30d7\u30fc\u30eb \"{0}\""},
        {"Message","\u30e1\u30c3\u30bb\u30fc\u30b8"},
        {"Method successfuly invoked", "\u30e1\u30bd\u30c3\u30c9\u306f\u547c\u3073\u51fa\u3055\u308c\u307e\u3057\u305f"},
        {"Minimize All","\u3059\u3079\u3066\u3092\u30a2\u30a4\u30b3\u30f3\u5316"},
        {"Minus Version", "\u3053\u308c\u306f {0} \u30d0\u30fc\u30b8\u30e7\u30f3 {1} \u3067\u3059"},
        {"Monitoring Self","{0} (\u81ea\u8eab\u3092\u30e2\u30cb\u30bf\u30fc\u4e2d)"},
        {"Motif","Motif"},
        {"Name Build and Mode","{0} (\u30d3\u30eb\u30c9 {1}, {2})"},
        {"Name and Build","{0} (\u30d3\u30eb\u30c9 {1})"},
        {"Name","\u540d\u524d"},
        {"Name: ","\u540d\u524d: "},
        {"Name State",
             "\u540d\u524d: {0}" + cr +
             "\u72b6\u614b: {1}" + cr},
        {"Name State LockName",
             "\u540d\u524d: {0}" + cr +
             "\u72b6\u614b: {1} ({2} \u4e0a)" + cr},
        {"Name State LockName LockOwner",
             "\u540d\u524d: {0}" + cr +
             "\u72b6\u614b: {1} ({2} \u4e0a) \u6240\u6709\u8005: {3}" + cr},
        {"New Connection...","\u65b0\u898f\u63a5\u7d9a..."},
        {"New value applied","\u65b0\u3057\u3044\u5024\u304c\u9069\u7528\u3055\u308c\u307e\u3057\u305f"},
        {"No attribute selected","\u5c5e\u6027\u304c\u9078\u629e\u3055\u308c\u3066\u3044\u307e\u305b\u3093"},
        {"No value selected","\u5024\u304c\u9078\u629e\u3055\u308c\u3066\u307e\u305b\u3093"},
        {"Non-Heap Memory Usage","\u975e\u30d2\u30fc\u30d7\u30e1\u30e2\u30ea\u306e\u4f7f\u7528\u72b6\u6cc1"},
        {"Non-Heap", "\u975e\u30d2\u30fc\u30d7"},
        {"Not Yet Implemented","\u5b9f\u88c5\u3055\u308c\u3066\u3044\u307e\u305b\u3093"},
        {"Not a valid event broadcaster", "\u6709\u52b9\u306a\u30a4\u30d9\u30f3\u30c8\u30d6\u30ed\u30fc\u30c9\u30ad\u30e3\u30b9\u30c8\u5143\u3067\u306f\u3042\u308a\u307e\u305b\u3093"},
        {"Notifications","\u901a\u77e5"},
	{"Number of Threads","\u30b9\u30ec\u30c3\u30c9\u6570"},
        {"Number of Loaded Classes","\u30ed\u30fc\u30c9\u3055\u308c\u305f\u30af\u30e9\u30b9\u306e\u6570"},
        {"Number of processors","\u30d7\u30ed\u30bb\u30c3\u30b5\u306e\u6570"},
        {"Objects pending for finalization","\u30d5\u30a1\u30a4\u30ca\u30e9\u30a4\u30ba\u3092\u4fdd\u7559\u4e2d\u306e\u30aa\u30d6\u30b8\u30a7\u30af\u30c8"},
        {"Operating System","\u30aa\u30da\u30ec\u30fc\u30c6\u30a3\u30f3\u30b0\u30b7\u30b9\u30c6\u30e0"},
        {"Operation return value", "\u64cd\u4f5c\u306e\u623b\u308a\u5024"},
        {"Operations","\u64cd\u4f5c"},
        {"Password: ", "\u30d1\u30b9\u30ef\u30fc\u30c9: "},
        {"Peak","\u30d4\u30fc\u30af"},
        {"Perform GC","GC \u306e\u5b9f\u884c"},
        {"Port: ","\u30dd\u30fc\u30c8: "},
        {"Problem adding listener","\u30ea\u30b9\u30ca\u30fc\u8ffd\u52a0\u6642\u306e\u554f\u984c"},
        {"Problem displaying MBean", "MBean \u8868\u793a\u6642\u306e\u554f\u984c"},
        {"Problem dropping object","\u30aa\u30d6\u30b8\u30a7\u30af\u30c8\u30c9\u30ed\u30c3\u30d7\u6642\u306e\u554f\u984c"},
        {"Problem invoking", "\u547c\u3073\u51fa\u3057\u6642\u306e\u554f\u984c"},
        {"Problem setting attribute","\u5c5e\u6027\u8a2d\u5b9a\u6642\u306e\u554f\u984c"},
        {"Process CPU time","\u30d7\u30ed\u30bb\u30b9 CPU \u6642\u9593"},
        {"R/W","R/W"},
        {"Received","\u53d7\u4fe1\u6e08\u307f"},
        {"Refresh","\u66f4\u65b0"},
	{"Remote","\u30ea\u30e2\u30fc\u30c8"},
        {"Remove","\u524a\u9664"},
        {"Restore All","\u3059\u3079\u3066\u3092\u5fa9\u5143"},
        {"Return value", "\u623b\u308a\u5024"},
	{"SeqNum","\u30b7\u30fc\u30b1\u30f3\u30b9\u756a\u53f7"},
        {"Size Bytes", "{0,number,integer} \u30d0\u30a4\u30c8"},
        {"Size Gb","{0} G \u30d0\u30a4\u30c8"},
        {"Size Kb","{0} K \u30d0\u30a4\u30c8"},
        {"Size Mb","{0} M \u30d0\u30a4\u30c8"},
        {"Source","\u30bd\u30fc\u30b9"},
        {"Stack trace",
             cr + "\u30b9\u30bf\u30c3\u30af\u30c8\u30ec\u30fc\u30b9: " + cr},
        {"Subscribe","\u767b\u9332"},
        {"Success:","\u6210\u529f:"},
        {"Summary","\u6982\u8981"},
        {"Tabular Navigation","\u8868\u30ca\u30d3\u30b2\u30fc\u30b7\u30e7\u30f3"},
        {"TabularData are not supported", "TabularData \u306f\u30b5\u30dd\u30fc\u30c8\u3055\u308c\u3066\u3044\u307e\u305b\u3093"},
        {"Threads","\u30b9\u30ec\u30c3\u30c9"},
	{"Threshold","\u3057\u304d\u3044\u5024"},
        {"Tile","\u4e26\u3079\u3066\u8868\u793a"},
        {"TipDescrTypeModifier","\u8aac\u660e: {0}\u3001\u578b: {1}\u3001\u4fee\u98fe\u5b50: {2}"},
        {"Time Range:", "\u6642\u9593\u7bc4\u56f2:"},
        {"Time", "\u6642\u9593"},
        {"TimeStamp","\u30bf\u30a4\u30e0\u30b9\u30bf\u30f3\u30d7"},
	{"Total Loaded", "\u7dcf\u30ed\u30fc\u30c9\u6570"},
        {"Total classes loaded","\u30ed\u30fc\u30c9\u3055\u308c\u305f\u30af\u30e9\u30b9\u306e\u7dcf\u6570"},
        {"Total classes unloaded","\u30a2\u30f3\u30ed\u30fc\u30c9\u3055\u308c\u305f\u30af\u30e9\u30b9\u306e\u7dcf\u6570"},
        {"Total compile time","\u30b3\u30f3\u30d1\u30a4\u30eb\u306e\u7dcf\u6642\u9593"},
        {"Total physical memory","\u7dcf\u7269\u7406\u30e1\u30e2\u30ea"},
	{"Total Started","\u7dcf\u958b\u59cb\u6570"},
        {"Total started","\u958b\u59cb\u3057\u305f\u7dcf\u6570"},
        {"Total swap space","\u7dcf\u30b9\u30ef\u30c3\u30d7\u30b9\u30da\u30fc\u30b9"},
        {"Tree","\u30c4\u30ea\u30fc"},
        {"Type","\u578b"},
        {"Unavailable","\u4f7f\u7528\u4e0d\u53ef\u80fd"},
        {"Unsubscribe","\u767b\u9332\u89e3\u9664"},
	{"Unregister", "\u767b\u9332\u89e3\u9664"},
        {"Uptime","\u30a2\u30c3\u30d7\u30bf\u30a4\u30e0"},
        {"Uptime: ","\u30a2\u30c3\u30d7\u30bf\u30a4\u30e0: "},
        {"Usage Threshold","\u4f7f\u7528\u91cf\u306e\u3057\u304d\u3044\u5024"},
        {"Used","\u4f7f\u7528\u6e08\u307f"},
        {"User Name: ","\u30e6\u30fc\u30b6\u540d: "},
        {"UserData","UserData"},
        {"VM Information","VM \u306e\u60c5\u5831"},
        {"VM arguments","VM \u306e\u5f15\u6570"},
        {"VM","VM"},
        {"Value","\u5024"},
        {"Vendor", "\u30d9\u30f3\u30c0"},
        {"Verbose Output","\u8a73\u7d30\u51fa\u529b"},
        {"View value", "\u5024\u3092\u8868\u793a\u3059\u308b"},
        {"View","\u8868\u793a"},
        {"Window","\u30a6\u30a3\u30f3\u30c9\u30a6"},
        {"Windows","\u30a6\u30a3\u30f3\u30c9\u30a6"},
        {"You cannot drop a class here", "\u30af\u30e9\u30b9\u3092\u3053\u3053\u306b\u30c9\u30ed\u30c3\u30d7\u3067\u304d\u307e\u305b\u3093"},
        {"collapse", "\u6298\u308a\u305f\u305f\u307f"},
        {"expand", "\u5c55\u958b"},
        {"kbytes","{0} k \u30d0\u30a4\u30c8"},
        {"operation","\u30aa\u30da\u30ec\u30fc\u30b7\u30e7\u30f3"},
        {"plot", "\u30d7\u30ed\u30c3\u30c8"},
        {"zz usage text",
             "\u4f7f\u7528\u6cd5: {0} [ -interval=n ] [ -notile ] [ -version ] [ pid | [connection ... ]]" + cr +
             cr +
             "  -interval \u66f4\u65b0\u9593\u9694\u3092 n \u79d2\u306b\u8a2d\u5b9a\u3059\u308b (\u30c7\u30d5\u30a9\u30eb\u30c8\u306f 4 \u79d2)" + cr +
             "  -notile   \u521d\u671f\u72b6\u614b\u3067\u30a6\u30a3\u30f3\u30c9\u30a6\u3092\u30bf\u30a4\u30eb\u72b6\u306b\u4e26\u3079\u306a\u3044 (\u63a5\u7d9a\u304c\u8907\u6570\u3042\u308b\u5834\u5408)" + cr +
             "  -version  \u30d7\u30ed\u30b0\u30e9\u30e0\u306e\u30d0\u30fc\u30b8\u30e7\u30f3\u3092\u51fa\u529b\u3059\u308b" + cr +
             cr +
	     "  pid	  \u30bf\u30fc\u30b2\u30c3\u30c8\u30d7\u30ed\u30bb\u30b9\u306e\u30d7\u30ed\u30bb\u30b9 ID" + cr +
             cr +
	     "  connection = host:port || JMX URL (service:jmx:<\u30d7\u30ed\u30c8\u30b3\u30eb>://...)" + cr +
             "  host      \u30ea\u30e2\u30fc\u30c8\u30db\u30b9\u30c8\u306e\u540d\u524d\u307e\u305f\u306f IP \u30a2\u30c9\u30ec\u30b9" + cr +
             "  port      \u30ea\u30e2\u30fc\u30c8\u63a5\u7d9a\u7528\u306e\u30dd\u30fc\u30c8\u756a\u53f7" },
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
