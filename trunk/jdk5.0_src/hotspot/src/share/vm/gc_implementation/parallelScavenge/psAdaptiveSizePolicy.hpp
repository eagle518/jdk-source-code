#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)psAdaptiveSizePolicy.hpp	1.48 04/04/06 21:19:12 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// This class keeps statistical information and computes the
// optimal free space for both the young and old generation
// based on current application characteristics (based on gc cost
// and application footprint).
//
// It also computes an optimal tenuring threshold between the young
// and old generations, so as to equalize the cost of collections
// of those generations, as well as optimial survivor space sizes
// for the young generation.
//
// While this class is specifically intended for a generational system
// consisting of a young gen (containing an Eden and two semi-spaces)
// and a tenured gen, as well as a perm gen for reflective data, it
// makes NO references to specific generations.
//
// 05/02/2003 Update
// The 1.5 policy makes use of data gathered for the costs of GC on
// specific generations.  That data does reference specific
// generation.  Also diagnostics specific to generations have 
// been added.

// Forward decls
class elapsedTimer;

class PSAdaptiveSizePolicy : public CHeapObj {
 friend class PSGCAdaptivePolicyCounters;
 private:
  // These values are used to record decisions made during the
  // policy.  For example, if the young generation was decreased
  // to decrease the GC cost of minor collections the value
  // decrease_young_gen_for_throughput_true is used.

  enum SizePolicyTrueValues {
    decrease_old_gen_for_throughput_true = -7,
    decrease_young_gen_for_througput_true = -6,

    increase_old_gen_for_min_pauses_true = -5,
    decrease_old_gen_for_min_pauses_true = -4,
    decrease_young_gen_for_maj_pauses_true = -3,
    increase_young_gen_for_min_pauses_true = -2,
    increase_old_gen_for_maj_pauses_true = -1,

    decrease_young_gen_for_min_pauses_true = 1,
    decrease_old_gen_for_maj_pauses_true = 2,
    increase_young_gen_for_maj_pauses_true = 3,

    increase_old_gen_for_throughput_true = 4,
    increase_young_gen_for_througput_true = 5,

    decrease_young_gen_for_footprint_true = 6,
    decrease_old_gen_for_footprint_true = 7,
    decide_at_full_gc_true = 8
  };
  // Last calculated sizes, in bytes, and aligned
  // NEEDS_CLEANUP should use sizes.hpp,  but it works in ints, not size_t's
  size_t _eden_size;        // calculated eden free space in bytes
  size_t _promo_size;       // calculated old gen free space in bytes
  size_t _survivor_size;    // calculated survivor size in bytes

  // Major and minor collection timers, used to determine both
  // pause and interval times for collections
  static elapsedTimer _minor_timer; 
  static elapsedTimer _major_timer;

  // Time statistics
  AdaptivePaddedAverage* _avg_minor_pause;
  AdaptivePaddedAverage* _avg_major_pause;
  AdaptiveWeightedAverage* _avg_minor_interval;
  AdaptiveWeightedAverage* _avg_major_interval;
  AdaptiveWeightedAverage* _avg_major_gc_cost;
  AdaptiveWeightedAverage* _avg_minor_gc_cost;

  // Footprint statistics
  AdaptiveWeightedAverage* _avg_base_footprint;
  AdaptiveWeightedAverage* _avg_young_live;
  AdaptiveWeightedAverage* _avg_eden_live;
  AdaptiveWeightedAverage* _avg_old_live;

  // Statistics for survivor space calculation for young generation
  AdaptivePaddedAverage*   _avg_survived;
 
  // Statistical data gathered for GC
  GCStats _gc_stats;

  // Objects that have been directly allocated in the old generation.
  AdaptivePaddedNoZeroDevAverage*   _avg_pretenured;

