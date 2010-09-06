#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)psGCAdaptivePolicyCounters.cpp	1.12 04/02/11 09:38:32 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_psGCAdaptivePolicyCounters.cpp.incl"



PSGCAdaptivePolicyCounters::PSGCAdaptivePolicyCounters(const char* name_arg,
                         		      int collectors, 
					      int generations,
                         		      PSAdaptiveSizePolicy* size_policy)
	: GCPolicyCounters(name_arg, collectors, generations),
	  _size_policy(size_policy) {
  if (UsePerfData) {
    EXCEPTION_MARK;
    ResourceMark rm;

    const char* cname = PerfDataManager::counter_name(name_space(), "edenSize");
    _eden_size = PerfDataManager::create_variable(SUN_GC, cname,
      PerfData::U_Bytes, _size_policy->calculated_eden_size_in_bytes(), CHECK);

    cname = PerfDataManager::counter_name(name_space(), "promoSize");
    _promo_size = PerfDataManager::create_variable(SUN_GC, cname,
      PerfData::U_Bytes, _size_policy->calculated_promo_size_in_bytes(), CHECK);

    cname = PerfDataManager::counter_name(name_space(), "oldPromoSize");
    _old_promo_size = PerfDataManager::create_variable(SUN_GC, cname,
      PerfData::U_Bytes, _size_policy->calculated_promo_size_in_bytes(), CHECK);

    cname = PerfDataManager::counter_name(name_space(), "oldEdenSize");
    _old_eden_size = PerfDataManager::create_variable(SUN_GC, cname,
      PerfData::U_Bytes, _size_policy->calculated_eden_size_in_bytes(), CHECK);

    cname = PerfDataManager::counter_name(name_space(), "oldCapacity");
    _old_capacity = PerfDataManager::create_variable(SUN_GC, cname,
      PerfData::U_Bytes, (jlong) Arguments::initial_heap_size(), CHECK);

    cname = PerfDataManager::counter_name(name_space(), "youngCapacity");
    _young_capacity = PerfDataManager::create_variable(SUN_GC, cname,
      PerfData::U_Bytes, (jlong) Arguments::initial_heap_size(), CHECK);

    cname = PerfDataManager::counter_name(name_space(), "boundaryMoved");
    _boundary_moved = PerfDataManager::create_variable(SUN_GC, cname,
      PerfData::U_Bytes, (jlong) 0, CHECK);

    cname = PerfDataManager::counter_name(name_space(), "avgSurvivedAvg");
    _avg_survived_avg = PerfDataManager::create_variable(SUN_GC, cname,
      PerfData::U_Bytes, _size_policy->calculated_survivor_size_in_bytes(),
        CHECK);

    cname = PerfDataManager::counter_name(name_space(), "avgSurvivedDev");
    _avg_survived_dev = PerfDataManager::create_variable(SUN_GC, cname,
      PerfData::U_Bytes, (jlong) 0 , CHECK);

    cname = PerfDataManager::counter_name(name_space(), "avgSurvivedPaddedAvg");
    _avg_survived_padded_avg =
      PerfDataManager::create_variable(SUN_GC, cname, PerfData::U_Bytes,
        _size_policy->calculated_survivor_size_in_bytes(), CHECK);

    cname = PerfDataManager::counter_name(name_space(), "avgPromotedAvg");
    _avg_promoted_avg = 
      PerfDataManager::create_variable(SUN_GC, cname, PerfData::U_Bytes,
        _size_policy->calculated_promo_size_in_bytes(), CHECK);

    cname = PerfDataManager::counter_name(name_space(), "avgPromotedDev");
    _avg_promoted_dev = 
      PerfDataManager::create_variable(SUN_GC, cname, PerfData::U_Bytes,
        (jlong) 0 , CHECK);

    cname = PerfDataManager::counter_name(name_space(), "avgPromotedPaddedAvg");
    _avg_promoted_padded_avg = 
      PerfDataManager::create_variable(SUN_GC, cname, PerfData::U_Bytes,
        _size_policy->calculated_promo_size_in_bytes(), CHECK);

    cname = PerfDataManager::counter_name(name_space(),
      "avgPretenuredPaddedAvg");
    _avg_pretenured_padded_avg = 
      PerfDataManager::create_variable(SUN_GC, cname, PerfData::U_Bytes,
        (jlong) 0, CHECK);

    cname = PerfDataManager::counter_name(name_space(), "survived");
    _survived = PerfDataManager::create_variable(SUN_GC, cname,
      PerfData::U_Bytes, (jlong) 0, CHECK);

    cname = PerfDataManager::counter_name(name_space(), "promoted");
    _promoted = PerfDataManager::create_variable(SUN_GC, cname,
      PerfData::U_Bytes, (jlong) 0, CHECK);

    cname = PerfDataManager::counter_name(name_space(), "survivorOverflowed");
    _survivor_overflowed = PerfDataManager::create_variable(SUN_GC, cname,
      PerfData::U_Events, (jlong)0, CHECK);

    cname = PerfDataManager::counter_name(name_space(), 
      "decrementTenuringThresholdForGcCost");
    _decrement_tenuring_threshold_for_gc_cost = 
      PerfDataManager::create_variable(SUN_GC, cname, PerfData::U_Events, 
        (jlong)0, CHECK);

    cname = PerfDataManager::counter_name(name_space(), 
      "incrementTenuringThresholdForGcCost");
    _increment_tenuring_threshold_for_gc_cost = 
      PerfDataManager::create_variable(SUN_GC, cname, PerfData::U_Events, 
        (jlong)0, CHECK);

    cname = PerfDataManager::counter_name(name_space(), 
      "decrementTenuringThresholdForSurvivorLimit");
    _decrement_tenuring_threshold_for_survivor_limit = 
      PerfDataManager::create_variable(SUN_GC, cname, PerfData::U_Events, 
        (jlong)0, CHECK);

    cname = PerfDataManager::counter_name(name_space(), 
      "changeOldGenForMajPauses");
    _change_old_gen_for_maj_pauses = 
      PerfDataManager::create_variable(SUN_GC, cname, PerfData::U_Events, 
        (jlong)0, CHECK);

    cname = PerfDataManager::counter_name(name_space(), 
      "changeYoungGenForMinPauses");
    _change_young_gen_for_min_pauses = 
      PerfDataManager::create_variable(SUN_GC, cname, PerfData::U_Events, 
        (jlong)0, CHECK);

    cname = PerfDataManager::counter_name(name_space(), 
      "changeYoungGenForMajPauses");
    _change_young_gen_for_maj_pauses = 
      PerfDataManager::create_variable(SUN_GC, cname, PerfData::U_Events, 
        (jlong)0, CHECK);

    cname = PerfDataManager::counter_name(name_space(), 
      "changeOldGenForMinPauses");
    _change_old_gen_for_min_pauses = 
      PerfDataManager::create_variable(SUN_GC, cname, PerfData::U_Events, 
        (jlong)0, CHECK);

    cname = PerfDataManager::counter_name(name_space(), 
      "increaseOldGenForThroughput");
    _change_old_gen_for_throughput = 
      PerfDataManager::create_variable(SUN_GC, cname, PerfData::U_Events, 
        (jlong)0, CHECK);

    cname = PerfDataManager::counter_name(name_space(), 
      "increaseYoungGenForThroughput");
    _change_young_gen_for_throughput = 
      PerfDataManager::create_variable(SUN_GC, cname, PerfData::U_Events, 
        (jlong)0, CHECK);

    cname = PerfDataManager::counter_name(name_space(), 
      "decreaseForFootprint");
    _decrease_for_footprint = PerfDataManager::create_variable(SUN_GC, cname,
      PerfData::U_Events, (jlong)0, CHECK);

    cname = PerfDataManager::counter_name(name_space(), "decideAtFullGc");
    _decide_at_full_gc = PerfDataManager::create_variable(SUN_GC, cname,
      PerfData::U_None, (jlong)0, CHECK);

    cname = PerfDataManager::counter_name(name_space(), "avgMinorPauseTime");
    _avg_minor_pause = PerfDataManager::create_variable(SUN_GC, cname,
      PerfData::U_Ticks, _size_policy->_avg_minor_pause->average(), CHECK);

    cname = PerfDataManager::counter_name(name_space(), "avgMajorPauseTime");
    _avg_major_pause = PerfDataManager::create_variable(SUN_GC, cname,
      PerfData::U_Ticks, _size_policy->_avg_major_pause->average(), CHECK);

    cname = PerfDataManager::counter_name(name_space(), "avgMinorIntervalTime");
    _avg_minor_interval = PerfDataManager::create_variable(SUN_GC, cname,
      PerfData::U_Ticks, _size_policy->_avg_minor_interval->average(), CHECK);

    cname = PerfDataManager::counter_name(name_space(), "avgMajorIntervalTime");
    _avg_major_interval = PerfDataManager::create_variable(SUN_GC, cname,
      PerfData::U_Ticks, _size_policy->_avg_major_interval->average(), CHECK);

    cname = PerfDataManager::counter_name(name_space(), "minorGcCost");
    _minor_gc_cost = PerfDataManager::create_variable(SUN_GC, cname,
      PerfData::U_Ticks, _size_policy->minor_gc_cost(), CHECK);

    cname = PerfDataManager::counter_name(name_space(), "majorGcCost");
    _major_gc_cost = PerfDataManager::create_variable(SUN_GC, cname,
       PerfData::U_Ticks, _size_policy->major_gc_cost(), CHECK);

    cname = PerfDataManager::counter_name(name_space(), "mutatorCost");
    _mutator_cost = PerfDataManager::create_variable(SUN_GC, cname,
      PerfData::U_Ticks, _size_policy->mutator_cost(), CHECK);

    cname = PerfDataManager::counter_name(name_space(), "liveSpace");
    _live_space = PerfDataManager::create_variable(SUN_GC, cname,
      PerfData::U_Bytes, _size_policy->live_space(), CHECK);

    cname = PerfDataManager::counter_name(name_space(), "freeSpace");
    _free_space = PerfDataManager::create_variable(SUN_GC, cname,
      PerfData::U_Bytes, _size_policy->free_space(), CHECK);

    cname = PerfDataManager::counter_name(name_space(), "avgBaseFootprint");
    _avg_base_footprint = PerfDataManager::create_variable(SUN_GC, cname,
      PerfData::U_Bytes, _size_policy->avg_base_footprint()->average(), CHECK);

    cname = PerfDataManager::counter_name(name_space(), "avgYoungLive");
    _avg_young_live = PerfDataManager::create_variable(SUN_GC, cname,
      PerfData::U_Bytes, _size_policy->avg_young_live()->average(), CHECK);

    cname = PerfDataManager::counter_name(name_space(), "avgOldLive");
    _avg_old_live = PerfDataManager::create_variable(SUN_GC, cname,
      PerfData::U_Bytes, _size_policy->avg_old_live()->average(), CHECK);

    cname = PerfDataManager::counter_name(name_space(), "gcTimeLimitExceeded");
    _gc_time_limit_exceeded = PerfDataManager::create_variable(SUN_GC, cname,
      PerfData::U_Events, _size_policy->gc_time_limit_exceeded(), CHECK);

    cname = PerfDataManager::counter_name(name_space(), "liveAtLastFullGc");
    _live_at_last_full_gc = PerfDataManager::create_variable(SUN_GC, cname,
      PerfData::U_Bytes, _size_policy->live_at_last_full_gc(), CHECK);

    cname = PerfDataManager::counter_name(name_space(), "majorPauseOldSlope");
    _major_pause_old_slope = PerfDataManager::create_variable(SUN_GC, cname,
      PerfData::U_None, (jlong) 0, CHECK);

    cname = PerfDataManager::counter_name(name_space(), "minorPauseOldSlope");
    _minor_pause_old_slope = PerfDataManager::create_variable(SUN_GC, cname,
      PerfData::U_None, (jlong) 0, CHECK);

    cname = PerfDataManager::counter_name(name_space(), "majorPauseYoungSlope");
    _major_pause_young_slope = PerfDataManager::create_variable(SUN_GC, cname,
      PerfData::U_None, (jlong) 0, CHECK);

    cname = PerfDataManager::counter_name(name_space(), "minorPauseYoungSlope");
    _minor_pause_young_slope = PerfDataManager::create_variable(SUN_GC, cname,
      PerfData::U_None, (jlong) 0, CHECK);

    cname = PerfDataManager::counter_name(name_space(), "majorCollectionSlope");
    _major_collection_slope = PerfDataManager::create_variable(SUN_GC, cname,
      PerfData::U_None, (jlong) 0, CHECK);

    cname = PerfDataManager::counter_name(name_space(), "minorCollectionSlope");
    _minor_collection_slope = PerfDataManager::create_variable(SUN_GC, cname,
      PerfData::U_None, (jlong) 0, CHECK);

    cname = PerfDataManager::counter_name(name_space(), "scavengeSkipped");
    _scavenge_skipped = PerfDataManager::create_variable(SUN_GC, cname,
      PerfData::U_Bytes, (jlong) 0, CHECK);

    cname = PerfDataManager::counter_name(name_space(), "fullFollowsScavenge");
    _full_follows_scavenge = PerfDataManager::create_variable(SUN_GC, cname,
      PerfData::U_Bytes, (jlong) 0, CHECK);

    _counter_time_stamp.update();
  }
}

