#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)cha.hpp	1.15 03/12/23 16:40:01 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Class Hierarchy Analysis 
// Computes the set of overriding methods for a particular call,
// using the subclass links in instanceKlass.
// Right now the CHA just traverses these links for every query;
// if this should become too slow we can put in a cache.

// result of a CHA query
class CHAResult : public ResourceObj {
  friend class CHA;
  const KlassHandle  _receiver;                                 // copies of the lookup (for better debugging)
  const symbolHandle _name;
  const symbolHandle _signature;
  const methodHandle _target;                                   // target method (if final)
  const bool         _valid;
  const GrowableArray<methodHandle>* const _target_methods;     // list of possible targets (NULL for final methods or if !UseCHA)
  const GrowableArray<KlassHandle>* const  _receivers;          // list of possible receiver klasses (NULL for final methods or if !UseCHA)

  CHAResult(KlassHandle receiver, symbolHandle name, symbolHandle signature,
            GrowableArray<KlassHandle>* receivers, GrowableArray<methodHandle>* methods, 
            methodHandle target, bool valid = true);
 public:
  KlassHandle  receiver() const                               { return _receiver; }
  symbolHandle name() const                                   { return _name; }
  symbolHandle signature() const                              { return _signature; }
  bool      is_accurate() const                               { return !_target_methods->is_full(); }
  bool      is_monomorphic() const;
  methodHandle monomorphic_target() const;                    // returns the single target (if is_monomorphic)
  KlassHandle  monomorphic_receiver() const;                  // receiver klass of monomorphic_target
  const GrowableArray<KlassHandle>*  receivers() const        { return _receivers; }
    // Returns the list of all subclasses that are possible receivers (empty array if none, capped at max_result).
    // The static receiver klass *is* included in the result (unless it is abstract).
    // The list is a class hierarchy preorder, i.e., subclasses precede their superclass.
    // All possible receiver classes are included, not just those that (re)define the method.
    // Abstract classes are suppressed.
  const GrowableArray<methodHandle>* target_methods() const   { return _target_methods; }
    // Returns the list of possible target methods, i.e., all methods potentially invoked
    // by this send (empty array if none, capped at max_result).
    // If the receiver klass (or one of its superclasses) defines the method, this definition 
    // is included in the result.  Abstract methods are suppressed.
  void print();
};


class CHA : AllStatic {
  static int _max_result;           // maximum result size (for efficiency)
  static bool _used;                // has CHA been used yet?  (will go away when deoptimization implemented)

  static void process_class(KlassHandle r, GrowableArray<KlassHandle>* receivers, GrowableArray<methodHandle>* methods, 
                            symbolHandle name, symbolHandle signature);
  static void process_interface(instanceKlassHandle r, GrowableArray<KlassHandle>* receivers, GrowableArray<methodHandle>* methods, 
                            symbolHandle name, symbolHandle signature);
 public:
  static bool has_been_used()       { return _used; }
  static int  max_result()          { return _max_result; }
  static void set_max_result(int n) { _max_result = n; }

  static CHAResult* analyze_call(KlassHandle calling_klass, KlassHandle static_receiver, 
                                 KlassHandle actual_receiver, symbolHandle name, symbolHandle signature);
};


