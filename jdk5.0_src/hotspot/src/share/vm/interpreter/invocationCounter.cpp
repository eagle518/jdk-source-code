#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)invocationCounter.cpp	1.50 03/12/23 16:40:40 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_invocationCounter.cpp.incl"


// Implementation of InvocationCounter

void InvocationCounter::init() {
  _counter = 0;  // reset all the bits, including the sticky carry
  reset();
}

void InvocationCounter::reset() {
  // Only reset the state and don't make the method look like it's never 
  // been executed
         if (UseCompiler) { set_state(wait_for_compile);
  } else                  { set_state(wait_for_nothing);
  }
}

void InvocationCounter::set_carry() {
  _counter |= carry_mask;

  // The carry bit now indicates that this counter had achieved a very
  // large value.  Now reduce the value, so that the method can be
  // executed many more times before re-entering the VM.
  int old_count = count();
  int new_count = MIN2(old_count, (int) (CompileThreshold / 2));
  if (old_count != new_count)  set(state(), new_count);
}


void InvocationCounter::set_state(State state) {
  assert(0 <= state && state < number_of_states, "illegal state");
  int init = _init[state];
  // prevent from going to zero, to distinguish from never-executed methods
  if (init == 0 && count() > 0)  init = 1;
  int carry = (_counter & carry_mask);    // the carry bit is sticky
  _counter = (init << number_of_noncount_bits) | carry | state;
}


void InvocationCounter::print() {
  tty->print_cr("invocation count: up = %d, limit = %d, carry = %s, state = %s",
                                   count(), limit(),
                                   carry() ? "true" : "false",
                                   state_as_string(state()));
}

void InvocationCounter::print_short() {
  tty->print(" [%d%s;%s]", count(), carry()?"+carry":"", state_as_short_string(state()));
}

// Initialization

int                       InvocationCounter::_init  [InvocationCounter::number_of_states];
InvocationCounter::Action InvocationCounter::_action[InvocationCounter::number_of_states];
int                       InvocationCounter::InterpreterInvocationLimit;
int                       InvocationCounter::InterpreterBackwardBranchLimit;
int                       InvocationCounter::InterpreterProfileLimit;


const char* InvocationCounter::state_as_string(State state) {
  switch (state) {
    case wait_for_nothing            : return "wait_for_nothing";
    case wait_for_compile            : return "wait_for_compile";
  }
  ShouldNotReachHere();
  return NULL;
}

const char* InvocationCounter::state_as_short_string(State state) {
  switch (state) {
    case wait_for_nothing            : return "not comp.";
    case wait_for_compile            : return "compileable";
  }
  ShouldNotReachHere();
  return NULL;
}


static address do_nothing(methodHandle method, TRAPS) {
  // dummy action for inactive invocation counters
  method->invocation_counter()->set_carry();
  method->invocation_counter()->set_state(InvocationCounter::wait_for_nothing);
  return NULL;
}


static address do_decay(methodHandle method, TRAPS) {
  // decay invocation counters so compilation gets delayed
  method->invocation_counter()->decay();
  return NULL;
}


void InvocationCounter::def(State state, int init, Action action) {
  assert(0 <= state && state < number_of_states, "illegal state");
  assert(0 <= init  && init  < count_limit, "initial value out of range");
  _init  [state] = init;
  _action[state] = action;
}

address dummy_invocation_counter_overflow(methodHandle m, TRAPS) {
  ShouldNotReachHere();
  return NULL;
}

void InvocationCounter::reinitialize(bool delay_overflow) {
  // define states
  guarantee(number_of_states <= state_limit, "adjust number_of_state_bits");
  def(wait_for_nothing, 0, do_nothing);
  if (delay_overflow) {
    def(wait_for_compile, 0, do_decay);
  } else {
    def(wait_for_compile, 0, dummy_invocation_counter_overflow);
  }

  InterpreterInvocationLimit = CompileThreshold << number_of_noncount_bits;
  InterpreterProfileLimit = ((CompileThreshold * InterpreterProfilePercentage) / 100)<< number_of_noncount_bits;

  // When methodData is collected, the backward branch limit is compared against a
  // methodData counter, rather than an InvocationCounter.  In the former case, we
  // don't need the shift by number_of_noncount_bits, but we do need to adjust 
  // the factor by which we scale the threshold.
  if (ProfileInterpreter) {
    InterpreterBackwardBranchLimit = (CompileThreshold * (OnStackReplacePercentage - InterpreterProfilePercentage)) / 100;
  } else {
    InterpreterBackwardBranchLimit = ((CompileThreshold * OnStackReplacePercentage) / 100) << number_of_noncount_bits;
  }

  assert(0 <= InterpreterBackwardBranchLimit,
	 "OSR threshold should be non-negative");
  assert(0 <= InterpreterProfileLimit && 
	 InterpreterProfileLimit <= InterpreterInvocationLimit, 
	 "profile threshold should be less than the compilation threshold "
	 "and non-negative");
}

void invocationCounter_init() {
  InvocationCounter::reinitialize(DelayCompilationDuringStartup);
}

