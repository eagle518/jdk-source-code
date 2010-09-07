/*
 *  @(#)XTextFieldPeer.java	1.30 04/06/02
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.X11;

import java.awt.*;
import java.awt.peer.*;
import java.awt.event.*;
import java.awt.datatransfer.*;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.TextEvent;
import java.awt.event.InputEvent;
import javax.swing.text.*;
import javax.swing.event.DocumentListener;
import javax.swing.event.DocumentEvent;
import javax.swing.plaf.ComponentUI;
import javax.swing.plaf.FontUIResource;
import javax.swing.InputMap;
import javax.swing.JTextField;
import javax.swing.JPasswordField;
import javax.swing.JComponent;
import javax.swing.SwingUtilities;
import javax.swing.TransferHandler;

import java.awt.event.MouseEvent;
import java.awt.event.FocusEvent;
import java.awt.event.KeyEvent;
import java.awt.event.PaintEvent;

import javax.swing.plaf.UIResource;
import javax.swing.UIDefaults;
import javax.swing.JTextField;
import javax.swing.JComponent;
import javax.swing.border.Border;
import com.sun.java.swing.plaf.motif.*;
import java.awt.im.InputMethodRequests;

import java.util.logging.*;


public class XTextFieldPeer extends XComponentPeer implements TextFieldPeer {
    private static final Logger log = Logger.getLogger("sun.awt.X11.XTextField");

    String text;
    XAWTTextField xtext;

    boolean firstChangeSkipped;
    
    public XTextFieldPeer(TextField target) {
        super(target);
        int start, end;
        firstChangeSkipped = false;
        text = target.getText();
        xtext = new XAWTTextField(text,this, target.getParent());
        xtext.getDocument().addDocumentListener(xtext);
        xtext.setCursor(target.getCursor());
        target.enableInputMethods(true);
        xtext.enableInputMethods(true);
        XToolkit.specialPeerMap.put(xtext,this);

        TextField txt = (TextField) target;
        initTextField();
        setText(txt.getText());
        if (txt.echoCharIsSet()) {
            setEchoChar(txt.getEchoChar());
        }
        else setEchoChar((char)0);

        start = txt.getSelectionStart();
        end = txt.getSelectionEnd();

        if (end > start) {
            select(start, end);
        } else {
	    setCaretPosition(start);
	}

        setEditable(txt.isEditable());

        // After this line we should not change the component's text
        firstChangeSkipped = true;
    }

    public void dispose() {
        XToolkit.specialPeerMap.remove(xtext);
        xtext.removeNotify();
        super.dispose();
    }    

    void initTextField() {
        setVisible(target.isVisible());
        
        setBounds(x, y, width, height, SET_BOUNDS);

        foreground = ComponentAccessor.getForeground(target);
        if (foreground == null)
            foreground = SystemColor.textText;

        setForeground(foreground);

        background = ComponentAccessor.getBackground(target);
        if (background == null) 
            background = SystemColor.text;
 
        setBackground(background);

        if (!target.isBackgroundSet()) {
            // This is a way to set the background color of the TextArea 
            // without calling setBackground - go through reflection
            ComponentAccessor.setBackground(target, background);
        }
        if (!target.isForegroundSet()) {
            target.setForeground(SystemColor.textText);
        }

        setFont(font);
    }


    /**
     * @see java.awt.peer.TextComponentPeer
     */
    public void setEditable(boolean editable) {
        if (xtext != null) { 
            xtext.setEditable(editable);
            xtext.repaint();
        }
        /* 4136955 - Calling setBackground() here works around an Xt
         * bug by forcing Xt to flush an internal widget cache
         */
        setBackground(target.getBackground());
    }

    
    /**
     * @see java.awt.peer.TextComponentPeer
     */

    public InputMethodRequests getInputMethodRequests() {
        if (xtext != null) return xtext.getInputMethodRequests();
        else  return null;

    }

    void handleJavaInputMethodEvent(InputMethodEvent e) {
        if (xtext != null)
            xtext.processInputMethodEventImpl(e);
    }


    /**
     * @see java.awt.peer.TextFieldPeer
     */
    public void setEchoChar(char c) {
        if (xtext != null) xtext.setEchoChar(c);
    }

    /**
     * @see java.awt.peer.TextComponentPeer
     */
    public int getSelectionStart() {
        return xtext.getSelectionStart();
    }

    /**
     * @see java.awt.peer.TextComponentPeer
     */
    public int getSelectionEnd() {
        return xtext.getSelectionEnd();
    }

    /**
     * @see java.awt.peer.TextComponentPeer
     */
    public String getText() {
        return xtext.getText();
    }

    /**
     * @see java.awt.peer.TextComponentPeer
     */
    public void setText(String txt) {
        setXAWTTextField(txt);
        repaint();
    }

    protected boolean setXAWTTextField(String txt) {
        text = txt;
        if (xtext != null)  {
            // JTextField.setText() posts two different events (remove & insert).
            // Since we make no differences between text events,
            // the document listener has to be disabled while 
            // JTextField.setText() is called.
            xtext.getDocument().removeDocumentListener(xtext);
            xtext.setText(txt);
            if (firstChangeSkipped) {
                postEvent(new TextEvent(target, TextEvent.TEXT_VALUE_CHANGED));
            }
            xtext.getDocument().addDocumentListener(xtext);
            xtext.setCaretPosition(0); 
        }
        return true;
    }

    /**
     * to be implemented.
     * @see java.awt.peer.TextComponentPeer
     */
    public void setCaretPosition(int position) {
        if (xtext != null) xtext.setCaretPosition(position); 
    }

    /**
     * DEPRECATED
     * @see java.awt.peer.TextFieldPeer
     */
    public void setEchoCharacter(char c) {
        setEchoChar(c);
    }

    void repaintText() {
        xtext.repaintNow();
    }

    public void setBackground(Color c) {
        if (log.isLoggable(Level.FINE)) log.fine("target="+ target + ", old=" + background + ", new=" + c);
        background = c;
        if (xtext != null) {
            xtext.setBackground(c);
        }
        repaintText();
    }

    public void setForeground(Color c) {
        foreground = c;
        if (xtext != null) {
            xtext.setForeground(foreground);
        }
        repaintText();
    }

    public void setFont(Font f) {
        synchronized (getStateLock()) {
            font = f;
            if (xtext != null) {
                xtext.setFont(font);
            }
        }
        xtext.validate();
    }

    /**
     * DEPRECATED
     * @see java.awt.peer.TextFieldPeer
     */
    public Dimension preferredSize(int cols) {
        return getPreferredSize(cols);
    }

    /**
     * Deselects the the highlighted text.
     */
    public void deselect() {
        int selStart=xtext.getSelectionStart();
        int selEnd=xtext.getSelectionEnd();
        if (selStart != selEnd) {
            xtext.select(selStart,selStart);
        }
    }

    
    /**
     * to be implemented.
     * @see java.awt.peer.TextComponentPeer
     */
    public int getCaretPosition() {
        return xtext.getCaretPosition();
    }



    /**
     * @see java.awt.peer.TextComponentPeer
     */
    public void select(int s, int e) {
        xtext.select(s,e);
    }


    public Dimension getMinimumSize() {
        return xtext.getMinimumSize();        
    }

    public Dimension getPreferredSize() {
        return xtext.getPreferredSize();
    }

    public Dimension getPreferredSize(int cols) {
        return getMinimumSize(cols);
    }

    private static final int PADDING = 16;
    
    public Dimension getMinimumSize(int cols) {
        Font f = xtext.getFont();
        FontMetrics fm = xtext.getFontMetrics(f);
        return new Dimension(fm.charWidth('0') * cols + 10,
                             fm.getMaxDescent() + fm.getMaxAscent() + PADDING);

    }

    public boolean isFocusable() {
        return true;
    }

    // NOTE: This method is called by privileged threads.
    //       DO NOT INVOKE CLIENT CODE ON THIS THREAD!
    public void action(final long when, final int modifiers) {
        postEvent(new ActionEvent(target, ActionEvent.ACTION_PERFORMED,
                                  text, when,
                                  modifiers));
    }


    protected void disposeImpl() {
    }


    public void repaint() {
        if (xtext  != null) xtext.repaint();
    }

    public void paint(Graphics g) {
        if (xtext  != null) xtext.paint(g);
    }


    public void print(Graphics g) {
        if (xtext != null) {
            xtext.print(g);
        }
    }

    public void focusLost(FocusEvent e) {
        super.focusLost(e);
        xtext.forwardFocusLost(e);
    }
 
    public void focusGained(FocusEvent e) {
        super.focusGained(e);
        xtext.forwardFocusGained(e);
    }

    void handleJavaKeyEvent(KeyEvent e) {
        ComponentAccessor.processEvent(xtext,e);
    }


    public void handleJavaMouseEvent( MouseEvent mouseEvent ) {
        super.handleJavaMouseEvent(mouseEvent);
        if (xtext != null)  {
            mouseEvent.setSource(xtext);
            int id = mouseEvent.getID(); 
            if (id == MouseEvent.MOUSE_DRAGGED || id == MouseEvent.MOUSE_MOVED)
                xtext.processMouseMotionEventImpl(mouseEvent);
            else
                xtext.processMouseEventImpl(mouseEvent);
        }
    }


    /**
     * DEPRECATED
     */
    public Dimension minimumSize() {
        return getMinimumSize();
    }

    /**
     * DEPRECATED
     */
    public Dimension minimumSize(int cols) {
        return getMinimumSize(cols);
    }

    public void setVisible(boolean b) {
        super.setVisible(b);
        if (xtext != null) xtext.setVisible(b);
    }

    public void setBounds(int x, int y, int width, int height, int op) {
        super.setBounds(x, y, width, height, op);
        if (xtext != null) { 
            xtext.setBounds(0,0,width,height);
            xtext.validate();
        }
    }
  
  
    //
    // Accessibility support
    //

    // stub functions: to be fully implemented in a future release
    public int getIndexAtPoint(int x, int y) { return -1; }
    public Rectangle getCharacterBounds(int i) { return null; }
    public long filterEvents(long mask) { return 0; }


    /*  To be fully implemented in a future release

        int oldSelectionStart;
        int oldSelectionEnd;

        public native int getIndexAtPoint(int x, int y);
        public native Rectangle getCharacterBounds(int i);
        public native long filterEvents(long mask);

        /**
         * Handle a change in the text selection endpoints
         * (Note: could be simply a change in the caret location)
         *
         public void selectionValuesChanged(int start, int end) {
         return;  // Need to write implemetation of this.  
         }
    */


    class  AWTTextFieldUI extends MotifPasswordFieldUI {

        /**
         * Creates a UI for a JTextField.
         *
         * @param c the text field
         * @return the UI
         */
        JTextField jtf;


        protected String getPropertyPrefix() { return "TextField"; }

        public void installUI(JComponent c) {
            super.installUI(c);

            jtf = (JTextField) c;

            JTextField editor = jtf;

            UIDefaults uidefaults = XToolkit.getUIDefaults();

            String prefix = getPropertyPrefix();
            Font f = editor.getFont();
            if ((f == null) || (f instanceof UIResource)) {
                editor.setFont(uidefaults.getFont(prefix + ".font"));
            }

            Color bg = editor.getBackground();
            if ((bg == null) || (bg instanceof UIResource)) {
                editor.setBackground(uidefaults.getColor(prefix + ".background"));
            }

            Color fg = editor.getForeground();
            if ((fg == null) || (fg instanceof UIResource)) {
                editor.setForeground(uidefaults.getColor(prefix + ".foreground"));
            }

            Color color = editor.getCaretColor();
            if ((color == null) || (color instanceof UIResource)) {
                editor.setCaretColor(uidefaults.getColor(prefix + ".caretForeground"));
            }

            Color s = editor.getSelectionColor();
            if ((s == null) || (s instanceof UIResource)) {
                editor.setSelectionColor(uidefaults.getColor(prefix + ".selectionBackground"));
            }

            Color sfg = editor.getSelectedTextColor();
            if ((sfg == null) || (sfg instanceof UIResource)) {
                editor.setSelectedTextColor(uidefaults.getColor(prefix + ".selectionForeground"));
            }

            Color dfg = editor.getDisabledTextColor();
            if ((dfg == null) || (dfg instanceof UIResource)) {
                editor.setDisabledTextColor(uidefaults.getColor(prefix + ".inactiveForeground"));
            }

            Border b = editor.getBorder();
            if ((b == null) || (b instanceof UIResource)) {
                editor.setBorder(uidefaults.getBorder(prefix + ".border"));
            }

            Insets margin = editor.getMargin();
            if (margin == null || margin instanceof UIResource) {
                editor.setMargin(uidefaults.getInsets(prefix + ".margin"));
            }
        }

        protected void installKeyboardActions() {
            super.installKeyboardActions();

            JTextComponent comp = getComponent();

            UIDefaults uidefaults = XToolkit.getUIDefaults();

            String prefix = getPropertyPrefix();

            InputMap map = (InputMap)uidefaults.get(prefix + ".focusInputMap");

            if (map != null) {
                SwingUtilities.replaceUIInputMap(comp, JComponent.WHEN_FOCUSED,
                                                 map);
            }
        }

        protected Caret createCaret() {
            return new XAWTCaret();
        }
    }

    class XAWTCaret extends DefaultCaret {
        public void focusGained(FocusEvent e) {
	    super.focusGained(e);
	    getComponent().repaint();
	}        
        public void focusLost(FocusEvent e) {
	    super.focusLost(e);
	    getComponent().repaint();
	}
    }

    class XAWTTextField extends JPasswordField 
        implements ActionListener,
                   DocumentListener
    {
    
        boolean isFocused = false;
    
        XComponentPeer peer;
    
        public XAWTTextField(String text, XComponentPeer peer, Container parent) {
            super(text);
            this.peer = peer;
            putClientProperty("JPasswordField.cutCopyAllowed", Boolean.TRUE);
            setFocusable(false);
            ComponentAccessor.setParent(this,parent); 
            setBackground(peer.getPeerBackground());
            setForeground(peer.getPeerForeground());
            setFont(peer.getPeerFont());
            setCaretPosition(0); 
            addActionListener(this);
            addNotify();

        }

        public void actionPerformed( ActionEvent actionEvent ) {
            peer.postEvent(new ActionEvent(peer.target, 
                                           ActionEvent.ACTION_PERFORMED, 
                                           getText()));

        }

        public void insertUpdate(DocumentEvent e) {
            if (peer != null) {
                peer.postEvent(new TextEvent(peer.target, 
                                             TextEvent.TEXT_VALUE_CHANGED));
            }
        }

        public void removeUpdate(DocumentEvent e) {
            if (peer != null) {
                peer.postEvent(new TextEvent(peer.target, 
                                             TextEvent.TEXT_VALUE_CHANGED));
            }
        }

        public void changedUpdate(DocumentEvent e) {
            if (peer != null) {
                peer.postEvent(new TextEvent(peer.target, 
                                             TextEvent.TEXT_VALUE_CHANGED));
            }
        }

        public ComponentPeer getPeer() {
            return (ComponentPeer) peer;
        }


        public void repaintNow() {
            paintImmediately(getBounds());
        }

        public Graphics getGraphics() {
            return peer.getGraphics(); 
        }

        public void updateUI() {
            ComponentUI ui = new AWTTextFieldUI();
            setUI(ui);
        }


        void forwardFocusGained( FocusEvent e) {
            isFocused = true;
            FocusEvent fe = new FocusEvent(this,FocusEvent.FOCUS_GAINED);
            super.processFocusEvent(fe);

        }


        void forwardFocusLost( FocusEvent e) {
            isFocused = false;
            FocusEvent fe = new FocusEvent(this,FocusEvent.FOCUS_LOST);
            super.processFocusEvent(fe);

        }

        public boolean hasFocus() {
            return isFocused;
        }


        public void processInputMethodEventImpl(InputMethodEvent e) {
            processInputMethodEvent(e);
        }

        public void processMouseEventImpl(MouseEvent e) {
            processMouseEvent(e);
        }

        public void processMouseMotionEventImpl(MouseEvent e) {
            processMouseMotionEvent(e);
        }

        // Fix for 4915454 - override the default implementation to avoid
        // loading SystemFlavorMap and associated classes.
        public void setTransferHandler(TransferHandler newHandler) {
            TransferHandler oldHandler = (TransferHandler)
                getClientProperty(XTextTransferHelper.getTransferHandlerKey());
            putClientProperty(XTextTransferHelper.getTransferHandlerKey(), 
                              newHandler);

            firePropertyChange("transferHandler", oldHandler, newHandler);
        }
    }
}

