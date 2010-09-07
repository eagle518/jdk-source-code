/*
 * @(#)ConsoleWindow.java	1.99 08/03/12
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.util;

import java.awt.BorderLayout;
import java.awt.FlowLayout;
import java.awt.Insets;
import java.awt.Frame;
import java.awt.Dialog;
import java.awt.Rectangle;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.KeyEvent;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import javax.swing.JButton;
import javax.swing.JComponent;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JRootPane;
import javax.swing.JScrollBar;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JViewport;
import javax.swing.KeyStroke;
import javax.swing.WindowConstants;
import java.text.MessageFormat;
import java.util.TreeSet;
import java.util.Collection;
import java.util.Iterator;
import com.sun.deploy.resources.ResourceManager;
import com.sun.deploy.util.DeploySysAction;
import com.sun.deploy.util.DeploySysRun;
import com.sun.deploy.util.Trace;
import com.sun.deploy.util.TraceLevel;
import com.sun.deploy.config.Config;
import com.sun.deploy.ui.UIFactory;
import java.util.StringTokenizer;
import javax.swing.text.Document;
import javax.swing.text.PlainDocument;


/*
 * Java Console is used for displaying messages sent by the 
 * running applets, JavaBeans, JNLP apps, plugin/webstart
 * insternal/
 *
 * @version 1.0
 * @author Stanley Man-Kit Ho
 */

public final class ConsoleWindow extends JFrame
{
    // ConsoleWindowStrategy object controls the behavior of the console
    // during user interaction.
    //
    private final ConsoleController controller;

    public static ConsoleWindow create(final ConsoleController controller)
    {
	return (ConsoleWindow) DeploySysRun.execute(new DeploySysAction() {
	    public Object execute() {
	        return new ConsoleWindow(controller);
	    }
	}, null);
    }

