#include "miss_categorisation.h"

using namespace std;

/***** The following linked list functions are used to implement lru cache as a linked list with hashed map, insert at front is used to add the most recently used element at the head of list. Remove node is used to remove a node which is accessed just now to insert it at head or to remove the last node in case of eviction. *****/

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
    coldMisses = 0, coldAndCapacityMisses = 0, misses = 0, count = 0; countBelady = 0; coldAndCapacityMissesBelady = 0;
    size = 1 << 15;
    head = NULL; tail = NULL;
}

void CacheFullyAssociative::updatelruMap(uint64_t addr)
{
    misses++;
    uint64_t tag = (addr >> 6);
    if(lruMap.find(tag) == lruMap.end())
    {
        // If the entry doesn't exist in cache and there are empty blocks, insert this entry in hashmap and at the head of linked list
        if(count < size)
        {
            insertAtFront(head, tag);
            lruMap[tag] = head;
            count++;
            if(count == 1) { tail = head; }
        }
        // If the entry doesn't exist and cache is full, evict the tail of linked list from hashmap and insert new element at the head.
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
    // If the element exists in the cache, remove it from its position and insert it at head position in th linked list
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

void CacheFullyAssociative::fillBeladyMissVector(uint64_t addr)
{
    // Cold misses calculation
    // Whenever a block comes for the first time, insert it with the time in beladyMissVector hashmap which stores the vector of times when a particular address comes for implementing belady replacement policy
    if(beladyMissVector.find(addr>>6) == beladyMissVector.end())
    {
        beladyMissVector.insert(pair<uint64_t, vector<int>> (addr>>6, {misses}));
        coldMisses++;
    }
    // When a block comes again, just push the time in the vector
    else
    {
        beladyMissVector.at(addr>>6).push_back(misses);
    }
}
void CacheFullyAssociative::evictBlock()
{
    // in Belady replacement policy, the block in cache which is used the farthest in the future(or not used at all) is evicted
    int max = 0; uint64_t addr;
    for (const auto& address: beladyMap) 
    {
        if(beladyMissVector[address].size() == 0)
        {
            beladyMissVector.erase(address);
            beladyMap.erase(address);
            return;
        }
        else if(beladyMissVector[address].front() > max)
        {
            max = beladyMissVector[address].front();
            addr = address;
        }
    }
    beladyMap.erase(addr);
}

void CacheFullyAssociative::beladyMissCalculation(vector<uint64_t> &misses)
{   
    for (auto& missAddress : misses) 
    {
        if(beladyMap.find(missAddress >> 6) == beladyMap.end())
        {   
            // If entry is not present in cache and empty blocks are present, insert the element in hashmap and remove the time value from the vector
            if(countBelady < size)
            {
                beladyMap.insert(missAddress >> 6);
                beladyMissVector[missAddress >> 6].erase(beladyMissVector[missAddress>>6].begin());
                countBelady++;
            }
            // If entry is not present and cache is full, evict a block and insert the element in the cache
            else
            {
                evictBlock();
                beladyMap.insert(missAddress >> 6);
                beladyMissVector[missAddress >> 6].erase(beladyMissVector[missAddress>>6].begin());
            }
            coldAndCapacityMissesBelady++;
        }
        // If element is present in cache, just remove the time from miss vector
        else
        {
            beladyMissVector[missAddress >> 6].erase(beladyMissVector[missAddress>>6].begin());
        }
    }
} 

void CacheFullyAssociative::printMisses() {
    cout << "############################################" << endl;
    cout << "Miss Categorisation(Part 2)" << endl;

    cout << right << setw(30) << "LRU" << endl;
    cout << left;
    cout << setw(30) << "Cold Misses: " << coldMisses << endl;    
    cout << setw(30) << "Capacity Misses: " << (coldAndCapacityMisses-coldMisses) << endl;
    cout << setw(30) << "Conflict Misses: " << (misses - coldAndCapacityMisses) << endl;
    cout << setw(30) << "Belady:" << endl;
    cout << setw(30) << "Cold Misses: " << coldMisses << endl;
    cout << setw(30) << "Capacity Misses: " << (coldAndCapacityMissesBelady-coldMisses) << endl;
    cout << setw(30) << "Conflict Misses: " << (misses - coldAndCapacityMissesBelady) << endl;
    cout << "############################################" << endl;
}