//
//  diskLevel.hpp
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

#ifndef diskLevel_h
#define diskLevel_h
#include <vector>
#include <cstdint>
#include <string>
#include <cstring>
#include "run.hpp"
#include "diskRun.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <cassert>
#include <algorithm>

#define LEFTCHILD(x) 2 * x + 1
#define RIGHTCHILD(x) 2 * x + 2
#define PARENT(x) (x - 1) / 2

int TOMBSTONE = INT_MIN;

using namespace std;


template <class K, class V>
class DiskLevel {
    
public: // TODO make some of these private
    typedef KVPair<K,V> KVPair_t;
    typedef pair<KVPair<K, V>, int> KVIntPair_t;
    KVPair_t KVPAIRMAX;
    KVIntPair_t KVINTPAIRMAX;
    V V_TOMBSTONE = (V) TOMBSTONE;

    // 用堆来表示disk层级关系
    // 好好看看，没看明白
    struct StaticHeap {
        int size;
        vector<KVIntPair_t> arr;
        KVIntPair_t max;
        
        StaticHeap(unsigned sz, KVIntPair_t mx) {
            size = 0;
            arr = vector<KVIntPair_t>(sz, mx); // 定义sz个向量，每个元素初值为mx
            max = mx;
        }

        // 哦哦，堆的代码
        // 小根堆吧
        void push(KVIntPair_t blob) {
            unsigned i = size++;
            while(i && blob < arr[PARENT(i)]) {
                arr[i] = arr[PARENT(i)] ;
                i = PARENT(i) ;
            }
            arr[i] = blob ;
        }

        // 数组建堆，应该是小根堆
        void heapify(int i) {
            int smallest = (LEFTCHILD(i) < size && arr[LEFTCHILD(i)] < arr[i]) ? LEFTCHILD(i) : i ;
            if(RIGHTCHILD(i) < size && arr[RIGHTCHILD(i)] < arr[smallest]) {
                smallest = RIGHTCHILD(i);
            }
            if(smallest != i) {
                KVIntPair_t temp = arr[i];
                arr[i] = arr[smallest];
                arr[smallest] = temp;
                heapify(smallest) ;
            }
        }

        // 删除元素
        // 弹出根节点，把最后一个节点赋值给根节点，然后进行堆排序，明白了明白了
        KVIntPair_t pop() {
            KVIntPair_t ret = arr[0];
            arr[0] = arr[--size];
            heapify(0);
            return ret;
        }
    };


    int _level;
    unsigned _pageSize;     // number of elements per fence pointer
    unsigned long _runSize; // number of elts in a run
    unsigned _numRuns;      // number of runs in a level
    unsigned _activeRun;    // index of active run
    unsigned _mergeSize;    // # of runs to merge downwards
    double _bf_fp;          // bloom filter false positive
    vector<DiskRun<K,V> *> runs;

    DiskLevel<K,V>(unsigned int pageSize, int level, unsigned long runSize, unsigned numRuns, unsigned mergeSize, double bf_fp):_numRuns(numRuns), _runSize(runSize),_level(level), _pageSize(pageSize), _mergeSize(mergeSize), _activeRun(0), _bf_fp(bf_fp){
        KVPAIRMAX = (KVPair_t) {INT_MAX, 0};
        KVINTPAIRMAX = KVIntPair_t(KVPAIRMAX, -1);

        // 好的给每一层设置_numRuns个run；由此可见diskRun的capacity就是_runSize，哦哦是这样哦哦哦哦哦哦哦哦哦
        for (int i = 0; i < _numRuns; i++){
            DiskRun<K,V> * run = new DiskRun<K, V>(_runSize, pageSize, level, i, _bf_fp);
            runs.push_back(run);
        }
    }
    
    ~DiskLevel<K,V>(){
        for (int i = 0; i< runs.size(); ++i){
            delete runs[i];
        }
    }

