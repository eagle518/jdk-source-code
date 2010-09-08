/*
 * @(#)TextRenderTests.java	1.6 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/*
 * (C) Copyright IBM Corp. 2003, All Rights Reserved.
 * This technology is protected by multiple US and International
 * patents. This notice and attribution to IBM may not be removed.
 */

package j2dbench.tests.text;

import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.font.GlyphVector;
import java.awt.font.TextLayout;

import j2dbench.Group;
import j2dbench.Result;
import j2dbench.TestEnvironment;

public abstract class TextRenderTests extends TextTests {
    static Group renderroot;
    static Group rendertestroot;

    public static void init() {
	renderroot = new Group(textroot, "Rendering", "Rendering Benchmarks");
	rendertestroot = new Group(renderroot, "tests", "Rendering Tests");

	new DrawStrings();
        new DrawChars();
	new DrawBytes();

	if (hasGraphics2D) {
            new DrawGlyphVectors();
            new DrawTextLayouts();
	}
    }

    public TextRenderTests(Group parent, String nodeName, String description) {
	super(parent, nodeName, description);
    }

    public static class DrawStrings extends TextRenderTests {
	public DrawStrings() {
	    super(rendertestroot, "drawString", "Drawing Strings");
	}

	public void runTest(Object ctx, int numReps) {
	    TextContext tctx = (TextContext)ctx;
            Graphics g = tctx.graphics;
            g.setFont(tctx.font);
            String text = tctx.text;
            do {
                g.drawString(text, 40, 40);
            } while (--numReps >= 0);
	}
    }

    public static class DrawChars extends TextRenderTests {
	public DrawChars() {
	    super(rendertestroot, "drawChars", "Drawing Char Arrays");
	}

	public void runTest(Object ctx, int numReps) {
	    TextContext tctx = (TextContext)ctx;
            Graphics g = tctx.graphics;
            char[] chars = tctx.chars;
            g.setFont(tctx.font);
            do {
                g.drawChars(chars, 0, chars.length, 40, 40);
            } while (--numReps >= 0);
	}
    }

    public static class DrawBytes extends TextRenderTests {
	public DrawBytes() {
	    super(rendertestroot, "drawBytes", "Drawing Byte Arrays");
	}

	public void runTest(Object ctx, int numReps) {
	    TextContext tctx = (TextContext)ctx;
            Graphics g = tctx.graphics;
            g.setFont(tctx.font);
            try {
                byte[] bytes = tctx.text.getBytes("ASCII"); // only good for english
                do {
                    g.drawBytes(bytes, 0, bytes.length, 40, 40);
                } while (--numReps >= 0);
            }
            catch (Exception e) {
                throw new RuntimeException(e);
            }
	}
    }

    public static class GVContext extends G2DContext {
        GlyphVector gv;

        public void init(TestEnvironment env, Result results) {
            super.init(env, results);
            gv = font.createGlyphVector(frc, text);
        }
    }

    public static class DrawGlyphVectors extends TextRenderTests {
	public DrawGlyphVectors() {
	    super(rendertestroot, "drawGlyphVectors", "Drawing GlyphVectors");
	}

        public Context createContext() {
            return new GVContext();
        }

	public void runTest(Object ctx, int numReps) {
	    GVContext gvctx = (GVContext)ctx;
            Graphics2D g2d = gvctx.g2d;
            GlyphVector gv = gvctx.gv;
            do {
                g2d.drawGlyphVector(gv, 40, 40);
            } while (--numReps >= 0);
	}
    }

    public static class TLContext extends G2DContext {
        TextLayout tl;

        public void init(TestEnvironment env, Result results) {
            super.init(env, results);
            tl = new TextLayout(text, font, frc);
        }
    }

    public static class DrawTextLayouts extends TextRenderTests {
	public DrawTextLayouts() {
	    super(rendertestroot, "drawTextLayout", "Drawing TextLayouts");
	}

        public Context createContext() {
            return new TLContext();
        }

	public void runTest(Object ctx, int numReps) {
	    TLContext tlctx = (TLContext)ctx;
            Graphics2D g2d = tlctx.g2d;
            TextLayout tl = tlctx.tl;
            do {
                tl.draw(g2d, 40, 40);
            } while (--numReps >= 0);
	}
    }
}