  size_t _survivor_size_limit;   // Limit in bytes of survivor size
  // Allowed difference between major and minor gc times, used
  // for computing tenuring_threshold.
  const double _threshold_tolerance_percent;
  const double _collection_cost_margin_fraction;

  // Variable for estimating the major and minor pause times.
  // These variables represent linear least-squares fits of
  // the data.
  //   minor pause time vs. old gen size
  LinearLeastSquareFit* _minor_pause_old_estimator;
  //   major pause time vs. old gen size
  LinearLeastSquareFit* _major_pause_old_estimator;
  //   minor pause time vs. young gen size
  LinearLeastSquareFit* _minor_pause_young_estimator;
  //   major pause time vs. young gen size
  LinearLeastSquareFit* _major_pause_young_estimator;

  // Variables for estimating the major and minor collection costs
  //   minor collection time vs. young gen size
  LinearLeastSquareFit* _minor_collection_estimator;
  //   major collection time vs. old gen size
  LinearLeastSquareFit* _major_collection_estimator;

  // These record the most recent collection times.  They
  // are available as an alternative to using the averages
  // for making ergonomic decisions.
  double _latest_minor_mutator_interval_seconds;
  double _latest_major_mutator_interval_seconds;

  const size_t _generation_alignment;       // alignment for generations
  const size_t _intra_generation_alignment; // alignment for eden, survivors

  const double _gc_pause_goal_sec;     // goal for maximum gc pause
  const double _gc_minor_pause_goal_sec; // goal for maximum gc pause
  const double _throughput_goal;       // goal for throughput

  // This is a hint for the heap:  we've detected that gc times
  // are taking longer than GCTimeLimit allows.
  bool _gc_time_limit_exceeded; 
  // Use for diagnostics only.  If UseGCTimeLimit is false,
  // this variable is still set.
  bool _print_gc_time_limit_would_be_exceeded;

  // The amount of live data in the heap at the last full GC, used
  // as a baseline to help us determine when we need to perform the
  // next full GC.
  size_t _live_at_last_full_gc;

  // Flag indicating that the policy would 
  //   increase the tenuring threshold because of the total major gc cost
  //   is greater than the total minor gc cost
  bool _increment_tenuring_threshold_for_gc_cost;
  //   decrease the tenuring threshold because of the the total minor gc
  //   cost is greater than the total major gc cost
  bool _decrement_tenuring_threshold_for_gc_cost;
  //   decrease due to survivor size limit
  bool _decrement_tenuring_threshold_for_survivor_limit;

  // Variables for recording the decisions to increase or decrease
  // a generation size for the pause goal.  These variables take
  // the values of the enum SizePolicyTrueValues.

  // decrease/increase the old generation for major pause time
  int _change_old_gen_for_maj_pauses;

  // decrease/increase the young generation for minor pause time
  int _change_young_gen_for_min_pauses;

  // decrease/increase the old generation for minor pause time
  int _change_old_gen_for_min_pauses;

  // increase/decrease the young generation for major pause time
  int _change_young_gen_for_maj_pauses;

  //   change old geneneration for throughput
  int _change_old_gen_for_throughput;

  //   change young generation for throughput
  int _change_young_gen_for_throughput;
  
  //   increase generation sizes for footprint
  int _decrease_for_footprint;

  // Set if the ergonomic decisions were made at a full GC.
  int _decide_at_full_gc;

  // Flag indicating that the adaptive policy is ready to use
  bool _old_gen_policy_is_ready;
  bool _young_gen_policy_is_ready;

  // Changing the generation sizing depends on the data that is
  // gathered about the effects of changes on the pause times and
  // throughput.  These variable count the number of data points
  // gathered.  The policy may use these counters as a threshhold
  // for reliable data.
  julong _young_gen_change_for_major_pause_count;
  julong _young_gen_change_for_minor_throughput;
  julong _old_gen_change_for_major_throughput;

