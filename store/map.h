#pragma once
#include<utility>
#include <stdexcept>

enum Color {
    RED,
    BLACK,
};

template<class K,class V>
struct Node {
    K key;
    V value;
    Color color;
    Node<K,V>* left;
    Node<K,V>* right;
    Node<K,V>* parent;

    Node(K key, V value);
    Node();
};

template<class K,class V>
class Map {
private: 
    size_t _size = 0;
    Node<K,V>* root;
    Node<K,V>* null; // sentinal node
    void rightRotate(Node<K,V>* node);
    void leftRotate(Node<K,V>* node);
    void balanceInsert(Node<K,V>* node);
    Node<K,V>* successor(Node<K,V>* node);
    Node<K,V>* ReplaceNode(Node<K,V>* node);
    void redBlackTransplant(Node<K,V>* u,Node<K,V>* v);
    Node<K,V>* getMinimumNode(Node<K,V>* node);
    Node<K,V>* getMaximumNode(Node<K,V>* node);
    Node<K,V>* predecessor(Node<K,V>* node);
    void balanceDelete(Node<K,V>* node);
    void destroyTree(Node<K,V>* node);
    void copyTree(Node<K,V>*& thisNode, Node<K,V>* otherNode, Node<K,V>* parent, Node<K,V>* otherNull);


public:
    ~Map();
    Map();
    Map(const Map& map); // deep copy
    Map& operator=(const Map& map); // deep copy assignment
    Map(Map&& map) noexcept; // move constructor
    Map& operator=(Map&& map) noexcept; // move assignment

    V& operator[](const K& key); // access, insert, update
    V& at(const K& key); // access with bound check(throw if not found)
    const V& at(const K& key) const; // const version of at()

    size_t size();
    bool empty();
    void clear();
    void insert(K key,V value);
    void erase(K key);
    V find(K key);
};