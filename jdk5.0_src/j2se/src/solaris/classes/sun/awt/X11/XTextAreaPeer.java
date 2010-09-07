/*
 * @(#)XTextAreaPeer.java	1.37 04/06/02
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.X11;

import java.awt.*;
import java.awt.peer.ComponentPeer;
import java.awt.peer.TextAreaPeer;
import java.awt.event.*;
import java.awt.datatransfer.*;
import java.io.Reader;
import java.text.BreakIterator;
import javax.swing.event.DocumentListener;
import javax.swing.event.DocumentEvent;
import javax.swing.JTextArea;
import javax.swing.JComponent;
import javax.swing.JLabel;
import javax.swing.JScrollPane;
import javax.swing.JScrollBar;
import javax.swing.plaf.ComponentUI;
import com.sun.java.swing.plaf.motif.MotifTextAreaUI;
import javax.swing.plaf.UIResource;
import javax.swing.UIDefaults;
import javax.swing.border.Border;
import javax.swing.border.EmptyBorder;
import javax.swing.border.LineBorder;
import javax.swing.border.CompoundBorder;
import javax.swing.border.AbstractBorder;
import javax.swing.JButton;
import javax.swing.JViewport;
import javax.swing.InputMap;
import javax.swing.SwingUtilities;
import javax.swing.TransferHandler;
import javax.swing.plaf.basic.BasicArrowButton;
import javax.swing.plaf.basic.BasicScrollBarUI;
import javax.swing.plaf.basic.BasicScrollPaneUI;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import javax.swing.text.Caret;
import javax.swing.text.DefaultCaret;
import javax.swing.text.JTextComponent;

import javax.swing.plaf.BorderUIResource;
import java.awt.im.InputMethodRequests;



class XTextAreaPeer extends XComponentPeer implements TextAreaPeer {
    boolean editable;

    boolean scrollGrab;
    Component grabbedScrollbar;

    AWTTextPane textPane;
    AWTTextArea jtext;

    boolean firstChangeSkipped;

    /* FIXME  */

    public long filterEvents(long mask) {
        Thread.dumpStack();
        return 0;
    }

    /* FIXME   */
    public Rectangle getCharacterBounds(int i) {
        Thread.dumpStack();
        return null;
    }

    public int getIndexAtPoint(int x, int y) {
        Thread.dumpStack();
        return 0;
    }


    /**
     * Create a Text area.
     */
    XTextAreaPeer(TextArea target) {
        super( target  );
    
        // some initializations require that target be set even
        // though init(target) has not been called
        this.target = target;


        //ComponentAccessor.enableEvents(target,AWTEvent.MOUSE_WHEEL_EVENT_MASK); 
        target.enableInputMethods(true);

        firstChangeSkipped = false;
        String text = ((TextArea)target).getText();
        jtext = new AWTTextArea(text, this);
        jtext.setWrapStyleWord(true);
        jtext.getDocument().addDocumentListener(jtext);
        XToolkit.specialPeerMap.put(jtext,this);
        jtext.enableInputMethods(true);
        textPane = new AWTTextPane(jtext,this, target.getParent());

        setBounds(x, y, width, height, SET_BOUNDS);
        textPane.setVisible(true);
        textPane.validate();

        foreground = ComponentAccessor.getForeground(target);
        if (foreground == null)  {
            foreground = SystemColor.textText;
        }
        setForeground(foreground);

        background = ComponentAccessor.getBackground(target);
        if (background == null) { 
            if (target.isEditable()) background = SystemColor.text; 
            else background = SystemColor.control; 
        }
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

        int start = target.getSelectionStart();
        int end = target.getSelectionEnd();

        if (end > start) {
            select(start, end);
        } else {
	    setCaretPosition(start);
	}

        setEditable(target.isEditable());

        setScrollBarVisibility();
        // set the text of this object to the text of its target
        setTextImpl(target.getText());  //?? should this be setText

        // After this line we should not change the component's text
        firstChangeSkipped = true;
    }

    public void dispose() {
        XToolkit.specialPeerMap.remove(jtext);
        jtext.removeNotify();
        textPane.removeNotify();
        super.dispose();
    }

    void setScrollBarVisibility() {
        int visibility = ((TextArea)target).getScrollbarVisibility();
        jtext.setLineWrap(false);

        if (visibility == TextArea.SCROLLBARS_NONE) {
            textPane.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
            textPane.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_NEVER);
            jtext.setLineWrap(true);
        }
        else if (visibility == TextArea.SCROLLBARS_BOTH) {

            textPane.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS);
            textPane.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
        }
        else if (visibility == TextArea.SCROLLBARS_VERTICAL_ONLY) {
            textPane.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_NEVER);
            textPane.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_ALWAYS);
            jtext.setLineWrap(true);
        }
        else if (visibility == TextArea.SCROLLBARS_HORIZONTAL_ONLY) {
            textPane.setVerticalScrollBarPolicy(JScrollPane.VERTICAL_SCROLLBAR_NEVER);
            textPane.setHorizontalScrollBarPolicy(JScrollPane.HORIZONTAL_SCROLLBAR_ALWAYS);
        }

    }

    /**
     * Compute minimum size.
     */
    public Dimension getMinimumSize() {
        return getMinimumSize(10, 60);
    }

    public Dimension getPreferredSize(int rows, int cols) {
        return getMinimumSize(rows, cols);
    }

    /**
     * @see java.awt.peer.TextAreaPeer
     */

    public Dimension getMinimumSize(int rows, int cols) {
        /*    Dimension d = null;
              if (jtext != null) {
              d = jtext.getMinimumSize(rows,cols);
              }
              return d;
        */

        int vsbwidth=0;
        int hsbheight=0;

        JScrollBar vsb = textPane.getVerticalScrollBar();
        if (vsb != null) {
            vsbwidth = vsb.getMinimumSize().width;
        }

        JScrollBar hsb = textPane.getHorizontalScrollBar();
        if (hsb != null) {
            hsbheight = hsb.getMinimumSize().height;
        }

        Font f = jtext.getFont();
        FontMetrics fm = jtext.getFontMetrics(f);

        return new Dimension(fm.charWidth('0') * cols + /*2*XMARGIN +*/ vsbwidth,
                             fm.getHeight() * rows + /*2*YMARGIN +*/ hsbheight);

    }

    public boolean isFocusable() {
        return true;
    }

    public void setVisible(boolean b) {
        super.setVisible(b);
        if (textPane != null)
            textPane.setVisible(b);
    }


    void repaintText() {
        jtext.repaintNow();
    }

    public void focusGained(FocusEvent e) {
        super.focusGained(e);
        jtext.forwardFocusGained(e);
    }



    public void focusLost(FocusEvent e) {
        super.focusLost(e);
        jtext.forwardFocusLost(e);
    }




    /**
     * Paint the component
     * this method is called when the repaint instruction has been used
     */

    public void repaint() {
        if (textPane  != null)  {
            //textPane.validate();
            textPane.repaint();
        }
    }

    public void paint(Graphics g) {
        if (textPane  != null)  {
            textPane.paint(g);
        }
    }

    public void setBounds(int x, int y, int width, int height, int op) {
        super.setBounds(x, y, width, height, op);
        if (textPane != null) {
            textPane.setBounds(0,0,width,height);
            textPane.validate();
        }
    }


    void handleJavaKeyEvent(KeyEvent e) {
        ComponentAccessor.processEvent(jtext,e);
    }

    public boolean handlesWheelScrolling() { return true; }

    void handleJavaMouseWheelEvent(MouseWheelEvent e) {
        ComponentAccessor.processEvent(textPane,e);
    }


    public void handleJavaMouseEvent( MouseEvent e ) {
        super.handleJavaMouseEvent(e);

        Component comp=null;
        int id = e.getID();

        JScrollBar scroll = textPane.getVerticalScrollBar();
        if (scroll != null) {
            Rectangle r = scroll.getBounds();
            if (r.contains(e.getX(),e.getY())) 
                comp = scroll;
        }

        scroll =  textPane.getHorizontalScrollBar();
        if (scroll != null) {
            Rectangle r = scroll.getBounds();
            if (r.contains( e.getX(),e.getY())) 
                comp = scroll;
        }
        if ((comp == null) && (!scrollGrab)) {  // This event is for the text area.

            pSetCursor(target.getCursor());// Update cursor 
            JViewport port = textPane.getViewport();
            Point p = port.getViewPosition();
            int x = e.getX()+p.x;
            int y = e.getY()+p.y;
            MouseEvent retargeted;

            retargeted = new MouseEvent(jtext,
                                        id, 
                                        e.getWhen(), 
                                        e.getModifiersEx() | e.getModifiers(),
                                        x, 
                                        y, 
                                        e.getClickCount(), 
                                        e.isPopupTrigger(),
                                        e.getButton());

            if (id == MouseEvent.MOUSE_DRAGGED || id == MouseEvent.MOUSE_MOVED)
                jtext.processMouseMotionEventPublic(retargeted);
            else
                jtext.processMouseEventPublic(retargeted);
        }
        else {
            if (scrollGrab) { // We are in the middle of scrolling 
                comp = grabbedScrollbar;
            }
            pSetCursor(comp.getCursor()); // Update cursor
            Point p = comp.getLocation();
            int x = e.getX()-p.x;
            int y = e.getY()-p.y;
            MouseEvent retargeted;

            if (id == MouseEvent.MOUSE_PRESSED) {
                scrollGrab = true;
                grabbedScrollbar = comp;
            }
            else if (id == MouseEvent.MOUSE_RELEASED) {
                scrollGrab = false;
            }


            retargeted = new MouseEvent(target,
                                        id, 
                                        e.getWhen(), 
                                        e.getModifiersEx() | e.getModifiers(),
                                        x, 
                                        y, 
                                        e.getClickCount(), 
                                        e.isPopupTrigger(),
                                        e.getButton());
            ComponentAccessor.processEvent(comp,retargeted); 
        }

    }


    void handleJavaInputMethodEvent(InputMethodEvent e) {
        if (jtext != null)
            jtext.processInputMethodEventPublic((InputMethodEvent)e);
    }



    /**
     * @see java.awt.peer.TextComponentPeer
     */
    public void select(int s, int e) {
        if (jtext != null) jtext.select(s,e);
    }

    public void setBackground(Color c) {
        super.setBackground(c);
//          synchronized (getStateLock()) {
//              background = c;
//          }
        if (jtext != null) {
            jtext.setBackground(c);
        }
//          repaintText();
    }


    public void setForeground(Color c) {
        super.setForeground(c);
//          synchronized (getStateLock()) {
//              foreground = c;
//          }
        if (jtext != null) {
            jtext.setForeground(foreground);
        }
//          repaintText();
    }
    public void setFont(Font f) {
        super.setFont(f);
//          synchronized (getStateLock()) {
//              font = f;
//          }
        if (jtext != null) {
            jtext.setFont(font);
        }
        textPane.validate();
    }


    /**
     * @see java.awt.peer.TextComponentPeer
     */
    public void setEditable(boolean editable) {
        this.editable = editable;
        if (jtext != null) jtext.setEditable(editable);
        repaintText();

        /* 4136955 - Calling setBackground() here works around an Xt
         * bug by forcing Xt to flush an internal widget cache
         */
        setBackground(target.getBackground());
    }


    /**
     * @see java.awt.peer.TextComponentPeer
     */

    public InputMethodRequests getInputMethodRequests() {
        if (jtext != null) return jtext.getInputMethodRequests();
        else  return null;
    }


    /**
     * @see java.awt.peer.TextComponentPeer
     */
    public int getSelectionStart() {
        return jtext.getSelectionStart();
    }

    /**
     * @see java.awt.peer.TextComponentPeer
     */
    public int getSelectionEnd() {
        return jtext.getSelectionEnd();
    }

    /**
     * @see java.awt.peer.TextComponentPeer
     */
    public String getText() {
        return jtext.getText();
    }


    /**
     * @see java.awt.peer.TextComponentPeer
     */
    public void setText(String txt) {
        setTextImpl(txt);
        repaintText();
    }

    protected boolean setTextImpl(String txt) {
        if (jtext != null) {
            // Please note that we do not want to post an event
            // if setText() replaces an empty text by an empty text,
            // that is, if component's text remains unchanged.
            if (jtext.getDocument().getLength() == 0 && txt.length() == 0) {
                return true;
            }

            // JTextArea.setText() posts two different events (remove & insert).
            // Since we make no differences between text events,
            // the document listener has to be disabled while 
            // JTextArea.setText() is called.
            jtext.getDocument().removeDocumentListener(jtext);
            jtext.setText(txt);
            if (firstChangeSkipped) {
                postEvent(new TextEvent(target, TextEvent.TEXT_VALUE_CHANGED));
            }
            jtext.getDocument().addDocumentListener(jtext);
        }
        return true;
    }

    /**
     * insert the text "txt on position "pos" in the array lines
     * @see java.awt.peer.TextAreaPeer
     */
    public void insert(String txt, int p) {
        if (jtext != null) {
            boolean doScroll = (p >= jtext.getDocument().getLength() && jtext.getDocument().getLength() != 0);
            jtext.insert(txt,p); 
            if (doScroll) {
                JScrollBar bar = textPane.getVerticalScrollBar();
                if (bar != null) {
                    bar.setValue(bar.getMaximum());
                }
            }
        }
    }

    /**
     * replace the text between the position "s" and "e" with "txt"
     * @see java.awt.peer.TextAreaPeer
     */
    public void replaceRange(String txt, int s, int e) {
        if (jtext != null) {
            // JTextArea.replaceRange() posts two different events.
            // Since we make no differences between text events,
            // the document listener has to be disabled while 
            // JTextArea.replaceRange() is called.
            jtext.getDocument().removeDocumentListener(jtext);
            jtext.replaceRange(txt, s, e);
            postEvent(new TextEvent(target, TextEvent.TEXT_VALUE_CHANGED));
            jtext.getDocument().addDocumentListener(jtext);
        }
    }


    /**
     * to be implemented.
     * @see java.awt.peer.TextComponentPeer
     */
    public void setCaretPosition(int position) {
        jtext.setCaretPosition(position);
    }

    /**
     * to be implemented.
     * @see java.awt.peer.TextComponentPeer
     */
    public int getCaretPosition() {
        return jtext.getCaretPosition();
    }

    /**
     * DEPRECATED
     * @see java.awt.peer.TextAreaPeer
     */
    public void insertText(String txt, int pos) {
        insert(txt, pos);
    }

    /**
     * DEPRECATED
     * @see java.awt.peer.TextAreaPeer
     */
    public void replaceText(String txt, int start, int end) {
        replaceRange(txt, start, end);
    }

    /**
     * DEPRECATED
     * @see java.awt.peer.TextAreaPeer
     */
    public Dimension minimumSize(int rows, int cols) {
        return getMinimumSize(rows, cols);
    }

    /**
     * DEPRECATED
     * @see java.awt.peer.TextAreaPeer
     */
    public Dimension preferredSize(int rows, int cols) {
        return getPreferredSize(rows, cols);
    }

    class  AWTTextAreaUI extends MotifTextAreaUI {

        /**
         * Creates a UI for a JTextArea.
         *
         * @param c the text field
         * @return the UI
         */
        JTextArea jta;

        protected String getPropertyPrefix() { return "TextArea"; }


        public void installUI(JComponent c) {
            super.installUI(c);

            jta = (JTextArea) c;

            JTextArea editor = jta;

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

            Border b = new BevelBorder(false,SystemColor.controlDkShadow,SystemColor.controlLtHighlight);
            editor.setBorder(new BorderUIResource.CompoundBorderUIResource(
                b,new EmptyBorder(2, 2, 2, 2)));


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

    // TODO : fix this duplicate code
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

    class XAWTScrollBarButton extends BasicArrowButton
    {

        UIDefaults uidefaults = XToolkit.getUIDefaults();
        private Color darkShadow = SystemColor.controlShadow;
        private Color lightShadow = SystemColor.controlLtHighlight;
        private Color buttonBack = uidefaults.getColor("ScrollBar.track");

        public XAWTScrollBarButton(int direction)
        {
            super(direction);

            switch (direction) {
            case NORTH:
            case SOUTH:
            case EAST:
            case WEST:
                this.direction = direction;
                break;
            default:
                throw new IllegalArgumentException("invalid direction");
            }

            setRequestFocusEnabled(false);
            setOpaque(true);
            setBackground(uidefaults.getColor("ScrollBar.thumb"));
            setForeground(uidefaults.getColor("ScrollBar.foreground"));
        }


        public Dimension getPreferredSize() {
            switch (direction) {
            case NORTH: 
            case SOUTH: 
                return new Dimension(11, 12);
            case EAST:
            case WEST:
            default:
                return new Dimension(12, 11);
            }
        }

        public Dimension getMinimumSize() {
            return getPreferredSize();
        }

        public Dimension getMaximumSize() {
            return getPreferredSize();
        }

        public boolean isFocusTraversable() {
            return false;
        }

        public void paint(Graphics g) 
        {
            int w = getWidth();
            int h = getHeight();

            if (isOpaque()) {
                g.setColor(buttonBack);
                g.fillRect(0, 0, w, h);
            }

            boolean isPressed = getModel().isPressed();
            Color lead = (isPressed) ? darkShadow : lightShadow;
            Color trail = (isPressed) ? lightShadow : darkShadow;
            Color fill = getBackground();

            int cx = w / 2;
            int cy = h / 2;
            int s = Math.min(w, h);

            switch (direction) {
            case NORTH:
                g.setColor(lead);
                g.drawLine(cx, 0, cx, 0);
                for (int x = cx - 1, y = 1, dx = 1; y <= s - 2; y += 2) {
                    g.setColor(lead);
                    g.drawLine(x, y, x, y);
                    if (y >= (s - 2)) {
                        g.drawLine(x, y + 1, x, y + 1);
                    }
                    g.setColor(fill);
                    g.drawLine(x + 1, y, x + dx, y);
                    if (y < (s - 2)) {
                        g.drawLine(x, y + 1, x + dx + 1, y + 1);
                    }
                    g.setColor(trail);
                    g.drawLine(x + dx + 1, y, x + dx + 1, y);
                    if (y >= (s - 2)) {
                        g.drawLine(x + 1, y + 1, x + dx + 1, y + 1);
                    }
                    dx += 2;
                    x -= 1;
                }
                break;

            case SOUTH:
                g.setColor(trail);
                g.drawLine(cx, s, cx, s);
                for (int x = cx - 1, y = s - 1, dx = 1; y >= 1; y -= 2) {
                    g.setColor(lead);
                    g.drawLine(x, y, x, y);
                    if (y <= 2) {
                        g.drawLine(x, y - 1, x + dx + 1, y - 1);
                    }
                    g.setColor(fill);
                    g.drawLine(x + 1, y, x + dx, y);
                    if (y > 2) {
                        g.drawLine(x, y - 1, x + dx + 1, y - 1);
                    }
                    g.setColor(trail);
                    g.drawLine(x + dx + 1, y, x + dx + 1, y);

                    dx += 2;
                    x -= 1;
                }
                break;

            case EAST:
                g.setColor(lead);
                g.drawLine(s, cy, s, cy);
                for (int y = cy - 1, x = s - 1, dy = 1; x >= 1; x -= 2) {
                    g.setColor(lead);
                    g.drawLine(x, y, x, y);
                    if (x <= 2) {
                        g.drawLine(x - 1, y, x - 1, y + dy + 1);
                    }
                    g.setColor(fill);
                    g.drawLine(x, y + 1, x, y + dy);
                    if (x > 2) {
                        g.drawLine(x - 1, y, x - 1, y + dy + 1);
                    }
                    g.setColor(trail);
                    g.drawLine(x, y + dy + 1, x, y + dy + 1);

                    dy += 2;
                    y -= 1;
                }
                break;

            case WEST:
                g.setColor(trail);
                g.drawLine(0, cy, 0, cy);
                for (int y = cy - 1, x = 1, dy = 1; x <= s - 2; x += 2) {
                    g.setColor(lead);
                    g.drawLine(x, y, x, y);
                    if (x >= (s - 2)) {
                        g.drawLine(x + 1, y, x + 1, y);
                    }
                    g.setColor(fill);
                    g.drawLine(x, y + 1, x, y + dy);
                    if (x < (s - 2)) {
                        g.drawLine(x + 1, y, x + 1, y + dy + 1);
                    }
                    g.setColor(trail);
                    g.drawLine(x, y + dy + 1, x, y + dy + 1);
                    if (x >= (s - 2)) {
                        g.drawLine(x + 1, y + 1, x + 1, y + dy + 1);
                    }
                    dy += 2;
                    y -= 1;
                }
                break;
            }
        }
    }

    class XAWTScrollBarUI extends BasicScrollBarUI 
    {

        public XAWTScrollBarUI() {
            super();
        }

        protected void installDefaults()
        {
            super.installDefaults();
            scrollbar.setBorder(new BevelBorder(false,SystemColor.controlDkShadow,SystemColor.controlLtHighlight) );
        }

        protected void configureScrollBarColors() {
            UIDefaults uidefaults = XToolkit.getUIDefaults();
            Color bg = scrollbar.getBackground();
            if (bg == null || bg instanceof UIResource) {
                scrollbar.setBackground(uidefaults.getColor("ScrollBar.background"));
            }

            Color fg = scrollbar.getForeground();
            if (fg == null || fg instanceof UIResource) {
                scrollbar.setForeground(uidefaults.getColor("ScrollBar.foreground"));
            } 

            thumbHighlightColor = uidefaults.getColor("ScrollBar.thumbHighlight");
            thumbLightShadowColor = uidefaults.getColor("ScrollBar.thumbShadow");
            thumbDarkShadowColor = uidefaults.getColor("ScrollBar.thumbDarkShadow");
            thumbColor = uidefaults.getColor("ScrollBar.thumb");
            trackColor = uidefaults.getColor("ScrollBar.track");

            trackHighlightColor = uidefaults.getColor("ScrollBar.trackHighlight");

        }
        protected JButton createDecreaseButton(int orientation) {
            JButton b = new XAWTScrollBarButton(orientation);
            return b;

        } 
        protected JButton createIncreaseButton(int orientation) {
            JButton b = new XAWTScrollBarButton(orientation);
            return b;
        }

        public void paint(Graphics g, JComponent c) {
            paintTrack(g, c, getTrackBounds());		
            Rectangle thumbBounds = getThumbBounds();
            paintThumb(g, c, thumbBounds);
        }

        

        public void paintThumb(Graphics g, JComponent c, Rectangle thumbBounds)
        {

            if(!scrollbar.isEnabled())      {
                return;
            }

            if (thumbBounds.isEmpty()) 
                thumbBounds = getTrackBounds();

            int w = thumbBounds.width;
            int h = thumbBounds.height;

            g.translate(thumbBounds.x, thumbBounds.y);
            g.setColor(thumbColor);
            g.fillRect(0, 0, w-1, h-1);

            g.setColor(thumbHighlightColor);
            g.drawLine(0, 0, 0, h-1);
            g.drawLine(1, 0, w-1, 0);

            g.setColor(thumbLightShadowColor);
            g.drawLine(1, h-1, w-1, h-1);
            g.drawLine(w-1, 1, w-1, h-2);

            g.translate(-thumbBounds.x, -thumbBounds.y);
        }
    }

    class AWTTextArea extends JTextArea implements DocumentListener {

        boolean isFocused = false;
        XTextAreaPeer peer;
        public AWTTextArea(String text, XTextAreaPeer peer) {
            super(text);
            setFocusable(false);
            this.peer = peer;
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

        public void repaintNow() {
            paintImmediately(getBounds());
        }

        public void processMouseEventPublic(MouseEvent e) {
            processMouseEvent(e);
        }

        public void processMouseMotionEventPublic(MouseEvent e) {
            processMouseMotionEvent(e);
        }

        public void processInputMethodEventPublic(InputMethodEvent e) {
            processInputMethodEvent(e);
        }

        public void updateUI() {
            ComponentUI ui = new AWTTextAreaUI();
            setUI(ui);
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

    class XAWTScrollPaneUI extends BasicScrollPaneUI
    {
        private final Border vsbMarginBorderR = new EmptyBorder(0, 2, 0, 0);
        private final Border vsbMarginBorderL = new EmptyBorder(0, 0, 0, 2);
        private final Border hsbMarginBorder = new EmptyBorder(2, 0, 0, 0);

        private Border vsbBorder;
        private Border hsbBorder;

        private PropertyChangeListener propertyChangeHandler;

        protected void installListeners(JScrollPane scrollPane) {
            super.installListeners(scrollPane);
            propertyChangeHandler = createPropertyChangeHandler();
            scrollPane.addPropertyChangeListener(propertyChangeHandler);
        }

        public void paint(Graphics g, JComponent c) {
            Border vpBorder = scrollpane.getViewportBorder();
            if (vpBorder != null) {
                Rectangle r = scrollpane.getViewportBorderBounds();
                vpBorder.paintBorder(scrollpane, g, r.x, r.y, r.width, r.height);
            }
        }

        protected void uninstallListeners(JScrollPane scrollPane) {
            super.uninstallListeners(scrollPane);
            scrollPane.removePropertyChangeListener(propertyChangeHandler);
        }

        private PropertyChangeListener createPropertyChangeHandler() {
            return new PropertyChangeListener() {
                    public void propertyChange(PropertyChangeEvent e) {
                        String propertyName = e.getPropertyName();

                        if (propertyName.equals("componentOrientation")) {
                            JScrollPane pane = (JScrollPane)e.getSource();
                            JScrollBar vsb = pane.getVerticalScrollBar();
                            if (vsb != null) {
                                if (isLeftToRight(pane)) {
                                    vsbBorder = new CompoundBorder(new EmptyBorder(0, 4, 0, -4),
                                                                   vsb.getBorder());
                                } else {
                                    vsbBorder = new CompoundBorder(new EmptyBorder(0, -4, 0, 4),
                                                                   vsb.getBorder());
                                }
                                vsb.setBorder(vsbBorder);
                            }
                        }
                    }};
        }

        boolean isLeftToRight( Component c ) {
            return c.getComponentOrientation().isLeftToRight();
        }


        protected void installDefaults(JScrollPane scrollpane) {
            Border b = scrollpane.getBorder();
            UIDefaults uidefaults = XToolkit.getUIDefaults();
            scrollpane.setBorder(uidefaults.getBorder("ScrollPane.border"));
            scrollpane.setBackground(uidefaults.getColor("ScrollPane.background"));
            scrollpane.setViewportBorder(uidefaults.getBorder("TextField.border"));
            JScrollBar vsb = scrollpane.getVerticalScrollBar();
            if (vsb != null) {
                if (isLeftToRight(scrollpane)) {
                    vsbBorder = new CompoundBorder(vsbMarginBorderR, 
                                                   vsb.getBorder());
                }
                else {
                    vsbBorder = new CompoundBorder(vsbMarginBorderL, 
                                                   vsb.getBorder());
                }
                vsb.setBorder(vsbBorder);
            }

            JScrollBar hsb = scrollpane.getHorizontalScrollBar();
            if (hsb != null) {
                hsbBorder = new CompoundBorder(hsbMarginBorder, hsb.getBorder());
                hsb.setBorder(hsbBorder);
            }
        }

        protected void uninstallDefaults(JScrollPane c) {
            super.uninstallDefaults(c);

            JScrollBar vsb = scrollpane.getVerticalScrollBar();
            if (vsb != null) {
                if (vsb.getBorder() == vsbBorder) {
                    vsb.setBorder(null);
                }
                vsbBorder = null;
            }

            JScrollBar hsb = scrollpane.getHorizontalScrollBar();
            if (hsb != null) {
                if (hsb.getBorder() == hsbBorder) {
                    hsb.setBorder(null);
                }
                hsbBorder = null;
            }
        }


    }


    class AWTTextPane extends JScrollPane implements FocusListener {

        JTextArea jtext;
        XWindow xwin;

        Color control = SystemColor.control;
        Color focus = SystemColor.activeCaptionBorder; 

        public AWTTextPane(JTextArea jt, XWindow xwin, Container parent) {
            super(jt);
            this.xwin = xwin;
            jt.addFocusListener(this);
            ComponentAccessor.setParent(this,parent); 
            setViewportBorder(new BevelBorder(false,SystemColor.controlDkShadow,SystemColor.controlLtHighlight) );
            this.jtext = jt;
            setFocusable(false);
            addNotify();
        }

        public void focusGained(FocusEvent e) {
            Graphics g = getGraphics();
            Rectangle r = getViewportBorderBounds();
            g.setColor(focus);
            g.drawRect(r.x,r.y,r.width,r.height); 
            g.dispose();
        }

        public void focusLost(FocusEvent e) {
            Graphics g = getGraphics();
            Rectangle r = getViewportBorderBounds();
            g.setColor(control);
            g.drawRect(r.x,r.y,r.width,r.height); 
            g.dispose();
        }

        public Window getRealParent() {
            return (Window) xwin.target;
        }

        public ComponentPeer getPeer() {
            return (ComponentPeer) (xwin);
        }
        public void updateUI() {
            ComponentUI ui = new XAWTScrollPaneUI();
            setUI(ui);
        }


        public JScrollBar createVerticalScrollBar() {
            return new XAWTScrollBar(JScrollBar.VERTICAL);
        }

        public JScrollBar createHorizontalScrollBar() {
            return new XAWTScrollBar(JScrollBar.HORIZONTAL);
        }



        public JTextArea getTextArea () {
            return this.jtext;
        }

        public Graphics getGraphics() {
            return xwin.getGraphics();
        }


        public void processMouseEventPublic(MouseEvent e) {
            processMouseEvent(e);
        }

        public void processMouseMotionEventPublic(MouseEvent e) {
            processMouseMotionEvent(e);
        }


        class XAWTScrollBar extends ScrollBar {

            public XAWTScrollBar(int i) {
                super(i);
                setFocusable(false);
            }

            public void updateUI() {
                ComponentUI ui = new XAWTScrollBarUI();
                setUI(ui);
            }
        }
    }

    static class BevelBorder extends AbstractBorder implements UIResource {
        private Color darkShadow = SystemColor.controlDkShadow;
        private Color lightShadow = SystemColor.controlLtHighlight;
        private Color control = SystemColor.controlShadow;
        private boolean isRaised;

        public BevelBorder(boolean isRaised, Color darkShadow, Color lightShadow) {
            this.isRaised = isRaised;
            this.darkShadow = darkShadow;
            this.lightShadow = lightShadow;
        }

        public void paintBorder(Component c, Graphics g, int x, int y, int w, int h) {
            g.setColor((isRaised) ? lightShadow : darkShadow);
            g.drawLine(x, y, x+w-1, y);           // top
            g.drawLine(x, y+h-1, x, y+1);         // left

            g.setColor(control);
            g.drawLine(x+1, y+1, x+w-2, y+1);           // top
            g.drawLine(x+1, y+h-1, x+1, y+1);         // left

            g.setColor((isRaised) ? darkShadow : lightShadow);
            g.drawLine(x+1, y+h-1, x+w-1, y+h-1); // bottom
            g.drawLine(x+w-1, y+h-1, x+w-1, y+1); // right

            g.setColor(control);
            g.drawLine(x+1, y+h-2, x+w-2, y+h-2); // bottom
            g.drawLine(x+w-2, y+h-2, x+w-2, y+1); // right
        }

        public Insets getBorderInsets(Component c) { 
            return getBorderInsets(c, new Insets(0,0,0,0));
        }

        public Insets getBorderInsets(Component c, Insets insets) {
            insets.top = insets.left = insets.bottom = insets.right = 2;
            return insets;
        }

        public boolean isOpaque(Component c) { 
            return true;
        }

    }
}
