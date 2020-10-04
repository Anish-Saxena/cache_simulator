#include "miss_categorisation.h"
#include "knobs.h"

using namespace std;

#define PBSTR "||||||||||||||||||||||||||||||||||||||||||||||||||"
#define PBWIDTH 50

/*********************************************************
 * The following linked list functions are used to implement 
 * lru cache as a linked list with hashed map, insert at 
 * front is used to add the most recently used element at 
 * the head of list. Remove node is used to remove a node 
 * which is accessed just now to insert it at head or to 
 * remove the last node in case of eviction. 
 *********************************************************/

void insertAtFront(blockNode* &head, int tag)
{
    blockNode* newNode = new blockNode();
    newNode -> tag = tag;
    newNode -> prev = NULL;
    newNode -> next = head;
    if(head != NULL)
    {
        head -> prev = newNode;
    }
    head = newNode;
}

void removeNode(blockNode* node)
{   
    if(node -> prev)
        node -> prev -> next = node -> next;
    if(node -> next)
        node -> next -> prev = node -> prev;
    free(node);
}

CacheFullyAssociative::CacheFullyAssociative(){
    coldMisses = 0, coldAndCapacityMisses = 0, num_misses = 0, num_accesses = 0, count = 0; 
    countBelady = 0; coldAndCapacityMissesBelady = 0;
    size = L3_SETS * L3_WAYS;
    head = NULL; tail = NULL;
}

void CacheFullyAssociative::updatelruMap(uint64_t addr, bool isMiss)
{
    num_accesses++;
    if (isMiss){
        num_misses++;
    }
    uint64_t tag = (addr >> BLK_OFFSET);
    if(lruMap.find(tag) == lruMap.end())
    {
        // If the entry doesn't exist in cache and there are empty blocks, 
        // insert this entry in hashmap and at the head of linked list
        if(count < size)
        {
            insertAtFront(head, tag);
            lruMap[tag] = head;
            count++;
            if(count == 1) { tail = head; }
        }
        // If the entry doesn't exist and cache is full, evict the tail of 
        // linked list from hashmap and insert new element at the head.
        else
        {   
            insertAtFront(head, tag);
            lruMap[tag] = head;
            lruMap.erase(tail -> tag);
            tail = tail -> prev;
            removeNode(tail -> next);
        }
        coldAndCapacityMisses++;
    }
    // If the element exists in the cache, remove it from its position 
    // and insert it at head position in th linked list
    else
    {   
        if(lruMap[tag] != head)
        {
            if(lruMap[tag] == tail) { tail = tail -> prev; }
            removeNode(lruMap[tag]);
            insertAtFront(head, tag);
            lruMap[tag] = head;
        }
    }
}

void CacheFullyAssociative::fillBeladyAccessVector(uint64_t addr)
{
    // Cold misses calculation
    // Whenever a block comes for the first time, insert it with the time 
    // in beladyAccessVector hashmap which stores the vector of times when a 
    // particular address comes for implementing belady replacement policy
    uint64_t tag = (addr >> BLK_OFFSET);
    if(beladyAccessVector.find(tag) == beladyAccessVector.end())
    {
        beladyAccessVector.insert(pair<uint64_t, vector<int>> (tag, {num_accesses}));
        coldMisses++;
    }
    // When a block comes again, just push the time in the vector
    else
    {
        beladyAccessVector.at(tag).push_back(num_accesses);
    }
}

void CacheFullyAssociative::evictBlock()
{
    // in Belady replacement policy, the block in cache which is used 
    // the farthest in the future (or not used at all) is evicted
    int max = 0; uint64_t addr;
    for (const auto& address: beladyMap) 
    {
        if(beladyAccessVector[address].size() == 0)
        {
            beladyAccessVector.erase(address);
            beladyMap.erase(address);
            return;
        }
        else if(beladyAccessVector[address].front() > max)
        {
            max = beladyAccessVector[address].front();
            addr = address;
        }
    }
    beladyMap.erase(addr);
}

void printProgress(double percentage) {
    int val = (int) (percentage * 100);
    int lpad = (int) (percentage * PBWIDTH);
    int rpad = PBWIDTH - lpad;
    printf("\r%3d%% [%.*s%*s]", val, lpad, PBSTR, rpad, "");
    fflush(stdout);
}

void CacheFullyAssociative::beladyMissCalculation(vector<uint64_t> &accesses)
{   
    cout << "Classifying misses in fully-associative Belady, progress: " << endl;
    int acc_count = 0;
    int progress_acc = num_accesses - num_accesses%50;
    for (auto& accessed_addr : accesses) 
    {
        acc_count++;
        if (((acc_count*50)%progress_acc) == 0){
            printProgress((double)(acc_count)/(double)(progress_acc));
        }
        uint64_t tag = (accessed_addr >> BLK_OFFSET);
        if(beladyMap.find(tag) == beladyMap.end())
        {   
            // If entry is not present in cache and empty blocks are present, 
            // insert the element in hashmap and remove the time value from the vector
            if(countBelady < size)
            {
                beladyMap.insert(tag);
                beladyAccessVector[tag].erase(beladyAccessVector[tag].begin());
                countBelady++;
            }
            // If entry is not present and cache is full, evict a block 
            // and insert the element in the cache
            else
            {
                evictBlock();
                beladyMap.insert(tag);
                beladyAccessVector[tag].erase(beladyAccessVector[tag].begin());
            }
            coldAndCapacityMissesBelady++;
        }
        // If element is present in cache, just remove the time from miss vector
        else
        {
            beladyAccessVector[tag].erase(beladyAccessVector[tag].begin());
        }
    }
    cout << endl;
} 

void CacheFullyAssociative::printMisses() {
    cout << "############################################" << endl;
    cout << right << setw(30) << "Miss Classification (Part 2)" << endl;
    cout << "LRU" << endl;
    cout << left;
    cout << setw(30) << "Misses: " << coldAndCapacityMisses << endl;
    cout << setw(30) << "Cold Misses: " << coldMisses << endl;    
    cout << setw(30) << "Capacity Misses: " << (coldAndCapacityMisses-coldMisses) << endl;
    cout << setw(30) << "Conflict Misses: " << (num_misses - coldAndCapacityMisses) << endl;
    cout << "Belady" << endl;
    cout << setw(30) << "Misses: " << coldAndCapacityMissesBelady << endl;
    cout << setw(30) << "Cold Misses: " << coldMisses << endl;
    cout << setw(30) << "Capacity Misses: " << (coldAndCapacityMissesBelady-coldMisses) << endl;
    cout << setw(30) << "Conflict Misses: " << (num_misses - coldAndCapacityMissesBelady) << endl;
    cout << "############################################" << endl;
}