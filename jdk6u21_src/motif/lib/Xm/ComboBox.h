/* $XConsortium: ComboBox.h /main/6 1995/07/15 20:49:27 drk $ */
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
/* ComboBox.h */

#ifndef _XmComboBox_h
#define _XmComboBox_h

#ifdef __cplusplus
extern "C" {
#endif

externalref WidgetClass xmComboBoxWidgetClass;

typedef struct _XmComboBoxClassRec	* XmComboBoxWidgetClass;
typedef struct _XmComboBoxRec 		* XmComboBoxWidget;


/* XmIsComboBox may already be defined for Fast Subclassing  */
#ifndef XmIsComboBox
#define XmIsComboBox(w)		XtIsSubclass(w, xmComboBoxWidgetClass)
#endif  /* XmIsComboBox */

/********    Public Function Declarations    ********/

extern Widget XmCreateComboBox (Widget parent, 
				char *name, 
				ArgList args, 
				Cardinal argCount);
extern Widget XmCreateDropDownComboBox (Widget parent, 
					char *name, 
					ArgList args, 
					Cardinal argCount);
extern Widget XmCreateDropDownList (Widget parent, 
				    char *name, 
				    ArgList args, 
				    Cardinal argCount);

extern void XmComboBoxAddItem (Widget   widget,
			       XmString item,
			       int      pos,
			       Boolean  unique);
extern void XmComboBoxDeletePos (Widget widget,
				 int    pos);
extern void XmComboBoxSelectItem (Widget   widget,
				  XmString item);
extern void XmComboBoxSetItem (Widget   widget,
			       XmString item);
extern void XmComboBoxUpdate (Widget widget);

/********    End Public Function Declarations    ********/


#ifdef __cplusplus
}  /* Close scope of 'extern "C"' declaration which encloses file. */
#endif

#endif /* _XmComboBox_h */
/* DON'T ADD ANYTHING AFTER THIS #endif */

