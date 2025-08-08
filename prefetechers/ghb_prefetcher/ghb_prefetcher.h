#ifndef GHB_PREFETCHER_H
#define GHB_PREFETCHER_H

#include <cstdint>
#include <vector>
#include <optional>

#include "address.h"
#include "champsim.h"
#include "modules.h"
#include "msl/lru_table.h"

struct ghb_prefetcher : public champsim::modules::prefetcher {
  // Constants
  constexpr static std::size_t IT_SIZE = 256;       // Index Table size
  constexpr static std::size_t GHB_SIZE = 256;      // Global History Buffer size
  constexpr static int PREFETCH_DEGREE = 4;         // Max prefetches to issue
  constexpr static int DELTA_THRESHOLD = 2;         // Min required deltas for pattern
  
  // Index table entry - maps IP to GHB entry
  struct it_entry {
    champsim::address ip{};  // Instruction pointer
    int ghb_ptr = -1;        // Pointer to GHB entry
    
    // Indexing function required by ChampSim tables
    auto index() const {
      using namespace champsim::data::data_literals;
      return ip.slice_upper<2_b>();
    }
    
    // Tag function required by ChampSim tables
    auto tag() const {
      using namespace champsim::data::data_literals;
      return ip.slice_upper<2_b>();
    }
  };
  
  // Global History Buffer entry
  struct ghb_entry {
    champsim::block_number addr{};  // Block address
    int prev_ptr = -1;              // Pointer to previous entry with same IP
  };
  
  // LRU table for the index table
  champsim::msl::lru_table<it_entry> index_table{IT_SIZE, 1};
  
  // Global History Buffer - circular buffer of memory accesses
  std::vector<ghb_entry> ghb{GHB_SIZE};
  int ghb_head = 0;  // Next position to insert in GHB
  
  // Helper functions
  bool detect_pattern(int ghb_idx, std::vector<champsim::address::difference_type>& deltas);
  void issue_prefetches(champsim::block_number addr, const std::vector<champsim::address::difference_type>& deltas);

public:
  // Inherit constructor from base class
  using champsim::modules::prefetcher::prefetcher;

  // Required interface functions
  uint32_t prefetcher_cache_operate(champsim::address addr, champsim::address ip, uint8_t cache_hit, 
                                  bool useful_prefetch, access_type type, uint32_t metadata_in);
  uint32_t prefetcher_cache_fill(champsim::address addr, long set, long way, uint8_t prefetch, 
                               champsim::address evicted_addr, uint32_t metadata_in);
  void prefetcher_cycle_operate();
};

#endif