    // 添加runs
    // 这个没有完全读懂
    void addRuns(vector<DiskRun<K, V> *> &runList, const unsigned long runLen, bool lastLevel) {

        // 相当于这个向量里面_numRuns个KVINTPAIRMAX
        StaticHeap h = StaticHeap((int) runList.size(), KVINTPAIRMAX);
        // _numRuns个0
        vector<int> heads(runList.size(), 0);
        for (int i = 0; i < runList.size(); i++){
            // 每个run的map[0]
            KVPair_t kvp = runList[i]->map[0];
            // 建堆必经步骤
            h.push(KVIntPair_t(kvp, i));
        }
        
        int j = -1;
        K lastKey = INT_MAX;
        unsigned lastk = INT_MIN;
        while (h.size != 0){
            // 弹出最小值还是最大值啊；最小值吧
            auto val_run_pair = h.pop();
            assert(val_run_pair != KVINTPAIRMAX); // TODO delete asserts
            if (lastKey == val_run_pair.first.key){
                if( lastk < val_run_pair.second){
                    runs[_activeRun]->map[j] = val_run_pair.first;
                }
            }
            else {
                ++j;
                // 如果是最后一层而且激活状态的run含有墓碑标记
                if ( j != -1 && lastLevel && runs[_activeRun]->map[j].value == V_TOMBSTONE){
                    --j;
                }
                runs[_activeRun]->map[j] = val_run_pair.first;
            }
            
            lastKey = val_run_pair.first.key;
            lastk = val_run_pair.second;
            
            unsigned k = val_run_pair.second;
            if (++heads[k] < runList[k]->getCapacity()){
                KVPair_t kvp = runList[k]->map[heads[k]];
                h.push(KVIntPair_t(kvp, k));
            }
                
        }
        
        if (lastLevel && runs[_activeRun]->map[j].value == V_TOMBSTONE){
            --j;
        }
        runs[_activeRun]->setCapacity(j + 1);
        runs[_activeRun]->constructIndex();
        if(j + 1 > 0){
            ++_activeRun;
        }
        
    }

    // ？？？
    void addRunByArray(KVPair_t * runToAdd, const unsigned long runLen){
        assert(_activeRun < _numRuns);
        assert(runLen == _runSize);
        runs[_activeRun]->writeData(runToAdd, 0, runLen);
        runs[_activeRun]->constructIndex();
        _activeRun++;
    }

    // 合并mergeSize个runs
    vector<DiskRun<K,V> *> getRunsToMerge(){
        vector<DiskRun<K, V> *> toMerge;
        for (int i = 0; i < _mergeSize; i++){
            toMerge.push_back(runs[i]);
        }

        return toMerge;
        
    }

    // 释放已经合并的runs
    void freeMergedRuns(vector<DiskRun<K,V> *> &toFree){
        assert(toFree.size() == _mergeSize);
        for (int i = 0; i < _mergeSize; i++){
            assert(toFree[i]->_level == _level);
            delete toFree[i];
        }
        // 删除这层runs中已经合并的run们，后面的元素自动前移补位
        runs.erase(runs.begin(), runs.begin() + _mergeSize);
        _activeRun -= _mergeSize;
        for (int i = 0; i < _activeRun; i++){

            runs[i]->_runID = i;

            // filename重新命名
            string newName = ("C_" + to_string(runs[i]->_level) + "_" + to_string(runs[i]->_runID) + ".txt");

            //？前面不应该加一个！吗
            if (rename(runs[i]->_filename.c_str(), newName.c_str())){
                perror(("Error renaming file " + runs[i]->_filename + " to " + newName).c_str());
                exit(EXIT_FAILURE);
            }
            runs[i]->_filename = newName;
        }

        // ok，因为删除了几个run，所以添加几个新run
        for (int i = _activeRun; i < _numRuns; i++){
            DiskRun<K, V> * newRun = new DiskRun<K,V>(_runSize, _pageSize, _level, i, _bf_fp);
            runs.push_back(newRun);
        }
    }

    // 该层是不是满了
    bool levelFull(){
        return (_activeRun == _numRuns);
    }

    // 该层是不是空的
    bool levelEmpty(){
        return (_activeRun == 0);
    }

    // 在runs里面找key对应的value
    V lookup (const K &key, bool &found) {
        int maxRunToSearch = levelFull() ? _numRuns - 1 : _activeRun - 1;
        for (int i = maxRunToSearch; i >= 0; --i){
            if (runs[i]->maxKey == INT_MIN || key < runs[i]->minKey || key > runs[i]->maxKey || !runs[i]->bf.mayContain(&key, sizeof(K))){
                continue;
            }
            V lookupRes = runs[i]->lookup(key, found);
            if (found) {
                return lookupRes;
            }
        }
        
        return (V) NULL;
    }

    // 该层总元素个数
    unsigned long num_elements(){
        unsigned long total = 0;
        for (int i = 0; i < _activeRun; ++i)
            total += runs[i]->getCapacity();
        return total;
    }
};
#endif /* diskLevel_h */
