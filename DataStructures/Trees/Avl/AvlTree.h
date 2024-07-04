//
// Created by shei on 23/06/24.
//

#ifndef LIKEREDIS2_0_AVLTREE_H
#define LIKEREDIS2_0_AVLTREE_H

#include <cstdint>

template<typename T>
struct AVLNode {
    uint8_t depth = 0;
    uint32_t cnt = 0;
    AVLNode* left = nullptr;
    AVLNode* right = nullptr;
    AVLNode* parent = nullptr;

    T* data = nullptr;
};

template <typename T> class AVLTree {
private:
    AVLNode<T>* root = nullptr;

    static AVLNode<T>* lr_rotation(AVLNode<T>* node);
    static AVLNode<T>* rl_rotation(AVLNode<T>* node);
    static AVLNode<T>* ll_rotation(AVLNode<T>* node);
    static AVLNode<T>* rr_rotation(AVLNode<T>* node);

    static uint32_t avl_max(uint32_t a, uint32_t b);
    static uint8_t avl_height(AVLNode<T>* node);
    static void avl_update(AVLNode<T>* node);
    static void avl_parent_update(AVLNode<T>* node, AVLNode<T>* new_node);

    static AVLNode<T>* avl_iterate(AVLNode<T>* node, T key, uint8_t &match);
    static AVLNode<T>* avl_min_node(AVLNode<T>* node);
public:
    explicit AVLTree();
    void avl_insert(T data);
    AVLNode<T>* avl_search(T data);
    void avl_destroy (T data);
};

#endif //LIKEREDIS2_0_AVLTREE_H
