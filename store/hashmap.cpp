
#include "hashMap.h"
#include <string>

template<class K, class V>
HashNode<K,V>::HashNode(const K& key, const V& value)
: key(key), value(value), next(nullptr){};


template<class K, class V>
size_t HashMap<K,V>::get_bucket_index(const K& key) const{
    const size_t current_bucket_count = _HASH_SIZE_LIST[hash_size_index];
    return std::hash<K>{}(key) % current_bucket_count;
}

template<class K, class V>
HashMap<K,V>::HashMap()
:element_count(0), load_factor_cap(0.75f), hash_size_index(0){
    element_count_cap = load_factor_cap * _HASH_SIZE_LIST[hash_size_index];
    buckets.resize(_HASH_SIZE_LIST[hash_size_index], nullptr);
}

template<class K, class V>
HashMap<K,V>::~HashMap(){
    for(HashNode<K,V>* head : buckets){
        HashNode<K,V>* curr = head;
        while(curr != nullptr){
            HashNode<K,V>* prev = curr;
            curr = curr->next;
            delete prev;
        }
    }
}

template<class K, class V>
size_t HashMap<K,V>::size() const{
    return element_count;
}

template<class K, class V>
void HashMap<K,V>::rehash(){
    if(hash_size_index+1 >= _HASH_SIZE_LIST.size()){
        return;
    }

    hash_size_index++;
    const size_t new_size = _HASH_SIZE_LIST[hash_size_index];
    element_count_cap = load_factor_cap * new_size;
    std::vector<HashNode<K,V>*> new_buckets(new_size, nullptr);

    for(HashNode<K,V>* head : buckets){
        HashNode<K,V>* curr = head;
        while(curr != nullptr){
            size_t new_idx = get_bucket_index(curr->key);
            HashNode<K,V>* next = curr->next;
            curr->next = new_buckets[new_idx];
            new_buckets[new_idx] = curr;
            curr = next;
        }
    }

    buckets = new_buckets;
}   

template<class K, class V>
void HashMap<K,V>::insert(const K& key, const V& value){
    if(element_count > element_count_cap){
        rehash();
    }

    const size_t idx = get_bucket_index(key);
    HashNode<K,V>* curr = buckets[idx];
    while(curr != nullptr){
        if(curr->key == key){
            curr->value = value;
            return;
        }
        curr = curr->next;
    }

    HashNode<K,V>* new_node = new HashNode<K,V>(key, value);
    new_node->next = buckets[idx];
    buckets[idx] = new_node;
    element_count++;
}

template<class K, class V>
V* HashMap<K,V>::find(const K& key) const{
    const size_t idx = get_bucket_index(key);
    
    HashNode<K,V>* head = buckets[idx];
    while(head != nullptr){
        if(head->key == key){
            return &head->value;
        }
        head = head->next;
    }
    return nullptr;
}

template<class K, class V>
size_t HashMap<K,V>::erase(const K& key){ // returns number of elements removed
    size_t idx = get_bucket_index(key);
    
    HashNode<K,V>* head = buckets[idx];
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
        buckets[idx] = head->next;
    }else{
        prev->next = head->next;
    }

    delete head;
    element_count--;
    return 1;
}


template class HashMap<int, int>;
template class HashMap<int, std::string>;
template class HashMap<std::string, std::string>;
template class HashMap<std::string, double>;
template class HashMap<std::string, bool>;