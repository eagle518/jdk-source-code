#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)psGCAdaptivePolicyCounters.hpp	1.10 04/02/05 12:13:05 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// PSGCAdaptivePolicyCounters is a holder class for performance counters
// that track the data and decisions for the ergonomics policy for the
// parallel scavenge collector.

class PSGCAdaptivePolicyCounters : public GCPolicyCounters {
  friend class VMStructs;

 private:
  PSAdaptiveSizePolicy*	_size_policy;
  // survivor space vs. tenuring threshold
  PerfVariable* _old_promo_size;
  PerfVariable* _old_eden_size;
  PerfVariable* _promo_size;
  PerfVariable* _eden_size;
  PerfVariable* _avg_survived_avg;
  PerfVariable* _avg_survived_dev;
  PerfVariable* _avg_survived_padded_avg;
  PerfVariable* _avg_promoted_avg;
  PerfVariable* _avg_promoted_dev;
  PerfVariable* _avg_promoted_padded_avg;
  PerfVariable* _avg_pretenured_padded_avg;
  PerfVariable* _survived;
  PerfVariable* _promoted;
  PerfVariable* _survivor_overflowed;
  PerfVariable* _increment_tenuring_threshold_for_gc_cost;
  PerfVariable* _decrement_tenuring_threshold_for_gc_cost;
  PerfVariable* _decrement_tenuring_threshold_for_survivor_limit;

  // young gen vs. old gen sizing
  PerfVariable* _avg_minor_pause;
  PerfVariable* _avg_major_pause;
  PerfVariable* _avg_minor_interval;
  PerfVariable* _avg_major_interval;
  PerfVariable* _minor_gc_cost;
  PerfVariable* _major_gc_cost;
  PerfVariable* _mutator_cost;
  PerfVariable* _live_space;
  PerfVariable* _free_space;
  PerfVariable* _avg_base_footprint;
  PerfVariable* _avg_young_live;
  PerfVariable* _avg_old_live;
  PerfVariable* _gc_time_limit_exceeded;
  PerfVariable* _live_at_last_full_gc;
  PerfVariable* _old_capacity;
  PerfVariable* _young_capacity;
  PerfVariable* _boundary_moved;

  PerfVariable* _change_old_gen_for_maj_pauses;
  PerfVariable* _change_young_gen_for_min_pauses;
  PerfVariable* _change_old_gen_for_min_pauses;
  PerfVariable* _change_young_gen_for_maj_pauses;

  PerfVariable* _change_old_gen_for_throughput;
  PerfVariable* _change_young_gen_for_throughput;
  PerfVariable* _decrease_for_footprint;
  PerfVariable* _decide_at_full_gc;

  PerfVariable* _major_pause_old_slope;
  PerfVariable* _minor_pause_old_slope;
  PerfVariable* _major_pause_young_slope;
  PerfVariable* _minor_pause_young_slope;
  PerfVariable* _major_collection_slope;
  PerfVariable* _minor_collection_slope;

  PerfVariable* _scavenge_skipped;
  PerfVariable* _full_follows_scavenge;

  // Use this time stamp if the gc time stamp is not available.
  TimeStamp	_counter_time_stamp;

 public:
  PSGCAdaptivePolicyCounters(const char* name, int collectors, int generations, 
			     PSAdaptiveSizePolicy* size_policy);
  inline void update_eden_size() {
    _eden_size->set_value(_size_policy->calculated_eden_size_in_bytes());
  }
  inline void update_promo_size() {
    _promo_size->set_value(_size_policy->calculated_promo_size_in_bytes());
  }
  inline void update_survivor_size() {
    desired_survivor_size()->set_value(
      _size_policy->calculated_survivor_size_in_bytes())
;
  }

  inline void update_young_capacity(size_t size_in_bytes) {
    _young_capacity->set_value(size_in_bytes);
  }
  inline void update_old_capacity(size_t size_in_bytes) {
    _old_capacity->set_value(size_in_bytes);
  }
  inline void update_old_eden_size(size_t old_size) {
    _old_eden_size->set_value(old_size);
  }
  inline void update_old_promo_size(size_t old_size) {
    _old_promo_size->set_value(old_size);
  }
  inline void update_boundary_moved(int size_in_bytes) {
    _boundary_moved->set_value(size_in_bytes);
  }

  inline void update_avg_survived_avg() {
    _avg_survived_avg->set_value(_size_policy->_avg_survived->average());
  }
  inline void update_avg_survived_dev() {
    _avg_survived_dev->set_value(_size_policy->_avg_survived->deviation());
  }
  inline void update_avg_survived_padded_avg() {
    _avg_survived_padded_avg->set_value(_size_policy->_avg_survived->padded_average());
  }

  inline void update_avg_promoted_avg() {
    _avg_promoted_avg->set_value(_size_policy->avg_promoted()->average());
  }
  inline void update_avg_promoted_dev() {
    _avg_promoted_dev->set_value(_size_policy->avg_promoted()->deviation());
  }
  inline void update_avg_promoted_padded_avg() {
    _avg_promoted_padded_avg->set_value(_size_policy->avg_promoted()->padded_average());
  }