  // To facilitate faster growth at start up, supplement the normal
  // growth percentage for the young gen eden and the
  // old gen space for promotion with these value which decay
  // with increasing collections.
  uint _young_gen_size_increment_supplement;
  uint _old_gen_size_increment_supplement;

  // The number of bytes absorbed from eden into the old gen by moving the
  // boundary over live data.
  size_t _bytes_absorbed_from_eden;

 private:

  // Change the young generation size to achieve a minor GC pause time goal
  void adjust_for_minor_pause_time(bool is_full_gc,
				   size_t* desired_promo_size_ptr,
				   size_t* desired_eden_size_ptr);
  // Change the generation sizes to achieve a GC pause time goal
  // Returned sizes are not necessarily aligned.
  void adjust_for_pause_time(bool is_full_gc,
                         size_t* desired_promo_size_ptr,
                         size_t* desired_eden_size_ptr);
  // Change the generation sizes to achieve an application throughput goal
  // Returned sizes are not necessarily aligned.
  void adjust_for_throughput(bool is_full_gc,
                             size_t* desired_promo_size_ptr,
                             size_t* desired_eden_size_ptr);
  // Change the generation sizes to achieve minimum footprint
  // Returned sizes are not aligned.
  size_t adjust_promo_for_footprint(size_t desired_promo_size,
			            size_t desired_total);
  size_t adjust_eden_for_footprint(size_t desired_promo_size,
			           size_t desired_total);

  // Size in bytes for an increment or decrement of eden.
  size_t eden_increment(size_t cur_eden);
  size_t eden_increment(size_t cur_eden, uint percent_change);
  size_t eden_decrement(size_t cur_eden);
  size_t eden_decrement_aligned_down(size_t cur_eden);
  size_t eden_increment_with_supplement_aligned_up(size_t cur_eden);
  size_t eden_increment_aligned_up(size_t cur_eden);

  // Size in bytes for an increment or decrement of the promotion area
  size_t promo_increment(size_t cur_promo);
  size_t promo_increment(size_t cur_promo, uint percent_change);
  size_t promo_decrement(size_t cur_promo);
  size_t promo_decrement_aligned_down(size_t cur_promo);
  size_t promo_increment_with_supplement_aligned_up(size_t cur_promo);
  size_t promo_increment_aligned_up(size_t cur_promo);

  // Decay the supplemental growth additive.
  void decay_supplemental_growth(bool is_full_gc);

  // Returns a change that has been scaled down.  Result
  // is not aligned.  (If useful, move to some shared
  // location.)
  size_t scale_down(size_t change, double part, double total);

 protected:
  // Time accessors

  // The value returned is unitless:  it's the proportion of time
  // spent in a particular collection type.
  // An interval time will be 0.0 if a collection type hasn't occurred yet.
  // The 1.4.2 implementation put a floor on the values of major_gc_cost
  // and minor_gc_cost.  This was useful because of the way major_gc_cost
  // and minor_gc_cost was used in calculating the sizes of the generations.
  // Do not use a floor in this implementation because any finite value
  // will put a limit on the throughput that can be achieved and any
  // throughput goal above that limit will drive the generations sizes
  // to extremes.
  double minor_gc_cost() const {
    return MAX2(0.0F, _avg_minor_gc_cost->average());
  }
  double major_gc_cost() const {
    return MAX2(0.0F, _avg_major_gc_cost->average());
  }

  // Because we're dealing with averages, gc_cost() can be
  // larger than 1.0 if just the sum of the minor cost the 
  // the major cost is used.  Worse than that is the
  // fact that the minor cost and the major cost each
  // tend toward 1.0 in the extreme of high gc costs.
  // Limit the value of gc_cost to 1.0 so that the mutator
  // cost stays non-negative.
  double gc_cost() const {
    double result = MIN2(1.0, minor_gc_cost() + major_gc_cost());
    assert(result >= 0.0, "Both minor and major costs are non-negative");
    return result;
  }

