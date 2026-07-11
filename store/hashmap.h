#pragma once
#include <iostream>
#include <vector>
#include <utility>
#include <memory>
#include <array>
#include <atomic>
#include <optional>
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

    // CHANGED: Returns the existing value if found, and a bool indicating if a new node was inserted.
    std::pair<std::optional<V>, bool> insert(const K& key, const V& value);
    
    std::optional<V> find(const K& key) const;
    size_t erase(const K& key);
    size_t size() const;
};