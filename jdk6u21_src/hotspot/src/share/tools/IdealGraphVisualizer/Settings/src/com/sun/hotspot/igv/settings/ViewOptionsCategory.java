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
package com.sun.hotspot.igv.settings;

import javax.swing.Icon;
import javax.swing.ImageIcon;
import org.netbeans.spi.options.OptionsCategory;
import org.netbeans.spi.options.OptionsPanelController;
import org.openide.util.NbBundle;
import org.openide.util.Utilities;

/**
 *
 * @author Thomas Wuerthinger
 */
public final class ViewOptionsCategory extends OptionsCategory {

    @Override
    public Icon getIcon() {
        return new ImageIcon(Utilities.loadImage("com/sun/hotspot/igv/settings/settings.gif"));
    }

    public String getCategoryName() {
        return NbBundle.getMessage(ViewOptionsCategory.class, "OptionsCategory_Name_View");
    }

    public String getTitle() {
        return NbBundle.getMessage(ViewOptionsCategory.class, "OptionsCategory_Title_View");
    }

    public OptionsPanelController create() {
        return new ViewOptionsPanelController();
    }
}
