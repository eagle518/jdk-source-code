/*
 * Copyright (c) 2001, 2009, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */

class G1CollectorPolicy;

class SurvRateGroup : public CHeapObj {
private:
  G1CollectorPolicy* _g1p;
  const char* _name;

  size_t  _stats_arrays_length;
  double* _surv_rate;
  double* _accum_surv_rate_pred;
  double  _last_pred;
  double  _accum_surv_rate;
  TruncatedSeq** _surv_rate_pred;
  NumberSeq**    _summary_surv_rates;
  size_t         _summary_surv_rates_len;
  size_t         _summary_surv_rates_max_len;

  int _all_regions_allocated;
  size_t _region_num;
  size_t _scan_only_prefix;
  size_t _setup_seq_num;

public:
  SurvRateGroup(G1CollectorPolicy* g1p,
                const char* name,
                size_t summary_surv_rates_len);
  void reset();
  void start_adding_regions();
  void stop_adding_regions();
  void record_scan_only_prefix(size_t scan_only_prefix);
  void record_surviving_words(int age_in_group, size_t surv_words);
  void all_surviving_words_recorded(bool propagate);
  const char* name() { return _name; }

  size_t region_num() { return _region_num; }
  size_t scan_only_length() { return _scan_only_prefix; }
  double accum_surv_rate_pred(int age) {
    assert(age >= 0, "must be");
    if ((size_t)age < _stats_arrays_length)
      return _accum_surv_rate_pred[age];
    else {
      double diff = (double) (age - _stats_arrays_length + 1);
      return _accum_surv_rate_pred[_stats_arrays_length-1] + diff * _last_pred;
    }
  }

  double accum_surv_rate(size_t adjustment);

  TruncatedSeq* get_seq(size_t age) {
    if (age >= _setup_seq_num) {
      guarantee( _setup_seq_num > 0, "invariant" );
      age = _setup_seq_num-1;
    }
    TruncatedSeq* seq = _surv_rate_pred[age];
    guarantee( seq != NULL, "invariant" );
    return seq;
  }

  int next_age_index();
  int age_in_group(int age_index) {
    int ret = (int) (_all_regions_allocated -  age_index);
    assert( ret >= 0, "invariant" );
    return ret;
  }
  int recalculate_age_index(int age_index) {
    int new_age_index = (int) _scan_only_prefix - age_in_group(age_index);
    guarantee( new_age_index >= 0, "invariant" );
    return new_age_index;
  }
  void finished_recalculating_age_indexes() {
    _all_regions_allocated = (int) _scan_only_prefix;
  }

#ifndef PRODUCT
  void print();
  void print_surv_rate_summary();
#endif // PRODUCT
};
