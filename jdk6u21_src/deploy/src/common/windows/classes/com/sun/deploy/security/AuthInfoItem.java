/*
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

package com.sun.deploy.security;

class AuthInfoItem {
	private char[] serverCredential;
	private char[] proxyCredential;

	private boolean serverCredentialFetched = false;
	private boolean proxyCredentialFetched = false;

	public AuthInfoItem(char[] serverCredential, char[] proxyCredential) {
		this.serverCredential = serverCredential;
		this.proxyCredential = proxyCredential;
	}

	public char[] getServerCredential() {
		serverCredentialFetched = true;
		return this.serverCredential;
	}

	public char[] getProxyCredential() {
		proxyCredentialFetched = true;
		return this.proxyCredential;
	}

	public boolean shouldRemove() {
		return ((serverCredential == null || serverCredentialFetched) &&
				(proxyCredential == null || proxyCredentialFetched));
	}
}
