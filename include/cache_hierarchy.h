#include "functional_cache.h"

enum INCLUSION_POLICY{
    INCLUSIVE,
    EXCLUSIVE,
    NINE
};

class Cache_Hierarchy{
private:
    uint32_t m_incl_policy;

    uint64_t s_simulated_addrs;
    uint64_t s_hits;
    uint64_t s_back_inval_lookups;
    uint64_t s_back_invals;

    Functional_Cache *m_L2;
    Functional_Cache *m_L3;
    void Inclusive_Policy_Simulate(uint64_t addr);
    void Exclusive_Policy_Simulate(uint64_t addr);
    void NINE_Policy_Simulate(uint64_t addr);

public:
    Cache_Hierarchy(uint32_t inclusion_policy);
    void Simulate(uint64_t addr);
    void Print_Stats();
};
