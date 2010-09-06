/*
 * @(#)CommandTool.java	1.14 03/12/19
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

package com.sun.tools.example.debug.gui;

import java.io.*;
import java.util.*;

import javax.swing.*;
import java.awt.BorderLayout;
import java.awt.event.*;

import com.sun.jdi.*;
import com.sun.jdi.event.*;

import com.sun.tools.example.debug.bdi.*;
import com.sun.tools.example.debug.event.*;

public class CommandTool extends JPanel {

    private Environment env;

    private ContextManager context;
    private ExecutionManager runtime;
    private SourceManager sourceManager;

    private TypeScript script;

    private static final String DEFAULT_CMD_PROMPT = "Command:";

    public CommandTool(Environment env) {

	super(new BorderLayout());

	this.env = env;
	this.context = env.getContextManager();
	this.runtime = env.getExecutionManager();
	this.sourceManager = env.getSourceManager();

	script = new TypeScript(DEFAULT_CMD_PROMPT, false); //no echo
	this.add(script);

	final CommandInterpreter interpreter = 
	    new CommandInterpreter(env);

	// Establish handler for incoming commands.

	script.addActionListener(new ActionListener() {
	    public void actionPerformed(ActionEvent e) {
		interpreter.executeCommand(script.readln());
	    }
	});

	// Establish ourselves as the listener for VM diagnostics.
	
	OutputListener diagnosticsListener =
	    new TypeScriptOutputListener(script, true);
        runtime.addDiagnosticsListener(diagnosticsListener);

	// Establish ourselves as the shared debugger typescript.

	env.setTypeScript(new PrintWriter(new TypeScriptWriter(script)));

	// Handle VM events.

        TTYDebugListener listener = new TTYDebugListener(diagnosticsListener);
	
        runtime.addJDIListener(listener);
        runtime.addSessionListener(listener);
        runtime.addSpecListener(listener);
        context.addContextListener(listener);

        //### remove listeners on exit!
    	
    }

    private class TTYDebugListener implements
            JDIListener, SessionListener, SpecListener, ContextListener {

	private OutputListener diagnostics;

        TTYDebugListener(OutputListener diagnostics) {
	    this.diagnostics = diagnostics;
	}

        // JDIListener

        public void accessWatchpoint(AccessWatchpointEventSet e) {
            setThread(e);
            for (EventIterator it = e.eventIterator(); it.hasNext(); ) {
                Event evt = it.nextEvent();
                diagnostics.putString("Watchpoint hit: " + 
                                      locationString(e));
            }
	}

        public void classPrepare(ClassPrepareEventSet e) {
	    if (context.getVerboseFlag()) {
		String name = e.getReferenceType().name();
		diagnostics.putString("Class " + name + " loaded");
	    }
	}

        public void classUnload(ClassUnloadEventSet e) {
	    if (context.getVerboseFlag()) {
		diagnostics.putString("Class " + e.getClassName() + 
                                      " unloaded.");
	    }
	}

        public void exception(ExceptionEventSet e) {
            setThread(e);
	    String name = e.getException().referenceType().name();
	    diagnostics.putString("Exception: " + name);
	}

        public void locationTrigger(LocationTriggerEventSet e) {
            String locString = locationString(e);
            setThread(e);
            for (EventIterator it = e.eventIterator(); it.hasNext(); ) {
                Event evt = it.nextEvent();
                if (evt instanceof BreakpointEvent) {
                    diagnostics.putString("Breakpoint hit: " + locString);
                } else if (evt instanceof StepEvent) {
                    diagnostics.putString("Step completed: " + locString);
                } else if (evt instanceof MethodEntryEvent) {
                    diagnostics.putString("Method entered: " + locString);
                } else if (evt instanceof MethodExitEvent) {
                    diagnostics.putString("Method exited: " + locString);
                } else {
                    diagnostics.putString("UNKNOWN event: " + e);
                }
            }
	}

        public void modificationWatchpoint(ModificationWatchpointEventSet e) {
            setThread(e);
            for (EventIterator it = e.eventIterator(); it.hasNext(); ) {
                Event evt = it.nextEvent();
                diagnostics.putString("Watchpoint hit: " + 
                                      locationString(e));
            }
	}

        public void threadDeath(ThreadDeathEventSet e) {
	    if (context.getVerboseFlag()) {
		diagnostics.putString("Thread " + e.getThread() + 
                                      " ended.");
	    }
	}

        public void threadStart(ThreadStartEventSet e) {
	    if (context.getVerboseFlag()) {
		diagnostics.putString("Thread " + e.getThread() + 
                                      " started.");
	    }
	}

        public void vmDeath(VMDeathEventSet e) {
	    script.setPrompt(DEFAULT_CMD_PROMPT);
	    diagnostics.putString("VM exited");
        }

        public void vmDisconnect(VMDisconnectEventSet e) {
	    script.setPrompt(DEFAULT_CMD_PROMPT);
	    diagnostics.putString("Disconnected from VM");
        }

        public void vmStart(VMStartEventSet e) {
	    script.setPrompt(DEFAULT_CMD_PROMPT);
	    diagnostics.putString("VM started");
	}

        // SessionListener

        public void sessionStart(EventObject e) {}

        public void sessionInterrupt(EventObject e) {
            Thread.yield();  // fetch output
	    diagnostics.putString("VM interrupted by user.");
	    script.setPrompt(DEFAULT_CMD_PROMPT);
	}

        public void sessionContinue(EventObject e) {
	    diagnostics.putString("Execution resumed.");
	    script.setPrompt(DEFAULT_CMD_PROMPT);
	}

        // SpecListener

        public void breakpointSet(SpecEvent e) {
            EventRequestSpec spec = e.getEventRequestSpec();
            diagnostics.putString("Breakpoint set at " + spec + ".");
        }
        public void breakpointDeferred(SpecEvent e) {
            EventRequestSpec spec = e.getEventRequestSpec();
            diagnostics.putString("Breakpoint will be set at " +
                                  spec + " when its class is loaded.");
        }
        public void breakpointDeleted(SpecEvent e) {
            EventRequestSpec spec = e.getEventRequestSpec();
	    diagnostics.putString("Breakpoint at " + spec.toString() + " deleted.");
        }
        public void breakpointResolved(SpecEvent e) {
            EventRequestSpec spec = e.getEventRequestSpec();
	    diagnostics.putString("Breakpoint resolved to " + spec.toString() + ".");
        }
        public void breakpointError(SpecErrorEvent e) {
            EventRequestSpec spec = e.getEventRequestSpec();
	    diagnostics.putString("Deferred breakpoint at " +
				  spec + " could not be resolved:" +
                                  e.getReason());
        }

//### Add info for watchpoints and exceptions

        public void watchpointSet(SpecEvent e) {
        }
        public void watchpointDeferred(SpecEvent e) {
        }
        public void watchpointDeleted(SpecEvent e) {
        }
        public void watchpointResolved(SpecEvent e) {
        }
        public void watchpointError(SpecErrorEvent e) {
        }

        public void exceptionInterceptSet(SpecEvent e) {
        }
        public void exceptionInterceptDeferred(SpecEvent e) {
        }
        public void exceptionInterceptDeleted(SpecEvent e) {
        }
        public void exceptionInterceptResolved(SpecEvent e) {
        }
        public void exceptionInterceptError(SpecErrorEvent e) {
        }


	// ContextListener.

	// If the user selects a new current thread or frame, update prompt.

	public void currentFrameChanged(CurrentFrameChangedEvent e) {
	    // Update prompt only if affect thread is current.
	    ThreadReference thread = e.getThread();
	    if (thread == context.getCurrentThread()) {
		script.setPrompt(promptString(thread, e.getIndex()));
	    }
        }

    }

    private String locationString(LocatableEventSet e) {
        Location loc = e.getLocation();
        return "thread=\"" + e.getThread().name() +
            "\", " + Utils.locationString(loc);
    }

    private void setThread(LocatableEventSet e) {
        if (!e.suspendedNone()) {
            Thread.yield();  // fetch output
            script.setPrompt(promptString(e.getThread(), 0));
            //### Current thread should be set elsewhere, e.g.,
            //### in ContextManager
            //### context.setCurrentThread(thread);
        }
    }
						  
    private String promptString(ThreadReference thread, int frameIndex) {
        if (thread == null) {
	    return DEFAULT_CMD_PROMPT;
        } else {
	    // Frame indices are presented to user as indexed from 1.
	    return (thread.name() + "[" + (frameIndex + 1) + "]:");
	}
    }
}




