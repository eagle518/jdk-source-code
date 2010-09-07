/*
 * @(#)awt_DesktopProperties.h	1.14 03/12/19
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef AWT_DESKTOP_PROPERTIES_H
#define AWT_DESKTOP_PROPERTIES_H

#include "awt.h"
#include "jni.h"

class AwtDesktopProperties {
    public:
	enum {
	    MAX_PROPERTIES = 100,
	    AWT_DESKTOP_PROPERTIES_1_3 = 1, // properties version for Java 2 SDK 1.3
	    // NOTE: MUST INCREMENT this whenever you add new
	    // properties for a given public release
	    AWT_DESKTOP_PROPERTIES_1_4 = 2, // properties version for Java 2 SDK 1.4
	    AWT_DESKTOP_PROPERTIES_1_5 = 3, // properties version for Java 2 SDK 1.5
	    AWT_DESKTOP_PROPERTIES_VERSION = AWT_DESKTOP_PROPERTIES_1_5
	};

	AwtDesktopProperties(jobject self);	
	~AwtDesktopProperties();
	
	void GetWindowsParameters();
	void PlayWindowsSound(LPCTSTR eventName);
        static BOOL IsXPStyle();

	static jfieldID pDataID;
	static jmethodID setStringPropertyID;
	static jmethodID setIntegerPropertyID;
	static jmethodID setBooleanPropertyID;
	static jmethodID setColorPropertyID;
	static jmethodID setFontPropertyID;
	static jmethodID setSoundPropertyID;

    private:
	void GetXPStyleProperties();
	void GetSystemProperties();
	void GetNonClientParameters();
	void GetIconParameters();
	void GetColorParameters();
	void GetOtherParameters();
	void GetSoundEvents();

	static BOOL GetBooleanParameter(UINT spi);
	static UINT GetIntegerParameter(UINT spi);

	void SetBooleanProperty(LPCTSTR, BOOL);
	void SetIntegerProperty(LPCTSTR, int);
	void SetStringProperty(LPCTSTR, LPTSTR);
	void SetColorProperty(LPCTSTR, DWORD);
	void SetFontProperty(HDC, int, LPCTSTR);
	void SetFontProperty(LPCTSTR, const LOGFONT &);
	void SetSoundProperty(LPCTSTR, LPCTSTR);

	JNIEnv * GetEnv() { 
	    return (JNIEnv *)JNU_GetEnv(jvm, JNI_VERSION_1_2);
	}

	jobject		self;
};

#endif // AWT_DESKTOP_PROPERTIES_H
