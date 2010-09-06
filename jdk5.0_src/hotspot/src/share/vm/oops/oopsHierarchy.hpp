#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)oopsHierarchy.hpp	1.21 03/12/23 16:42:07 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// OBJECT hierarchy
// This hierarchy is a representation hierarchy, i.e. if A is a superclass
// of B, A's representation is a prefix of B's representation.

typedef class oopDesc*			    oop;
typedef class   instanceOopDesc*	    instanceOop;
typedef class   methodOopDesc*		    methodOop;
typedef class   constMethodOopDesc*	    constMethodOop;
#ifndef CORE
typedef class   methodDataOopDesc*	    methodDataOop;
#endif // !CORE
typedef class   arrayOopDesc*		    arrayOop;
typedef class     constantPoolOopDesc*	    constantPoolOop;
typedef class     constantPoolCacheOopDesc* constantPoolCacheOop;
typedef class     objArrayOopDesc*	    objArrayOop;
typedef class     typeArrayOopDesc*	    typeArrayOop;
typedef class   symbolOopDesc*	            symbolOop;
typedef class   klassOopDesc*		    klassOop;
typedef class   markOopDesc*		    markOop;
typedef class   compiledICHolderOopDesc*    compiledICHolderOop;


// The klass hierarchy is separate from the oop hierarchy.

class Klass;
class   instanceKlass;
class     instanceRefKlass;
class   methodKlass;
class   constMethodKlass;
#ifndef CORE
class   methodDataKlass;
#endif // !CORE
class   klassKlass;
class     instanceKlassKlass;
class     arrayKlassKlass;
class       objArrayKlassKlass;
class       typeArrayKlassKlass;
class   arrayKlass;
class     constantPoolKlass;
class     constantPoolCacheKlass;
class     objArrayKlass;
class     typeArrayKlass;
class       symbolKlass;
class   compiledICHolderKlass;
