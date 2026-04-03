#include "map.h"
#include "rediszset.h"
#include <string>


/**********************************PUBLIC MAP FUNCTIONS*****************************************/

template<class K,class V>
Node<K,V>::Node(K key, V value)
:key(key), value(value), color(RED), left(nullptr), right(nullptr), parent(nullptr){}

template<class K,class V>
Node<K,V>::Node() 
:key(), value(), color(RED), left(nullptr), right(nullptr), parent(nullptr) {}


/***********************************************************************************************/






/**********************************PRIVATE MAP FUNCTIONS****************************************/

template<class K,class V>
Node<K,V>* Map<K,V>::getMinimumNode(Node<K,V>* node){
    while(node->left != null){
        node = node->left;
    }
    return node;
}

template<class K,class V>
Node<K,V>* Map<K,V>::getMaximumNode(Node<K,V>* node){
    while(node->right != null){
        node = node->right;
    }
    return node;
}


template<class K,class V>
void Map<K,V>::rightRotate(Node<K,V>* node){
    Node<K,V>* x = node->left;
    
    node->left = x->right;
    if(x->right != null){
        x->right->parent = node;
    }

    x->parent = node->parent;
    if(node->parent == null){
        root = x;
    }else if(node->parent->left == x){
        node->parent->left = x;
    }else{
        node->parent->right = x;
    }

    x->right = node;
    node->parent = x;
}

template<class K,class V>
void Map<K,V>::leftRotate(Node<K,V>* node){
    Node<K,V>* x = node->right;

    node->right = x->left;
    if(x->left != null){
        x->left->parent = node;
    }

    x->parent = node->parent;
    if(node->parent == null){
        root = x;
    }else if(node->parent->left == node){
        node->parent->left = x;
    }else{
        node->parent->right = x;
    }

    x->left = node;
    node->parent = x;
}

template<class K,class V>
void Map<K,V>::balanceInsert(Node<K,V>* node){
    while(node->parent->color == RED){
        if(node->parent == node->parent->parent->right){
            Node<K,V>* uncle = node->parent->parent->left;
            if(uncle->color == RED){
                node->parent->color = uncle->color = BLACK;
                node->parent->parent->color = RED;
                node = node->parent->parent;
            }else{
                if(node == node->parent->left){
                    node = node->parent;
                    rightRotate(node);
                }
                node->parent->color = BLACK;
                node->parent->parent->color = RED;
                leftRotate(node->parent->parent);
            }
        }else{
            Node<K,V>* uncle = node->parent->parent->right;
            if(uncle->color == RED){
                uncle->color = BLACK;
                node->parent->color = BLACK;
                node->parent->parent->color = RED;
                node = node->parent->parent;
            }else{
                if(node == node->parent->right){
                    node = node->parent;
                    leftRotate(node);
                }
                node->parent->color = BLACK;
                node->parent->parent->color = RED;
                rightRotate(node->parent->parent);
            }
        }
        if(node == root){
            break;
        }
    }
    root->color = BLACK;
}

template<class K,class V>
Node<K,V>* Map<K,V>::successor(Node<K,V>* node){
    if(node->right != null){
        return getMinimumNode(node->right);
    }
    Node<K,V>* temp = node->parent;
    while(temp != null and node == temp->right){
        node = temp;
        temp = temp->parent;
    }
    return temp;
}

template<class K,class V>
Node<K,V>* Map<K,V>::predecessor(Node<K,V>* node){
    if(node->left != null){
        return getMaximumNode(node->left);
    }

    Node<K,V>* temp = node->parent;
    while(temp != null and node == temp->left){
        node = temp;
        temp = temp->left;
    }
    return temp;
}

template<class K,class V>
Node<K,V>* Map<K,V>::ReplaceNode(Node<K,V>* node){
    if(node->left != nullptr and node->right != nullptr){
        return successor(node->right);
    }

    if(node->left == nullptr and node->right == nullptr){
        return nullptr;
    }

    if(node->left == nullptr){
        return node->right;
    }

    return node->left;
}

template<class K,class V>
void Map<K,V>::redBlackTransplant(Node<K,V>* u,Node<K,V>* v){
    if(u->parent == nullptr){
        root = v;
    }else if(u == u->parent->left){
        u->parent->left = v;
    }else{
        u->parent->right = v;
    }
    v->parent = u->parent;
}

