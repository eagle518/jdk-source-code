/* $XConsortium: DrSep.c /main/6 1995/10/25 20:00:21 cde-sun $ */
/*
 * COPYRIGHT NOTICE
 * Copyright (c) 1990, 1991, 1992, 1993 Open Software Foundation, Inc.
 * ALL RIGHTS RESERVED (MOTIF).  See the file named COPYRIGHT.MOTIF
 * for the full copyright text.
 * 
 */
/*
 * HISTORY
 */


#include "XmI.h"
#include "DrawI.h"



/******************************XmeDrawSeparator**********************/
void XmeDrawSeparator(Display *display, Drawable d, 
                     GC top_gc, GC bottom_gc, GC separator_gc, 
#if NeedWidePrototypes
                     int x, int y, 
                     int width, int height, 
                     int shadow_thick, 
                     int margin, unsigned int orientation, 
                     unsigned int separator_type)
#else
                     Position x, Position y, 
                     Dimension width, Dimension height, 
                     Dimension shadow_thick, 
                     Dimension margin, unsigned char orientation, 
                     unsigned char separator_type)
#endif /* NeedWidePrototypes */

{
   Position center;
   XSegment segs[2];
   GC   tmp_gc;
   int i, ndash, shadow_dash_size ;
   XtAppContext app;

   if (!d || (separator_type == XmNO_LINE)) return ;

   app = XtDisplayToApplicationContext(display);
   _XmAppLock(app);

   if (orientation == XmHORIZONTAL) {
       center = y + height / 2;
   } else {
       center = x + width / 2;
   }
           
   if (separator_type == XmSINGLE_LINE ||
       separator_type == XmSINGLE_DASHED_LINE) {
       if (orientation == XmHORIZONTAL) {
           segs[0].x1 = x + margin;
           segs[0].y1 = segs[0].y2 = center;
           segs[0].x2 = x + width - margin - 1;
       } else {
           segs[0].y1 = y + margin;
           segs[0].x1 = segs[0].x2 = center;
           segs[0].y2 = y + height - margin - 1;
       }
       XDrawSegments (display, d, separator_gc, segs, 1);
       _XmAppUnlock(app);
       return;
   }

   if (separator_type == XmDOUBLE_LINE ||
       separator_type == XmDOUBLE_DASHED_LINE) {
       if (orientation == XmHORIZONTAL) {
           segs[0].x1 = segs[1].x1 = x + margin;
           segs[0].x2 = segs[1].x2 = x + width - margin - 1;
           segs[0].y1 = segs[0].y2 = center - 1;
           segs[1].y1 = segs[1].y2 = center + 1;
       } else {
           segs[0].y1 = segs[1].y1 = y + margin;
           segs[0].y2 = segs[1].y2 = y + height - margin - 1;
           segs[0].x1 = segs[0].x2 = center - 1;
           segs[1].x1 = segs[1].x2 = center + 1;
       }
       XDrawSegments (display, d, separator_gc, segs, 2);
       _XmAppUnlock(app);
       return;
   }

   /* only shadowed stuff in the following, so shadowThickness has to be
      something real */
   if (!shadow_thick) {  _XmAppUnlock(app); return ; }
   
   /* do the in/out effect now */
   if (separator_type == XmSHADOW_ETCHED_IN || 
       separator_type == XmSHADOW_ETCHED_IN_DASH) {
       tmp_gc = top_gc ;
       top_gc = bottom_gc ;
       bottom_gc = tmp_gc ;
   }

   /* In the following, we need to special case the shadow_thick = 2 or 3 case,
      since : it's the default and we don't like changes in visual here, 
      and also it looks non symetrical the way it is without special code:
                                    ......
	                            .,,,,,

	and you really want to have ......
                                    ,,,,,,
      So we won't use _XmDrawSimpleShadow in this case, painful but hey..
   */


   /* Now the regular shadowed cases, in one pass with one looong dash 
      for the non dashed case */

   if (separator_type == XmSHADOW_ETCHED_IN_DASH ||
       separator_type == XmSHADOW_ETCHED_OUT_DASH)
   /* for now, shadowed dash use three time the shadow thickness as a 
      dash size, and worried about the shadow_thick odd values as well */
       shadow_dash_size = (shadow_thick/2)*2*3 ;
   else

   /* regular case, only one dash, the length of the separator */
       shadow_dash_size = (orientation == XmHORIZONTAL)?
           (width - 2*margin):(height - 2*margin) ;

   /* special case for shadowThickness = 1 */
   if (shadow_dash_size == 0) shadow_dash_size = 5 ;

   /* ndash value will be 1 for the regular shadow case (not dashed) */
   if (orientation == XmHORIZONTAL) {
       ndash = ((width - 2*margin)/shadow_dash_size + 1)/2 ;       
       for (i=0; i<ndash; i++)
           if (shadow_thick < 4) {
	       XDrawLine(display, d, top_gc, 
			 x + margin + 2*i*shadow_dash_size, 
			 center - shadow_thick/2, 
			 x + margin + (2*i + 1)*shadow_dash_size -1, 
			 center - shadow_thick/2); 
	       if (shadow_thick > 1)
		   XDrawLine(display, d, bottom_gc, 
			 x + margin + 2*i*shadow_dash_size, 
			 center, 
			 x + margin + (2*i + 1)*shadow_dash_size -1, 
			 center); 
	   } else {
	       _XmDrawSimpleShadow(display, d, top_gc, bottom_gc, 
				  x + margin + i*2*shadow_dash_size, 
				  center - shadow_thick/2, 
				  shadow_dash_size, (shadow_thick/2)*2, 
				  shadow_thick/2, 0);
	   }
       /* draw the last dash, with possibly a different size */
       if (i*2*shadow_dash_size < (width - 2*margin))
           if (shadow_thick < 4) {
	       XDrawLine(display, d, top_gc, 
			 x + margin + 2*i*shadow_dash_size, 
			 center - shadow_thick/2, 
			 x + (width - 2*margin), 
			 center - shadow_thick/2); 
	       if (shadow_thick > 1)
		   XDrawLine(display, d, bottom_gc, 
			 x + margin + 2*i*shadow_dash_size, 
			 center, 
			 x + (width - 2*margin), 
			 center); 
	   } else {
	       _XmDrawSimpleShadow(display, d, top_gc, bottom_gc, 
				  x + margin + i*2*shadow_dash_size, 
				  center - shadow_thick/2, 
				  (width - 2*margin) - i*2*shadow_dash_size, 
				  (shadow_thick/2)*2, 
				  shadow_thick/2, 0);
	   }
   } else {
       ndash = ((height - 2*margin)/shadow_dash_size + 1)/2 ;
       for (i=0; i<ndash; i++)
           if (shadow_thick < 4) {
	       XDrawLine(display, d, top_gc, 
			 center - shadow_thick/2, 
			 y + margin + 2*i*shadow_dash_size, 
			 center - shadow_thick/2,
			 y + margin + (2*i + 1)*shadow_dash_size -1); 
	       if (shadow_thick > 1)
		   XDrawLine(display, d, bottom_gc, 
			 center, 
			 y + margin + 2*i*shadow_dash_size, 
			 center, 
			 y + margin + (2*i + 1)*shadow_dash_size -1); 
	   } else {
	       _XmDrawSimpleShadow(display, d, top_gc, bottom_gc, 
				  center - shadow_thick/2, 
				  y + margin + i*2*shadow_dash_size, 
				  (shadow_thick/2)*2, shadow_dash_size, 
				  shadow_thick/2, 0);
	   }
       if (i*2*shadow_dash_size < (height - 2*margin))
           if (shadow_thick < 4) {
	       XDrawLine(display, d, top_gc, 
			 center - shadow_thick/2, 
			 y + margin + 2*i*shadow_dash_size, 
			 center - shadow_thick/2,
			 y + (height - 2*margin)); 
	       if (shadow_thick > 1)
		   XDrawLine(display, d, bottom_gc, 
			 center, 
			 y + margin + 2*i*shadow_dash_size, 
			 center, 
			 y + (height - 2*margin)); 
	   } else {
	       _XmDrawSimpleShadow(display, d, top_gc, bottom_gc, 
				  center - shadow_thick/2, 
				  y + margin + i*2*shadow_dash_size, 
				  (shadow_thick/2)*2, 
				  (height - 2*margin) - i*2*shadow_dash_size, 
				  shadow_thick/2, 0);
	   }
   }
   _XmAppUnlock(app);
}


