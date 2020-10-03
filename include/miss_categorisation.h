#include <stdio.h>
#include <cstdlib>
#include <assert.h>
#include <iostream>
#include <math.h>
#include <vector>
#include <bits/stdc++.h> 

using namespace std;

class blockNode{
public:
    int tag;
    blockNode* prev;
    blockNode* next;
};

void insertAtFront(blockNode* &head, int tag);
void removeNode(blockNode* node);

class CacheFullyAssociative{
private:    
    int size;
    int coldMisses, coldAndCapacityMisses, coldAndCapacityMissesBelady, misses;
    int count; int countBelady;
    blockNode* head; blockNode* tail;
    unordered_set <uint64_t> storeColdMisses;
    unordered_map<uint64_t, blockNode*> lruMap;
    unordered_set <uint64_t> beladyMap;
    unordered_map <uint64_t, vector<int>> beladyMissVector;

public:
    CacheFullyAssociative();
    void updatelruMap(uint64_t addr);
    void fillBeladyMissVector(uint64_t addr);
    void evictBlock();
    void beladyMissCalculation(vector<uint64_t> &misses);
    void printMisses();
};