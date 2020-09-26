#include "cache_hierarchy.h"
#include "miss_categorisation.h"
#include "knobs.h"

using namespace std;

CacheFullyAssociative cacheFullyAssociative;
vector<uint64_t> misses;

Cache_Hierarchy::Cache_Hierarchy(uint32_t inclusion_policy){
    m_incl_policy = inclusion_policy;
    m_L2 = new Functional_Cache(L2_SETS, L2_WAYS);
    m_L3 = new Functional_Cache(L3_SETS, L3_WAYS);
    s_back_inval_lookups = 0;
    s_back_invals = 0;
    s_simulated_addrs = 0;
}

void Cache_Hierarchy::Simulate(uint64_t addr){
    s_simulated_addrs++;
    switch (m_incl_policy)
    {
    case INCLUSIVE:
        Inclusive_Policy_Simulate(addr);
        break;
    case EXCLUSIVE:
        Exclusive_Policy_Simulate(addr);
        break;
    case NINE:
        NINE_Policy_Simulate(addr);
        break;
    default:
        exit(1);
    }
}

void Cache_Hierarchy::Inclusive_Policy_Simulate(uint64_t addr){
    if (m_L2->Lookup(addr, LOOKUP_AND_UPDATE)){
        /*********************************************************
         * L2 hit:
         * no action required
        *********************************************************/
        s_hits++;
    }
    else if (m_L3->Lookup(addr, LOOKUP_AND_UPDATE)){
        /*********************************************************
         * L2 miss, L3 hit:
         * Find victim at L2, invalidate it, fill block in L2
        *********************************************************/
        s_hits++;
        uint32_t L2_vic_way = m_L2->Find_Victim(addr);
        m_L2->Invalidate(addr, L2_vic_way);
        m_L2->Fill(addr, L2_vic_way);
    }
    else{
        /*********************************************************
         * L2 miss, L3 miss:
         * Find victim at L3, invalidate it, fill block in L3
         * if L3 victim was valid, lookup L3 victim in L2
         * if it is present in L2, invalidate it (back-invalidation)
         * Find victim at L2, invalidate it, fill block in L2
        *********************************************************/
        uint32_t L3_vic_way = m_L3->Find_Victim(addr);
        bool is_L3_vic_valid = false;
        uint64_t back_inval_addr;
        if (m_L3->Is_Block_Valid(addr, L3_vic_way)){
            is_L3_vic_valid = true;
            back_inval_addr = m_L3->Get_Block_Addr(addr, L3_vic_way);
            m_L3->Invalidate(addr, L3_vic_way);
        }
        m_L3->Fill(addr, L3_vic_way);

        if(is_L3_vic_valid){
            s_back_inval_lookups++;
            if (m_L2->Lookup(back_inval_addr, ONLY_LOOKUP)){
                s_back_invals++;
                uint32_t L2_back_inval_way = m_L2->Get_Block_Way(back_inval_addr);
                m_L2->Invalidate(back_inval_addr, L2_back_inval_way);
            }
        }

        uint32_t L2_vic_way = m_L2->Find_Victim(addr);
        m_L2->Invalidate(addr, L2_vic_way);
        m_L2->Fill(addr, L2_vic_way);

        // Miss categorisation
        cacheFullyAssociative.updatelruMap(addr);
        cacheFullyAssociative.fillBeladyMissVector(addr);
        misses.push_back(addr);
    }

    return;
}

