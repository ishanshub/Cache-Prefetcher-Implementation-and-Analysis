#include "ghb_prefetcher.h"
#include "cache.h"

uint32_t ghb_prefetcher::prefetcher_cache_operate(champsim::address addr, champsim::address ip, uint8_t cache_hit, 
                                               bool useful_prefetch, access_type type, uint32_t metadata_in)
{
  if (type != access_type::LOAD && type != access_type::RFO) {
    return metadata_in;
  }
  
  champsim::block_number cl_addr{addr};
  
  auto found = index_table.check_hit({ip, -1});
  int prev_ghb_ptr = -1;
  
  if (found.has_value()) {
    prev_ghb_ptr = found->ghb_ptr;
  }

  ghb[ghb_head] = {cl_addr, prev_ghb_ptr};

  index_table.fill({ip, ghb_head});
  
  // detect and issue
  std::vector<champsim::address::difference_type> deltas;
  if (detect_pattern(ghb_head, deltas)) {
    
    issue_prefetches(cl_addr, deltas);
  }
  
  ghb_head = (ghb_head + 1) % GHB_SIZE;
  
  return metadata_in;
}

bool ghb_prefetcher::detect_pattern(int ghb_idx, std::vector<champsim::address::difference_type>& deltas)
{
  // need atleast 3 entries
  int curr_ptr = ghb_idx;
  
  // Check if a previous access
  if (ghb[curr_ptr].prev_ptr == -1) {
    return false;
  }
  
  int prev_ptr = ghb[curr_ptr].prev_ptr;
  
  // check for a second previous access
  if (ghb[prev_ptr].prev_ptr == -1) {
    return false;
  }
  
  // collect deltas
  std::vector<champsim::address::difference_type> history_deltas;
  
  int ptr = curr_ptr;
  int prev = ghb[ptr].prev_ptr;
  
  while (prev != -1 && history_deltas.size() < 2 * PREFETCH_DEGREE) {
    auto delta = champsim::offset(ghb[prev].addr, ghb[ptr].addr);
    history_deltas.push_back(delta);
    
    ptr = prev;
    prev = ghb[ptr].prev_ptr;
  }
  
  // if less than minimum threshold  
  if (history_deltas.size() < DELTA_THRESHOLD) {
    return false;
  }
  
  deltas.push_back(history_deltas[0]);
  
  // looking for pattern
  for (int pattern_len = 2; pattern_len <= 3; pattern_len++) {
    if (history_deltas.size() < 2 * pattern_len) {
      continue;
    }
    
    bool pattern_found = true;
    for (int i = 0; i < pattern_len; i++) {
      if (history_deltas[i] != history_deltas[i + pattern_len]) {
        pattern_found = false;
        break;
      }
    }
    
    if (pattern_found) {
      deltas.clear();
      for (int i = 0; i < pattern_len; i++) {
        deltas.push_back(history_deltas[i]);
      }
      return true;
    }
  }
  
  // no complex patter, using the single delta. if repeated
  if (history_deltas.size() >= 2 && history_deltas[0] == history_deltas[1]) {
    return true;
  }
  
  deltas.clear();
  return false;
}

void ghb_prefetcher::issue_prefetches(champsim::block_number addr, 
                                    const std::vector<champsim::address::difference_type>& deltas)
{
  champsim::block_number pf_addr = addr;
  
  // issue prefetch according to the pattern
  for (int degree = 0; degree < PREFETCH_DEGREE; degree++) {
    for (const auto& delta : deltas) {
      pf_addr = pf_addr + delta;
      
      // Check if prefetch crosses page boundary
      if (intern_->virtual_prefetch || champsim::page_number{pf_addr} == champsim::page_number{addr}) {

        bool mshr_under_light_load = intern_->get_mshr_occupancy_ratio() < 0.5;
        
        // issue prefetch
        prefetch_line(champsim::address{pf_addr}, mshr_under_light_load, 0);
      }
    }
  }
}

uint32_t ghb_prefetcher::prefetcher_cache_fill(champsim::address addr, long set, long way, uint8_t prefetch,
                                           champsim::address evicted_addr, uint32_t metadata_in)
{
  return metadata_in;
}

void ghb_prefetcher::prefetcher_cycle_operate()
{
  
}
