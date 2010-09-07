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
package com.sun.hotspot.igv.view.actions;

import com.sun.hotspot.igv.view.DiagramViewModel;
import com.sun.hotspot.igv.data.ChangedListener;
import com.sun.hotspot.igv.util.ContextAction;
import javax.swing.Action;
import javax.swing.ImageIcon;
import org.openide.util.HelpCtx;
import org.openide.util.Lookup;
import org.openide.util.NbBundle;
import org.openide.util.Utilities;

/**
 *
 * @author Thomas Wuerthinger
 */
public final class NextDiagramAction extends ContextAction<DiagramViewModel> implements ChangedListener<DiagramViewModel> {

    private DiagramViewModel model;

    public NextDiagramAction() {
        this(Utilities.actionsGlobalContext());
    }

    public NextDiagramAction(Lookup lookup) {
        putValue(Action.SHORT_DESCRIPTION, "Show next graph of current group");
        putValue(Action.SMALL_ICON, new ImageIcon(Utilities.loadImage("com/sun/hotspot/igv/view/images/next_diagram.png")));
    }

    public String getName() {
        return NbBundle.getMessage(NextDiagramAction.class, "CTL_NextDiagramAction");
    }

    public HelpCtx getHelpCtx() {
        return HelpCtx.DEFAULT_HELP;
    }

    @Override
    public Class<DiagramViewModel> contextClass() {
        return DiagramViewModel.class;
    }

    @Override
    public void performAction(DiagramViewModel model) {
        int fp = model.getFirstPosition();
        int sp = model.getSecondPosition();
        if (sp != model.getPositions().size() - 1) {
            int nfp = fp + 1;
            int nsp = sp + 1;
            model.setPositions(nfp, nsp);
        }
    }

    @Override
    public void update(DiagramViewModel model) {
        super.update(model);

        if (this.model != model) {
            if (this.model != null) {
                this.model.getDiagramChangedEvent().removeListener(this);
            }

            this.model = model;
            if (this.model != null) {
                this.model.getDiagramChangedEvent().addListener(this);
            }
        }
    }

    @Override
    public boolean isEnabled(DiagramViewModel model) {
        return model.getSecondPosition() != model.getPositions().size() - 1;
    }

    public Action createContextAwareInstance(Lookup arg0) {
        return new NextDiagramAction(arg0);
    }

    public void changed(DiagramViewModel source) {
        update(source);
    }
}
