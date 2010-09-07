import java.awt.*;
import java.awt.event.*;
import java.io.*;
import java.net.*;
import javax.swing.*;
import netscape.javascript.JSObject;

// Basic test of drag-and-drop of applets to the desktop as well as
// disconnected applet context functionality

public class DragTest1 extends JApplet {
    private URL documentURL;

    public void init() {
        getContentPane().setBackground(Color.GRAY);
        JSObject win = JSObject.getWindow(this);
        try {
            documentURL =
                new URL((String) win.eval("document.URL"));
            setLayout(new FlowLayout());
            JButton button = new JButton("Reopen Web Page");
            button.addActionListener(new ActionListener() {
                    public void actionPerformed(ActionEvent e) {
                        getAppletContext().showDocument(documentURL);
                    }
                });
            add(button);
            button = new JButton("Connect Back to Server");
            button.addActionListener(new ActionListener() {
                    public void actionPerformed(ActionEvent e) {
                        try {
                            URLConnection conn =
                                documentURL.openConnection();
                            conn.connect();
                            InputStream in = conn.getInputStream();
                            byte[] bytes = new byte[8192];
                            int totalRead = 0;
                            int numRead;
                            while ((numRead = in.read(bytes)) > 0) {
                                totalRead += numRead;
                            }
                            in.close();
                            System.out.println("Read " + totalRead + " bytes");
                        } catch (IOException ex) {
                            ex.printStackTrace();
                        }
                    }
                });
            add(button);
        } catch (MalformedURLException e) {
            throw new RuntimeException(e);
        }
    }
}