void PSGCAdaptivePolicyCounters::update_counters_from_policy() {
  if (UsePerfData) {
    update_eden_size();
    update_promo_size();
    update_survivor_size();
    update_avg_survived_avg();
    update_avg_survived_dev();
    update_avg_survived_padded_avg();
    update_avg_promoted_avg();
    update_avg_promoted_dev();
    update_avg_promoted_padded_avg();
    update_avg_pretenured_padded_avg();

    update_avg_minor_pause();
    update_avg_major_pause();
    update_avg_minor_interval();
    update_avg_major_interval();
    update_minor_gc_cost();
    update_major_gc_cost();
    update_mutator_cost();
    update_live_space();
    update_free_space();
    update_avg_base_footprint();
    update_avg_young_live();
    update_avg_old_live();
    update_decrement_tenuring_threshold_for_gc_cost();
    update_increment_tenuring_threshold_for_gc_cost();
    update_decrement_tenuring_threshold_for_survivor_limit();

    update_change_old_gen_for_maj_pauses();
    update_change_young_gen_for_min_pauses();
    update_change_young_gen_for_maj_pauses();
    update_change_old_gen_for_min_pauses();

    update_change_old_gen_for_throughput();
    update_change_young_gen_for_throughput();

    update_decrease_for_footprint();
    update_decide_at_full_gc();

    update_major_pause_old_slope();
    update_minor_pause_old_slope();
    update_major_pause_young_slope();
    update_minor_pause_young_slope();
    update_major_collection_slope();
    update_minor_collection_slope();
  }
}

void PSGCAdaptivePolicyCounters::update_counters() {
  if (UsePerfData) {
    update_counters_from_policy();
  }
}

