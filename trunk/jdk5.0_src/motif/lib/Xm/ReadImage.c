/* 
 *  @OSF_COPYRIGHT@
 *  COPYRIGHT NOTICE
 *  Copyright (c) 1990, 1991, 1992, 1993 Open Software Foundation, Inc.
 *  ALL RIGHTS RESERVED (MOTIF). See the file named COPYRIGHT.MOTIF for
 *  the full copyright text.
*/ 
/* 
 * HISTORY
*/ 
#ifdef REV_INFO
#ifndef lint
static char rcsid[] = "$XConsortium: ReadImage.c /main/15 1996/10/21 11:40:15 cde-osf $"
#endif
#endif
/* (c) Copyright 1987, 1988, 1989, 1990, 1991, 1992 HEWLETT-PACKARD COMPANY */
/* (c) Copyright 1988 MASSACHUSETTS INSTITUTE OF TECHNOLOGY  */

#include "XmI.h"		/* for _XmCreateImage() */
#include "ReadImageI.h"

/************************************************************************
 *
 *  _XmReadImageAndHotSpotFromFile
 *	Given a filename, extract and create an image from the file data.
 *      This one takes a Display.
 ************************************************************************/
XImage * 
_XmReadImageAndHotSpotFromFile(
        Display * display,
        char *filename,
	int *hot_x, 
	int *hot_y)
{
   unsigned int width; 
   unsigned int height;
   unsigned char * data;

   if (BitmapSuccess == XReadBitmapFileData(filename, &width, &height, &data,
			       hot_x, hot_y))
   {
      XImage * image;
      _XmCreateImage(image, display, (char*)data, width, height, LSBFirst);

      return (image);
   }

   return (NULL);
}


