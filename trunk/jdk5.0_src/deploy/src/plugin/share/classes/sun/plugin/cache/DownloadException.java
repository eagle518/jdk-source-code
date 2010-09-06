/*
 *
 * Copyright 2004 Sun Microsystems, Inc. All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package sun.plugin.cache;

import java.io.IOException;
import java.net.HttpURLConnection;
import sun.net.www.MessageHeader;

public class DownloadException extends IOException {
    private IOException	    cause = null;
    private MessageHeader   headers = null;

    public DownloadException(IOException e, HttpURLConnection uc) {
	this.cause = e;
	this.headers = createHeaders(uc);
    }

    public IOException getIOException() {
	return cause;
    }

    public MessageHeader getHeaders() {
	return headers;
    }

    private MessageHeader createHeaders(HttpURLConnection uc) {
	if(uc == null)
	    return null;

	String key;
	String value;
	MessageHeader theHeaders = new MessageHeader();

	for(int index = 0;; index ++) {
	    key = uc.getHeaderFieldKey(index);
	    value = uc.getHeaderField(index);
	    if(key == null && value == null)
		break;
	    
	    theHeaders.add(key, value); 
	}

	return theHeaders;
    }
}
