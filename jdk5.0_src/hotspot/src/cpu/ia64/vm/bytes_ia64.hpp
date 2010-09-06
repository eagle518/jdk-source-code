#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)bytes_ia64.hpp	1.5 03/12/23 16:36:35 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class Bytes: AllStatic {
 public:
  // Efficient reading and writing of unaligned unsigned data in platform-specific byte ordering
  // IA64 needs to check for alignment.

  // can I count on address always being a pointer to an unsigned char? Yes 

  // Returns true, if the byte ordering used by Java is different from the nativ byte ordering
  // of the underlying machine. For example, true for Intel x86, False, for Solaris on Sparc, true for ia64
  static inline bool is_Java_byte_ordering_different() { return true; }

  // Thus, a swap between native and Java ordering is always work:

  // This should be compiler dependent. This will do for now.
  static inline u2   swap_u2(u2 x)  { 
    address p = (address) &x;
    return  ( (u2(p[0]) << 8 ) | ( u2(p[1])) ); 
  }

  static inline u4   swap_u4(u4 x)  {
    address p = (address) &x;
    return ( (u4(p[0]) << 24) | (u4(p[1]) << 16) | (u4(p[2]) << 8) | u4(p[3])) ;
  }

  static inline u8   swap_u8(u8 x)  { 
    address p = (address) &x;
    return ( (u8(p[0]) << 56) | (u8(p[1]) << 48) | (u8(p[2]) << 40) | (u8(p[3]) << 32) |
             (u8(p[4]) << 24) | (u8(p[5]) << 16) | (u8(p[6]) << 8)  | u8(p[7])) ;
  }

  static inline u2   get_native_u2(address p){ 
    return (intptr_t(p) & 1) == 0
             ?   *(u2*)p
             :   ( u2(p[1]) << 8 ) 
               | ( u2(p[0])      );
  }

  static inline u4   get_native_u4(address p) {
    switch (intptr_t(p) & 3) {
     case 0:  return *(u4*)p;

     case 2:  return (  u4( ((u2*)p)[1] ) << 16  ) 
                   | (  u4( ((u2*)p)[0] )                  );

    default:  return ( u4(p[3]) << 24 )
                   | ( u4(p[2]) << 16 ) 
                   | ( u4(p[1]) <<  8 )
                   |   u4(p[0]);
    }
    ShouldNotReachHere();
  }

  static inline u8   get_native_u8(address p) {
    switch (intptr_t(p) & 7) {
      case 0:  return *(u8*)p;

      case 4:  return (  u8( ((u4*)p)[1] ) << 32  )
                    | (  u8( ((u4*)p)[0] )        );

      case 2:  return (  u8( ((u2*)p)[3] ) << 48  )
                    | (  u8( ((u2*)p)[2] ) << 32  )
                    | (  u8( ((u2*)p)[1] ) << 16  )
                    | (  u8( ((u2*)p)[0] )        );

     default:  return ( u8(p[7]) << 56 )
                    | ( u8(p[6]) << 48 )
                    | ( u8(p[5]) << 40 )
                    | ( u8(p[4]) << 32 )
                    | ( u8(p[3]) << 24 )
                    | ( u8(p[2]) << 16 )
                    | ( u8(p[1]) <<  8 )
                    |   u8(p[0]);
    }
    ShouldNotReachHere();
  }



  static inline void put_native_u2(address p, u2 x)   { 
    if ( (intptr_t(p) & 1) == 0 )  *(u2*)p = x;
    else {
      p[1] = x >> 8;
      p[0] = x;
    }
  }

  static inline void put_native_u4(address p, u4 x) {
    switch ( intptr_t(p) & 3 ) {
    case 0:  *(u4*)p = x;
              break;

    case 2:  ((u2*)p)[1] = x >> 16;
             ((u2*)p)[0] = x;
             break;

    default: ((u1*)p)[3] = x >> 24;
             ((u1*)p)[2] = x >> 16; 
             ((u1*)p)[1] = x >>  8; 
             ((u1*)p)[0] = x;
             break;
    }
  }

  static inline void put_native_u8(address p, u8 x) {
    switch ( intptr_t(p) & 7 ) {
    case 0:  *(u8*)p = x;
             break;

    case 4:  ((u4*)p)[1] = x >> 32;
             ((u4*)p)[0] = x;
             break;

    case 2:  ((u2*)p)[3] = x >> 48;
             ((u2*)p)[2] = x >> 32;
             ((u2*)p)[1] = x >> 16;
             ((u2*)p)[0] = x;
             break;

    default: ((u1*)p)[7] = x >> 56;
             ((u1*)p)[6] = x >> 48;
             ((u1*)p)[5] = x >> 40;
             ((u1*)p)[4] = x >> 32;
             ((u1*)p)[3] = x >> 24;
             ((u1*)p)[2] = x >> 16;
             ((u1*)p)[1] = x >>  8;
             ((u1*)p)[0] = x;
    }
  }


  // Efficient reading and writing of unaligned unsigned data in Java byte ordering (i.e. big-endian ordering)
  // (no byte-order reversal is needed since SPARC CPUs are big-endian oriented)
  static inline u2   get_Java_u2(address p) { return swap_u2(get_native_u2(p)); }
  static inline u4   get_Java_u4(address p) { return swap_u4(get_native_u4(p)); }
  static inline u8   get_Java_u8(address p) { return swap_u8(get_native_u8(p)); }

  static inline void put_Java_u2(address p, u2 x)     { put_native_u2(p, swap_u2(x)); }
  static inline void put_Java_u4(address p, u4 x)     { put_native_u4(p, swap_u4(x)); }
  static inline void put_Java_u8(address p, u8 x)     { put_native_u8(p, swap_u8(x)); }
};
