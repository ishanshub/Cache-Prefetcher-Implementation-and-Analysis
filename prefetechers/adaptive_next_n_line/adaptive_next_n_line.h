#ifndef ADAPTIVE_NEXT_N_LINE_H
#define ADAPTIVE_NEXT_N_LINE_H

#include <cstdint>
#include <optional>

#include "address.h"
#include "champsim.h"
#include "modules.h"
#include "msl/lru_table.h"

struct adaptive_next_n_line : public champsim::modules::prefetcher {
  constexpr static int MAX_PREFETCH_DEGREE = 8;
  constexpr static int MIN_PREFETCH_DEGREE = 1;
  constexpr static int INITIAL_PREFETCH_DEGREE = 2;
  constexpr static int PREFETCH_HISTORY_SIZE = 1024;
  constexpr static double ACCURACY_THRESHOLD_HIGH = 0.6;
  constexpr static double ACCURACY_THRESHOLD_LOW = 0.2;
  
  struct prefetch_tracker {
    uint64_t total_prefetches = 0;
    uint64_t useful_prefetches = 0;
    
    double get_accuracy() const {
      if (total_prefetches == 0) return 0.0;
      return static_cast<double>(useful_prefetches) / total_prefetches;
    }
    
    void reset() {
      total_prefetches = 0;
      useful_prefetches = 0;
    }
  };
  
  struct prefetch_history_entry {
    champsim::address addr{};
    bool used = false;

    auto index() const {
      using namespace champsim::data::data_literals;
      return addr.slice_upper<2_b>();
    }
    
    auto tag() const {
      using namespace champsim::data::data_literals;
      return addr.slice_upper<2_b>();
    }
  };
  
  int prefetch_degree = INITIAL_PREFETCH_DEGREE;
  
  prefetch_tracker tracker;
  
  const uint64_t adjustment_period = 1000;
  uint64_t access_count = 0;
  
  champsim::msl::lru_table<prefetch_history_entry> prefetch_history{PREFETCH_HISTORY_SIZE, 1};

public:
  using champsim::modules::prefetcher::prefetcher;

  uint32_t prefetcher_cache_operate(champsim::address addr, champsim::address ip, uint8_t cache_hit, 
                                   bool useful_prefetch, access_type type, uint32_t metadata_in);
  uint32_t prefetcher_cache_fill(champsim::address addr, long set, long way, uint8_t prefetch, 
                                champsim::address evicted_addr, uint32_t metadata_in);
  void prefetcher_cycle_operate();
  
  void adjust_prefetch_degree();
};

#endif