template<class K,class V>
void Map<K,V>::balanceDelete(Node<K,V>* node){
    Node<K,V>* sibling = nullptr;
    while(node != root and node->color == BLACK){
        if(node == node->parent->left){
            sibling = node->parent->right;
            if(sibling->color == RED){
                sibling->color = BLACK;
                node->parent->color = RED;
                leftRotate(node->parent);
                sibling = node->parent->right;
            }   

            if(sibling->left->color == BLACK and sibling->right->color == BLACK){
                sibling->color = RED;
                node = node->parent;
            }else{
                if(sibling->right->color == BLACK){
                    sibling->left->color = BLACK;
                    sibling->color = RED;
                    rightRotate(sibling);
                    sibling = node->parent->right;
                }

                sibling->color = node->parent->color;
                node->parent->color = BLACK;
                sibling->right->color = BLACK;
                leftRotate(node->parent);
                node = root;
            }
        }else{
            sibling = node->parent->left;
            if(sibling->color == RED){
                sibling->color = BLACK;
                node->parent->color = RED;
                rightRotate(node->parent);
                sibling = node->parent->left;  
            }

            if(sibling->right->color == BLACK and sibling->left->color == BLACK){
                sibling->color = RED;
                node = node->parent;
            }else{
                if(sibling->left->color == BLACK){
                    sibling->right->color = BLACK;
                    sibling->color = RED;
                    leftRotate(sibling);
                    sibling = node->parent->left;
                }

                sibling->color = node->parent->color;
                node->parent->color = BLACK;
                sibling->left->color = BLACK;
                rightRotate(node->parent);
                node = root;
            }
        } 
    }
    node->color = BLACK;
}

template<class K,class V>
void Map<K,V>::destroyTree(Node<K,V>* node){
    if(node != null){
        destroyTree(node->left);
        destroyTree(node->right);
        delete node;
    }
}

template<class K,class V>
void Map<K,V>::copyTree(Node<K,V>*& thisNode, Node<K,V>* otherNode, Node<K,V>* parent, Node<K,V>* otherNull){
    if(otherNode == otherNull){
        thisNode = this->null;
        return;
    }

    thisNode = new Node<K,V>(otherNode->key, otherNode->value);
    thisNode->color = otherNode->color;
    thisNode->parent = parent;
    
    copyTree(thisNode->left, otherNode->left, thisNode, otherNull);
    copyTree(thisNode->right, otherNode->right, thisNode, otherNull);
}

template<class K,class V>
void Map<K,V>::inorder(Node<K,V>* node, Node<K,V>* nullNode, std::vector<K>& result){
    if (node != nullNode) {
        inorder(node->left, nullNode, result);
        result.push_back(node->key);
        inorder(node->right, nullNode, result);
    }   
}


/***********************************************************************************************/






/**********************************PUBLIC MAP FUNCTIONS*****************************************/

template<class K,class V>
Map<K,V>::Map(){
    null = new Node<K,V>();
    null->color = BLACK;
    root = null;
}

template<class K,class V>
Map<K,V>::~Map(){
    destroyTree(root);
    delete null;
}

template<class K, class V>
Map<K,V>::Map(const Map& map) {
    null = new Node<K,V>();
    null->color = BLACK;
    root = null;
    _size = map._size;
    copyTree(root, map.root, nullptr, map.null);
}

template<class K, class V>
Map<K,V>& Map<K,V>::operator=(const Map& map) {
    if (this != &map) {
        clear();
        _size = map._size;
        copyTree(root, map.root, nullptr, map.null);
    }
    return *this;
}

// Move Constructor
template<class K, class V>
Map<K,V>::Map(Map&& map) noexcept : root(map.root), null(map.null), _size(other._size) {
    map.null = new Node<K,V>();
    map.null->color = BLACK;
    map.root = map.null;
    map._size = 0;
}

// Move Assignment Operator
template<class K, class V>
Map<K,V>& Map<K,V>::operator=(Map&& map) noexcept {
    if (this != &map) {
        destroyTree(root);
        delete null;

        root = map.root;
        null = map.null;
        _size = map._size;

        map.null = new Node<K,V>();
        map.null->color = BLACK;
        map.root = map.null;
        map._size = 0;
    }
    return *this;
}