  // Return the cost of the GC where the major gc cost
  // has been decayed based on the time since the last
  // major collection.
  double decaying_gc_cost() const;

  double mutator_cost() const { 
    double result = 1.0 - gc_cost(); 
    assert(result >= 0.0, "mutator cost calculation is incorrect");
    return result;
  }

  // Return the mutator cost using the decayed
  // GC cost.
  double adjusted_mutator_cost() const { 
    double result = 1.0 - decaying_gc_cost(); 
    assert(result >= 0.0, "adjusted mutator cost calculation is incorrect");
    return result;
  }

  // Footprint accessors
  size_t live_space() const { 
    return _avg_base_footprint->average() +
           _avg_young_live->average() +
           _avg_old_live->average();  
  }
  size_t free_space() const { 
    return _eden_size + _promo_size; 
  }

  void set_eden_size(size_t new_size) { 
    _eden_size = new_size; 
  }
  void set_promo_size(size_t new_size) { 
    _promo_size = new_size;
  }
  void set_survivor_size(size_t new_size) { 
    _survivor_size = new_size; 
  }

 public:
  // Accessors for use by performance counters
  AdaptivePaddedAverage*  avg_survived() { return _avg_survived; }
  AdaptivePaddedAverage*  avg_promoted() const { 
    return _gc_stats.avg_promoted(); 
  }
  AdaptivePaddedNoZeroDevAverage*  avg_pretenured() { return _avg_pretenured; }
  AdaptiveWeightedAverage* avg_base_footprint() { return _avg_base_footprint; }
  AdaptiveWeightedAverage* avg_eden_live() { return _avg_eden_live; }
  AdaptiveWeightedAverage* avg_old_live() { return _avg_old_live; }
  AdaptiveWeightedAverage* avg_young_live() { return _avg_young_live; }

  // Input arguments are initial free space sizes for young and old
  // generations, the initial survivor space size, the 
  // alignment values and the pause & throughput goals.
  //
  // NEEDS_CLEANUP this is a singleton object
  PSAdaptiveSizePolicy(size_t init_eden_size, 
		       size_t init_promo_size, 
		       size_t init_survivor_size, 
		       size_t generation_alignment,
		       size_t intra_generation_alignment,
		       double gc_pause_goal_sec,
		       double gc_minor_pause_goal_sec,
		       uint gc_time_ratio);

  // Methods indicating events of interest to the adaptive size policy,
  // called by GC algorithms. It is the responsibility of users of this
  // policy to call these methods at the correct times!
  void minor_collection_begin();
  void major_collection_begin();
  void minor_collection_end(GCCause::Cause gc_cause);
  void major_collection_end(size_t amount_live, GCCause::Cause gc_cause);

  // 
  void tenured_allocation(size_t size) {
    _avg_pretenured->sample(size);
  }

  // Accessors
  // NEEDS_CLEANUP   should use sizes.hpp
  size_t calculated_eden_size_in_bytes() const { 
    return _eden_size; 
  }
  size_t calculated_promo_size_in_bytes() const { 
    return _promo_size;
  }
  size_t calculated_survivor_size_in_bytes() const { 
    return _survivor_size; 
  }

  size_t calculated_old_free_size_in_bytes() const {
    return _promo_size + avg_promoted()->padded_average();
  }

  size_t average_old_live_in_bytes() const {
    return (size_t) _avg_old_live->average();
  }

  size_t average_promoted_in_bytes() const {
    return (size_t)avg_promoted()->average();
  }

  size_t padded_average_promoted_in_bytes() const {
    return (size_t)avg_promoted()->padded_average();
  }

