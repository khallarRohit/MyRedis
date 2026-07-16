#include <string>

template<class K, class V>
HashNode<K,V>::HashNode(const K& key, const V& value)
: key(key), value(value), next(nullptr){};

template<class K, class V>
HashMap<K,V>::HashMap()
: load_factor_cap(0.75f), hash_size_index(0){
    element_count_cap = load_factor_cap * _HASH_SIZE_LIST[hash_size_index];
    size_t initial_size = _HASH_SIZE_LIST[hash_size_index];

    buckets.reserve(initial_size);
    for(size_t i = 0; i < initial_size; ++i) {
        buckets.push_back(std::make_unique<HashBucket<K,V>>());
    }
}

template<class K, class V>
void HashMap<K,V>::insert_or_assign(const K& key, const V& value) {
    bool requires_rehash = false;
    {
        std::shared_lock<std::shared_mutex> table_lock(table_mutex);
        size_t idx = get_bucket_index(key);
        std::unique_lock<std::shared_mutex> bucket_lock(buckets[idx]->mutex);

        HashNode<K,V>* curr = buckets[idx]->head;
        while(curr != nullptr) {
            if(curr->key == key) {
                curr->value = value; // Overwrite
                return; 
            }
            curr = curr->next;
        }

        // Not found, insert new
        HashNode<K,V>* new_node = new HashNode<K,V>(key, value);
        new_node->next = buckets[idx]->head;
        buckets[idx]->head = new_node;

        if (element_count.fetch_add(1, std::memory_order_relaxed) + 1 > element_count_cap) {
            requires_rehash = true;
        }
    }
    if (requires_rehash) rehash();
}

template<class K, class V>
HashMap<K,V>::~HashMap(){
    for(auto& bucket : buckets){
        HashNode<K,V>* curr = bucket->head;
        while(curr != nullptr){
            HashNode<K,V>* prev = curr;
            curr = curr->next;
            delete prev;
        }
    }
}

template<class K, class V>
size_t HashMap<K,V>::get_bucket_index(const K& key) const{
    const size_t current_bucket_count = _HASH_SIZE_LIST[hash_size_index];
    return std::hash<K>{}(key) % current_bucket_count;
}

template<class K, class V>
size_t HashMap<K,V>::size() const{
    return element_count.load(std::memory_order_relaxed);
}

template<class K, class V>
void HashMap<K,V>::rehash(){
    std::unique_lock<std::shared_mutex> table_lock(table_mutex);

    if(hash_size_index + 1 >= _HASH_SIZE_LIST.size() || 
       element_count.load(std::memory_order_relaxed) <= element_count_cap){
        return;
    }

    hash_size_index++;
    const size_t new_size = _HASH_SIZE_LIST[hash_size_index];
    element_count_cap = load_factor_cap * new_size;

    std::vector<std::unique_ptr<HashBucket<K,V>>> new_buckets;
    new_buckets.reserve(new_size);
    for(size_t i = 0; i < new_size; ++i) {
        new_buckets.push_back(std::make_unique<HashBucket<K,V>>());
    }

    for(auto& old_bucket : buckets) {
        HashNode<K,V>* curr = old_bucket->head;
        while(curr != nullptr) {
            size_t new_idx = std::hash<K>{}(curr->key) % new_size;
            HashNode<K,V>* next = curr->next;
            
            curr->next = new_buckets[new_idx]->head;
            new_buckets[new_idx]->head = curr;
            
            curr = next;
        }
        old_bucket->head = nullptr;
    }

    buckets = std::move(new_buckets);
} 

template<class K, class V>
std::pair<std::optional<V>, bool> HashMap<K,V>::insert(const K& key, const V& value){
    bool requires_rehash = false;

    {
        std::shared_lock<std::shared_mutex> table_lock(table_mutex);
        size_t idx = get_bucket_index(key);
        HashBucket<K,V>& bucket = *buckets[idx];

        {
            // Lock bucket for READ (allows other threads to search this bucket)
            std::shared_lock<std::shared_mutex> bucket_lock(bucket.mutex);

            HashNode<K,V>* curr = bucket.head;
            while(curr != nullptr) {
                if(curr->key == key) {
                    return {curr->value, false}; 
                }
                curr = curr->next;
            }
        }

        // --- SLOW PATH (Insert new node) ---
        {
            std::unique_lock<std::shared_mutex> bucket_lock(bucket.mutex);

            // Double check in case another thread inserted it while we upgraded locks
            HashNode<K,V>* curr = bucket.head;
            while(curr != nullptr) {
                if(curr->key == key) {
                    return {curr->value, false}; 
                }
                curr = curr->next;
            }

            HashNode<K,V>* new_node = new HashNode<K,V>(key, value);
            new_node->next = bucket.head;
            bucket.head = new_node;

            size_t current_count = element_count.fetch_add(1, std::memory_order_relaxed) + 1;
            if (current_count > element_count_cap) {
                requires_rehash = true;
            }
        }
    }

    if (requires_rehash) {
        rehash();
    }
    
    // We inserted a new node, so we return true
    return {std::nullopt, true};
}

template<class K, class V>
std::optional<V> HashMap<K,V>::find(const K& key) const{
    std::shared_lock<std::shared_mutex> table_lock(table_mutex);
    const size_t idx = get_bucket_index(key);
    
    std::shared_lock<std::shared_mutex> bucket_lock(buckets[idx]->mutex);

    HashNode<K,V>* head = buckets[idx]->head;
    while(head != nullptr){
        if(head->key == key){
            return head->value;
        }
        head = head->next;
    }

    return std::nullopt;
}

template<class K, class V>
size_t HashMap<K,V>::erase(const K& key){ 
    std::shared_lock<std::shared_mutex> table_lock(table_mutex);
    size_t idx = get_bucket_index(key);
    
    std::unique_lock<std::shared_mutex> bucket_lock(buckets[idx]->mutex);

    HashNode<K,V>* head = buckets[idx]->head;
    HashNode<K,V>* prev = nullptr;

    while(head != nullptr){
        if(head->key == key){
            break;
        }
        prev = head;
        head = head->next;
    }

    if(head == nullptr){
        return 0;
    }

    if(prev == nullptr){
        buckets[idx]->head = head->next;
    }else{
        prev->next = head->next;
    }

    delete head;
    element_count.fetch_sub(1, std::memory_order_relaxed);
    return 1;
}

namespace MyRedis {
    class RedisHash;
    struct AtomicValue;
}

// template class HashMap<int, int>;
// template class HashMap<int, std::string>;
// template class HashMap<std::string, std::string>;
// template class HashMap<std::string, double>;
// template class HashMap<std::string, bool>;
// template class HashMap<std::string, std::shared_ptr<MyRedis::AtomicValue>>;
// template class HashMap<std::string, std::shared_ptr<MyRedis::RedisHash::AtomicValue>>;