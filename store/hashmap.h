#pragma once
#include <iostream>
#include <vector>
#include <utility>
#include <array>
 
template<class K, class V> 
struct HashNode{
public:
    K key;
    V value;
    HashNode* next;

    HashNode(const K& key, const V& value);
};


template<class K, class V>
class HashMap{
private:
    static constexpr std::array<size_t, 10> _HASH_SIZE_LIST{7, 17, 37, 79, 163, 331, 673, 1361, 2729, 5471};
    std::vector<HashNode<K,V>*> buckets; // hash table
    size_t element_count;
    double load_factor_cap;
    size_t hash_size_index;
    size_t element_count_cap;
    size_t get_bucket_index(const K& key) const;
    void rehash();

public:
    HashMap();
    ~HashMap();
    void insert(const K& key, const V& value);
    V* find(const K& key) const;
    size_t erase(const K& key);
    
};