  bool decrement_tenuring_threshold_for_gc_cost() { 
    return _decrement_tenuring_threshold_for_gc_cost; 
  }
  void set_decrement_tenuring_threshold_for_gc_cost(bool v) { 
    _decrement_tenuring_threshold_for_gc_cost = v; 
  }
  bool increment_tenuring_threshold_for_gc_cost() { 
    return _increment_tenuring_threshold_for_gc_cost; 
  }
  void set_increment_tenuring_threshold_for_gc_cost(bool v) { 
    _increment_tenuring_threshold_for_gc_cost = v; 
  }
  bool decrement_tenuring_threshold_for_survivor_limit() { 
    return _decrement_tenuring_threshold_for_survivor_limit; 
  }
  void set_decrement_tenuring_threshold_for_survivor_limit(bool v) { 
    _decrement_tenuring_threshold_for_survivor_limit = v; 
  }

  // accessors for flags recording the decisions to resize the
  // generations to meet the pause goal.
  int change_old_gen_for_maj_pauses() { 
    return _change_old_gen_for_maj_pauses; 
  }
  void set_change_old_gen_for_maj_pauses(int v) { 
    _change_old_gen_for_maj_pauses = v; 
  }

  int change_young_gen_for_min_pauses() { 
    return _change_young_gen_for_min_pauses; 
  }
  void set_change_young_gen_for_min_pauses(int v) { 
    _change_young_gen_for_min_pauses = v; 
  }

  int change_young_gen_for_maj_pauses() { 
    return _change_young_gen_for_maj_pauses; 
  }
  void set_change_young_gen_for_maj_pauses(int v) { 
    _change_young_gen_for_maj_pauses = v; 
  }

  int change_old_gen_for_min_pauses() { 
    return _change_old_gen_for_min_pauses; 
  }
  void set_change_old_gen_for_min_pauses(int v) { 
    _change_old_gen_for_min_pauses = v; 
  }

  // Return true if the old generation size was changed
  // to try to reach a pause time goal.
  bool old_gen_changed_for_pauses() {
    bool result = _change_old_gen_for_maj_pauses != 0 ||
		  _change_old_gen_for_min_pauses != 0;
    return result;
  }

  // Return true if the young generation size was changed
  // to try to reach a pause time goal.
  bool young_gen_changed_for_pauses() {
    bool result = _change_young_gen_for_min_pauses != 0 ||
		  _change_young_gen_for_maj_pauses != 0;
    return result;
  }
  // end flags for pause goal

  int change_old_gen_for_throughput() { 
    return _change_old_gen_for_throughput; 
  }
  void set_change_old_gen_for_throughput(int v) { 
    _change_old_gen_for_throughput = v; 
  }
  int change_young_gen_for_throughput() { 
    return _change_young_gen_for_throughput; 
  }
  void set_change_young_gen_for_throughput(int v) { 
    _change_young_gen_for_throughput = v; 
  }

  // Return true if the old generation size was changed
  // to try to reach a throughput goal.
  bool old_gen_changed_for_throughput() {
    bool result = _change_old_gen_for_throughput != 0;
    return result;
  }

  // Return true if the young generation size was changed
  // to try to reach a throughput goal.
  bool young_gen_changed_for_throughput() {
    bool result = _change_young_gen_for_throughput != 0;
    return result;
  }
		  
  int decrease_for_footprint() { return _decrease_for_footprint; }

  int decide_at_full_gc() { return _decide_at_full_gc; }
  void set_decide_at_full_gc(int v) { _decide_at_full_gc = v; }

  // Accessors for estimators.  The slope of the linear fit is
  // currently all that is used for making decisions.

  LinearLeastSquareFit* minor_pause_old_estimator() {
    return _minor_pause_old_estimator;
  }

  LinearLeastSquareFit* major_pause_old_estimator() {
    return _major_pause_old_estimator;
  }

  LinearLeastSquareFit* minor_pause_young_estimator() {
    return _minor_pause_young_estimator;
  }

  LinearLeastSquareFit* major_pause_young_estimator() {
    return _major_pause_young_estimator;
  }

  LinearLeastSquareFit* minor_collection_estimator() {
    return _minor_collection_estimator;
  }

