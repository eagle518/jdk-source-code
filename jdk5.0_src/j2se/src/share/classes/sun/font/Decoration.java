/*
 *
 * (C) Copyright IBM Corp. 1999-2003, All Rights Reserved
 *
 */

package sun.font;

import java.util.Map;

import java.awt.BasicStroke;
import java.awt.Color;
import java.awt.Graphics2D;
import java.awt.Paint;
import java.awt.Shape;
import java.awt.Stroke;

import java.awt.font.TextAttribute;

import java.awt.geom.Area;
import java.awt.geom.Line2D;
import java.awt.geom.Rectangle2D;
import java.awt.geom.GeneralPath;

/**
 * This class handles underlining, strikethrough, and foreground and
 * background styles on text.  Clients simply acquire instances
 * of this class and hand them off to ExtendedTextLabels or GraphicComponents.
 */
public class Decoration {
    
    /**
     * This interface is implemented by clients that use Decoration.
     * Unfortunately, interface methods have to public;  ideally these
     * would be package-private.
     */
    public interface Label {
        CoreMetrics getCoreMetrics();
        Rectangle2D getLogicalBounds();

        void handleDraw(Graphics2D g2d, float x, float y);
        Rectangle2D handleGetCharVisualBounds(int index);
        Rectangle2D handleGetVisualBounds();
        Shape handleGetOutline(float x, float y);
    }
    
    private Decoration() {
    }
    
    /**
     * Return a Decoration which does nothing.
     */
    public static Decoration getPlainDecoration() {
        
        return PLAIN;
    }
        
    /** 
     * Return a Decoration appropriate for the the given Map.
     * @param attributes the Map used to determine the Decoration
     */
    public static Decoration getDecoration(Map attributes) {
        if (attributes == null) {
            return PLAIN;
        }

        Object foreground = attributes.get(TextAttribute.FOREGROUND);
        Object background = attributes.get(TextAttribute.BACKGROUND);
        
        Object value = attributes.get(TextAttribute.SWAP_COLORS);
        boolean swapColors = TextAttribute.SWAP_COLORS_ON.equals(value);
        value = attributes.get(TextAttribute.STRIKETHROUGH);
        boolean strikethrough = TextAttribute.STRIKETHROUGH_ON.equals(value);
        
        value = attributes.get(TextAttribute.UNDERLINE);
        Underline stdUnderline = Underline.getUnderline(value);
        value = attributes.get(TextAttribute.INPUT_METHOD_UNDERLINE);
        Underline imUnderline = Underline.getUnderline(value);
        
        if (stdUnderline == null && foreground == null && background == null 
                    && !swapColors && !strikethrough && imUnderline == null) {
            return PLAIN;
        }
        else {
            return new DecorationImpl((Paint)foreground, 
                                      (Paint)background, 
                                      swapColors, 
                                      strikethrough,
                                      stdUnderline,
                                      imUnderline);
        }
    }
    
    public void drawTextAndDecorations(Label label,
                                Graphics2D g2d, 
                                float x, 
                                float y) {
                                
        label.handleDraw(g2d, x, y);
    }
    
    public Rectangle2D getVisualBounds(Label label) {
        
        return label.handleGetVisualBounds();
    }
    
    public Rectangle2D getCharVisualBounds(Label label, int index) {
        
        return label.handleGetCharVisualBounds(index);
    }
    
    Shape getOutline(Label label,
                     float x, 
                     float y) {
        
        return label.handleGetOutline(x, y);
    }
    
    private static final Decoration PLAIN = new Decoration();
    
    private static final class DecorationImpl extends Decoration {
        
        private Paint fgPaint = null;
        private Paint bgPaint = null;
        private boolean swapColors = false;
        private boolean strikethrough = false;
        private Underline stdUnderline = null; // underline from TextAttribute.UNDERLINE_ON
        private Underline imUnderline = null; // input method underline

        DecorationImpl(Paint foreground,
                       Paint background,
                       boolean swapColors,
                       boolean strikethrough,
                       Underline stdUnderline,
                       Underline imUnderline) {
                        
            fgPaint = (Paint) foreground;
            bgPaint = (Paint) background;

            this.swapColors = swapColors;
            this.strikethrough = strikethrough;
                       
            this.stdUnderline = stdUnderline;
            this.imUnderline = imUnderline;
        }
        
        private static boolean areEqual(Object lhs, Object rhs) {
            
            if (lhs == null) {
                return rhs == null;
            }
            else {
                return lhs.equals(rhs);
            }
        }
        
        public boolean equals(Object rhs) {
            
            if (rhs == this) {
                return true;
            }
            if (rhs == null) {
                return false;
            }
            
            DecorationImpl other = null;
            try {
                other = (DecorationImpl) rhs;
            }
            catch(ClassCastException e) {
                return false;
            }
            
            if (!(swapColors == other.swapColors &&
                        strikethrough == other.strikethrough)) {
                return false;
            }
            
            if (!areEqual(stdUnderline, other.stdUnderline)) {
                return false;
            }
            if (!areEqual(fgPaint, other.fgPaint)) {
                return false;
            }
            if (!areEqual(bgPaint, other.bgPaint)) {
                return false;
            }
            return areEqual(imUnderline, other.imUnderline);
        }
        
        public int hashCode() {
            
            int hc = 1;
            if (strikethrough) {
                hc |= 2;
            }
            if (swapColors) {
                hc |= 4;
            }
            if (stdUnderline != null) {
                hc += stdUnderline.hashCode();
            }
            return hc;
        }
        