  inline void update_avg_pretenured_padded_avg() {
    _avg_pretenured_padded_avg->set_value(_size_policy->_avg_pretenured->padded_average());
  }
  inline void update_survived(size_t survived) {
    _survived->set_value(survived);
  }
  inline void update_promoted(size_t promoted) {
    _promoted->set_value(promoted);
  }
  inline void update_survivor_overflowed(bool survivor_overflowed) {
    _survivor_overflowed->set_value(survivor_overflowed);
  }
  inline void update_tenuring_threshold(int threshold) {
    tenuring_threshold()->set_value(threshold);
  }
  inline void update_increment_tenuring_threshold_for_gc_cost() {
    _increment_tenuring_threshold_for_gc_cost->set_value(
      _size_policy->increment_tenuring_threshold_for_gc_cost());
  }
  inline void update_decrement_tenuring_threshold_for_gc_cost() {
    _decrement_tenuring_threshold_for_gc_cost->set_value(
      _size_policy->decrement_tenuring_threshold_for_gc_cost());
  }
  inline void update_decrement_tenuring_threshold_for_survivor_limit() {
    _decrement_tenuring_threshold_for_survivor_limit->set_value(
      _size_policy->decrement_tenuring_threshold_for_survivor_limit());
  }

  // For adjustments for pause times
  inline void update_change_old_gen_for_maj_pauses() {
    _change_old_gen_for_maj_pauses->set_value(
      _size_policy->change_old_gen_for_maj_pauses());
  }
  inline void update_change_young_gen_for_min_pauses() {
    _change_young_gen_for_min_pauses->set_value(
      _size_policy->change_young_gen_for_min_pauses());
  }
  inline void update_change_young_gen_for_maj_pauses() {
    _change_young_gen_for_maj_pauses->set_value(
      _size_policy->change_young_gen_for_maj_pauses());
  }
  inline void update_change_old_gen_for_min_pauses() {
    _change_old_gen_for_min_pauses->set_value(
      _size_policy->change_old_gen_for_min_pauses());
  }

  inline void update_change_old_gen_for_throughput() {
    _change_old_gen_for_throughput->set_value(
      _size_policy->change_old_gen_for_throughput());
  }
  inline void update_change_young_gen_for_throughput() {
    _change_young_gen_for_throughput->set_value(
      _size_policy->change_young_gen_for_throughput());
  }
  inline void update_decrease_for_footprint() {
    _decrease_for_footprint->set_value(
      _size_policy->decrease_for_footprint());
  }
  inline void update_decide_at_full_gc() {
    _decide_at_full_gc->set_value(
      _size_policy->decide_at_full_gc());
  }

  // compute_generation_free_space() statistics

  inline void update_avg_minor_pause() {
    _avg_minor_pause->set_value(_size_policy->_avg_minor_pause->average() * 
      1000.0);
  }
  inline void update_avg_major_pause() {
    _avg_major_pause->set_value(_size_policy->_avg_major_pause->average() * 
      1000.0);
  }
  inline void update_avg_minor_interval() {
    _avg_minor_interval->set_value(_size_policy->_avg_minor_interval->average() 
      * 1000.0);
  }
  inline void update_avg_major_interval() {
    _avg_major_interval->set_value(_size_policy->_avg_major_interval->average() 
      * 1000.0);
  }

  inline void update_minor_gc_cost() {
    _minor_gc_cost->set_value(_size_policy->minor_gc_cost() * 100.0);
  }
  inline void update_major_gc_cost() {
    _major_gc_cost->set_value(_size_policy->major_gc_cost() * 100.0);
  }
  inline void update_mutator_cost() {
    _mutator_cost->set_value(_size_policy->mutator_cost() * 100.0);
  }
  inline void update_live_space() {
    _live_space->set_value(_size_policy->live_space());
  }
  inline void update_free_space() {
    _free_space->set_value(_size_policy->free_space());
  }

  inline void update_avg_base_footprint() {
    _avg_base_footprint->set_value(_size_policy->avg_base_footprint()->average());
  }
  inline void update_avg_young_live() {
    _avg_young_live->set_value(_size_policy->avg_young_live()->average());
  }
  inline void update_avg_old_live() {
    _avg_old_live->set_value(_size_policy->avg_old_live()->average());
  }
  // Scale up all the slopes
  inline void update_major_pause_old_slope() {
    _major_pause_old_slope->set_value(
      _size_policy->major_pause_old_slope() * 1000);
  }
  inline void update_minor_pause_old_slope() {
    _minor_pause_old_slope->set_value(
      _size_policy->minor_pause_old_slope() * 1000);
  }
  inline void update_major_pause_young_slope() {
    _major_pause_young_slope->set_value(
      _size_policy->major_pause_young_slope() * 1000);
  }
  inline void update_minor_pause_young_slope() {
    _minor_pause_young_slope->set_value(
      _size_policy->minor_pause_young_slope() * 1000);
  }
  inline void update_major_collection_slope() {
    _major_collection_slope->set_value(
      _size_policy->major_collection_slope() * 1000);
  }
  inline void update_minor_collection_slope() {
    _minor_collection_slope->set_value(
      _size_policy->minor_collection_slope() * 1000);
  }

  inline void update_scavenge_skipped(int cause) {
    _scavenge_skipped->set_value(cause);
  }

  inline void update_full_follows_scavenge(int event) {
    _full_follows_scavenge->set_value(event);
  }

  // Update all the counters that can be updated from the size policy.
  // This should be called after all policy changes have been made
  // and reflected internall in the size policy.
  void update_counters_from_policy();

  // Update counters that can be updated from fields internal to the
  // counter or from globals.  This is distinguished from counters
  // that are updated via input parameters.
  void update_counters();
};