    /*
     * Create the new console window
     */
    public ConsoleWindow(final ConsoleController controller)
    {
    	super(ResourceManager.getMessage("console.caption"));
	if (Config.isJavaVersionAtLeast16()) {
	    setModalExclusionType(Dialog.ModalExclusionType.TOOLKIT_EXCLUDE);
	}

	this.controller = controller;

        Rectangle screen = UIFactory.getMouseScreenBounds();
        setBounds(screen.x, screen.y, 450, 400);
	setResizable(true);

	// Make sure we don't do anything exception in the listener.
	//
	setDefaultCloseOperation(WindowConstants.DO_NOTHING_ON_CLOSE);

	getContentPane().setLayout(new BorderLayout());

	textArea = new JTextArea();
	textArea.setFont(ResourceManager.getUIFont());
	textArea.setEditable(false);
	textArea.setMargin(new Insets(0, 5, 0, 0));
	scroller = new JScrollPane(textArea);

	// only setScrollMode if running JRE 1.3+
	if (System.getProperty("java.version").startsWith("1.2") == false) {
	    JViewport viewport = scroller.getViewport();
	    viewport.setScrollMode(JViewport.BLIT_SCROLL_MODE);
	}

	sbVer = scroller.getVerticalScrollBar();
	sbHor = scroller.getHorizontalScrollBar();
	getContentPane().add(scroller, BorderLayout.CENTER);

	ActionListener dumpThreadStack = new ActionListener()
	{
	    public void actionPerformed(ActionEvent e)
	    {
		StringBuffer buffer = new StringBuffer();

		buffer.append(
		    ResourceManager.getMessage("console.dump.stack"));
		buffer.append(
		    ResourceManager.getMessage("console.menu.text.top"));
		buffer.append(controller.dumpAllStacks());
		buffer.append(
		    ResourceManager.getMessage("console.menu.text.tail"));
		buffer.append(ResourceManager.getMessage("console.done"));

		System.out.println(buffer.toString());
	    }
	};

	ActionListener showThreads = new ActionListener()
	{
	    public void actionPerformed(ActionEvent e)
	    {
		StringBuffer buffer = new StringBuffer();

		buffer.append(
		    ResourceManager.getMessage("console.dump.thread"));

		Thread t = Thread.currentThread();
		ThreadGroup tg = controller.getMainThreadGroup();
		ConsoleHelper.dumpThreadGroup(tg, buffer);

		buffer.append(ResourceManager.getMessage("console.done"));

		System.out.println(buffer.toString());
	    }
	};

	ActionListener reloadPolicyConfig = new ActionListener()
	{
	    public void actionPerformed(ActionEvent e)
	    {
		System.out.print(
		    ResourceManager.getMessage("console.reload.policy"));
		controller.reloadSecurityPolicy();
		System.out.println(
		    ResourceManager.getMessage("console.completed"));
	    }
	};

	ActionListener reloadProxyConfig = new ActionListener()
	{
	    public void actionPerformed(ActionEvent e)
	    {
		System.out.println(
		    ResourceManager.getMessage("console.reload.proxy"));
		controller.reloadProxyConfig();
		System.out.println(
		    ResourceManager.getMessage("console.done"));
	    }
	};

	ActionListener showSystemProperties = new ActionListener()
	{
	    public void actionPerformed(ActionEvent e)
	    {
                // CR 6850604:
                // The list of properties can be relatively long. We truncate
                // console output to prevent OOM conditions, i.e. when
                // (accidentally) trying to dump a JAR file there. See
                // 6742564 for example. In order to avoid the properties
                // list to be truncated, we split it up by lines and print
                // each line separately.
                String props = ConsoleHelper.displaySystemProperties();
                StringTokenizer tokens = new StringTokenizer(props, "\n");
                while (tokens.hasMoreElements()) {
                    System.out.println(tokens.nextElement());
                }
	    }
	};

	ActionListener showHelp = new ActionListener()
	{
	    public void actionPerformed(ActionEvent e)
	    {
		String helpString = ConsoleHelper.displayHelp();
		System.out.print(helpString);
	    }
	};

	ActionListener showClassLoaderCache = new ActionListener()
	{
	    public void actionPerformed(ActionEvent e)
	    {
		System.out.println(controller.dumpClassLoaders());
	    }
	};

	ActionListener clearClassLoaderCache = new ActionListener()
	{
	    public void actionPerformed(ActionEvent e)
	    {
		controller.clearClassLoaders();

    		System.out.println(
		    ResourceManager.getMessage("console.clear.classloader"));
	    }
	};

	ActionListener clearConsole = new ActionListener()
	{
	    public void actionPerformed(ActionEvent e)
	    {
                PerfLogger.setEndTime("End at clear console");
                PerfLogger.outputLog();
		// Add header in the beginning of Console window
		String strVersion = ConsoleHelper.displayVersion();
		String strHelp = ConsoleHelper.displayHelp();
		String strHeader = strVersion + "\n" + strHelp;

                Document document = new PlainDocument();
                textArea.setDocument(document);

		textArea.setText(strHeader);

 		//textArea.setText("");
		textArea.revalidate();
	    }
	};

	ActionListener copyConsole = new ActionListener()
	{
	    public void actionPerformed(ActionEvent e)
	    {
		// Obtain current text selections
		int selectionStart = textArea.getSelectionStart();
		int selectionEnd = textArea.getSelectionEnd();

		// If no selection, copy all
		if (selectionEnd - selectionStart <= 0)
		    textArea.selectAll();

		// Copy text to clipboard
		textArea.copy();
	    }
	};

	ActionListener closeConsole = new ActionListener()
	{
	    public void actionPerformed(ActionEvent e)
	    {
                PerfLogger.setEndTime("End at close console");
                PerfLogger.outputLog();
		if (controller.isIconifiedOnClose())
		{
		    // On other platform with Netscape 4.x, closing the 
		    // window will only iconified the console.
		    //
		    setState(Frame.ICONIFIED);
		} else {
		    setVisible(false);
		    dispose();
		}
                controller.notifyConsoleClosed();
	    }
	};

	final ActionListener showMemory = new ActionListener()
	{
	    public void actionPerformed(ActionEvent e)
	    {
		long freeMemory = Runtime.getRuntime().freeMemory() / 1024;
		long totalMemory = Runtime.getRuntime().totalMemory() / 1024;
		long percentFree = 
		    (long) (100.0 / (((double)totalMemory) / freeMemory));

		MessageFormat mf = new MessageFormat(
		    ResourceManager.getMessage("console.memory"));

		Object[] args = { 
		    new Long(totalMemory), 
		    new Long(freeMemory), 
		    new Long(percentFree) 
		};

    		System.out.print(mf.format(args));
		System.out.println(
		    ResourceManager.getMessage("console.completed"));
	    }
	};


	ActionListener runFinalize = new ActionListener()
	{
	    public void actionPerformed(ActionEvent e)
	    {
		System.out.print(
		    ResourceManager.getMessage("console.finalize"));
		System.runFinalization();
		System.out.println(
		    ResourceManager.getMessage("console.completed"));

		// Display memory after finalization
		showMemory.actionPerformed(e);
	    }
	};


	ActionListener runGC = new ActionListener()
	{
	    public void actionPerformed(ActionEvent e)
	    {
    		System.out.print(ResourceManager.getMessage("console.gc"));
		System.gc();
		System.out.println(
		    ResourceManager.getMessage("console.completed"));
		// Display memory after garbage collection
		showMemory.actionPerformed(e);
	    }
	};

	ActionListener traceLevel0 = new ActionListener()
	{
	    public void actionPerformed(ActionEvent e)
	    {
		Trace.setBasicTrace(false);
		Trace.setCacheTrace(false);
		Trace.setNetTrace(false);
		Trace.setSecurityTrace(false);
		Trace.setExtTrace(false);
		Trace.setLiveConnectTrace(false);
		Trace.setTempTrace(false);
    		System.out.println(
		    ResourceManager.getMessage("console.trace.level.0"));
	    }
	};

	ActionListener traceLevel1 = new ActionListener()
	{
	    public void actionPerformed(ActionEvent e)
	    {
		Trace.setBasicTrace(true);
		Trace.setCacheTrace(false);
		Trace.setNetTrace(false);
		Trace.setSecurityTrace(false);
		Trace.setExtTrace(false);
		Trace.setLiveConnectTrace(false);
		Trace.setTempTrace(false);
    		System.out.println(
		    ResourceManager.getMessage("console.trace.level.1"));
	    }
	};

	ActionListener traceLevel2 = new ActionListener()
	{
	    public void actionPerformed(ActionEvent e)
	    {
		Trace.setBasicTrace(true);
		Trace.setCacheTrace(true);
		Trace.setNetTrace(true);
		Trace.setSecurityTrace(false);
		Trace.setExtTrace(false);
		Trace.setLiveConnectTrace(false);
		Trace.setTempTrace(false);
    		System.out.println(
		    ResourceManager.getMessage("console.trace.level.2"));
	    }
	};

	ActionListener traceLevel3 = new ActionListener()
	{
	    public void actionPerformed(ActionEvent e)
	    {
		Trace.setBasicTrace(true);
		Trace.setCacheTrace(true);
		Trace.setNetTrace(true);
		Trace.setSecurityTrace(true);
		Trace.setExtTrace(false);
		Trace.setLiveConnectTrace(false);
		Trace.setTempTrace(false);
    		System.out.println(
		    ResourceManager.getMessage("console.trace.level.3"));
	    }
	};

	ActionListener traceLevel4 = new ActionListener()
	{
	    public void actionPerformed(ActionEvent e)
	    {
		Trace.setBasicTrace(true);
		Trace.setCacheTrace(true);
		Trace.setNetTrace(true);
		Trace.setSecurityTrace(true);
		Trace.setExtTrace(true);
		Trace.setLiveConnectTrace(false);
		Trace.setTempTrace(false);
    		System.out.println(
		    ResourceManager.getMessage("console.trace.level.4"));
	    }
	};

	ActionListener traceLevel5 = new ActionListener()
	{
	    public void actionPerformed(ActionEvent e)
	    {
		Trace.setBasicTrace(true);
		Trace.setCacheTrace(true);
		Trace.setNetTrace(true);
		Trace.setSecurityTrace(true);
		Trace.setExtTrace(true);
		Trace.setLiveConnectTrace(true);
		Trace.setTempTrace(true);
    		System.out.println(
		    ResourceManager.getMessage("console.trace.level.5"));
	    }
	};

	ActionListener logging = new ActionListener() {
	    public void actionPerformed(ActionEvent e)
	    {
		System.out.println(
		    ResourceManager.getMessage("console.log")
		        + controller.toggleLogging()
		        + ResourceManager.getMessage("console.completed"));
	    }
	};

	if (controller.isJCovSupported()) {
	    ActionListener runJcov = new ActionListener()
	    {
		public void actionPerformed(ActionEvent e)
	        {
		    if (controller.dumpJCovData() == false)
		    {
			System.out.println(ResourceManager.getMessage(
						"console.jcov.error"));
		    }
		    else
		    {
			System.out.println(ResourceManager.getMessage(
						"console.jcov.info"));
		    }
		 }
	    };

	    textArea.registerKeyboardAction(runJcov,
			KeyStroke.getKeyStroke(KeyEvent.VK_J, 0),
			JComponent.WHEN_IN_FOCUSED_WINDOW	);
	}

	if (controller.isDumpStackSupported()) {
	    textArea.registerKeyboardAction(dumpThreadStack,
			KeyStroke.getKeyStroke(KeyEvent.VK_V, 0),
			JComponent.WHEN_IN_FOCUSED_WINDOW	);
	}

	if (controller.isProxyConfigReloadSupported()) {
	    textArea.registerKeyboardAction(reloadProxyConfig,
			KeyStroke.getKeyStroke(KeyEvent.VK_P, 0),
			JComponent.WHEN_IN_FOCUSED_WINDOW	);
	}

	if (controller.isSecurityPolicyReloadSupported()) {
	    textArea.registerKeyboardAction(reloadPolicyConfig,
			KeyStroke.getKeyStroke(KeyEvent.VK_R, 0),
			JComponent.WHEN_IN_FOCUSED_WINDOW	);
	}

	if (controller.isClearClassLoaderSupported()) {
	    textArea.registerKeyboardAction(clearClassLoaderCache,
			KeyStroke.getKeyStroke(KeyEvent.VK_X, 0),
			JComponent.WHEN_IN_FOCUSED_WINDOW	);
	}

	if (controller.isDumpClassLoaderSupported()) {
	    textArea.registerKeyboardAction(showClassLoaderCache,
			KeyStroke.getKeyStroke(KeyEvent.VK_L, 0),
			JComponent.WHEN_IN_FOCUSED_WINDOW	);
	}

	if (controller.isLoggingSupported()) {
	    textArea.registerKeyboardAction(logging,
			KeyStroke.getKeyStroke(KeyEvent.VK_O, 0),
			JComponent.WHEN_IN_FOCUSED_WINDOW	);
	}

	textArea.registerKeyboardAction(showThreads,
		KeyStroke.getKeyStroke(KeyEvent.VK_T, 0),
		JComponent.WHEN_IN_FOCUSED_WINDOW	);
	textArea.registerKeyboardAction(showSystemProperties,
		KeyStroke.getKeyStroke(KeyEvent.VK_S, 0),
		JComponent.WHEN_IN_FOCUSED_WINDOW	);
	textArea.registerKeyboardAction(showHelp,
		KeyStroke.getKeyStroke(KeyEvent.VK_H, 0),
		JComponent.WHEN_IN_FOCUSED_WINDOW	);
	textArea.registerKeyboardAction(showMemory,
		KeyStroke.getKeyStroke(KeyEvent.VK_M, 0),
		JComponent.WHEN_IN_FOCUSED_WINDOW	);
	textArea.registerKeyboardAction(clearConsole,
		KeyStroke.getKeyStroke(KeyEvent.VK_C, 0),
		JComponent.WHEN_IN_FOCUSED_WINDOW	);
	textArea.registerKeyboardAction(runGC,
		KeyStroke.getKeyStroke(KeyEvent.VK_G, 0),
		JComponent.WHEN_IN_FOCUSED_WINDOW	);
	textArea.registerKeyboardAction(runFinalize,
		KeyStroke.getKeyStroke(KeyEvent.VK_F, 0),
		JComponent.WHEN_IN_FOCUSED_WINDOW	);
	textArea.registerKeyboardAction(closeConsole,
		KeyStroke.getKeyStroke(KeyEvent.VK_Q, 0),
		JComponent.WHEN_IN_FOCUSED_WINDOW	);
	textArea.registerKeyboardAction(traceLevel0,
		KeyStroke.getKeyStroke(KeyEvent.VK_0, 0),
		JComponent.WHEN_IN_FOCUSED_WINDOW	);
	textArea.registerKeyboardAction(traceLevel1,
		KeyStroke.getKeyStroke(KeyEvent.VK_1, 0),
		JComponent.WHEN_IN_FOCUSED_WINDOW	);
	textArea.registerKeyboardAction(traceLevel2,
		KeyStroke.getKeyStroke(KeyEvent.VK_2, 0),
		JComponent.WHEN_IN_FOCUSED_WINDOW	);
	textArea.registerKeyboardAction(traceLevel3,
		KeyStroke.getKeyStroke(KeyEvent.VK_3, 0),
		JComponent.WHEN_IN_FOCUSED_WINDOW	);
	textArea.registerKeyboardAction(traceLevel4,
		KeyStroke.getKeyStroke(KeyEvent.VK_4, 0),
		JComponent.WHEN_IN_FOCUSED_WINDOW	);
	textArea.registerKeyboardAction(traceLevel5,
		KeyStroke.getKeyStroke(KeyEvent.VK_5, 0),
		JComponent.WHEN_IN_FOCUSED_WINDOW	);


	/* Get our resource strings.  We are guaranteed that ResourceManager
         * has already loaded its resource because the same method that
         * did the resource loading subsequently calls showConsoleWindow.
         */
	JButton clear = new JButton(
		ResourceManager.getMessage("console.clear"));
	clear.setMnemonic(
		ResourceManager.getAcceleratorKey("console.clear"));
	JButton copy = new JButton(
		ResourceManager.getMessage("console.copy"));
	copy.setMnemonic(
		ResourceManager.getAcceleratorKey("console.copy"));
	JButton close = new JButton(
		ResourceManager.getMessage("console.close"));
	close.setMnemonic(
		ResourceManager.getAcceleratorKey("console.close"));

	JPanel panel = new JPanel();
	panel.setLayout(new FlowLayout(FlowLayout.CENTER));
	panel.add(clear);
	panel.add(new JLabel("    "));
	panel.add(copy);
	panel.add(new JLabel("    "));
	panel.add(close);

	getContentPane().add(panel, BorderLayout.SOUTH);

        addWindowListener(new WindowAdapter() {
 		public void windowClosing(WindowEvent e) {
		    if (controller.isIconifiedOnClose()) {
			// On other platform, closing the window
			// will only iconified the console.
			//
			setState(Frame.ICONIFIED);
		    } else {
			setVisible(false);
		        dispose();
		    }
                    controller.notifyConsoleClosed();
		}
	});

	clear.addActionListener(clearConsole);
	copy.addActionListener(copyConsole);
	close.addActionListener(closeConsole);
    }

