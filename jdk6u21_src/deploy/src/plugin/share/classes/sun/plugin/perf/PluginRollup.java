/*
 * @(#)PluginRollup.java	1.2 10/03/24
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.perf;

import java.lang.Object;
import java.lang.String;
import java.io.PrintStream;
import java.util.HashMap;
import com.sun.deploy.perf.PerfLabel;
import com.sun.deploy.perf.PerfRollup;

public class PluginRollup extends Object
                          implements PerfRollup {
    /**
     * Perform a rollup of the given <code>PerfLabels</code>.
     */
    public void doRollup(PerfLabel [] labels, PrintStream out) {
        if (labels.length > 0) {
            EventSet events = new EventSet(labels);

            // figure out the overall time from startup of the plug-in to the
            // point when the applet's init method was invoked
            long overallDelta = labels[labels.length - 1].getTime() -
                                labels[0].getTime();

            // figure out the time spent getting the JVM ready to go
            long jvmOverall = events.getEventDelta(JVM_INITIALIZE_JAVA);
            long jvmStartup = events.getEventDelta(JVM_START_JAVA_VM);
            long runtime    = events.getEventDelta(JVM_RUNTIME_INIT);
            long console    = events.getEventDelta(JVM_CONSOLE);

            // figure out which browser is used based on whether or not there
            // is an event for the given browser key
            long  browser = 0;
            Event event   = null;

            event = events.getEvent(JVM_IE_JNI_REG);
            if (event != null) {
                browser = event.getDelta();
            }
            else {
                // if it wasn't IE it must be Mozilla
                browser = events.getEventDelta(JVM_MOZILLA_JNI_REG);
            }

            // the overall JVM startup time is the total generic JVM init phase,
            // plus the browser specific native JNI registration
            jvmOverall += browser;

            // figure out the time spent getting the applet initialized
            long viewInit     = events.getEventDelta(ENV_VIEWER_INIT);
            long threadGrp    = events.getEventDelta(ENV_THREAD_GRP);
            long traceEnv     = events.getEventDelta(ENV_TRACE_ENV);
            long proxyAuth    = events.getEventDelta(ENV_PROXY_AUTH);
            long proxyReset   = events.getEventDelta(ENV_PROXY_SELECT);
            long sysTray      = events.getEventDelta(ENV_WIN_SYS_TRAY);
            long upgradeCache = events.getEventDelta(ENV_UPGRADE_CACHE);
            long createLoader = events.getEventDelta(ENV_CREATE_LOADER);
            long initApplet   = events.getEventDelta(ENV_APPLET_INIT);

            // the Plug-in object and applet window stuff is browser/platform
            // specific
            long createObj = 0;
            long createWnd = 0;

            event = events.getEvent(ENV_CREATE_IE_OBJ);
            if (event != null) {
                createObj = event.getDelta();
                createWnd = events.getEventDelta(ENV_CREATE_IE_WND);
            }
            else {
                event = events.getEvent(ENV_CREATE_MOZW_OBJ);
                if (event != null) {
                    createObj = event.getDelta();
                    createWnd = events.getEventDelta(ENV_CREATE_MOZW_WND);
                }
                else {
                    // if it isn't either Windows browser, then it must be
                    // Mozilla on UNIX
                    createObj = events.getEventDelta(ENV_CREATE_MOZU_OBJ);
                    createWnd = events.getEventDelta(ENV_CREATE_MOZU_WND);
                }
            }

            // Some of the various applet times measured above are actually
            // sub-timings.  There are also holes in the overall applet init
            // in areas that aren't interesting enough to measure.  Because
            // of the, the correct way to get the actual overall is by taking
            // the delta from the start of the first applet event and the end
            // of the last applet event.
            long appOverall = 0;

            event = events.getEvent(OVERALL_APPLET_INIT_START);
            if (event != null) {
                Event end = events.getEvent(OVERALL_APPLET_INIT_END);
                if (end != null) {
                    appOverall = end.getEnd().getTime() -
                                 event.getStart().getTime();
                }
            }

            out.println();
            out.println("Overall Plug-in startup time................... " + overallDelta + " ms");
            out.println("     Total time starting JVM................... " + jvmOverall + " ms");
            out.println("         JVM startup........................... " + jvmStartup + " ms");
            out.println("         Runtime initialization................ " + runtime + " ms");
            out.println("         Console initialization................ " + console + " ms");
            out.println("         Browser specific JVM initialization... " + browser + " ms");
            out.println("     Total time preparing applet............... " + appOverall + " ms");
            out.println("         Viewer initialization................. " + viewInit + " ms");
            out.println("              get Plug-in thread group......... " + threadGrp + " ms");
            out.println("              init trace environment........... " + traceEnv + " ms");
            out.println("              enable proxy authentication...... " + proxyAuth + " ms");
            out.println("              proxy selector reset............. " + proxyReset + " ms");
            out.println("              update system tray message....... " + sysTray + " ms");
            out.println("              upgrade cache.................... " + upgradeCache + " ms");
            out.println("         Create Plug-in object................. " + createObj + " ms");
            out.println("         Create applet window.................. " + createWnd + " ms");
            out.println("         Create Plug-in class loader........... " + createLoader + " ms");
            out.println("         Invoke applet.init().................. " + initApplet + " ms");
        }
        else {
            out.println("Plug-in perf logging is not currently implemented for UNIX platforms.");
        }
    }


    /***************************************************************************
     * This inner class is used to track the set of events.  The class only
     * recognizes labels in the  correct order, i.e. start and end pairs.  The
     * class ignores labels that are recorded out of order, or with label types
     * that are not start or end.
     */
    class EventSet extends Object {

        /***********************************************************************
         * Gets the event for the given key.
         *
         * @param key  the key for the event.
         *
         * @returns the event for the given key.
         */
        Event getEvent(String key) {
            return ((Event) map.get(key));
        }

        /***********************************************************************
         * Gets the time elapsed between the start and end of an event.
         *
         * @param key  the key for the event.
         *
         * @returns the time elapsed between the start and end of an event, or
         * zero if the event doesn't exist.
         */
        long getEventDelta(String key) {
            Event event = getEvent(key);

            return (((event != null) ? event.getDelta() : 0));
        }

        /***********************************************************************
         * Construct an EventSet from an array of labels.
         *
         * @param labale  the labels for this event set.
         */
        EventSet(PerfLabel [] labels) {
            map = new HashMap();

            for (int i = 0; i < labels.length; i++) {
                long    current = labels[i].getTime();
                String  label   = labels[i].getLabel();
                String  type    = label.substring(TYPE_START, TYPE_END).trim();
                String  key     = label.substring(KEY_START).trim();

                // If type is "POINT" or anything other than "START" or "END"
                // then it isn't tracked by the rollup.  Also, there is no
                // possible way for labels to be out of order, i.e. "END" before
                // "START", except if there is a bug in the where the labels
                // where added.  Therefore, this code doesn't try to deal with
                // such stuff.
                if (type.equals("START") == true) {
                    map.put(key, new Event(key, labels[i]));
                }
                else if (type.equals("END") == true) {
                    Event start = (Event) map.get(key);

                    if (start != null) {
                        start.setEnd(labels[i]);
                    }
                }
            }
        }

        HashMap map;

        // Java Plug-in uses PerfLabels with as standard text format, as
        // follows:
        //
        //     <type> - <area> - <category> - <event-key>
        // 
        // <type> This field, which is exactly 5 characters wide, is used to
        //        indicate the type of event the label represents.  There are
        //        two specific types recognized by this rollup class: "START"
        //        and "END  ".
        //
        // <area> This field, which is exactly 6 characters wide, indicates the
        //        area in the code where the labeled event occurred.  Plug-in
        //        labels currently specify one of two areas: "Native" or
        //        "Java  ".  However, this rollup doesn't key off this data so
        //        any area should be accepted.
        //
        // <category> This field, which is exactly 3 characters wide, is used to
        //            indicate the general category for event represented by the
        //            label.  Plug-in monitors two basic categories currently:
        //            "JVM" and "ENV".  These represent JVM startup events and
        //            applet environment initialization events, respectively.
        //
        // <key> This field, which is no more than 98 characters wide, is used
        //       to uniquely identify related labels.  That is, all labels that
        //       have the same key text are considered part of the same event.
        //       In this manner, given several labels with the same key, the
        //       label with the "START" type represents the start of an event,
        //       and the label with the "END  " type represents the end of an
        //       event.  Any additional labels, such as "POINT" type labels are
        //       not used in the rollup calculations, but may be of use in human
        //       analysis.

        static final int TYPE_START = 0;
        static final int TYPE_END   = 5;
        static final int AREA_START = 8;
        static final int AREA_END   = 14;
        static final int CAT_START  = 17;
        static final int CAT_END    = 20;
        static final int KEY_START  = 23;
    }


    /***************************************************************************
     * This inner class is used to represent a pair of start and end labels that
     * signify an interesting event.
     */
    class Event {

        /***********************************************************************
         * Gets the label that represents the start of this event.
         *
         * @returns the label that represents the start of this event.
         */
        PerfLabel getStart() {
            return (start);
        }

        /***********************************************************************
         * Gets the label that represents the end of this event.
         *
         * @returns the label that represents the end of this event.
         */
        PerfLabel getEnd() {
            return (end);
        }

        /***********************************************************************
         * Sets the end for this event.
         *
         * Note: This event is incomplete until thi method is called.
         *
         * @param end  the label that represents the end of this event.
         */
        void setEnd(PerfLabel end) {
            this.end = end;
        }

        /***********************************************************************
         * Gets the time elapsed between the start and end of this event.
         *
         * @returns the time elapsed between the start and end of this event.
         */
        long getDelta() {
            return (end.getTime() - start.getTime());
        }

        /***********************************************************************
         * Construct a partial event for the given key and label.
         *
         * Note: This event is incomplete until the setEnd method is called.
         *
         * @param key    the key for this event.
         * @param start  the label that represents the start of this event.
         */
        Event(String key, PerfLabel start) {
            this.key   = key;
            this.start = start;
            this.end   = null;
        }

        String    key;
        PerfLabel start;
        PerfLabel end;
    }

    private static final String JVM_INITIALIZE_JAVA = "Plug-in Java VM initialization phase";
    private static final String JVM_START_JAVA_VM   = "Plug-in Java VM startup";
    private static final String JVM_RUNTIME_INIT    = "invoke JavaRunTime.initEnvironment";
    private static final String JVM_CONSOLE         = "Java console initialization";
    private static final String JVM_IE_JNI_REG      = "register IE specific JNI methods";
    private static final String JVM_MOZILLA_JNI_REG = "register Mozilla specific JNI methods";

    private static final String ENV_VIEWER_INIT     = "AppletViewer.initEnvironment";
    private static final String ENV_THREAD_GRP      = "AppletViewer.initEnvironment - PluginSysUtil.getPluginThreadGroup";
    private static final String ENV_TRACE_ENV       = "AppletViewer.initEnvironment - JavaRunTime.initTraceEnvironment";
    private static final String ENV_PROXY_AUTH      = "AppletViewer.initEnvironment - enable proxy/web server authentication";
    private static final String ENV_PROXY_SELECT    = "AppletViewer.initEnvironment - DeployProxySelector.reset";
    private static final String ENV_WIN_SYS_TRAY    = "AppletViewer.initEnvironment - show update message";
    private static final String ENV_UPGRADE_CACHE   = "AppletViewer.initEnvironment - upgrade cache";
    private static final String ENV_CREATE_IE_OBJ   = "create browser plugin object (IE)";
    private static final String ENV_CREATE_MOZW_OBJ = "create browser plugin object (Mozilla:Windows)";
    private static final String ENV_CREATE_MOZU_OBJ = "create browser plugin object (Unix:Windows)";
    private static final String ENV_CREATE_IE_WND   = "create embedded browser frame (IE)";
    private static final String ENV_CREATE_MOZW_WND = "create embedded browser frame (Mozilla:Windows)";
    private static final String ENV_CREATE_MOZU_WND = "create embedded browser frame (Mozilla:Unix)";
    private static final String ENV_CREATE_LOADER   = "AppletViewer.createClassLoader";
    private static final String ENV_APPLET_INIT     = "AppletViewer.initApplet";

    private static final String OVERALL_APPLET_INIT_START = ENV_VIEWER_INIT;
    private static final String OVERALL_APPLET_INIT_END   = ENV_APPLET_INIT;

}