        /**
        * Return the bottom of the Rectangle which encloses pixels
        * drawn by underlines.
        */
        private float getUnderlineMaxY(CoreMetrics cm) {

            float maxY = 0;
            if (stdUnderline != null) {

                float ulBottom = cm.underlineOffset;
                ulBottom += stdUnderline.getLowerDrawLimit(cm.underlineThickness);
                maxY = Math.max(maxY, ulBottom);
            }

            if (imUnderline != null) {

                float ulBottom = cm.underlineOffset;
                ulBottom += imUnderline.getLowerDrawLimit(cm.underlineThickness);
                maxY = Math.max(maxY, ulBottom);
            }

            return maxY;
        }

        private void drawTextAndEmbellishments(Label label,
                                               Graphics2D g2d,
                                               float x,
                                               float y) {
                                                
            label.handleDraw(g2d, x, y);

            if (!strikethrough && stdUnderline == null && imUnderline == null) {
                return;
            }

            float x1 = x;
            float x2 = x1 + (float)label.getLogicalBounds().getWidth();

            CoreMetrics cm = label.getCoreMetrics();
            if (strikethrough) {

                Stroke saveStroke = g2d.getStroke();
                float strikeY = y + cm.strikethroughOffset;
                g2d.setStroke(new BasicStroke(cm.strikethroughThickness));
                g2d.draw(new Line2D.Float(x1, strikeY, x2, strikeY));
                g2d.setStroke(saveStroke);
            }

            float ulOffset = cm.underlineOffset;
            float ulThickness = cm.underlineThickness;

            if (stdUnderline != null) {
                stdUnderline.drawUnderline(g2d, ulThickness, x1, x2, y + ulOffset);
            }

            if (imUnderline != null) {
                imUnderline.drawUnderline(g2d, ulThickness, x1, x2, y + ulOffset);
            }
        }
        
        public void drawTextAndDecorations(Label label,
                                    Graphics2D g2d, 
                                    float x, 
                                    float y) {
                                    
            if (fgPaint == null && bgPaint == null && swapColors == false) {
                drawTextAndEmbellishments(label, g2d, x, y);
            }
            else {
                Paint savedPaint = g2d.getPaint();
                Paint foreground, background;

                if (swapColors) {
                    foreground = bgPaint==null? Color.white : bgPaint;
                    background = fgPaint==null? savedPaint : fgPaint;
                }
                else {
                    foreground = fgPaint==null? savedPaint : fgPaint;
                    background = bgPaint;
                }

                if (background != null) {

                    Rectangle2D bgArea = label.getLogicalBounds();
                    bgArea = new Rectangle2D.Float(x + (float)bgArea.getX(),
                                                y + (float)bgArea.getY(),
                                                (float)bgArea.getWidth(),
                                                (float)bgArea.getHeight());

                    g2d.setPaint(background);
                    g2d.fill(bgArea);
                }

                g2d.setPaint(foreground);
                drawTextAndEmbellishments(label, g2d, x, y);
                g2d.setPaint(savedPaint);
            }
        }
        
        public Rectangle2D getVisualBounds(Label label) {
            
            Rectangle2D visBounds = label.handleGetVisualBounds();

            if (swapColors || bgPaint != null
                        || stdUnderline != null || imUnderline != null) {

                float minX = 0;
                Rectangle2D lb = label.getLogicalBounds();

                float minY = 0, maxY = 0;

                if (swapColors || bgPaint != null) {

                    minY = (float)lb.getY();
                    maxY = minY + (float)lb.getHeight();
                }

                maxY = Math.max(maxY, getUnderlineMaxY(label.getCoreMetrics()));

                Rectangle2D ab = new Rectangle2D.Float(minX, minY, (float)lb.getWidth(), maxY-minY);
                visBounds.add(ab);
            }

            return visBounds;
        }
        
        Shape getOutline(Label label,
                         float x, 
                         float y) {
            
            if (stdUnderline == null && imUnderline == null) {
                return label.handleGetOutline(x, y);
            }

            CoreMetrics cm = label.getCoreMetrics();
            
            // NOTE:  The performace of the following code may
            // be very poor.
            float ulThickness = cm.underlineThickness;

            Rectangle2D lb = label.getLogicalBounds();
            float x1 = x;
            float x2 = x1 + (float)lb.getWidth();

            Area area = null;

            if (stdUnderline != null) {
                Shape ul = stdUnderline.getUnderlineShape(ulThickness, x1, x2, y);
                area = new Area(ul);
            }

            if (imUnderline != null) {
                Shape ul = imUnderline.getUnderlineShape(ulThickness, x1, x2, y);
                Area ulArea = new Area(ul);
                if (area == null) {
                    area = ulArea;
                }
                else {
                    area.add(ulArea);
                }
            }

            // area won't be null here, since at least one underline exists.
            area.add(new Area(label.handleGetOutline(x, y)));

            return new GeneralPath(area);
        }


	public String toString() {
	    StringBuffer buf = new StringBuffer();
	    buf.append(super.toString());
	    buf.append("[");
	    if (fgPaint != null) buf.append("fgPaint: " + fgPaint);
	    if (bgPaint != null) buf.append(" bgPaint: " + bgPaint);
	    if (swapColors) buf.append(" swapColors: true");
	    if (strikethrough) buf.append(" strikethrough: true");
	    if (stdUnderline != null) buf.append(" stdUnderline: " + stdUnderline);
	    if (imUnderline != null) buf.append(" imUnderline: " + imUnderline);
	    buf.append("]");
	    return buf.toString();
	}
    }
}
