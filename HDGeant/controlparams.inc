      integer writenohits, showersincol, driftclusters, runtime_geom
      real tgtwidth
      integer get_next_evt
      real trigger_time_sigma_ns
      integer event_count
      integer override_run_number
      common /controlparams/ writenohits, showersincol, driftclusters
     +                       ,tgtwidth(2),runtime_geom,get_next_evt
     +                       ,trigger_time_sigma_ns
     +                       ,event_count,override_run_number

      integer genbeam_precol
      integer genbeam_postcol
      integer genbeam_mode
      common /genbeam_pars/ genbeam_precol, genbeam_postcol,
     +                      genbeam_mode(20)
