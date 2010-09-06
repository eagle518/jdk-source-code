/*
 * @(#)VMConnection.java	1.45 04/04/12
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*
 * Copyright (c) 1997-2001 by Sun Microsystems, Inc. All Rights Reserved.
 * 
 * Sun grants you ("Licensee") a non-exclusive, royalty free, license to use,
 * modify and redistribute this software in source and binary code form,
 * provided that i) this copyright notice and license appear on all copies of
 * the software; and ii) Licensee does not utilize the software in a manner
 * which is disparaging to Sun.
 * 
 * This software is provided "AS IS," without a warranty of any kind. ALL
 * EXPRESS OR IMPLIED CONDITIONS, REPRESENTATIONS AND WARRANTIES, INCLUDING ANY
 * IMPLIED WARRANTY OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NON-INFRINGEMENT, ARE HEREBY EXCLUDED. SUN AND ITS LICENSORS SHALL NOT BE
 * LIABLE FOR ANY DAMAGES SUFFERED BY LICENSEE AS A RESULT OF USING, MODIFYING
 * OR DISTRIBUTING THE SOFTWARE OR ITS DERIVATIVES. IN NO EVENT WILL SUN OR ITS
 * LICENSORS BE LIABLE FOR ANY LOST REVENUE, PROFIT OR DATA, OR FOR DIRECT,
 * INDIRECT, SPECIAL, CONSEQUENTIAL, INCIDENTAL OR PUNITIVE DAMAGES, HOWEVER
 * CAUSED AND REGARDLESS OF THE THEORY OF LIABILITY, ARISING OUT OF THE USE OF
 * OR INABILITY TO USE SOFTWARE, EVEN IF SUN HAS BEEN ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGES.
 * 
 * This software is not designed or intended for use in on-line control of
 * aircraft, air traffic, aircraft navigation or aircraft communications; or in
 * the design, construction, operation or maintenance of any nuclear
 * facility. Licensee represents and warrants that it will not use or
 * redistribute the Software for such purposes.
 */

package com.sun.tools.example.debug.tty;

import com.sun.jdi.*;
import com.sun.jdi.connect.*;
import com.sun.jdi.request.EventRequestManager;
import com.sun.jdi.request.ExceptionRequest;
import com.sun.jdi.request.ThreadStartRequest;
import com.sun.jdi.request.ThreadDeathRequest;

import java.util.*;
import java.util.regex.*;
import java.io.*;

class VMConnection {

    private VirtualMachine vm;    
    private Process process = null;
    private int outputCompleteCount = 0;

    private final Connector connector;
    private final Map connectorArgs;
    private final int traceFlags;

    synchronized void notifyOutputComplete() {
        outputCompleteCount++;
        notifyAll();
    }

    synchronized void waitOutputComplete() {
        // Wait for stderr and stdout
        if (process != null) {
            while (outputCompleteCount < 2) {
                try {wait();} catch (InterruptedException e) {}
            }
        }
    }

    private Connector findConnector(String name) {
        List connectors = Bootstrap.virtualMachineManager().allConnectors();
        Iterator iter = connectors.iterator();
        while (iter.hasNext()) {
            Connector connector = (Connector)iter.next();
            if (connector.name().equals(name)) {
                return connector;
            }
        }
        return null;
    }