template<class K, class V>
void Map<K,V>::clear(){
    destroyTree(root);
    root = null;
    _size = 0;
}

template<class K, class V>
std::vector<K> Map<K,V>::getSortedKeys() {
    std::vector<K> result;
    inorderHelper(root, null, result);
    return result;
}

template<class K, class V>
V& Map<K,V>::operator[](const K& key) {
    Node<K,V>* curr = root;
    Node<K,V>* prev = nullptr;

    while (curr != null) {
        prev = curr;
        if (key == curr->key) {
            return curr->value; 
        } else if (key < curr->key) {
            curr = curr->left;
        } else {
            curr = curr->right;
        }
    }

    // If we reach here, it's not in the map. Insert a default-constructed value.
    insert(key, V{}); 
    
    // Find and return a reference to the newly inserted node.
    // (Note: A more optimized version would have `insert` return a pointer to the new node directly).
    curr = root;
    while (curr != null) {
        if (key == curr->key) return curr->value;
        else if (key < curr->key) curr = curr->left;
        else curr = curr->right;
    }
    throw std::runtime_error("Insertion failed during operator[]");
}

// at(): Access with bounds checking
template<class K, class V>
V& Map<K,V>::at(const K& key) {
    Node<K,V>* curr = root;
    while(curr != null){
        if(key == curr->key) return curr->value;
        else if(key < curr->key) curr = curr->left;
        else curr = curr->right;
    }
    throw std::out_of_range("Map::at - Key not found");
}

// const at(): For reading from const Maps
template<class K, class V>
const V& Map<K,V>::at(const K& key) const {
    Node<K,V>* curr = root;
    while(curr != null){
        if(key == curr->key) return curr->value;
        else if(key < curr->key) curr = curr->left;
        else curr = curr->right;
    }
    throw std::out_of_range("Map::at - Key not found");
}

template<class K,class V>
V Map<K,V>::find(K key){
    Node<K,V>* curr = root;
    while(curr != null){
        if(key == curr->key){
            return curr->value;
        }else if(key < curr->key){
            curr = curr->left;
        }else{
            curr = curr->right;
        }
    }
    return V{};
}

template<class K,class V>
void Map<K,V>::insert(K key, V value){ // TODO
    Node<K,V>* node = new Node<K,V>(key, value);
    node->left = node->right = null;
    
    Node<K,V>* prev = nullptr;
    Node<K,V>* curr = root;

    while(curr != null){
        prev = curr;
        if(key == curr->key){
            curr->value = value;
            return;
        }else if(key < curr->key){
            curr = curr->left;
        }else{
            curr = curr->right;
        }
    }

    node->parent = prev;
    if(prev == nullptr){
        root = node;
    }else if(node->key < prev->key){
        prev->left = node;
    }else{
        prev->right = node;
    }

    if(node->parent == nullptr){
        node->color = BLACK;
        return;
    }

    if(node->parent->parent == nullptr){
        return;
    }

    balanceInsert(node);
}

template<class K,class V>
void Map<K,V>::erase(K key){
    Node<K,V>* node = null;
    Node<K,V>* curr = root;
    while(curr != null){
        if(curr->key == key){
            node = curr;
        }
        if(curr->key <= key){
            curr = curr->right;
        }else{
            curr = curr->left;
        }
    }

    if(node == null){
        return;
    }

    Node<K,V>* y = node;
    Color deletedNodeColor = y->color;
    Node<K,V>* x = nullptr;

    if(node->left == null){
        x = node->right;
        redBlackTransplant(node, node->right);
    }else if(node->right == null){
        x = node->left;
        redBlackTransplant(node, node->left);
    }else{
        Node<K,V>* y = getMinimumNode(node->right);
        deletedNodeColor = y->color;
        x = y->right;
        if(y->parent == node){
            y->right->parent = y;
        }else{
            redBlackTransplant(y, x);
            y->right = node->right;
            y->right->parent = y;
        }

        redBlackTransplant(node, y);
        y->left = node->left;
        y->left->parent = y;
        y->color = node->color;
    }

    delete node;
    if(deletedNodeColor == BLACK){
        balanceDelete(x);
    }
}


/***********************************************************************************************/


template class Map<int, std::string>;
template class Map<std::string, std::string>;
template class Map<std::string, bool>;
template class Map<MyRedis::ZSetKey, bool>;