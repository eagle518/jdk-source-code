/*
 * @(#)RawCommandLineLauncher.java	1.15 04/05/05
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.tools.jdi;

import com.sun.tools.jdi.*;
import com.sun.jdi.connect.*;
import com.sun.jdi.connect.spi.*;
import com.sun.jdi.VirtualMachine;
import java.util.Map;
import java.io.IOException;

public class RawCommandLineLauncher extends AbstractLauncher implements LaunchingConnector {

    static private final String ARG_COMMAND = "command";
    static private final String ARG_ADDRESS = "address";
    static private final String ARG_QUOTE   = "quote";

    TransportService transportService;
    Transport transport;

    public TransportService transportService() {
	return transportService;
    }

    public Transport transport() {
	return transport;
    }

    public RawCommandLineLauncher() {
	super();

	try {
	    Class c = Class.forName("com.sun.tools.jdi.SharedMemoryTransportService");
	    transportService = (TransportService)c.newInstance();
	    transport = new Transport() {
	 	public String name() {
		    return "dt_shmem";
	 	}
	    };
	} catch (ClassNotFoundException x) { 
	} catch (UnsatisfiedLinkError x) { 
	} catch (InstantiationException x) {
	} catch (IllegalAccessException x) {
	};

	if (transportService == null) {
	    transportService = new SocketTransportService();
	    transport = new Transport() {
		public String name() {
		    return "dt_socket";
		}
	    };
	}

        addStringArgument(
                ARG_COMMAND,
                getString("raw.command.label"),
                getString("raw.command"),
                "",
                true);
        addStringArgument(
                ARG_QUOTE,
                getString("raw.quote.label"),
                getString("raw.quote"),
                "\"",
                true);

	addStringArgument(
                ARG_ADDRESS,
                getString("raw.address.label"),
                getString("raw.address"),
		"",
                true);
    }


    public VirtualMachine
	launch(Map<String,? extends Connector.Argument> arguments)
	throws IOException, IllegalConnectorArgumentsException,
	       VMStartException
    {
        String command = argument(ARG_COMMAND, arguments).value();
        String address = argument(ARG_ADDRESS, arguments).value();

        String quote = argument(ARG_QUOTE, arguments).value();

        if (quote.length() > 1) {
            throw new IllegalConnectorArgumentsException("Invalid length", 
                                                         ARG_QUOTE);
        }

	TransportService.ListenKey listener = transportService.startListening(address);

        try {
            return launch(tokenizeCommand(command, quote.charAt(0)), 
                          address, listener, transportService);
        } finally {
            transportService.stopListening(listener);
        }
    }

    public String name() {
        return "com.sun.jdi.RawCommandLineLaunch";
    }

    public String description() {
        return getString("raw.description");
    }
}

