#include <cassert>
#include <cstdlib>
#include <new>
#include <stdexcept>
#include <cstdio> // for fprintf
#include "AvlTree.h"

template<typename T>
AVLTree<T>::AVLTree() {
    root = nullptr; // Initialize root to nullptr
}

template<typename T>
void AVLTree<T>::avl_insert(T data) {
    // Allocate data on the heap
    T* new_data = new T(data);
    if (!new_data) {
        throw std::bad_alloc();
    }

    // Check if root is empty
    if (!this->root) {
        this->root = new AVLNode<T>();
        this->root->data = new_data;
        this->root->left = this->root->right = nullptr;
        this->root->depth = 1;
        this->root->cnt = 1;
        this->root->parent = nullptr;
        return;
    }

    int direction = 0; // -1 for left, 1 for right, 0 for a match
    AVLNode<T> *cursor = avl_iterate(this->root, data, direction);
    if (direction == 0) {
        fprintf(stderr, "[-] key already exists\n");
        delete new_data;
        return;
    }

    // Create new node
    auto new_node = new AVLNode<T>();
    if (!new_node) {
        delete new_data;
        throw std::bad_alloc();
    }
    new_node->data = new_data;
    new_node->left = new_node->right = nullptr;
    new_node->parent = cursor;

    if (direction == -1) {
        cursor->left = new_node;
    } else {
        cursor->right = new_node;
    }

    avl_update(this->root);

    int balanceFactor = avl_height(cursor->left) - avl_height(cursor->right);
    if (balanceFactor > 1) {
        if (*new_data < *(cursor->left->data)) {
            rr_rotation(cursor);
        } else if (*new_data > *(cursor->left->data)) {
            cursor->left = ll_rotation(cursor->left);
            rr_rotation(cursor);
        }
    } else if (balanceFactor < -1) {
        if (*new_data > *(cursor->right->data)) {
            ll_rotation(cursor);
        } else if (*new_data < *(cursor->right->data)) {
            cursor->right = rr_rotation(cursor->right);
            ll_rotation(cursor);
        }
    }
}

template<typename T>
AVLNode<T>* AVLTree<T>::avl_search(T data) {
    int direction = 2;
    AVLNode<T>* cursor = avl_iterate(this->root, data, direction);
    if (direction == 0 && cursor) {
        return cursor;
    }
    return nullptr;
}

template<typename T>
void AVLTree<T>::avl_destroy(T data) {
    int direction = 2;
    AVLNode<T>* cursor = avl_iterate(this->root, data, direction);
    if (!cursor || direction != 0) {
        fprintf(stderr, "[-] item to destroy not found\n");
        return;
    }

    if ((cursor->left == nullptr) || (cursor->right == nullptr)) {
        AVLNode<T>* temp = cursor->left ? cursor->left : cursor->right;
        if (temp == nullptr) {
            temp = cursor;
            cursor = nullptr;
        } else {
            *cursor = *temp;
        }
        delete temp->data;
        delete temp;
    } else {
        AVLNode<T>* temp = avl_min_node(cursor->right);
        cursor->data = temp->data;
        avl_destroy(*(temp->data)); // Recursively delete the inorder successor
    }

    if (cursor != nullptr) {
        avl_update(cursor);

        int balanceFactor = avl_height(cursor->left) - avl_height(cursor->right);
        if (balanceFactor > 1) {
            if (avl_height(cursor->left->left) >= avl_height(cursor->left->right)) {
                rr_rotation(cursor);
            } else {
                cursor->left = ll_rotation(cursor->left);
                rr_rotation(cursor);
            }
        } else if (balanceFactor < -1) {
            if (avl_height(cursor->right->right) >= avl_height(cursor->right->left)) {
                ll_rotation(cursor);
            } else {
                cursor->right = rr_rotation(cursor->right);
                ll_rotation(cursor);
            }
        }
    }
}

template<typename T>
AVLNode<T>* AVLTree<T>::avl_iterate(AVLNode<T>* node, T key, uint8_t &direction) {
    AVLNode<T>* cursor = node;
    while (cursor) {
        if (*(cursor->data) == key) {
            direction = 0;
            return cursor;
        } else if (*(cursor->data) < key) {
            if (cursor->right) {
                cursor = cursor->right;
                continue;
            }
            direction = 1;
            return cursor;
        } else {
            if (cursor->left) {
                cursor = cursor->left;
                continue;
            }
            direction = -1;
            return cursor;
        }
    }
    assert("[-] how did we get here (avl_iterate)");
    return nullptr;
}

template<typename T>
AVLNode<T>* AVLTree<T>::avl_min_node(AVLNode<T>* node) {
    AVLNode<T>* cursor = node;
    while (cursor->left) {
        cursor = cursor->left;
    }
    return cursor;
}

template<typename T>
void AVLTree<T>::avl_parent_update(AVLNode<T>* node, AVLNode<T>* new_node) {
    if (node->parent) {
        // Update parent pointers
        if (node->parent->left == node) {
            node->parent->left = new_node;
        } else if (node->parent->right == node) {
            node->parent->right = new_node;
        } else {
            assert("(rr_rotation) not sure how we got here");
        }
    }
}

template<typename T>
void AVLTree<T>::avl_update(AVLNode<T>* node) {
    if (node) {
        node->depth = avl_max(avl_height(node->left), avl_height(node->right)) + 1;
    }
}

template<typename T>
uint8_t AVLTree<T>::avl_height(AVLNode<T>* node) {
    if (node == nullptr) return 0;
    return node->depth;
}

template<typename T>
uint32_t AVLTree<T>::avl_max(uint32_t a, uint32_t b) {
    return (a > b) ? a : b;
}

template<typename T>
AVLNode<T>* AVLTree<T>::rr_rotation(AVLNode<T>* node) {
    AVLNode<T>* new_node = node->left;
    AVLNode<T>* right_subtree = new_node->right;

    new_node->right = node;
    node->left = right_subtree;

    new_node->parent = node->parent;
    node->parent = new_node;
    if (right_subtree) {
        right_subtree->parent = node;
    }

    // Update parent pointers
    avl_parent_update(node, new_node);

    // Update node heights
    avl_update(node);
    avl_update(new_node);
    return new_node;
}

template<typename T>
AVLNode<T>* AVLTree<T>::ll_rotation(AVLNode<T>* node) {
    AVLNode<T>* new_node = node->right;
    AVLNode<T>* left_subtree = new_node->left;

    new_node->left = node;
    node->right = left_subtree;

    new_node->parent = node->parent;
    node->parent = new_node;
    if (left_subtree) {
        left_subtree->parent = node;
    }

    // Update heights
    avl_update(node);
    avl_update(new_node);

    return new_node;
}

template<typename T>
AVLNode<T>* AVLTree<T>::rl_rotation(AVLNode<T>* node) {
    node->right = rr_rotation(node->right);
    return ll_rotation(node);
}

template<typename T>
AVLNode<T>* AVLTree<T>::lr_rotation(AVLNode<T>* node) {
    node->left = ll_rotation(node->left);
    return rr_rotation(node);
}
