/*
 * @(#)ChildSession.java	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*
 * Copyright (c) 1997-1999 by Sun Microsystems, Inc. All Rights Reserved.
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

package com.sun.tools.example.debug.bdi;

import com.sun.jdi.*;
import com.sun.jdi.connect.LaunchingConnector;
import com.sun.jdi.connect.Connector;
import com.sun.jdi.connect.VMStartException;
import com.sun.jdi.connect.IllegalConnectorArgumentsException;
import java.io.*;
import java.util.Map;
import javax.swing.SwingUtilities;


class ChildSession extends Session {

    private Process process;

    private PrintWriter in;
    private BufferedReader out;
    private BufferedReader err;

    private InputWriter inputWriter;
    private OutputReader outputReader;
    private OutputReader errorReader;

    private InputListener input;
    private OutputListener output;
    private OutputListener error;

    public ChildSession(ExecutionManager runtime,
			String userVMArgs, String cmdLine,
			InputListener input,
			OutputListener output,
			OutputListener error,
			OutputListener diagnostics) {
	this(runtime, getVM(diagnostics, userVMArgs, cmdLine), 
	     input, output, error, diagnostics);
    }
	
    public ChildSession(ExecutionManager runtime,
			LaunchingConnector connector, Map arguments,
			InputListener input,
			OutputListener output,
			OutputListener error,
			OutputListener diagnostics) {
	this(runtime, generalGetVM(diagnostics, connector, arguments), 
	     input, output, error, diagnostics);
    }
	
    private ChildSession(ExecutionManager runtime,
			VirtualMachine vm,
			InputListener input,
			OutputListener output,
			OutputListener error,
			OutputListener diagnostics) {
	super(vm, runtime, diagnostics);
	this.input = input;
	this.output = output;
	this.error = error;
    }
	
    public boolean attach() {

	if (!connectToVMProcess()) {
	    diagnostics.putString("Could not launch VM");
	    return false;
	}

	/*
	 * Create a Thread that will retrieve and display any output.
	 * Needs to be high priority, else debugger may exit before
	 * it can be displayed.
	 */

	//### Rename InputWriter and OutputReader classes
	//### Thread priorities cribbed from ttydebug.  Think about them.

	OutputReader outputReader =
	    new OutputReader("output reader", "output", 
			     out, output, diagnostics);
	outputReader.setPriority(Thread.MAX_PRIORITY-1);
	outputReader.start();

	OutputReader errorReader =
	    new OutputReader("error reader", "error",
			     err, error, diagnostics);
	errorReader.setPriority(Thread.MAX_PRIORITY-1);
	errorReader.start();

	InputWriter inputWriter =
	    new InputWriter("input writer", in, input);
	inputWriter.setPriority(Thread.MAX_PRIORITY-1);
	inputWriter.start();

	if (!super.attach()) {
	    if (process != null) {
		process.destroy();
		process = null;
	    }
	    return false;
	}

	//### debug
	//System.out.println("IO after attach: "+ inputWriter + " " + outputReader + " "+ errorReader);

	return true;
    }

    public void detach() {

	//### debug
	//System.out.println("IO before detach: "+ inputWriter + " " + outputReader + " "+ errorReader);

	super.detach();

	/*
	inputWriter.quit();
	outputReader.quit();
	errorReader.quit();
	*/

        if (process != null) {
            process.destroy();
            process = null;
        }

    }

    /**
     * Launch child java interpreter, return host:port
     */

    static private void dumpStream(OutputListener diagnostics,
				   InputStream stream) throws IOException {
        BufferedReader in = 
            new BufferedReader(new InputStreamReader(stream));
        String line;
        while ((line = in.readLine()) != null) {
            diagnostics.putString(line);
        }
    }

    static private void dumpFailedLaunchInfo(OutputListener diagnostics,
					     Process process) {
        try {
            dumpStream(diagnostics, process.getErrorStream());
            dumpStream(diagnostics, process.getInputStream());
        } catch (IOException e) {
            diagnostics.putString("Unable to display process output: " +
                                  e.getMessage());
        }
    }

    static private VirtualMachine getVM(OutputListener diagnostics,
					String userVMArgs,
					String cmdLine) {
        VirtualMachineManager manager = Bootstrap.virtualMachineManager();
        LaunchingConnector connector = manager.defaultConnector();
        Map arguments = connector.defaultArguments();
        ((Connector.Argument)arguments.get("options")).setValue(userVMArgs);
        ((Connector.Argument)arguments.get("main")).setValue(cmdLine);
        return generalGetVM(diagnostics, connector, arguments);
    }

    static private VirtualMachine generalGetVM(OutputListener diagnostics,
                                               LaunchingConnector connector, 
                                               Map arguments) {
        VirtualMachine vm = null;
        try {
            diagnostics.putString("Starting child.");
            vm = connector.launch(arguments);
        } catch (IOException ioe) {
            diagnostics.putString("Unable to start child: " + ioe.getMessage());
        } catch (IllegalConnectorArgumentsException icae) {
            diagnostics.putString("Unable to start child: " + icae.getMessage());
        } catch (VMStartException vmse) {
            diagnostics.putString("Unable to start child: " + vmse.getMessage() + '\n');
            dumpFailedLaunchInfo(diagnostics, vmse.process());
        }
        return vm;
    }

    private boolean connectToVMProcess() {
        if (vm == null) {
            return false;
        }
        process = vm.process();
        in = new PrintWriter(new OutputStreamWriter(process.getOutputStream()));
        //### Note small buffer sizes!
        out = new BufferedReader(new InputStreamReader(process.getInputStream()), 1);
        err = new BufferedReader(new InputStreamReader(process.getErrorStream()), 1);
        return true;
    }

    /**	
     *	Threads to handle application input/output.
     */

    private static class OutputReader extends Thread {

	private String streamName;
	private BufferedReader stream;
	private OutputListener output;
	private OutputListener diagnostics;
	private boolean running = true;
	private char[] buffer = new char[512];

	OutputReader(String threadName,
		     String streamName,
		     BufferedReader stream,
		     OutputListener output,
		     OutputListener diagnostics) {
	    super(threadName);
	    this.streamName = streamName;
	    this.stream = stream;
	    this.output = output;
	    this.diagnostics = diagnostics;
	}

	public void quit() {
	    running = false;
	}

	public void run() {
	    try {
		int count;
		while (running && (count = stream.read(buffer, 0, 512)) != -1) {
		    if (count > 0) {
			// Run in Swing event dispatcher thread.
			final String chars = new String(buffer, 0, count);
			SwingUtilities.invokeLater(new Runnable() {
			    public void run() {
				output.putString(chars);
			    }
			});
		    }
		    //### Should we sleep briefly here?
		}
	    } catch (IOException e) {
		// Run in Swing event dispatcher thread.
		SwingUtilities.invokeLater(new Runnable() {
		    public void run() {
			diagnostics.putString("IO error reading " +
					      streamName +
					      " stream of child java interpreter");
		    }
		});
	    }
	}
    }

    private static class InputWriter extends Thread {

	private PrintWriter stream;
	private InputListener input;
	private boolean running = true;

	InputWriter(String threadName, 
		    PrintWriter stream,
		    InputListener input) {
	    super(threadName);
	    this.stream = stream;
	    this.input = input;
	}

	public void quit() {
	    //### Won't have much effect if blocked on input!
	    running = false;
	}

	public void run() {
	    String line;
	    while (running) {
		line = input.getLine();
		stream.println(line);
		// Should not be needed for println above!
		stream.flush();
	    }
	}
    }
    
}