void Cache_Hierarchy::Exclusive_Policy_Simulate(uint64_t addr){
    if (m_L2->Lookup(addr, LOOKUP_AND_UPDATE)){
        /*********************************************************
         * L2 hit:
         * no action required
        *********************************************************/
        s_hits++;
    }
    else if (m_L3->Lookup(addr, LOOKUP_AND_UPDATE)){
        /*********************************************************
         * L2 miss, L3 hit:
         * Invalidate block at L3
         * Find victim at L2, invalidate it, fill block in L2
         * If L2 victim was valid, then
         * find victim in L3, invalidate it, fill L2 victim in L3
        *********************************************************/
        s_hits++;
        uint32_t L3_blk_way = m_L3->Get_Block_Way(addr);
        m_L3->Invalidate(addr, L3_blk_way);
        uint32_t L2_vic_way = m_L2->Find_Victim(addr);
        bool is_L2_victim_valid = false;
        uint64_t evicted_addr;
        if (m_L2->Is_Block_Valid(addr, L2_vic_way)){
            is_L2_victim_valid = true;
            evicted_addr = m_L2->Get_Block_Addr(addr, L2_vic_way);
            m_L2->Invalidate(addr, L2_vic_way);
        }
        m_L2->Fill(addr, L2_vic_way);

        if (is_L2_victim_valid){
            assert(m_L3->Lookup(evicted_addr, ONLY_LOOKUP) == false);
            uint32_t L3_vic_way = m_L3->Find_Victim(evicted_addr);
            m_L3->Invalidate(evicted_addr, L3_vic_way);
            m_L3->Fill(evicted_addr, L3_vic_way);
        }
    }
    else{
        /*********************************************************
         * L2 miss, L3 miss:
         * Find victim at L2, invalidate it, fill block in L2
         * If L2 victim was valid, then
         * find victim in L3, invalidate it, fill L2 victim in L3
        *********************************************************/
        uint32_t L2_vic_way = m_L2->Find_Victim(addr);
        bool is_L2_victim_valid = false;
        uint64_t evicted_addr;
        if (m_L2->Is_Block_Valid(addr, L2_vic_way)){
            is_L2_victim_valid = true;
            evicted_addr = m_L2->Get_Block_Addr(addr, L2_vic_way);
            m_L2->Invalidate(addr, L2_vic_way);   
        }
        m_L2->Fill(addr, L2_vic_way);

        if (is_L2_victim_valid){
            assert(m_L3->Lookup(evicted_addr, ONLY_LOOKUP) == false);
            uint32_t L3_vic_way = m_L3->Find_Victim(evicted_addr);
            m_L3->Invalidate(evicted_addr, L3_vic_way);
            m_L3->Fill(evicted_addr, L3_vic_way);
        }
    }   

    return;
}

void Cache_Hierarchy::NINE_Policy_Simulate(uint64_t addr){
    if (m_L2->Lookup(addr, LOOKUP_AND_UPDATE)){
        /*********************************************************
         * L2 hit:
         * no action required
        *********************************************************/
        s_hits++;
    }
    else if (m_L3->Lookup(addr, LOOKUP_AND_UPDATE)){
        /*********************************************************
         * L2 miss, L3 hit:
         * Find victim at L2, invalidate it, fill block in L2
        *********************************************************/
        s_hits++;
        uint32_t L2_vic_way = m_L2->Find_Victim(addr);
        m_L2->Invalidate(addr, L2_vic_way);
        m_L2->Fill(addr, L2_vic_way);
    }
    else{
        /*********************************************************
         * L2 miss, L3 miss:
         * Find victim at L3, invalidate it, fill block in L3
         * Find victim at L2, invalidate it, fill block in L2
        *********************************************************/
        uint32_t L3_vic_way = m_L3->Find_Victim(addr);
        m_L3->Invalidate(addr, L3_vic_way);
        m_L3->Fill(addr, L3_vic_way);

        uint32_t L2_vic_way = m_L2->Find_Victim(addr);
        m_L2->Invalidate(addr, L2_vic_way);
        m_L2->Fill(addr, L2_vic_way);
    }

    return;
}

void Cache_Hierarchy::Print_Stats(){
    cout << left << setw(30) << "Simulation Result for ";
    switch (m_incl_policy){
        case INCLUSIVE:
             cout << "Inclusive ";
             break;
        case EXCLUSIVE:
            cout << "Exclusive ";
            break;
        case NINE:
            cout << "NINE ";
            break;
    }
    cout << "Cache Hierarchy " << endl;
    m_L2->Print_Stats(2);
    m_L3->Print_Stats(3);
    cout << right << setw(30) << "Cache Hierarchy Stats" << endl;
    cout << left;
    cout << setw(30) << "Simulated addresses: " << s_simulated_addrs << endl;
    cout << setw(30) << "Cache hits: " << s_hits << endl;
    if (m_incl_policy == INCLUSIVE){
        cout << setw(30) << "Back-invalidation lookups: " << s_back_inval_lookups << endl;
        cout << setw(30) << "Back-invalidations: " << s_back_invals << endl;
    }
    cout << "--------------------------------------------------" << endl;

    if(m_incl_policy == INCLUSIVE) {
        // Print the miss categories
        cacheFullyAssociative.beladyMissCalculation(misses);
        cacheFullyAssociative.printMisses();
    }

}