#ifdef USE_PRAGMA_IDENT_SRC
#pragma ident "@(#)gcUtil.cpp	1.11 03/12/23 16:40:25 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

# include "incls/_precompiled.incl"
# include "incls/_gcUtil.cpp.incl"

// Catch-all file for utility classes

float AdaptiveWeightedAverage::compute_adaptive_average(float new_sample, 
                                                        float average) {
  // We smooth the samples by not using weight() directly until we've
  // had enough data to make it meaningful. We'd like the first weight
  // used to be 1, the second to be 1/2, etc until we have 100/weight
  // samples.
  unsigned count_weight = 100/count();
  unsigned adaptive_weight = (MAX2(weight(), count_weight));

  float new_avg = exp_avg(average, new_sample, adaptive_weight);

  return new_avg;
}

void AdaptiveWeightedAverage::sample(float new_sample) {
  increment_count();

  // Compute the new weighted average
  float new_avg = compute_adaptive_average(new_sample, average());
  set_average(new_avg);
  _last_sample = new_sample;
}

void AdaptivePaddedAverage::sample(float new_sample) {
  // Compute our parent classes sample information
  AdaptiveWeightedAverage::sample(new_sample);

  // Now compute the deviation and the new padded sample
  float new_avg = average();
  float new_dev = compute_adaptive_average(labs(new_sample - new_avg),
                                           deviation());

  set_deviation(new_dev);
  set_padded_average(new_avg + padding() * new_dev);
}

void AdaptivePaddedNoZeroDevAverage::sample(float new_sample) {
  // Compute our parent classes sample information
  AdaptiveWeightedAverage::sample(new_sample);

  float new_avg = average();
  if (new_sample != 0) {
    // We only create a new deviation if the sample is non-zero
    float new_dev = compute_adaptive_average(labs(new_sample - new_avg),
                                             deviation());

    set_deviation(new_dev);
  }
  set_padded_average(new_avg + padding() * deviation());
}

LinearLeastSquareFit::LinearLeastSquareFit(unsigned weight) :
  _sum_x(0), _sum_y(0), _sum_xy(0),
  _mean_x(weight), _mean_y(weight) {}

void LinearLeastSquareFit::update(double x, double y) {
  _sum_x = _sum_x + x;
  _sum_x_squared = _sum_x_squared + x * x;
  _sum_y = _sum_y + y;
  _sum_xy = _sum_xy + x * y;
  _mean_x.sample(x);
  _mean_y.sample(y);
  assert(_mean_x.count() == _mean_y.count(), "Incorrect count");
  if ( _mean_x.count() > 1 ) {
    double slope_denominator;
    slope_denominator = (_mean_x.count() * _sum_x_squared - _sum_x * _sum_x);
    // Some tolerance should be injected here.  A denominator that is 
    // nearly 0 should be avoided.

    if (slope_denominator != 0.0) {
      double slope_numerator;
      slope_numerator = (_mean_x.count() * _sum_xy - _sum_x * _sum_y);
      _slope = slope_numerator / slope_denominator;

      // The _mean_y and _mean_x are decaying averages and can
      // be used to discount earlier data.  If they are used,
      // first consider whether all the quantities should be
      // kept as decaying averages.
      // _intercept = _mean_y.average() - _slope * _mean_x.average();
      _intercept = (_sum_y - _slope * _sum_x) / ((double) _mean_x.count());
    }
  }
}

double LinearLeastSquareFit::y(double x) {
  double new_y;
  
  if ( _mean_x.count() > 1 ) {
    new_y = (_intercept + _slope * x);
    return new_y;
  } else {
    return _mean_y.average();
  }
}

// Both decrement_will_decrease() and increment_will_decrease() return
// true for a slope of 0.  That is because a change is necessary before
// a slope can be calculated and a 0 slope will, in general, indicate
// that no calculation of the slope has yet been done.  Returning true
// for a slope equal to 0 reflects the intuitive expectation of the
// dependence on the slope.  Don't use the complement of these functions
// since that untuitive expectation is not built into the complement.
bool LinearLeastSquareFit::decrement_will_decrease() {
  return (_slope >= 0.00);
} 

bool LinearLeastSquareFit::increment_will_decrease() {
  return (_slope <= 0.00);
} 