    /**
     * <p>
     * Prints an output message to the console window
     * </p>
     *
     * @param text text to be printed
     */
    public void append(final String text)
    {
	// We need to post the event to the swing component's event queue
	controller.invokeLater(new Runnable(){
	    public void run() {
		// If the size exceed limit, it wipe the required
		// portion of text from the beginning.
		int diff = textArea.getText().length() + 
					text.length() - TEXT_LIMIT;
		if (diff > 0){
		    textArea.replaceRange("",0,diff);
		}

		textArea.append(text);
		textArea.revalidate();
		setScrollPosition();
	    }
	 });
    }

    /**
     * Set the horizontal and vertical scroll bar positions
     */
    public void setScrollPosition()
    {
 	scroller.validate();
	sbVer.setValue(sbVer.getMaximum());
	sbHor.setValue(sbHor.getMinimum());
    }


    /**
     * Show Java Console Window
     */
    public void showConsole(boolean visible)
    {
	if (controller.isIconifiedOnClose()) {
	    // For iconified state, visible means normal
	    // windows. invisible means iconified windows.
	    //
	    if (visible) {
		setState(Frame.NORMAL);
	    } else {
		setState(Frame.ICONIFIED);
	    }
	    setVisible(true);
	} else {
	    // Caution: changing the order of the following
	    // call may cause deadlock in the browser because
	    // of the way taskbar notification is handled in
	    // the platform.
	    //
	    if (isVisible() != visible) {
		setVisible(visible);
	    }

	    if (visible) {
		toFront();
	    } else {
		dispose();
	    }
	}
    }


    /**
     * Return true if console is visible.
     */
    public boolean isConsoleVisible()
    {
	if (controller.isIconifiedOnClose()) {
	    // For iconified state, visible means normal
	    // windows. invisible means iconified windows.
	    return (getState() == Frame.NORMAL);
	} else {
	    // Otherwise, visible means the windows is actually
	    // visible.
	    return isVisible();
	}
    }

    public JTextArea getTextArea() {
	return textArea;
    }

    private JTextArea textArea;
    private JScrollPane scroller;
    private JScrollBar sbHor, sbVer;

    /* Text limited size to show on console window */
    private static final int TEXT_LIMIT = 0xFFFFF;
}
