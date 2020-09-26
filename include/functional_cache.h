#include <iostream>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
#include <iomanip>
#include <math.h>

typedef struct CacheBlock{
    uint64_t tag;
    bool valid;
    uint32_t lru;
} CacheBlock;

enum LOOKUP_TYPE{
    ONLY_LOOKUP,
    LOOKUP_AND_UPDATE
};

class Functional_Cache{
private:
    uint32_t m_blk_offset;
    uint32_t m_sets;
    uint32_t m_ways;
    CacheBlock **m_cache;

    uint64_t s_lookups;
    uint64_t s_hits;
    uint64_t s_fills;
    uint64_t s_invalidations;
    uint64_t s_evictions;

public:
    Functional_Cache(uint32_t sets, uint32_t ways);
    bool Lookup(uint64_t addr, uint32_t lookup_type);
    void Invalidate(uint64_t addr, uint32_t blk_way);
    void Fill(uint64_t addr, uint32_t blk_way);
    uint32_t Find_Victim(uint64_t addr);
    bool Is_Block_Valid(uint64_t addr, uint32_t blk_way);
    uint64_t Get_Block_Addr(uint64_t addr, uint32_t blk_way);
    uint32_t Get_Block_Way(uint64_t addr);
    void Print_Stats(uint32_t level);
};
