//
//  skiplist.hpp
//  lsm-tree
//
//    sLSM: Skiplist-Based LSM Tree
//    Copyright © 2017 Aron Szanto. All rights reserved.
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//        You should have received a copy of the GNU General Public License
//        along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#pragma once

#ifndef SKIPLIST_H
#define SKIPLIST_H
#include <cstdint>
#include <cstdlib>
#include <strings.h>
#include <random>
#include <vector>
#include <string>

#include "run.hpp"
using namespace std;

// 获取两个数之间的伪随机数的新方法
// 创建引擎
default_random_engine generator;
// 创建取值范围
uniform_real_distribution<double> distribution(0.0,1.0);
const double NODE_PROBABILITY = 0.5;

// skipList节点类
template<class K,class V, unsigned MAXLEVEL>
class SkipList_Node {

public:
    const K key;
    V value;
    SkipList_Node<K,V,MAXLEVEL>* _forward[MAXLEVEL+1];
    
    // 两个构造函数；注意下标从1开始
    SkipList_Node(const K searchKey):key(searchKey) {
        for (int i=1; i<=MAXLEVEL; i++) {
            _forward[i] = NULL;
        }
    }
    
    SkipList_Node(const K searchKey,const V val):key(searchKey),value(val) {
        for (int i=1; i<=MAXLEVEL; i++) {
            _forward[i] = NULL;
        }
    }
    
    virtual ~SkipList_Node(){}
};

// skipList类；Run的派生类
template<class K, class V, int MAXLEVEL = 12>
class SkipList : public Run<K,V>
{

public:

    // 跳表节点新名字：Node
    typedef SkipList_Node<K,V,MAXLEVEL> Node;

    const int max_level; // 最大层数
    K min;
    K max;

    // 构造函数
    // 头节点指向尾节点，cur_max_level=1
    SkipList(const K minKey,const K maxKey):p_listHead(NULL),p_listTail(NULL),
    cur_max_level(1),max_level(MAXLEVEL), min((K) NULL), max((K) NULL),
    _minKey(minKey),_maxKey(maxKey), _n(0)
    {
        p_listHead = new Node(_minKey);
        p_listTail = new Node(_maxKey);
        for (int i=1; i<=MAXLEVEL; i++) {
            p_listHead->_forward[i] = p_listTail;
        }
    }

    // 析构函数；释放内存
    ~SkipList()
    {
        Node* currNode = p_listHead->_forward[1];
        while (currNode != p_listTail) {
            Node* tempNode = currNode;
            currNode = currNode->_forward[1];
            delete tempNode;
        }
        delete p_listHead;
        delete p_listTail;
    }

    // 插入节点
    void insert_key(const K &key, const V &value) {
        if (key > max){
            max = key;
        }
        else if (key < min){
            min = key;
        }

        Node* update[MAXLEVEL];
        Node* currNode = p_listHead;

        // level从cur_max_level走到最小为1。如果下一个节点比key小，
        // 找到每一层比插入的key小的最大key，存到update里去
        for(int level = cur_max_level; level > 0; level--) {
            while (currNode->_forward[level]->key < key) {
                currNode = currNode->_forward[level];
            }
            update[level] = currNode;
        }
        // currNode为第一层比插入的key小的最大key节点
        currNode = currNode->_forward[1];
        if (currNode->key == key) {
            // update the value if the key already exists
            currNode->value = value;
        }
        else {
            // if key isn't in the list, insert a new node! yes!
            int insertLevel = generateNodeLevel();
            
            if (insertLevel > cur_max_level && insertLevel < MAXLEVEL - 1) {
                // 从第二层开始到插入的那一层
                for (int lv = cur_max_level + 1; lv <= insertLevel; lv++) {
                    update[lv] = p_listHead;
                }
                cur_max_level = insertLevel;
            }

            currNode = new Node(key,value);

            for (int level = 1; level <= cur_max_level; level++) {
                currNode->_forward[level] = update[level]->_forward[level];
                update[level]->_forward[level] = currNode;
            }
            ++_n;

        }
        
        
    }

