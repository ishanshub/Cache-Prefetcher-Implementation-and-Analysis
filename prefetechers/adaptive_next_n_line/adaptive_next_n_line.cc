#include "adaptive_next_n_line.h"
#include "cache.h"

uint32_t adaptive_next_n_line::prefetcher_cache_operate(champsim::address addr, champsim::address ip, uint8_t cache_hit, 
                                                      bool useful_prefetch, access_type type, uint32_t metadata_in)
{
  champsim::block_number cl_addr{addr};
  
  // check if in prefetch history
  auto found = prefetch_history.check_hit({addr, false});
  if (found.has_value() && found->used == false) {
    // useful
    prefetch_history.fill({addr, true});
    tracker.useful_prefetches++;
  }

  // periodic adjustment of prefetch range
  access_count++;
  if (access_count >= adjustment_period) {
    adjust_prefetch_degree();
    access_count = 0;
  }

  // only prefetches on cache misses
  if (cache_hit == 0) {

    for (int i = 1; i <= prefetch_degree; i++) {
      champsim::block_number pf_cl_addr = cl_addr + i;
      
      // page boundary check
      if (intern_->virtual_prefetch || champsim::page_number{pf_cl_addr} == champsim::page_number{cl_addr}) {

        const bool mshr_under_light_load = intern_->get_mshr_occupancy_ratio() < 0.5;
        
        // issue prefetch
        champsim::address pf_addr{pf_cl_addr};
        bool success = prefetch_line(pf_addr, mshr_under_light_load, 0);
        
        if (success) {
          // track prefetch
          prefetch_history.fill({pf_addr, false});
          tracker.total_prefetches++;
        }
      }
    }
  }

  return metadata_in;
}

void adaptive_next_n_line::adjust_prefetch_degree()
{
  double accuracy = tracker.get_accuracy();
  
  // Adjust prefetch degree based on accuracy
  if (accuracy > ACCURACY_THRESHOLD_HIGH && prefetch_degree < MAX_PREFETCH_DEGREE) {
    prefetch_degree++;
  } else if (accuracy < ACCURACY_THRESHOLD_LOW && prefetch_degree > MIN_PREFETCH_DEGREE) {
    prefetch_degree--;
  }
  
  // Reset the tracker for the next period
  tracker.reset();
}

uint32_t adaptive_next_n_line::prefetcher_cache_fill(champsim::address addr, long set, long way, uint8_t prefetch,
                                                  champsim::address evicted_addr, uint32_t metadata_in)
{
  return metadata_in;
}

void adaptive_next_n_line::prefetcher_cycle_operate()
{
  // This prefetcher doesn't need to do anything every cycle
  // as it issues prefetches directly in prefetcher_cache_operate
}