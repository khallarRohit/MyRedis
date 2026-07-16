#pragma once
#include <iostream>
#include <vector>
#include <utility>
#include <memory>
#include <array>
#include <atomic>
#include <optional>
#include <mutex>
#include <random>
#include <shared_mutex>

 
template<class K, class V> 
struct HashNode{
public:
    K key;
    V value;
    HashNode* next{nullptr};

    HashNode(const K& key, const V& value);
};

template<class K, class V>
struct HashBucket {
    HashNode<K, V>* head = nullptr;
    mutable std::shared_mutex mutex; 
};

template<class K, class V>
class HashMap{
public:
    struct SampleResult {
        size_t sampled = 0;
        size_t expired = 0;
    };

private:
    static constexpr std::array<size_t, 10> _HASH_SIZE_LIST{7, 17, 37, 79, 163, 331, 673, 1361, 2729, 5471};

    std::vector<std::unique_ptr<HashBucket<K,V>>> buckets; 
    mutable std::shared_mutex table_mutex;
    std::atomic<size_t> element_count{0};

    double load_factor_cap;
    size_t hash_size_index;
    size_t element_count_cap;

    size_t get_bucket_index(const K& key) const;
    void rehash();

public:
    HashMap();
    ~HashMap();

    HashMap(const HashMap&) = delete;
    HashMap& operator=(const HashMap&) = delete;
    HashMap(HashMap&&) noexcept = default;
    HashMap& operator=(HashMap&&) noexcept = default;
    void insert_or_assign(const K& key, const V& value);

    std::pair<std::optional<V>, bool> insert(const K& key, const V& value);
    
    std::optional<V> find(const K& key) const;
    size_t erase(const K& key);
    size_t size() const;

    template <typename Predicate>
    SampleResult sample_and_erase_if(size_t sample_size, Predicate pred);
};

template<class K, class V>
template <typename Predicate>
typename HashMap<K,V>::SampleResult HashMap<K,V>::sample_and_erase_if(size_t sample_size, Predicate pred) {
    SampleResult result;
    if (size() == 0) return result;

    thread_local std::mt19937 rng(std::random_device{}());

    std::shared_lock<std::shared_mutex> table_lock(table_mutex);
    
    size_t num_buckets = _HASH_SIZE_LIST[hash_size_index];
    std::uniform_int_distribution<size_t> dist(0, num_buckets - 1);

    size_t max_probes = sample_size * 5;
    size_t probes = 0;

    while (result.sampled < sample_size && probes < max_probes) {
        probes++;
        size_t idx = dist(rng);
        
        std::unique_lock<std::shared_mutex> bucket_lock(buckets[idx]->mutex);
        
        HashNode<K,V>* curr = buckets[idx]->head;
        HashNode<K,V>* prev = nullptr;

        while (curr != nullptr && result.sampled < sample_size) {
            result.sampled++;
            
            if (pred(curr->key, curr->value)) {
                HashNode<K,V>* to_delete = curr;
                
                if (prev == nullptr) {
                    buckets[idx]->head = curr->next;
                } else {
                    prev->next = curr->next;
                }
                curr = curr->next;
                delete to_delete;
                
                element_count.fetch_sub(1, std::memory_order_relaxed);
                result.expired++;
            } else {
                prev = curr;
                curr = curr->next;
            }
        }
    }
    return result;
}

#include "hashmap.cpp"