    // 删除节点
    void delete_key(const K &searchKey) {
        //            SkipList_Node<K,V,MAXLEVEL>* update[MAXLEVEL];
        Node* update[MAXLEVEL];
        Node* currNode = p_listHead;
        for(int level=cur_max_level; level >=1; level--) {
            // 如果key比searchKey小，继续往后找
            while (currNode->_forward[level]->key < searchKey) {
                currNode = currNode->_forward[level];
            }
            // 每层大于等于searchKey的最小key节点
            update[level] = currNode;
        }
        currNode = currNode->_forward[1];
        if (currNode->key == searchKey) {
            for (int level = 1; level <= cur_max_level; level++) {
                if (update[level]->_forward[level] != currNode) {
                    break;
                }
                // level层有searchKey的话，删除
                update[level]->_forward[level] = currNode->_forward[level];
            }
            delete currNode;
            // update the max level
            while (cur_max_level > 1 && p_listHead->_forward[cur_max_level] == NULL) {
                cur_max_level--;
            }
        }
        _n--;
    }

    //查找节点
    V lookup(const K &searchKey, bool &found) {
        Node* currNode = p_listHead;
        for(int level=cur_max_level; level >=1; level--) {
            while (currNode->_forward[level]->key < searchKey) {
                currNode = currNode->_forward[level];
            }
        }
        currNode = currNode->_forward[1];
        if (currNode->key == searchKey) {
            found = true;
            return currNode->value;
        }
        else {
            return (V) NULL;
        }
    }

    // 把所有节点取出来存到vector里返回
    vector<KVPair<K,V>> get_all(){
        // KVPair vector
        vector<KVPair<K,V>> vec = vector<KVPair<K, V>>();
        // auto可以在声明变量的时候根据变量初始值的类型自动为此变量选择匹配的类型
        auto node = p_listHead->_forward[1];
        while ( node != p_listTail){
            KVPair<K,V> kv = {node->key, node->value};
            vec.push_back(kv);
            // TODO: optimize by reserving space before hand
            node = node->_forward[1];
        }
        return vec;
    }

    // 取出key1 <= key < key2的节点
    vector<KVPair<K,V>> get_all_in_range(const K &key1, const K &key2){

        if (key1 > max || key2 < min){
            return (vector<KVPair<K,V>>) {};
        }
        
        vector<KVPair<K,V>> vec = vector<KVPair<K, V>>();
        auto node = p_listHead->_forward[1];
        // node最后是key大于等于key1的最小key节点
        while ( node->key < key1){
            node = node->_forward[1];
        }

        // 所以key1 <= key2吧
        while ( node->key < key2){
            KVPair<K,V> kv = {node->key, node->value};
            vec.push_back(kv);
            node = node->_forward[1];
        }
        return vec;
        
        
    }

    // element是否存在
    bool eltIn(K &key) {
        return lookup(key);
    }

    // 内联函数
    inline bool empty() {
        return (p_listHead->_forward[1] == p_listTail);
    }
    
    // 节点个数
    unsigned long long num_elements() {
        return _n;
    }

    // 获取min
    K get_min(){
        return min;
    }

    // 获取max
    K get_max(){
        return max;
    }

    // 设置maxSIze
    void set_size(unsigned long size){
        _maxSize = size;
    }

    // 获取所有节点的总字节数
    size_t get_size_bytes(){
        return _n * (sizeof(K) + sizeof(V));
    }
    
    //    private:

    // 没有懂
    int generateNodeLevel() {
        // ffs()函数用于查找一个整数中的第一个置位值(也就是bit为1的位)。
        return ffs(rand() & ((1 << MAXLEVEL) - 1)) - 1;
    }
    
    K _minKey;
    K _maxKey;
    unsigned long long _n;
    size_t _maxSize;   // size_t带来了可移植性
    int cur_max_level; // 目前的最大层
    Node* p_listHead; // 跳表的头指针
    Node* p_listTail; // 跳表的尾指针
    uint32_t _keysPerLevel[MAXLEVEL];
    
};



#endif /* skiplist_h */
