#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)ciClassList.hpp	1.12 03/12/23 16:39:25 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class ciEnv;
class ciObjectFactory;
class ciConstantPoolCache;

class ciField;
class ciConstant;
class ciFlags;
class ciExceptionHandler;
class ciCallProfile;
class ciSignature;

class ciByteCodeStream;
class ciSignatureStream;
class ciExceptionHandlerStream;

class ciTypeFlow;

class ciObject;
class   ciNullObject;
class   ciInstance;
class   ciMethod;
class   ciMethodData;
class     ciVirtualCallData;  // part of ciMethodData
class   ciSymbol;
class   ciArray;
class     ciObjArray;
class     ciTypeArray;
class   ciType;
class    ciReturnAddress;
class    ciKlass;
class     ciInstanceKlass;
class     ciMethodKlass;
class     ciSymbolKlass;
class     ciArrayKlass;
class       ciObjArrayKlass;
class       ciTypeArrayKlass;
class     ciKlassKlass;
class       ciInstanceKlassKlass;
class       ciArrayKlassKlass;
class         ciObjArrayKlassKlass;
class         ciTypeArrayKlassKlass;

// Simulate Java Language style package-private access with
// friend declarations.
// This is a great idea but gcc and other C++ compilers give an
// error for being friends with yourself, so this macro does not
// compile on some platforms.

// Everyone gives access to ciObjectFactory
#define CI_PACKAGE_ACCESS \
friend class ciObjectFactory;

// These are the packages that have access to ciEnv
// Any more access must be given explicitly.
#define CI_PACKAGE_ACCESS_TO           \
friend class ciObjectFactory;          \
friend class ciConstantPoolCache;      \
friend class ciField;                  \
friend class ciConstant;               \
friend class ciFlags;                  \
friend class ciExceptionHandler;       \
friend class ciCallProfile;            \
friend class ciSignature;              \
friend class ciBytecodeStream;         \
friend class ciByteCodeStream;         \
friend class ciSignatureStream;        \
friend class ciExceptionHandlerStream; \
friend class ciObject;                 \
friend class ciNullObject;             \
friend class ciInstance;               \
friend class ciMethod;                 \
friend class ciMethodData;             \
friend class ciVirtualCallData;        \
friend class ciSymbol;                 \
friend class ciArray;                  \
friend class ciObjArray;               \
friend class ciTypeArray;              \
friend class ciType;                   \
friend class ciReturnAddress;          \
friend class ciKlass;                  \
friend class ciInstanceKlass;          \
friend class ciMethodKlass;            \
friend class ciSymbolKlass;            \
friend class ciArrayKlass;             \
friend class ciObjArrayKlass;          \
friend class ciTypeArrayKlass;         \
friend class ciKlassKlass;             \
friend class ciInstanceKlassKlass;     \
friend class ciArrayKlassKlass;        \
friend class ciObjArrayKlassKlass;     \
friend class ciTypeArrayKlassKlass;
