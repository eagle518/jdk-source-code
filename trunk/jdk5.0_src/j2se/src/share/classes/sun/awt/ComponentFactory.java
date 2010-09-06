/*
 * @(#)ComponentFactory.java	1.5 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.awt;

import java.awt.*;
import java.awt.dnd.*;
import java.awt.dnd.peer.DragSourceContextPeer;
import java.awt.peer.*;

/**
 * Interface for component creation support in toolkits
 */
public interface ComponentFactory {

    CanvasPeer createCanvas(Canvas target) throws HeadlessException;

    PanelPeer createPanel(Panel target) throws HeadlessException;

    WindowPeer createWindow(Window target) throws HeadlessException;

    FramePeer createFrame(Frame target) throws HeadlessException;

    DialogPeer createDialog(Dialog target) throws HeadlessException;
    
    ButtonPeer createButton(Button target) throws HeadlessException;
    
    TextFieldPeer createTextField(TextField target)
        throws HeadlessException;
    
    ChoicePeer createChoice(Choice target) throws HeadlessException;
    
    LabelPeer createLabel(Label target) throws HeadlessException;
    
    ListPeer createList(List target) throws HeadlessException;
    
    CheckboxPeer createCheckbox(Checkbox target)
        throws HeadlessException;
    
    ScrollbarPeer createScrollbar(Scrollbar target)
        throws HeadlessException;
    
    ScrollPanePeer createScrollPane(ScrollPane target)
        throws HeadlessException;
    
    TextAreaPeer createTextArea(TextArea target)
        throws HeadlessException;
    
    FileDialogPeer createFileDialog(FileDialog target)
        throws HeadlessException;
    
    MenuBarPeer createMenuBar(MenuBar target) throws HeadlessException;
    
    MenuPeer createMenu(Menu target) throws HeadlessException;
    
    PopupMenuPeer createPopupMenu(PopupMenu target)
        throws HeadlessException;
    
    MenuItemPeer createMenuItem(MenuItem target)
        throws HeadlessException;
    
    CheckboxMenuItemPeer createCheckboxMenuItem(CheckboxMenuItem target)
        throws HeadlessException;
    
    DragSourceContextPeer createDragSourceContextPeer(
        DragGestureEvent dge)
        throws InvalidDnDOperationException, HeadlessException;
    
    FontPeer getFontPeer(String name, int style);
    
    RobotPeer createRobot(Robot target, GraphicsDevice screen)
        throws AWTException, HeadlessException;
    
}