    private Map parseConnectorArgs(Connector connector, String argString) {
        Map arguments = connector.defaultArguments();

        /*
         * We are parsing strings of the form:
         *    name1=value1,[name2=value2,...]
         * However, the value1...valuen substrings may contain
         * embedded comma(s), so make provision for quoting inside
         * the value substrings. (Bug ID 4285874)
         */
        String regexPattern =
            "(quote=[^,]+,)|" +           // special case for quote=.,
            "(\\w+=)" +                   // name=
            "(((\"[^\"]*\")|" +           //   ( "l , ue"
            "('[^']*')|" +                //     'l , ue'
            "([^,'\"]+))+,)";             //     v a l u e )+ ,
        Pattern p = Pattern.compile(regexPattern);
        Matcher m = p.matcher(argString);
        while (m.find()) {
            int startPosition = m.start();
            int endPosition = m.end();
            if (startPosition > 0) {
                /*
                 * It is an error if parsing skips over any part of argString.
                 */
                throw new IllegalArgumentException
                    (MessageOutput.format("Illegal connector argument",
                                          argString));
            }

            String token = argString.substring(startPosition, endPosition);
            int index = token.indexOf('=');
            String name = token.substring(0, index);
            String value = token.substring(index + 1,
                                           token.length() - 1); // Remove comma delimiter

            Connector.Argument argument = (Connector.Argument)arguments.get(name);
            if (argument == null) {
                throw new IllegalArgumentException
                    (MessageOutput.format("Argument is not defined for connector:",
                                          new Object [] {name, connector.name()}));
            }
            argument.setValue(value);

            argString = argString.substring(endPosition); // Remove what was just parsed...
            m = p.matcher(argString);                     //    and parse again on what is left.
        }
        if ((! argString.equals(",")) && (argString.length() > 0)) {
            /*
             * It is an error if any part of argString is left over,
             * unless it was empty to begin with.
             */
            throw new IllegalArgumentException
                (MessageOutput.format("Illegal connector argument", argString));
        }
        return arguments;
    }

    VMConnection(String connectSpec, int traceFlags) {
        String nameString;
        String argString;
        int index = connectSpec.indexOf(':');
        if (index == -1) {
            nameString = connectSpec;
            argString = "";
        } else {
            nameString = connectSpec.substring(0, index);
            argString = connectSpec.substring(index + 1);
        }

        connector = findConnector(nameString);
        if (connector == null) {
            throw new IllegalArgumentException
                (MessageOutput.format("No connector named:", nameString));
        } 

        connectorArgs = parseConnectorArgs(connector, argString);
        this.traceFlags = traceFlags;
    }
        
    synchronized VirtualMachine open() {
        if (connector instanceof LaunchingConnector) {
            vm = launchTarget();
        } else if (connector instanceof AttachingConnector) {
            vm = attachTarget();
        } else if (connector instanceof ListeningConnector) {
            vm = listenTarget();
        } else {
            throw new InternalError
                (MessageOutput.format("Invalid connect type"));
        }
        vm.setDebugTraceMode(traceFlags);
        if (vm.canBeModified()){
            setEventRequests(vm);
            resolveEventRequests();
        }
        /*
         * Now that the vm connection is open, fetch the debugee
         * classpath and set up a default sourcepath.
         * (Unless user supplied a sourcepath on the command line)
         * (Bug ID 4186582)
         */
        if (Env.getSourcePath().length() == 0) {
            if (vm instanceof PathSearchingVirtualMachine) {
                PathSearchingVirtualMachine psvm =
                    (PathSearchingVirtualMachine) vm;
                Env.setSourcePath(psvm.classPath());
            } else {
                Env.setSourcePath(".");
            }
        }

        return vm;
    }

    boolean setConnectorArg(String name, String value) {
        /*
         * Too late if the connection already made
         */
        if (vm != null) {
            return false;
        }

        Connector.Argument argument = (Connector.Argument)connectorArgs.get(name);
        if (argument == null) {
            return false;
        }
        argument.setValue(value);
        return true;
    }

    String connectorArg(String name) {
        Connector.Argument argument = (Connector.Argument)connectorArgs.get(name);
        if (argument == null) {
            return "";
        }
        return argument.value();
    }

    public synchronized VirtualMachine vm() {
        if (vm == null) {
            throw new VMNotConnectedException();
        } else {
            return vm;
        }
    }         

    boolean isOpen() {
        return (vm != null);
    }

    boolean isLaunch() {
        return (connector instanceof LaunchingConnector);
    }

    public void disposeVM() {
        try {
            if (vm != null) {
                vm.dispose();
                vm = null;
            }
        } finally {
            if (process != null) {
                process.destroy();
                process = null;
            }
            waitOutputComplete();
        }
    }

