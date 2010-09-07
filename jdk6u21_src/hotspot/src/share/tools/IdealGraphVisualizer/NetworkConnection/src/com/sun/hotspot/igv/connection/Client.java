/*
 * Copyright (c) 1998, 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */
package com.sun.hotspot.igv.connection;

import com.sun.hotspot.igv.data.Group;
import com.sun.hotspot.igv.data.services.GroupCallback;
import com.sun.hotspot.igv.data.serialization.Parser;
import com.sun.hotspot.igv.data.Properties.RegexpPropertyMatcher;
import java.io.IOException;
import java.io.InputStream;
import java.net.Socket;
import javax.swing.JTextField;
import org.openide.util.Exceptions;
import org.openide.xml.XMLUtil;
import org.xml.sax.InputSource;
import org.xml.sax.SAXException;
import org.xml.sax.XMLReader;

/**
 *
 * @author Thomas Wuerthinger
 */
public class Client implements Runnable, GroupCallback {

    private Socket socket;
    private JTextField networkTextField;
    private GroupCallback callback;

    public Client(Socket socket, JTextField networkTextField, GroupCallback callback) {
        this.callback = callback;
        this.socket = socket;
        this.networkTextField = networkTextField;
    }

    public void run() {

        try {
            InputStream inputStream = socket.getInputStream();

            if (networkTextField.isEnabled()) {

                socket.getOutputStream().write('y');
                InputSource is = new InputSource(inputStream);

                try {
                    XMLReader reader = XMLUtil.createXMLReader();
                    Parser parser = new Parser(this);
                    parser.parse(reader, is, null);
                } catch (SAXException ex) {
                    ex.printStackTrace();
                }
            } else {
                socket.getOutputStream().write('n');
            }

            socket.close();
        } catch (IOException ex) {
            Exceptions.printStackTrace(ex);
        }
    }

    public void started(final Group g) {
        try {
            RegexpPropertyMatcher matcher = new RegexpPropertyMatcher("name", ".*" + networkTextField.getText() + ".*");
            if (g.getProperties().selectSingle(matcher) != null && networkTextField.isEnabled()) {
                socket.getOutputStream().write('y');
                callback.started(g);
            } else {
                socket.getOutputStream().write('n');
            }
        } catch (IOException e) {
        }
    }
}
