/*
 * @(#)DownloadHelper.h	1.5 10/03/23
 *
 * Copyright (c) 2007, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 2048
#endif

#define E_JDHELPER_TIMEOUT               12002
#define E_JDHELPER_NAME_NOT_RESOLVED     12007
#define E_JDHELPER_CANNOT_CONNECT        12029

#include <jni.h>
#include "DownloadDialog.h"

class DownloadHelper {
public:
    DownloadHelper();
    ~DownloadHelper();
    
    HRESULT doDownload();
    
    void setFile(LPCTSTR pszFileName) {
        m_pszFileName = pszFileName;
    }
    
    void setURL(LPCTSTR pszURL) {
        m_pszURL = pszURL;
    }
    
    void setNameText(LPTSTR pszNameText) {
        m_pszNameText = pszNameText;
    }
    
    void setShowProgressDialog(BOOL showProgress) {
        m_showProgressDialog = showProgress;
    }
    
    void setDownloadDialog(CDownloadDialog* dialog) {
        m_dlg = dialog;
    }
    
    void setJavaVM(JavaVM *jvm) {
        m_jvm = jvm;
    }

private:
    HRESULT DownloadFile(const TCHAR* szURL, const TCHAR* szLocalFile,
            BOOL bResumable, BOOL bUIFeedback);
    
    BOOL m_showProgressDialog;
    LPCTSTR m_pszURL;
    LPCTSTR m_pszFileName;
    LPTSTR m_pszNameText;
    time_t m_startTime;
    CComAutoCriticalSection m_csDownload;
    CDownloadDialog* m_dlg;
    JavaVM* m_jvm;
};
