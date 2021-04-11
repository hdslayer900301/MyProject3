#pragma once


#include "support.h"
using namespace std;

//***********************************************************************************************************************************************************************************************************************//
// Map Data
//***********************************************************************************************************************************************************************************************************************//


template<typename KeyType, typename ValueType>

class Map
{
public:
    Map() {
        root = nullptr;
        numAss = 0;
    }

    ~Map() {
        clear();
    }


    void clear()
    {
        remove(root);
        root = nullptr;
        numAss = 0;
        return;
    }

    int32 size() const
    {
        return numAss;
    }

    void associate(const KeyType& key, const ValueType& value)
    {
        if (insert(key, value))
            numAss++;
    }

    // for a map that can't be modified, return a pointer to const ValueType
    const ValueType* find(const KeyType& key) const
    {
        if (root == nullptr)
            return nullptr;
        Node* cur = root;
        while (cur != nullptr)
        {
            if (key == cur->m_key)
                return &(cur->m_value);
            if (key <= cur->m_key)
                cur = cur->left;
            else if (key > cur->m_key)
                cur = cur->right;
        }
        return nullptr;
    }

    // for a modifiable map, return a pointer to modifiable ValueType
    ValueType* find(const KeyType& key)
    {
        return const_cast<ValueType*>(const_cast<const Map*>(this)->find(key));
    }

    // C++11 syntax for preventing copying and assignment
    Map(const Map&) = delete;
    Map& operator=(const Map&) = delete;


private:
    struct Node
    {
        Node(const KeyType& key, const ValueType& value)
        {
            m_key = key;
            m_value = value;
            left = right = nullptr;
        }
        KeyType m_key;
        ValueType m_value;
        Node* left;
        Node* right;
    };

    Node* root;
    int32 numAss;

    bool insert(const KeyType& key, const ValueType& value)
    {
        if (root == nullptr) // empty BST
        {
            root = new Node(key, value);
            return true;
        }
        if (find(key) != nullptr) // already exists in BST
        {
            ValueType* f = find(key);
            *f = value;
            return false;
        }
        Node* cur = root;
        for (;;)
        {
            if (key <= cur->m_key) // move/add left if key is less than current key
            {
                if (cur->left != nullptr)
                    cur = cur->left;
                else
                {
                    cur->left = new Node(key, value);
                    return true;
                }
            }
            else if (key > cur->m_key) // move/add right if key greater than current key
            {
                if (cur->right != nullptr)
                    cur = cur->right;
                else
                {
                    cur->right = new Node(key, value);
                    return true;
                }
            }
        }
    }

    void remove(Node* cur)
    {
        // move all the way down the tree and delete leaf nodes
        if (cur == nullptr)
            return;
        else
        {
            if (cur->left != nullptr)
                remove(cur->left);
            if (cur->right != nullptr)
                remove(cur->right);
            delete cur;
        }
    }
};
