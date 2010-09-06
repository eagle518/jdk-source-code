#ifdef USE_PRAGMA_IDENT_HDR
#pragma ident "@(#)statSampler.hpp	1.6 03/12/23 16:44:13 JVM"
#endif
/*
 * Copyright 2004 Sun Microsystems, Inc.  All rights reserved.
 * SUN PROPRIETARY/CONFIDENTIAL.  Use is subject to license terms.
 */

class StatSamplerTask;

/*
 * The StatSampler class is responsible for periodically updating
 * sampled PerfData instances and writing the sampled values to the
 * PerfData memory region.
 *
 * In addition it is also responsible for providing a home for
 * PerfData instances that otherwise have no better home.
 */
class StatSampler : AllStatic {

  friend class StatSamplerTask;

  private:

    static StatSamplerTask* _task;
    static PerfDataList* _sampled;

    static void collect_sample();
    static void create_misc_perfdata();
    static void create_sampled_perfdata();
    static void sample_data(PerfDataList* list);
    static const char* get_system_property(const char* name, TRAPS); 
    static void create_system_property_instrumentation(TRAPS);

  public:
    // Start/stop the sampler
    static void engage();
    static void disengage();

    static bool is_active() { return _task != NULL; }

    static void initialize();
    static void destroy();
};

void statSampler_exit();
