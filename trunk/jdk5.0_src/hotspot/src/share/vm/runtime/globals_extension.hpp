#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)globals_extension.hpp	1.4 03/12/23 16:43:46 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Construct enum of Flag_<cmdline-arg> constants.

// Parens left off in the following for the enum decl below.
#define FLAG_MEMBER(flag) Flag_##flag

#define RUNTIME_PRODUCT_FLAG_MEMBER(type, name, value, doc)    FLAG_MEMBER(name),
#define RUNTIME_PD_PRODUCT_FLAG_MEMBER(type, name, doc)        FLAG_MEMBER(name),
#define RUNTIME_DIAGNOSTIC_FLAG_MEMBER(type, name, value, doc) FLAG_MEMBER(name),
#ifdef PRODUCT
  #define RUNTIME_DEVELOP_FLAG_MEMBER(type, name, value, doc)  /* flag is constant */ 
  #define RUNTIME_PD_DEVELOP_FLAG_MEMBER(type, name, doc)      /* flag is constant */ 
#else
  #define RUNTIME_DEVELOP_FLAG_MEMBER(type, name, value, doc)  FLAG_MEMBER(name),
  #define RUNTIME_PD_DEVELOP_FLAG_MEMBER(type, name, doc)      FLAG_MEMBER(name),
#endif

#define C1_PRODUCT_FLAG_MEMBER(type, name, value, doc)         FLAG_MEMBER(name),
#define C1_PD_PRODUCT_FLAG_MEMBER(type, name, doc)             FLAG_MEMBER(name),
#ifdef PRODUCT
  #define C1_DEVELOP_FLAG_MEMBER(type, name, value, doc)       /* flag is constant */ 
  #define C1_PD_DEVELOP_FLAG_MEMBER(type, name, doc)           /* flag is constant */ 
#else
  #define C1_DEVELOP_FLAG_MEMBER(type, name, value, doc)       FLAG_MEMBER(name),
  #define C1_PD_DEVELOP_FLAG_MEMBER(type, name, doc)           FLAG_MEMBER(name),
#endif


#define C2_PRODUCT_FLAG_MEMBER(type, name, value, doc)         FLAG_MEMBER(name),
#define C2_PD_PRODUCT_FLAG_MEMBER(type, name, doc)             FLAG_MEMBER(name),
#ifdef PRODUCT
  #define C2_DEVELOP_FLAG_MEMBER(type, name, value, doc)       /* flag is constant */ 
  #define C2_PD_DEVELOP_FLAG_MEMBER(type, name, doc)           /* flag is constant */ 
#else
  #define C2_DEVELOP_FLAG_MEMBER(type, name, value, doc)       FLAG_MEMBER(name),
  #define C2_PD_DEVELOP_FLAG_MEMBER(type, name, doc)           FLAG_MEMBER(name),
#endif

typedef enum {
 RUNTIME_FLAGS(RUNTIME_DEVELOP_FLAG_MEMBER, RUNTIME_PD_DEVELOP_FLAG_MEMBER, RUNTIME_PRODUCT_FLAG_MEMBER,
               RUNTIME_PD_PRODUCT_FLAG_MEMBER, RUNTIME_DIAGNOSTIC_FLAG_MEMBER)
#ifdef COMPILER1
 C1_FLAGS(C1_DEVELOP_FLAG_MEMBER, C1_PD_DEVELOP_FLAG_MEMBER, C1_PRODUCT_FLAG_MEMBER, C1_PD_PRODUCT_FLAG_MEMBER)
#endif
#ifdef COMPILER2
 C2_FLAGS(C2_DEVELOP_FLAG_MEMBER, C2_PD_DEVELOP_FLAG_MEMBER, C2_PRODUCT_FLAG_MEMBER, C2_PD_PRODUCT_FLAG_MEMBER)
#endif
 NUM_CommandLineFlag
} CommandLineFlag;

// Construct enum of Flag_<cmdline-arg>_<type> constants.

#define FLAG_MEMBER_WITH_TYPE(flag,type) Flag_##flag##_##type

#define RUNTIME_PRODUCT_FLAG_MEMBER_WITH_TYPE(type, name, value, doc)    FLAG_MEMBER_WITH_TYPE(name,type),
#define RUNTIME_PD_PRODUCT_FLAG_MEMBER_WITH_TYPE(type, name, doc)        FLAG_MEMBER_WITH_TYPE(name,type),
#define RUNTIME_DIAGNOSTIC_FLAG_MEMBER_WITH_TYPE(type, name, value, doc) FLAG_MEMBER_WITH_TYPE(name,type),
#ifdef PRODUCT
  #define RUNTIME_DEVELOP_FLAG_MEMBER_WITH_TYPE(type, name, value, doc)  /* flag is constant */ 
  #define RUNTIME_PD_DEVELOP_FLAG_MEMBER_WITH_TYPE(type, name, doc)      /* flag is constant */ 
