#include "functional_cache.h"
#include "knobs.h"

using namespace std;

/**************************************************
 * 
 * Constructs a set-associative cache level
 * 
**************************************************/
Functional_Cache::Functional_Cache(uint32_t sets, uint32_t ways){
    m_blk_offset = BLK_OFFSET;
    m_sets = sets;
    m_ways = ways;

    m_cache = (CacheBlock **)calloc(sets, sizeof(CacheBlock *));
    assert (m_cache != NULL);
    for (int i = 0; i < m_sets; i++){
        m_cache[i] = (CacheBlock *)calloc(ways, sizeof(CacheBlock));
        assert (m_cache[i] != NULL);
    }

    s_lookups = 0;
    s_hits = 0;
    s_fills = 0;
    s_invalidations = 0;
    s_evictions = 0;
}

/**************************************************
 * 
 * Returns true if addr exists in cache and updates 
 * LRU if lookup_type == LOOKUP_AND_UPDATE
 * 
**************************************************/
bool Functional_Cache::Lookup(uint64_t addr, uint32_t lookup_type){
    uint32_t set = (addr >> m_blk_offset) % m_sets;
    uint64_t tag_bits = addr >> (m_blk_offset);
    bool hit = false;
    uint32_t blk_age = 0;
    uint32_t blk_way = 0;

    for (int way = 0; way < m_ways; way++){
        if (m_cache[set][way].valid && m_cache[set][way].tag == tag_bits){
            hit = true;
            blk_age = m_cache[set][way].lru;
            blk_way = way;
            break;
        }
    }

    if (lookup_type == LOOKUP_AND_UPDATE){
        s_lookups++;
        if (hit){
            s_hits++;
            for (int way = 0; way < m_ways; way++){
                if (blk_age == 0){
                    break;
                }
                else if (m_cache[set][way].valid && m_cache[set][way].lru < blk_age){
                    m_cache[set][way].lru++;
                }
            }
            m_cache[set][blk_way].lru = 0;
        }
    }

    return hit;
}

/**************************************************
 * 
 * Updates LRU and fills block at (set, blk_way)
 * 
**************************************************/
void Functional_Cache::Fill(uint64_t addr, uint32_t blk_way){
    uint32_t set = (addr >> m_blk_offset) % m_sets;
    uint64_t tag_bits = addr >> (m_blk_offset);
    assert(m_cache[set][blk_way].valid == false);

    for (int way = 0; way < m_ways; way++){
        if (m_cache[set][way].valid){
            m_cache[set][way].lru++;
        }
    }

    m_cache[set][blk_way].valid = true;
    m_cache[set][blk_way].tag = tag_bits;
    m_cache[set][blk_way].lru = 0;
    s_fills++;

    return;
}

/**************************************************
 * 
 * Updates LRU and invalidates block at (set, blk_way)
 * 
**************************************************/
void Functional_Cache::Invalidate(uint64_t addr, uint32_t blk_way){
    uint32_t set = (addr >> m_blk_offset) % m_sets;
    if (m_cache[set][blk_way].valid == false){
        return;
    }

    uint32_t blk_age = m_cache[set][blk_way].lru;
    for (int way = 0; way < m_ways; way++){
        if (blk_age == m_ways - 1){
            break;
        }
        else if (m_cache[set][way].valid && m_cache[set][way].lru > blk_age){
            m_cache[set][way].lru--;
        }
    }

    m_cache[set][blk_way].valid = false;
    m_cache[set][blk_way].tag = 0;
    m_cache[set][blk_way].lru = 0;
    s_invalidations++;

    return;
}

/**************************************************
 * 
 * Returns victim way in set which is either any one
 * invalid way or LRU way if none are invalid
 * 
**************************************************/
uint32_t Functional_Cache::Find_Victim(uint64_t addr){
    uint32_t set = (addr >> m_blk_offset) % m_sets;
    uint32_t victim_way = m_ways;

    for (int way = 0; way < m_ways; way++){
        if (m_cache[set][way].valid == false){
            victim_way = way;
            break;
        }
    }

    if (victim_way == m_ways){
        s_evictions++;
        for (int way = 0; way < m_ways; way++){
            if (m_cache[set][way].lru == m_ways - 1){
                victim_way = way;
                break;
            }
        }
    }

    assert(victim_way < m_ways);

    return victim_way;
}

/**************************************************
 * 
 * Returns true if block at (set, blk_way) is valid
 * 
**************************************************/
bool Functional_Cache::Is_Block_Valid(uint64_t addr, uint32_t blk_way){
    uint32_t set = (addr >> m_blk_offset) % m_sets;
    
    return m_cache[set][blk_way].valid;
}

/**************************************************
 * 
 * Returns block aligned address
 * the block at (set, blk_way) must be valid
 * 
**************************************************/
uint64_t Functional_Cache::Get_Block_Addr(uint64_t addr, uint32_t blk_way){
    uint32_t set = (addr >> m_blk_offset) % m_sets;
    assert(m_cache[set][blk_way].valid == true);

    return (m_cache[set][blk_way].tag << m_blk_offset);
}

/**************************************************
 * 
 * Returns the way of block corresponding to addr
 * the block must exist in the set
 * 
**************************************************/
uint32_t Functional_Cache::Get_Block_Way(uint64_t addr){
    uint32_t set = (addr >> m_blk_offset) % m_sets;
    uint64_t tag_bits = addr >> (m_blk_offset);
    uint32_t blk_way = m_ways;

    for (int way = 0; way < m_ways; way++){
        if (m_cache[set][way].valid && m_cache[set][way].tag == tag_bits){
            blk_way = way;
            break;
        }
    }

    assert (blk_way < m_ways);

    return blk_way;
}

void Functional_Cache::Print_Stats(uint32_t level){
    cout << right << setw(15) << "L" << level << " Stats" << endl;
    cout << left;
    cout << setw(30) << "Lookups: " << s_lookups << endl;
    cout << setw(30) << "Hits: " << s_hits << endl;
    cout << setw(30) << "Misses: " << s_lookups - s_hits << endl;
    cout << setw(30) << "Fills: " << s_fills << endl;
    cout << setw(30) << "Evictions: " << s_evictions << endl;
    cout << setw(30) << "Invalidations: " << s_invalidations << endl;
}