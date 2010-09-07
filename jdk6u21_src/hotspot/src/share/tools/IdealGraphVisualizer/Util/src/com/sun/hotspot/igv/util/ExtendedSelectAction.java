/*
 * Copyright (c) 2008, Oracle and/or its affiliates. All rights reserved.
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
 */
package com.sun.hotspot.igv.util;

import java.awt.event.MouseEvent;
import javax.swing.JPanel;
import org.netbeans.api.visual.action.ActionFactory;
import org.netbeans.api.visual.action.SelectProvider;
import org.netbeans.api.visual.action.WidgetAction;
import org.netbeans.api.visual.action.WidgetAction.State;
import org.netbeans.api.visual.action.WidgetAction.WidgetKeyEvent;
import org.netbeans.api.visual.action.WidgetAction.WidgetMouseEvent;
import org.netbeans.api.visual.widget.Widget;

/**
 *
 * @author Thomas Wuerthinger
 */
public class ExtendedSelectAction extends WidgetAction.Adapter {

    private WidgetAction innerAction;
    private JPanel panel;

    public ExtendedSelectAction(SelectProvider provider) {
        innerAction = ActionFactory.createSelectAction(provider);
        panel = new JPanel();
    }

    @Override
    public State mousePressed(Widget widget, WidgetMouseEvent event) {
        // TODO: Solve this differently?
        if (event.getButton() != MouseEvent.BUTTON2) {
            return innerAction.mousePressed(widget, new WidgetMouseEvent(event.getEventID(), new MouseEvent(panel, (int) event.getEventID(), event.getWhen(), event.getModifiersEx(), event.getPoint().x, event.getPoint().y, event.getClickCount(), event.isPopupTrigger(), MouseEvent.BUTTON1)));
        } else {
            return super.mousePressed(widget, event);
        }
    }

    @Override
    public State mouseReleased(Widget widget, WidgetMouseEvent event) {
        return innerAction.mouseReleased(widget, event);
    }

    @Override
    public State keyTyped(Widget widget, WidgetKeyEvent event) {
        return innerAction.keyTyped(widget, event);
    }
}