    private void setEventRequests(VirtualMachine vm) {
        EventRequestManager erm = vm.eventRequestManager();

        // Normally, we want all uncaught exceptions.  We request them
        // via the same mechanism as Commands.commandCatchException()
        // so the user can ignore them later if they are not
        // interested.
        // FIXME: this works but generates spurious messages on stdout
        //        during startup:
        //          Set uncaught java.lang.Throwable
        //          Set deferred uncaught java.lang.Throwable
        Commands evaluator = new Commands();
        evaluator.commandCatchException
            (new StringTokenizer("uncaught java.lang.Throwable"));

        ThreadStartRequest tsr = erm.createThreadStartRequest();
        tsr.enable();
        ThreadDeathRequest tdr = erm.createThreadDeathRequest();
        tdr.enable();
    }

    private void resolveEventRequests() {
        Env.specList.resolveAll();
    }

    private void dumpStream(InputStream stream) throws IOException {
        BufferedReader in = 
            new BufferedReader(new InputStreamReader(stream));
        int i;
        try {
            while ((i = in.read()) != -1) {
                   MessageOutput.printDirect((char)i);// Special case: use 
                                                      //   printDirect()
            }
        } catch (IOException ex) {
            String s = ex.getMessage();
            if (!s.startsWith("Bad file number")) {
                  throw ex;
            }
            // else we got a Bad file number IOException which just means
            // that the debuggee has gone away.  We'll just treat it the
            // same as if we got an EOF.
        }
    }

    /**	
     *	Create a Thread that will retrieve and display any output.
     *	Needs to be high priority, else debugger may exit before
     *	it can be displayed.
     */
    private void displayRemoteOutput(final InputStream stream) {
	Thread thr = new Thread("output reader") { 
	    public void run() {
                try {
                    dumpStream(stream);
                } catch (IOException ex) {
                    MessageOutput.fatalError("Failed reading output");
                } finally {
                    notifyOutputComplete();
                }
	    }
	};
	thr.setPriority(Thread.MAX_PRIORITY-1);
	thr.start();
    }

    private void dumpFailedLaunchInfo(Process process) {
        try {
            dumpStream(process.getErrorStream());
            dumpStream(process.getInputStream());
        } catch (IOException e) {
            MessageOutput.println("Unable to display process output:",
                                  e.getMessage());
        }
    }

    /* launch child target vm */
    private VirtualMachine launchTarget() {
        LaunchingConnector launcher = (LaunchingConnector)connector;
        try {
            VirtualMachine vm = launcher.launch(connectorArgs);
            process = vm.process();
            displayRemoteOutput(process.getErrorStream());
            displayRemoteOutput(process.getInputStream());
            return vm;
        } catch (IOException ioe) {
            ioe.printStackTrace();
            MessageOutput.fatalError("Unable to launch target VM.");
        } catch (IllegalConnectorArgumentsException icae) {
            icae.printStackTrace();
            MessageOutput.fatalError("Internal debugger error.");
        } catch (VMStartException vmse) {
            MessageOutput.println("vmstartexception", vmse.getMessage());
            MessageOutput.println();
            dumpFailedLaunchInfo(vmse.process());
            MessageOutput.fatalError("Target VM failed to initialize.");
        }
        return null; // Shuts up the compiler
    }

    /* attach to running target vm */
    private VirtualMachine attachTarget() {
        AttachingConnector attacher = (AttachingConnector)connector;
        try {
            return attacher.attach(connectorArgs);
        } catch (IOException ioe) {
            ioe.printStackTrace();
            MessageOutput.fatalError("Unable to attach to target VM.");
        } catch (IllegalConnectorArgumentsException icae) {
            icae.printStackTrace();
            MessageOutput.fatalError("Internal debugger error.");
        }
        return null; // Shuts up the compiler
    }

    /* listen for connection from target vm */
    private VirtualMachine listenTarget() {
        ListeningConnector listener = (ListeningConnector)connector;
        try {
            String retAddress = listener.startListening(connectorArgs);
            MessageOutput.println("Listening at address:", retAddress);
            vm = listener.accept(connectorArgs);
            listener.stopListening(connectorArgs);
            return vm;
        } catch (IOException ioe) {
            ioe.printStackTrace();
            MessageOutput.fatalError("Unable to attach to target VM.");
        } catch (IllegalConnectorArgumentsException icae) {
            icae.printStackTrace();
            MessageOutput.fatalError("Internal debugger error.");
        }
        return null; // Shuts up the compiler
    }
}
