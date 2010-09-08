/*
 * @(#)pcsc_md.h	1.4 10/03/23
 *
 * Copyright (c) 2006, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

typedef LONG (*FPTR_SCardEstablishContext)(ULONG dwScope,
		const void *pvReserved1,
		const void *pvReserved2,
		LONG *phContext);

typedef	LONG (*FPTR_SCardConnect)(LONG hContext,
		const char *szReader,
		ULONG dwShareMode,
		ULONG dwPreferredProtocols,
		LONG *phCard, ULONG *pdwActiveProtocol);
		
typedef	LONG (*FPTR_SCardDisconnect)(LONG hCard, ULONG dwDisposition);
		
typedef	LONG (*FPTR_SCardStatus)(LONG hCard,
		char *mszReaderNames,
		ULONG *pcchReaderLen,
		ULONG *pdwState,
		ULONG *pdwProtocol,
		unsigned char *pbAtr, ULONG *pcbAtrLen);

typedef	LONG (*FPTR_SCardGetStatusChange)(LONG hContext,
		ULONG dwTimeout,
		LPSCARD_READERSTATE_A rgReaderStates, ULONG cReaders);

typedef	LONG (*FPTR_SCardTransmit)(LONG hCard,
		LPCSCARD_IO_REQUEST pioSendPci,
		const unsigned char *pbSendBuffer,
		ULONG cbSendLength,
		LPSCARD_IO_REQUEST pioRecvPci,
		unsigned char *pbRecvBuffer, ULONG *pcbRecvLength);

typedef	LONG (*FPTR_SCardListReaders)(LONG hContext,
		const char *mszGroups,
		char *mszReaders, ULONG *pcchReaders);

typedef LONG (*FPTR_SCardBeginTransaction)(LONG hCard);

typedef	LONG (*FPTR_SCardEndTransaction)(LONG hCard, ULONG dwDisposition);
		
typedef	LONG (*FPTR_SCardControl)(LONG hCard, ULONG dwControlCode, 
    const void* pbSendBuffer, ULONG cbSendLength, const void* pbRecvBuffer, 
    ULONG pcbRecvLength, ULONG *lpBytesReturned);
		
#define CALL_SCardEstablishContext(dwScope, pvReserved1, pvReserved2, phContext) \
    ((scardEstablishContext)(dwScope, pvReserved1, pvReserved2, phContext))

#define CALL_SCardConnect(hContext, szReader, dwSharedMode, dwPreferredProtocols, phCard, pdwActiveProtocols) \
    ((scardConnect)(hContext, szReader, dwSharedMode, dwPreferredProtocols, phCard, pdwActiveProtocols))
    
#define CALL_SCardDisconnect(hCard, dwDisposition) \
    ((scardDisconnect)(hCard, dwDisposition))

#define CALL_SCardStatus(hCard, mszReaderNames, pcchReaderLen, pdwState, pdwProtocol, pbAtr, pcbAtrLen) \
    ((scardStatus)(hCard, mszReaderNames, pcchReaderLen, pdwState, pdwProtocol, pbAtr, pcbAtrLen))

#define CALL_SCardGetStatusChange(hContext, dwTimeout, rgReaderStates, cReaders) \
    ((scardGetStatusChange)(hContext, dwTimeout, rgReaderStates, cReaders))

#define CALL_SCardTransmit(hCard, pioSendPci, pbSendBuffer, cbSendLength, \
			    pioRecvPci, pbRecvBuffer, pcbRecvLength) \
    ((scardTransmit)(hCard, pioSendPci, pbSendBuffer, cbSendLength, \
			    pioRecvPci, pbRecvBuffer, pcbRecvLength))

#define CALL_SCardListReaders(hContext, mszGroups, mszReaders, pcchReaders) \
    ((scardListReaders)(hContext, mszGroups, mszReaders, pcchReaders))

#define CALL_SCardBeginTransaction(hCard) \
    ((scardBeginTransaction)(hCard))

#define CALL_SCardEndTransaction(hCard, dwDisposition) \
    ((scardEndTransaction)(hCard, dwDisposition))

#define CALL_SCardControl(hCard, dwControlCode, pbSendBuffer, cbSendLength, \
	    pbRecvBuffer, pcbRecvLength, lpBytesReturned) \
    ((scardControl)(hCard, dwControlCode, pbSendBuffer, cbSendLength, \
	    pbRecvBuffer, pcbRecvLength, lpBytesReturned))

extern FPTR_SCardEstablishContext scardEstablishContext;
extern FPTR_SCardConnect scardConnect;
extern FPTR_SCardDisconnect scardDisconnect;
extern FPTR_SCardStatus scardStatus;
extern FPTR_SCardGetStatusChange scardGetStatusChange;
extern FPTR_SCardTransmit scardTransmit;
extern FPTR_SCardListReaders scardListReaders;
extern FPTR_SCardBeginTransaction scardBeginTransaction;
extern FPTR_SCardEndTransaction scardEndTransaction;
extern FPTR_SCardControl scardControl;
