/*
 * @(#)multi_font.h	1.19 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */
/*
 * header for Multi Font String
 */
#ifndef _MULTI_FONT_H_
#define _MULTI_FONT_H_

#ifndef HEADLESS
jobject awtJNI_GetFont(JNIEnv *env,jobject this);
jboolean awtJNI_IsMultiFont(JNIEnv *env,jobject this);
jboolean awtJNI_IsMultiFontMetrics(JNIEnv *env,jobject this);
#ifndef XAWT
XmString awtJNI_MakeMultiFontString(JNIEnv *env,jstring s,jobject font);
XmFontList awtJNI_GetFontList(JNIEnv *env,jobject font);
#endif
XFontSet awtJNI_MakeFontSet(JNIEnv *env,jobject font);
struct FontData *awtJNI_GetFontData(JNIEnv *env,jobject font, char **errmsg);
void awtJNI_DrawMFString(JNIEnv *env, jcharArray s, struct GraphicsData *gdata, 
                         jobject font, int32_t x, int32_t y, int32_t offset, 
                         int32_t length);
int32_t awtJNI_GetMFStringWidth(JNIEnv * env, jcharArray s, int32_t offset, 
                                int32_t length, jobject font);
#endif /* !HEADLESS */

#endif /* _MULTI_FONT_H_ */
