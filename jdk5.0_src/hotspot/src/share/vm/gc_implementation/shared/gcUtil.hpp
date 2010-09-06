#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)gcUtil.hpp	1.11 03/12/23 16:40:25 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

// Catch-all file for utility classes

// A weighted average maintains a running, weighted average
// of some float value (templates would be handy here if we
// need different types).
//
// The average is adaptive in that we smooth it for the 
// initial samples; we don't use the weight until we have
// enough samples for it to be meaningful.
//
// This serves as our best estimate of a future unknown.
//
class AdaptiveWeightedAverage : public CHeapObj {
 private: 
  float            _average;        // The last computed average
  unsigned         _sample_count;   // How often we've sampled this average
  const unsigned   _weight;         // The weight used to smooth the averages
                                    //   A higher weight favors the most
                                    //   recent data.
  float		   _last_sample;    // The last value sampled.

 protected:
  void  increment_count()       { _sample_count++;       }
  void  set_average(float avg)  { _average = avg;        }

  // Helper function, computes an adaptive weighted average 
  // given a sample and the last average
  float compute_adaptive_average(float new_sample, float average);

 public:
  // Input weight must be between 0 and 100
  AdaptiveWeightedAverage(unsigned weight) : 
    _average(0.0), _sample_count(0), _weight(weight) {
  }

  // Accessors
  float    average() const       { return _average;       }
  unsigned weight()  const       { return _weight;        }
  unsigned count()   const       { return _sample_count;  }
  float    last_sample() const	 { return _last_sample; }

  // Update data with a new sample.
  void sample(float new_sample);

  static inline float exp_avg(float avg, float sample,
                               unsigned int weight) {
    assert(0 <= weight && weight <= 100, "weight must be a percent");
    return (100.0F - weight) * avg / 100.0F + weight * sample / 100.0F;
  }
  static inline size_t exp_avg(size_t avg, size_t sample,
                               unsigned int weight) {
    // Convert to float and back to avoid integer overflow.
    return (size_t)exp_avg((float)avg, (float)sample, weight);
  }
};


// A weighted average that includes a deviation from the average,
// some multiple of which is added to the average.
//
// This serves as our best estimate of an upper bound on a future
// unknown.
class AdaptivePaddedAverage : public AdaptiveWeightedAverage {
 private:
  float    _padded_avg;           // The last computed padded average 
  float    _deviation;            // Running deviation from the average
  const unsigned _padding;        // A multiple which, added to the average,
                                  // gives us an upper bound guess.

 protected:
  void set_padded_average(float avg)  { _padded_avg = avg;  }
  void set_deviation(float dev)       { _deviation  = dev;  }

 public:
  AdaptivePaddedAverage(unsigned weight, unsigned padding) :
    AdaptiveWeightedAverage(weight), 
    _padded_avg(0.0), _deviation(0.0), _padding(padding) {}

  // Accessor
  float padded_average() const         { return _padded_avg; }
  float deviation()      const         { return _deviation;  }
  unsigned padding()     const         { return _padding;    }

  // Override
  void  sample(float new_sample);
};

// A weighted average that includes a deviation from the average,
// some multiple of which is added to the average.
//
// This serves as our best estimate of an upper bound on a future
// unknown.
// A special sort of padded average:  it doesn't update deviations
// if the sample is zero. The average is allowed to change. We're
// preventing the zero samples from drastically changing our padded
// average.
class AdaptivePaddedNoZeroDevAverage : public AdaptivePaddedAverage {
public:
  AdaptivePaddedNoZeroDevAverage(unsigned weight, unsigned padding) :
    AdaptivePaddedAverage(weight, padding)  {}
  // Override
  void  sample(float new_sample);
};
// Use a least squares fit to a set of data to generate a linear
// equation.
//              y = intercept + slope * x

class LinearLeastSquareFit : public CHeapObj {
  double _sum_x;  	// sum of all independent data points x
  double _sum_x_squared; // sum of all independent data points x**2
  double _sum_y;  	// sum of all dependent data points y
  double _sum_xy; 	// sum of all x * y.
  double _intercept;     // constant term
  double _slope;     	// slope
  // The weighted averages are not currently used but perhaps should
  // be used to get decaying averages.
  AdaptiveWeightedAverage _mean_x; // weighted mean of independent variable
  AdaptiveWeightedAverage _mean_y; // weighted mean of dependent variable
  
 public:
  LinearLeastSquareFit(unsigned weight);
  void update(double x, double y);
  double y(double x);
  double slope() { return _slope; }
  // Methods to decide if a change in the dependent variable will
  // achive a desired goal.  Note that these methods are not
  // complementary and both are needed.
  bool decrement_will_decrease();
  bool increment_will_decrease();
};
