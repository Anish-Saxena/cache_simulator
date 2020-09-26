#include "miss_categorisation.h"

using namespace std;

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

void printList(blockNode* node)
{
    blockNode* temp = node;
    while(temp != NULL)
    {
        cout << "temp data is " << temp -> tag << "\n";
        temp = temp -> next;
    }
}

CacheFullyAssociative::CacheFullyAssociative(){
    coldMisses = 0, capacityMisses = 0, misses = 0, count = 0; countBelady = 0; capacityMissesBelady = 0;
    size = pow(2,15);
    head = NULL; tail = NULL;
}
// void updateColdMisses(uint64_t addr)
// {
//     if(storeColdMisses.find(addr>>6) == storeColdMisses.end())
//     {
//         storeColdMisses.insert(addr>>6);
//         coldMisses++;
//     }
// }
void CacheFullyAssociative::updatelruMap(uint64_t addr)
{
    misses++;
    uint64_t tag = (addr >> 6);
    if(lruMap.find(tag) == lruMap.end())
    {
        if(count < size)
        {
            insertAtFront(head, tag);
            lruMap[tag] = head;
            count++;
            if(count == 1) { tail = head; }
        }
        else
        {   
            insertAtFront(head, tag);
            lruMap[tag] = head;
            lruMap.erase(tail -> tag);
            tail = tail -> prev;
            removeNode(tail -> next);
            capacityMisses++;
        }
    }
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
    if(beladyMissVector.find(addr>>6) == beladyMissVector.end())
    {
        beladyMissVector.insert(pair<uint64_t, vector<int>> (addr>>6, {misses}));
        coldMisses++;
        // beladyMissVector[addr>>6];
        // beladyMissVector[addr>>6].push_back(misses);
    }
    else
    {
        // cout << beladyMissVector.at(addr>>6) << "\n";
        beladyMissVector.at(addr>>6).push_back(misses);
    }
}
void CacheFullyAssociative::evictBlock()
{
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
    // beladyMissVector[addr].erase(beladyMissVector[addr].begin());
    beladyMap.erase(addr);
}

void CacheFullyAssociative::beladyMissCalculation(vector<uint64_t> &misses)
{   
    cout << "Belady miss calculation started\n";
    int c = 0;
    for (auto& missAddress : misses) 
    {
        c++;
        cout << "Processing Miss: " << c << "\n";
        if(beladyMap.find(missAddress >> 6) == beladyMap.end())
        {
            if(countBelady < size)
            {
                beladyMap.insert(missAddress >> 6);
                beladyMissVector[missAddress >> 6].erase(beladyMissVector[missAddress>>6].begin());
                countBelady++;
            }
            else
            {
                evictBlock();
                beladyMap.insert(missAddress >> 6);
                beladyMissVector[missAddress >> 6].erase(beladyMissVector[missAddress>>6].begin());
                capacityMissesBelady++;
            }
        }
        else
        {
            beladyMissVector[missAddress >> 6].erase(beladyMissVector[missAddress>>6].begin());
        }
    }
} 

void CacheFullyAssociative::printMisses() {
    cout << "############################################" << endl;
    cout << "Miss Categorisation(Part 2)" << endl;
    cout << "LRU:" << endl;
    cout << "Cold Misses: " << coldMisses << endl;
    cout << "Capacity Misses: " << capacityMisses << endl;
    cout << "Conflict Misses: " << (misses - capacityMisses - coldMisses) << endl;
    cout << "Belady:" << endl;
    cout << "Cold Misses: " << coldMisses << endl;
    cout << "Capacity Misses: " << capacityMissesBelady << endl;
    cout << "Conflict Misses: " << (misses - capacityMissesBelady - coldMisses) << endl;
    cout << "############################################" << endl;
}