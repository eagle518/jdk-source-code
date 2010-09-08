/*
 * @(#)MiscTests.java	1.3 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package j2dbench.tests;

import java.awt.Dimension;
import java.awt.Graphics;

import j2dbench.Group;
import j2dbench.Option;
import j2dbench.TestEnvironment;

public abstract class MiscTests extends GraphicsTests {
    static Group miscroot;
    static Group copytestroot;

    public MiscTests(Group parent, String nodeName, String description) {
	super(parent, nodeName, description);
    }

    public static void init() {
        miscroot = new Group(graphicsroot, "misc",
                             "Misc Benchmarks");
        copytestroot = new Group(miscroot, "copytests",
                                 "copyArea() Tests");

        new CopyArea("copyAreaVert", "Vertical copyArea()", 0, 1);
        new CopyArea("copyAreaHoriz", "Horizontal copyArea()", 1, 0);
        new CopyArea("copyAreaDiag", "Diagonal copyArea()", 1, 1);
    }

    private static class CopyArea extends MiscTests {
        private int dx, dy;

        CopyArea(String nodeName, String desc, int dx, int dy) {
            super(copytestroot, nodeName, desc);
            this.dx = dx;
            this.dy = dy;
        }

	public Dimension getOutputSize(int w, int h) {
            // we add one to each dimension to avoid copying outside the
            // bounds of the destination when "bounce" is enabled
            return new Dimension(w+1, h+1);
        }

        public void runTest(Object ctx, int numReps) {
            GraphicsTests.Context gctx = (GraphicsTests.Context)ctx;
            int size = gctx.size;
	    int x = gctx.initX;
	    int y = gctx.initY;
	    Graphics g = gctx.graphics;
	    g.translate(gctx.orgX, gctx.orgY);
	    if (gctx.animate) {
		do {
                    g.copyArea(x, y, size, size, dx, dy);
		    if ((x -= 3) < 0) x += gctx.maxX;
		    if ((y -= 1) < 0) y += gctx.maxY;
		} while (--numReps > 0);
	    } else {
		do {
                    g.copyArea(x, y, size, size, dx, dy);
		} while (--numReps > 0);
	    }
	    g.translate(-gctx.orgX, -gctx.orgY);
        }
    }
}