  LinearLeastSquareFit* major_collection_estimator() {
    return _major_collection_estimator;
  }

  float major_pause_old_slope() { return _major_pause_old_estimator->slope(); }
  float minor_pause_old_slope() { 
    return _minor_pause_old_estimator->slope(); 
  }
  float major_pause_young_slope() { 
    return _major_pause_young_estimator->slope(); 
  }
  float minor_pause_young_slope() { 
    return _minor_pause_young_estimator->slope(); 
  }
  float major_collection_slope() { return _major_collection_estimator->slope();}
  float minor_collection_slope() { return _minor_collection_estimator->slope();}
  void set_decrease_for_footprint(int v) { _decrease_for_footprint = v; }

  void clear_generation_free_space_flags();

  bool old_gen_policy_is_ready() { return _old_gen_policy_is_ready; }
  bool young_gen_policy_is_ready() { return _young_gen_policy_is_ready; }

  // Given the amount of live data in the heap, should we
  // perform a Full GC?
  bool should_full_GC(size_t live_in_old_gen);

  // Calculates optimial free space sizes for both the old and young
  // generations.  Stores results in _eden_size and _promo_size.
  // Takes current used space in all generations as input, as well
  // as an indication if a full gc has just been performed, for use
  // in deciding if an OOM error should be thrown.
  void compute_generation_free_space(size_t young_live,
				     size_t eden_live,
                                     size_t old_live,
                                     size_t perm_live,
				     size_t cur_eden,  // current eden in bytes
				     size_t max_old_gen_size,
				     size_t max_eden_size,
                                     bool   is_full_gc);

  // Calculates new survivor space size;  returns a new tenuring threshold
  // value. Stores new survivor size in _survivor_size.
  int compute_survivor_space_size_and_threshold(bool   is_survivor_overflow,
                                                int    tenuring_threshold,
                                                size_t survivor_limit);

  // Return the maximum size of a survivor space if the young generation were of
  // size gen_size.
  size_t max_survivor_size(size_t gen_size) {
    // Never allow the target survivor size to grow more than MinSurvivorRatio
    // of the young generation size.  We cannot grow into a two semi-space
    // system, with Eden zero sized.  Even if the survivor space grows, from()
    // might grow by moving the bottom boundary "down" -- so from space will
    // remain almost full anyway (top() will be near end(), but there will be a
    // large filler object at the bottom).
    const size_t sz = gen_size / MinSurvivorRatio;
    const size_t alignment = _intra_generation_alignment;
    return sz > alignment ? align_size_down(sz, alignment) : alignment;
  }

  // This is a hint for the heap:  we've detected that gc times
  // are taking longer than GCTimeLimit allows.
  // Most heaps will choose to throw an OutOfMemoryError when
  // this occurs but it is up to the heap to request this information
  // of the policy
  bool gc_time_limit_exceeded() {
    return _gc_time_limit_exceeded;
  }
  void set_gc_time_limit_exceeded(bool v) {
    _gc_time_limit_exceeded = v;
  }
  bool print_gc_time_limit_would_be_exceeded() {
    return _print_gc_time_limit_would_be_exceeded;
  }
  void set_print_gc_time_limit_would_be_exceeded(bool v) {
    _print_gc_time_limit_would_be_exceeded = v;
  }

  size_t live_at_last_full_gc() { 
    return _live_at_last_full_gc; 
  }

  size_t bytes_absorbed_from_eden() const { return _bytes_absorbed_from_eden; }
  void   reset_bytes_absorbed_from_eden() { _bytes_absorbed_from_eden = 0; }

  void set_bytes_absorbed_from_eden(size_t val) {
    _bytes_absorbed_from_eden = val;
  }

  // Update averages that are always used (even
  // if adaptive sizing is turned off).
  void update_averages(bool is_survivor_overflow, 
		       size_t survived, 
		       size_t promoted);
};