#else
  #define RUNTIME_DEVELOP_FLAG_MEMBER_WITH_TYPE(type, name, value, doc)  FLAG_MEMBER_WITH_TYPE(name,type),
  #define RUNTIME_PD_DEVELOP_FLAG_MEMBER_WITH_TYPE(type, name, doc)      FLAG_MEMBER_WITH_TYPE(name,type),
#endif

#define C1_PRODUCT_FLAG_MEMBER_WITH_TYPE(type, name, value, doc)         FLAG_MEMBER_WITH_TYPE(name,type),
#define C1_PD_PRODUCT_FLAG_MEMBER_WITH_TYPE(type, name, doc)             FLAG_MEMBER_WITH_TYPE(name,type),
#ifdef PRODUCT
  #define C1_DEVELOP_FLAG_MEMBER_WITH_TYPE(type, name, value, doc)       /* flag is constant */ 
  #define C1_PD_DEVELOP_FLAG_MEMBER_WITH_TYPE(type, name, doc)           /* flag is constant */ 
#else
  #define C1_DEVELOP_FLAG_MEMBER_WITH_TYPE(type, name, value, doc)       FLAG_MEMBER_WITH_TYPE(name,type),
  #define C1_PD_DEVELOP_FLAG_MEMBER_WITH_TYPE(type, name, doc)           FLAG_MEMBER_WITH_TYPE(name,type),
#endif


#define C2_PRODUCT_FLAG_MEMBER_WITH_TYPE(type, name, value, doc)         FLAG_MEMBER_WITH_TYPE(name,type),
#define C2_PD_PRODUCT_FLAG_MEMBER_WITH_TYPE(type, name, doc)             FLAG_MEMBER_WITH_TYPE(name,type),
#ifdef PRODUCT
  #define C2_DEVELOP_FLAG_MEMBER_WITH_TYPE(type, name, value, doc)       /* flag is constant */ 
  #define C2_PD_DEVELOP_FLAG_MEMBER_WITH_TYPE(type, name, doc)           /* flag is constant */ 
#else
  #define C2_DEVELOP_FLAG_MEMBER_WITH_TYPE(type, name, value, doc)       FLAG_MEMBER_WITH_TYPE(name,type),
  #define C2_PD_DEVELOP_FLAG_MEMBER_WITH_TYPE(type, name, doc)           FLAG_MEMBER_WITH_TYPE(name,type),
#endif

typedef enum {
 RUNTIME_FLAGS(RUNTIME_DEVELOP_FLAG_MEMBER_WITH_TYPE, RUNTIME_PD_DEVELOP_FLAG_MEMBER_WITH_TYPE,
	       RUNTIME_PRODUCT_FLAG_MEMBER_WITH_TYPE, RUNTIME_PD_PRODUCT_FLAG_MEMBER_WITH_TYPE,
               RUNTIME_DIAGNOSTIC_FLAG_MEMBER_WITH_TYPE)
#ifdef COMPILER1
 C1_FLAGS(C1_DEVELOP_FLAG_MEMBER_WITH_TYPE, C1_PD_DEVELOP_FLAG_MEMBER_WITH_TYPE, C1_PRODUCT_FLAG_MEMBER_WITH_TYPE,
          C1_PD_PRODUCT_FLAG_MEMBER_WITH_TYPE)
#endif
#ifdef COMPILER2
 C2_FLAGS(C2_DEVELOP_FLAG_MEMBER_WITH_TYPE, C2_PD_DEVELOP_FLAG_MEMBER_WITH_TYPE, C2_PRODUCT_FLAG_MEMBER_WITH_TYPE,
          C2_PD_PRODUCT_FLAG_MEMBER_WITH_TYPE)
#endif
 NUM_CommandLineFlagWithType
} CommandLineFlagWithType;

#define FLAG_IS_DEFAULT(name)         (CommandLineFlagsEx::is_default(FLAG_MEMBER(name)))

#define FLAG_SET_DEFAULT(name, value) ((name) = (value))

#define FLAG_SET(type, name, value)   (CommandLineFlagsEx::##type##AtPut(FLAG_MEMBER_WITH_TYPE(name,type), (type)(value)))

// Can't put the following in CommandLineFlags because
// of a circular dependency on the enum definition.
class CommandLineFlagsEx : CommandLineFlags {
 public:
  static void boolAtPut(CommandLineFlagWithType flag, bool value);
  static void intxAtPut(CommandLineFlagWithType flag, intx value);
  static void uintxAtPut(CommandLineFlagWithType flag, uintx value);
  static void ccstrAtPut(CommandLineFlagWithType flag, ccstr value);

  static bool is_default(CommandLineFlag flag);
};
