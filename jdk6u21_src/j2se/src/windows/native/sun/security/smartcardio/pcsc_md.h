/*
 * @(#)pcsc_md.h	1.4 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

#define CALL_SCardEstablishContext(dwScope, pvReserved1, pvReserved2, phContext) \
    (SCardEstablishContext(dwScope, pvReserved1, pvReserved2, phContext))

#define CALL_SCardConnect(hContext, szReader, dwSharedMode, dwPreferredProtocols, phCard, pdwActiveProtocols) \
    (SCardConnect(hContext, szReader, dwSharedMode, dwPreferredProtocols, phCard, pdwActiveProtocols))
    
#define CALL_SCardDisconnect(hCard, dwDisposition) \
    (SCardDisconnect(hCard, dwDisposition))

#define CALL_SCardStatus(hCard, mszReaderNames, pcchReaderLen, pdwState, pdwProtocol, pbAtr, pcbAtrLen) \
    (SCardStatus(hCard, mszReaderNames, pcchReaderLen, pdwState, pdwProtocol, pbAtr, pcbAtrLen))

#define CALL_SCardGetStatusChange(hContext, dwTimeout, rgReaderStates, cReaders) \
    (SCardGetStatusChange(hContext, dwTimeout, rgReaderStates, cReaders))

#define CALL_SCardTransmit(hCard, pioSendPci, pbSendBuffer, cbSendLength, \
			    pioRecvPci, pbRecvBuffer, pcbRecvLength) \
    (SCardTransmit(hCard, pioSendPci, pbSendBuffer, cbSendLength, \
			    pioRecvPci, pbRecvBuffer, pcbRecvLength))

#define CALL_SCardListReaders(hContext, mszGroups, mszReaders, pcchReaders) \
    (SCardListReaders(hContext, mszGroups, mszReaders, pcchReaders))

#define CALL_SCardBeginTransaction(hCard) \
    (SCardBeginTransaction(hCard))

#define CALL_SCardEndTransaction(hCard, dwDisposition) \
    (SCardEndTransaction(hCard, dwDisposition))

#define CALL_SCardControl(hCard, dwControlCode, lpInBuffer, nInBufferSize, \
	lpOutBuffer, nOutBufferSize, lpBytesReturns) \
    (SCardControl(hCard, dwControlCode, lpInBuffer, nInBufferSize, \
	lpOutBuffer, nOutBufferSize, lpBytesReturns))
