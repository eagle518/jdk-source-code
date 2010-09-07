/*
 * @(#)XChoicePeer.java	1.41 04/03/01
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt.X11;

import java.awt.*;
import java.awt.peer.*;
import java.awt.event.*;
import java.util.Vector;
import java.util.List;
import java.util.logging.*;

// FIXME: tab traversal should be disabled when mouse is captured (4816336)

// FIXME: key and mouse events should not be delivered to listeners when the Choice is unfurled.  Must override handleNativeKey/MouseEvent (4816336)

// FIXME: test programmatic add/remove/clear/etc

// FIXME: account for unfurling at the edge of the screen
// Note: can't set x,y on layout(), 'cause moving the top-level to the
// edge of the screen won't call layout().  Just do it on paint, I guess

// TODO: make painting more efficient (i.e. when down arrow is pressed, only two items should need to be repainted.

public class XChoicePeer extends XComponentPeer implements ChoicePeer {
    private static final Logger log = Logger.getLogger("sun.awt.X11.XChoicePeer");

    private static final int MAX_UNFURLED_ITEMS = 10;  // Maximum number of
    // items to be displayed
    // at a time in an
    // unfurled Choice
    // Description of these constants in ListHelper
    public final static int TEXT_SPACE = 1;
    public final static int BORDER_WIDTH = 1;
    public final static int ITEM_MARGIN = 1;
    public final static int SCROLLBAR_WIDTH = 15;


    // SHARE THESE!
    private static final Insets focusInsets = new Insets(0,0,0,0);
    private static final Insets borderInsets = new Insets(2,2,2,2);


    static final int WIDGET_OFFSET = 18;

    // Stolen from Tiny
    static final int            TEXT_XPAD = 8;
    static final int            TEXT_YPAD = 6;

    // FIXME: Motif uses a different focus color for the item within
    // the unfurled Choice list and for when the Choice itself is focused and
    // popped up.
    static final Color focusColor = Color.black;

    // TODO: there is a time value that the mouse is held down.  If short
    // enough,  the Choice stays popped down.  If long enough, Choice 
    // is furled when the mouse is released

    private boolean unfurled = false;        // Choice list is popped down

    private boolean dragging = false;        // Mouse was pressed and is being
                                             // dragged over the (unfurled)
                                             // Choice
    
    private boolean mouseInSB = false;       // Mouse is interacting with the
                                             // scrollbar

    private boolean firstPress = false;      // mouse was pressed on
                                             // furled Choice so we
                                             // not need to furl the
                                             // Choice when MOUSE_RELEASED occured
    private ListHelper helper;
    private UnfurledChoice unfurledChoice;

    // TODO: Choice remembers where it was scrolled to when unfurled - it's not 
    // always to the currently selected item.

    private Rectangle textRect;
    private Rectangle focusRect;

    // Indicates whether or not to paint selected item in the choice.
    // Default is to paint
    private boolean drawSelectedItem = true;

    // If set, indicates components under which choice popup should be showed.
    // The choice's popup width and location should be adjust to appear
    // under both choice and alignUnder component.
    private Component alignUnder;

    // If cursor is outside of an unfurled Choice when the mouse is
    // released, Choice item should NOT be updated.  Remember the proper index.
    private int dragStartIdx = -1;

    XChoicePeer(Choice target) {
        super(target);
    }

    void preInit(XCreateWindowParams params) {
        super.preInit(params);
        Choice target = (Choice)this.target;
        int numItems = target.getItemCount();
        unfurledChoice = new UnfurledChoice(target);

        helper = new ListHelper(unfurledChoice,
                                getGUIcolors(),
                                numItems,
                                false,
                                true,
                                false,
                                target.getFont(),
                                MAX_UNFURLED_ITEMS,
                                TEXT_SPACE,
                                ITEM_MARGIN,
                                BORDER_WIDTH,
                                SCROLLBAR_WIDTH);
    }

    void postInit(XCreateWindowParams params) {
        super.postInit(params);
        Choice target = (Choice)this.target;
        int numItems = target.getItemCount();

        // Add all items
        for (int i = 0; i < numItems; i++) {
            helper.add(target.getItem(i));
        }
        if (!helper.isEmpty()) {
            helper.select(target.getSelectedIndex());
            helper.setFocusedIndex(target.getSelectedIndex());
        }
        helper.updateColors(getGUIcolors());
        updateMotifColors(getPeerBackground());
    }

    public boolean isFocusable() { return true; }

    public void focusGained(FocusEvent e) { 
        // TODO: only need to paint the focus bit
        super.focusGained(e);
        repaint();
    }

    public void focusLost(FocusEvent e) { 
        // TODO: only need to paint the focus bit?
        super.focusLost(e);
        repaint();
    }

    void handleJavaKeyEvent(KeyEvent e) {
        if (e.getID() == KeyEvent.KEY_PRESSED) { 
            keyPressed(e);
        }
    }

    public void keyPressed(KeyEvent e) {
        switch(e.getKeyCode()) {
            // UP & DOWN are same if furled or unfurled
          case KeyEvent.VK_DOWN:
          case KeyEvent.VK_KP_DOWN: {
              if (helper.getItemCount() > 1) {
                  helper.down();
                  int newIdx = helper.getSelectedIndex();

                  ((Choice)target).select(newIdx);
                  postEvent(new ItemEvent((Choice)target,
                                          ItemEvent.ITEM_STATE_CHANGED,
                                          ((Choice)target).getItem(newIdx),
                                          ItemEvent.SELECTED));
                  repaint();
              }
              break;
          }
          case KeyEvent.VK_UP:
          case KeyEvent.VK_KP_UP: {
              if (helper.getItemCount() > 1) {
                  helper.up();
                  int newIdx = helper.getSelectedIndex();

                  ((Choice)target).select(newIdx);
                  postEvent(new ItemEvent((Choice)target,
                                          ItemEvent.ITEM_STATE_CHANGED,
                                          ((Choice)target).getItem(newIdx),
                                          ItemEvent.SELECTED));
                  repaint();
              }
              break;
          }
          case KeyEvent.VK_PAGE_DOWN:
              if (unfurled && !dragging) {
                  int oldIdx = helper.getSelectedIndex();
                  helper.pageDown();
                  int newIdx = helper.getSelectedIndex();
                  if (oldIdx != newIdx) {
                      ((Choice)target).select(newIdx);
                      postEvent(new ItemEvent((Choice)target,
                                              ItemEvent.ITEM_STATE_CHANGED,
                                              ((Choice)target).getItem(newIdx),
                                              ItemEvent.SELECTED));
                      repaint();
                  }
              }
              break;
          case KeyEvent.VK_PAGE_UP:
              if (unfurled && !dragging) {
                  int oldIdx = helper.getSelectedIndex();
                  helper.pageUp();
                  int newIdx = helper.getSelectedIndex();
                  if (oldIdx != newIdx) {
                      ((Choice)target).select(newIdx);
                      postEvent(new ItemEvent((Choice)target,
                                              ItemEvent.ITEM_STATE_CHANGED,
                                              ((Choice)target).getItem(newIdx),
                                              ItemEvent.SELECTED));
                      repaint();
                  }
              }
              break;
          case KeyEvent.VK_ESCAPE:
          case KeyEvent.VK_ENTER:
              if (unfurled) {
                  unfurled = false;
                  dragging = false;
                  mouseInSB = false;
                  ungrabInput();
                  unfurledChoice.setVisible(false);
              }
              break;
          default:
              if (unfurled) {
                  Toolkit.getDefaultToolkit().beep();
              }
              break;
        }
    }

    public boolean handlesWheelScrolling() { return true; }

    void handleJavaMouseWheelEvent(MouseWheelEvent e) {
        if (unfurled && helper.isVSBVisible()) {
            if (ListHelper.doWheelScroll(helper.getVSB(), null, e)) {
                repaint();
            }
        }
    }

    void handleJavaMouseEvent(MouseEvent e) {
        super.handleJavaMouseEvent(e);
        int i = e.getID();
        switch (i) {
          case MouseEvent.MOUSE_PRESSED: 
              mousePressed(e);
              break;
          case MouseEvent.MOUSE_RELEASED:
              mouseReleased(e);
              break;
          case MouseEvent.MOUSE_DRAGGED:
              mouseDragged(e);
              break;
        }
    }

    public void mousePressed(MouseEvent e) {
        dragStartIdx = helper.getSelectedIndex();
        if (unfurled) {
            // Press on unfurled Choice.  Highlight the item under the cursor,
            // but don't send item event or set the text on the button yet
            unfurledChoice.trackMouse(e);
        }
        else {
            // Choice is up - unfurl it
            grabInput();
            unfurledChoice.toFront();
            firstPress = true;
            unfurled = true;
        }
    }

    public void mouseReleased(MouseEvent e) {
        if (unfurled) {
            if (mouseInSB) {
                unfurledChoice.trackMouse(e);
            }
            else {
                // We pressed and dragged onto the Choice, or, this is the
                // second release after clicking to make the Choice "stick"
                // unfurled.
                // This release should ungrab/furl, and set the new item if
                // release was over the unfurled Choice.
                if ( !firstPress || unfurledChoice.isMouseEventInside( e ) ) {
                    ungrabInput();
                    unfurledChoice.setVisible(false);
                    unfurled = false;
                }
                
                if (!helper.isEmpty()) {
                    // Only update the Choice if the mouse button is released
                    // over the list of items.
                    if (unfurledChoice.isMouseInListArea(e)) {
                        int newIdx = helper.getSelectedIndex();
                        if (newIdx >= 0) {
                            // Update the selected item in the target now that
                            // the mouse selection is complete.
                            if (newIdx != dragStartIdx) {
                                ((Choice)target).select(newIdx);
                                // NOTE: We get a repaint when Choice.select()
                                // calls our peer.select().
                            }
                            postEvent(new ItemEvent((Choice)target,
                                ItemEvent.ITEM_STATE_CHANGED,
                                ((Choice)target).getItem(newIdx),
                                ItemEvent.SELECTED));
                        }
                    }
                    else {
                        // Set the selected item back how it was.
                        helper.select(dragStartIdx);
                    }
                }
            }
        }
        dragging = false;
        firstPress = false;
        dragStartIdx = -1;
    }

    public void mouseDragged(MouseEvent e) {
        dragging = true;
        unfurledChoice.trackMouse(e);
    }

    // Stolen from TinyChoicePeer
    public Dimension getMinimumSize() {
        // TODO: move this impl into ListHelper?
        FontMetrics fm = getFontMetrics(target.getFont());
        Choice c = (Choice)target;
        int w = 0;
        for (int i = c.countItems() ; i-- > 0 ;) {
            w = Math.max(fm.stringWidth(c.getItem(i)), w);
        }
        return new Dimension(w + TEXT_XPAD + WIDGET_OFFSET,
                             fm.getMaxAscent() + fm.getMaxDescent() + TEXT_YPAD);
    }

    /*
     * Layout the...
     */
    public void layout() {
        /*
          Dimension size = target.getSize();
          Font f = target.getFont();
          FontMetrics fm = target.getFontMetrics(f);
          String text = ((Choice)target).getLabel();

          textRect.height = fm.getHeight();

          checkBoxSize = getChoiceSize(fm);

          // Note - Motif appears to use an left inset that is slightly
          // scaled to the checkbox/font size.
          cbX = borderInsets.left + checkBoxInsetFromText;
          cbY = size.height / 2 - checkBoxSize / 2;
          int minTextX = borderInsets.left + 2 * checkBoxInsetFromText + checkBoxSize;
          // FIXME: will need to account for alignment?
          // FIXME: call layout() on alignment changes
          //textRect.width = fm.stringWidth(text);
          textRect.width = fm.stringWidth(text == null ? "" : text);
          textRect.x = Math.max(minTextX, size.width / 2 - textRect.width / 2);
          textRect.y = size.height / 2 - textRect.height / 2 + borderInsets.top;

          focusRect.x = focusInsets.left;
          focusRect.y = focusInsets.top;
          focusRect.width = size.width-(focusInsets.left+focusInsets.right)-1;
          focusRect.height = size.height-(focusInsets.top+focusInsets.bottom)-1;

          myCheckMark = AffineTransform.getScaleInstance((double)target.getFont().getSize() / MASTER_SIZE, (double)target.getFont().getSize() / MASTER_SIZE).createTransformedShape(MASTER_CHECKMARK);
        */

    }

    /**
     * Paint the choice
     */
    public void paint(Graphics g) {
        Dimension size = getPeerSize();

        // TODO: when mouse is down over button, widget should be drawn depressed
        g.setColor(getPeerBackground());
        g.fillRect(0, 0, width, height);

        drawMotif3DRect(g, 1, 1, width-2, height-2, false);
        drawMotif3DRect(g, width - WIDGET_OFFSET, (height / 2) - 3, 12, 6, false);

        if (!helper.isEmpty() && helper.getSelectedIndex() != -1) {
            g.setFont(getPeerFont());
            FontMetrics fm = g.getFontMetrics();
            String lbl = helper.getItem(helper.getSelectedIndex());
            if (lbl != null && drawSelectedItem) {
                g.setClip(1, 1, width - WIDGET_OFFSET - 2, height);
                if (isEnabled()) {
                    g.setColor(getPeerForeground());
                    g.drawString(lbl, 5, (height + fm.getMaxAscent()-fm.getMaxDescent())/2);        
                }
                else {
                    g.setColor(getPeerBackground().brighter());
                    g.drawString(lbl, 5, (height + fm.getMaxAscent()-fm.getMaxDescent())/2);        
                    g.setColor(getPeerBackground().darker());
                    g.drawString(lbl, 4, ((height + fm.getMaxAscent()-fm.getMaxDescent())/2)-1);
                }
                g.setClip(0, 0, width, height);
            }
        }
        if (hasFocus()) {            
            paintFocus(g,focusInsets.left,focusInsets.top,size.width-(focusInsets.left+focusInsets.right)-1,size.height-(focusInsets.top+focusInsets.bottom)-1);
        }
        if (unfurled) {
            unfurledChoice.repaint();
        }
    }

    protected void paintFocus(Graphics g, 
                              int x, int y, int w, int h) {
        g.setColor(focusColor);
        g.drawRect(x,y,w,h);
    } 



    /*
     * ChoiePeer methods stolen from TinyChoicePeer
     */

    public void select(int index) {
        helper.select(index);
        helper.setFocusedIndex(index);
        repaint();
    }

    public void add(String item, int index) {
        helper.add(item, index);
        repaint();
    }

    public void remove(int index) {
        if (index == helper.getSelectedIndex()) {
            helper.remove(index);
            if (helper.isEmpty()) {
                helper.select(-1);
            }
            else {
                helper.select(0);
                postEvent(new ItemEvent((Choice)target,
                                        ItemEvent.ITEM_STATE_CHANGED,
                                        ((Choice)target).getItem(0),
                                        ItemEvent.SELECTED));
            }
            repaint();
        }
        else {
            helper.remove(index);
        }
    }

    public void removeAll() {
        helper.removeAll();
        helper.select(-1);
        repaint();
    }

    /**
     * DEPRECATED: Replaced by add(String, int).
     */
    public void addItem(String item, int index) {
        add(item, index);
    }

    public void setFont(Font font) {
        super.setFont(font);
        helper.setFont(this.font);
    }

    public void setForeground(Color c) {
        super.setForeground(c);
        helper.updateColors(getGUIcolors());
    }

    public void setBackground(Color c) {
        super.setBackground(c);
        unfurledChoice.setBackground(c);
        helper.updateColors(getGUIcolors());
        updateMotifColors(c);
    }

    public void setDrawSelectedItem(boolean value) {
        drawSelectedItem = value;
    }

    public void setAlignUnder(Component comp) {
        alignUnder = comp;
    }
    /**************************************************************************/
    /* Common functionality between List & Choice
       /**************************************************************************/

    /**
     * Inner class for the unfurled Choice list
     * Much, much more docs
     */
    class UnfurledChoice extends XWindow /*implements XScrollbarClient*/ {
 
        // First try - use Choice as the target

        public UnfurledChoice(Component target) {
            super(target);
        }

        // Override so we can do our own create()
        public void preInit(XCreateWindowParams params) {
            super.preInit(params);
            // Reset parent window, bounds(we'll set them later), set overrideRedirect
            params.add(PARENT_WINDOW, (long)0);
            params.remove(BOUNDS);
            params.add(OVERRIDE_REDIRECT, Boolean.TRUE);
        }

        // Generally, bounds should be:
        //  x = target.x
        //  y = target.y + target.height
        //  w = Max(target.width, getLongestItemWidth) + possible vertScrollbar
        //  h = Min(MAX_UNFURLED_ITEMS, target.getItemCount()) * itemHeight
        public void toFront() {

            // TODO: do this in layout()

            int numItemsDisplayed;
            // Motif paints an empty Choice the same size as a single item
            if (helper.isEmpty()) {
                numItemsDisplayed = 1;
            }
            else {
                int numItems = helper.getItemCount();
                numItemsDisplayed = Math.min(MAX_UNFURLED_ITEMS, numItems); 
            } 
            Point global = XChoicePeer.this.toGlobal(0,0);
            Dimension screen = Toolkit.getDefaultToolkit().getScreenSize();

            if (alignUnder != null) {
                Rectangle choiceRec = XChoicePeer.this.getBounds();
                choiceRec.setLocation(0, 0);
                choiceRec = XChoicePeer.this.toGlobal(choiceRec);
                Rectangle alignUnderRec = new Rectangle(alignUnder.getLocationOnScreen(), alignUnder.getSize()); // TODO: Security?
                Rectangle result = choiceRec.union(alignUnderRec);
                // we've got the left and width, calculate top and height
                width = result.width;
                x = result.x;
                y = result.y + result.height;
                height = 2*BORDER_WIDTH + 
                    numItemsDisplayed*(helper.getItemHeight()+2*ITEM_MARGIN);
            } else {
                x = global.x;
                y = global.y + XChoicePeer.this.height;
                width = Math.max(XChoicePeer.this.width,
                                 helper.getMaxItemWidth() + 2 * (BORDER_WIDTH + ITEM_MARGIN + TEXT_SPACE) + (helper.isVSBVisible() ? SCROLLBAR_WIDTH : 0));
                height = 2*BORDER_WIDTH + 
                    numItemsDisplayed*(helper.getItemHeight()+2*ITEM_MARGIN);
            }
            // Don't run off the edge of the screen
            if (x < 0) {
                x = 0;
            }
            else if (x + width > screen.width) {
                x = screen.width - width;
            }
            if (y < 0) {
                y = 0;
            }
            else if (y + height > screen.height) {
                y = screen.height - height;
            }

            xSetBounds(x, y, width, height);
            super.toFront();
            setVisible(true);
        }

        /*
         * Track a MouseEvent (either a drag or a press) and paint a new
         * selected item, if necessary.
         */
        // FIXME: first unfurl after move is not at edge of the screen  onto second monitor doesn't
        // track mouse correctly.  Problem is w/ UnfurledChoice coords
        public void trackMouse(MouseEvent e) {
            // Event coords are relative to the button, so translate a bit
            Point local = toLocalCoords(e);

            // If x,y is over unfurled Choice,
            // highlight item under cursor

            switch (e.getID()) {
              case MouseEvent.MOUSE_PRESSED:
                  // FIXME: If the Choice is unfurled and the mouse is pressed
                  // outside of the Choice, the mouse should ungrab on the
                  // the press, not the release
                  if (helper.isInVertSB(getBounds(), local.x, local.y)) {
                      mouseInSB = true;
                      helper.handleVSBEvent(e, getBounds(), local.x, local.y);
                  }
                  else {
                      trackSelection(local.x, local.y);
                  }
                  break;
              case MouseEvent.MOUSE_RELEASED:
                  if (mouseInSB) {
                      mouseInSB = false;
                      helper.handleVSBEvent(e, getBounds(), local.x, local.y);
                  }
                  /*
                    else {
                    trackSelection(local.x, local.y);
                    }
                  */
                  break;
              case MouseEvent.MOUSE_DRAGGED:
                  if (mouseInSB) {
                      helper.handleVSBEvent(e, getBounds(), local.x, local.y);
                  }
                  else {
                      trackSelection(local.x, local.y);
                  }
                  break;
            }                 
        }

        private void trackSelection(int transX, int transY) {
            if (!helper.isEmpty()) {
                if (transX > 0 && transX < width &&
                    transY > 0 && transY < height) {
                    int newIdx = helper.y2index(transY);
                    if (log.isLoggable(Level.FINE)) {
                        log.fine("transX=" + transX + ", transY=" + transY
                                 + ",width=" + width + ", height=" + height
                                 + ", newIdx=" + newIdx + " on " + target);
                    }
                    if ((newIdx >=0) && (newIdx < helper.getItemCount())
                        && (newIdx != helper.getSelectedIndex()))
                    {
                        helper.select(newIdx);
                        unfurledChoice.repaint();
                    }
                }
            }
            // FIXME: If dragged off top or bottom, scroll if there's a vsb
            // (ICK - we'll need a timer or our own event or something)
        }

        public void paint(Graphics g) {
            //System.out.println("UC.paint()");
            Choice choice = (Choice)target;
            Color colors[] = XChoicePeer.this.getGUIcolors();
            draw3DRect(g, getSystemColors(), 0, 0, width - 1, height - 1, true);
            draw3DRect(g, getSystemColors(), 1, 1, width - 3, height - 3, true);

            helper.paintAllItems(g,
                                 colors,
                                 getBounds());
        }

        public void setVisible(boolean vis) {
            xSetVisible(vis);

            if (!vis && alignUnder != null) {
                alignUnder.requestFocusInWindow();
            }
        }

        /**
         * Return a MouseEvent's Point in coordinates relative to the
         * UnfurledChoice.
         */
        private Point toLocalCoords(MouseEvent e) {
            // Event coords are relative to the button, so translate a bit
            Point global = XChoicePeer.this.toGlobal(e.getX(),e.getY());

            global.x -= x;
            global.y -= y;
            return global;
        }

        /* Returns true if the MouseEvent coords (which are based on the Choice)
         * are inside of the UnfurledChoice.
         */
        private boolean isMouseEventInside(MouseEvent e) {
            Point local = toLocalCoords(e);
            if (local.x > 0 && local.x < width &&
                local.y > 0 && local.y < height) {
                return true;
            }
            return false;
        }

        /**
         * Tests if the mouse cursor is in the Unfurled Choice, yet not in
         * in the vertical scrollbar
         */
        private boolean isMouseInListArea(MouseEvent e) {
            if (isMouseEventInside(e)) {
                Point local = toLocalCoords(e);
                Rectangle bounds = getBounds();
                if (!helper.isInVertSB(bounds, local.x, local.y)) {
                    return true;
                }
            }
            return false;
        }

        /*
         * Overridden from XWindow() because we don't want to send
         * ComponentEvents
         */
        public void handleConfigureNotifyEvent(long ptr) {}
        public void handleMapNotifyEvent(long ptr) {}
        public void handleUnmapNotifyEvent(long ptr) {}
    }

    public void dispose() {
        if (unfurledChoice != null) {
            unfurledChoice.destroy();
        }
        super.dispose();
    }
}

