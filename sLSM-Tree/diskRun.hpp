//
//  diskRun.hpp
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
#ifndef diskRun_h
#define diskRun_h
#include <vector>
#include <cstdint>
#include <cstring>
#include <string>
#include "run.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <cassert>
#include <algorithm>


using namespace std;

template <class K, class V> class DiskLevel;

template <class K, class V>
class DiskRun {
    friend class DiskLevel<K,V>;
public:
    typedef KVPair<K,V> KVPair_t;

    // const void*表示指针指向的地址可以变，但是指向地址的内容只读不可写；保护变量
    // 这里a和b应该会是KVPair<K,V>*类型
    static int compareKVs (const void * a, const void * b)
    {
        if ( *(KVPair<K,V>*)a <  *(KVPair<K,V>*)b ) return -1;
        if ( *(KVPair<K,V>*)a == *(KVPair<K,V>*)b ) return 0;
        if ( *(KVPair<K,V>*)a >  *(KVPair<K,V>*)b ) return 1;
        return 10;
    }

    KVPair_t *map;          // 硬盘映射到内存
    int fd;                 // 文件标识符
    unsigned int pageSize;  // 页面大小
    BloomFilter<K> bf;      // 布隆过滤器
    
    K minKey = INT_MIN;
    K maxKey = INT_MIN;

    // 构造函数
    DiskRun<K,V> (unsigned long capacity, unsigned int pageSize, int level, int runID, double bf_fp):_capacity(capacity),_level(level), _iMaxFP(0), pageSize(pageSize), _runID(runID), _bf_fp(bf_fp), bf(capacity, bf_fp) {
        
        _filename = "C_" + to_string(level) + "_" + to_string(runID) + ".txt";
        
        size_t filesize = capacity * sizeof(KVPair_t);

        long result;

        // O_RDWR可读可写打开；O_CREATE若文件不存在则创建它，使用此选项时需说明参数mode，用于说明该新文件的存取许可权限；O_TRUNC若文件里有内容则把内容清零然后写入新的
        // 0600表示分配给文件的权限；共四位数，第一位数表示gid/uid一般不用；剩下三位分别表示owner和group和other的权限，每个数可以转换为三位二进制数，分别表示rwx读写执行三种权限
        // 6表示为110，表示owner有读写权限，无执行权限
        fd = open(_filename.c_str(), O_RDWR | O_CREAT | O_TRUNC, (mode_t) 0600);
        if (fd == -1) {
            perror("Error opening file for writing");
            exit(EXIT_FAILURE);
        }
        
        /* Stretch the file size to the size of the (mmapped) array of KVPairs
         */
        // lseek移动文件的读写位置；fd文件描述符，offset为根据参数whence来移动读写位置的位移数；SEEK_SET表示参数offset即为新的读写位置
        result = lseek(fd, filesize - 1, SEEK_SET);
        if (result == -1) {
            close(fd);
            perror("Error calling lseek() to 'stretch' the file");
            exit(EXIT_FAILURE);
        }
        
        // buf是指定的缓冲区，即指针指向一段内存单元;n是要写入文件指定的字节数
        // write函数把buf中n bytes写入文件描述符fd所指的文档
        // 如果顺利write()会返回实际写入的字节数len，有错误发生时返回-1
        result = write(fd, "", 1);
        if (result != 1) {
            close(fd);
            perror("Error writing last byte of the file");
            exit(EXIT_FAILURE);
        }
        
        // mmap将文件map到内存，是的内存中的字节与文件中的字节一一对应
        // addr一般设为0，是建议地址；len文件长度；prot表明对这块内存的保护方式，不可与文件访问方式冲突；flags描述映射的类型，MAP_SHARED表示和其他进程共享这个文件，往内存中写入相当于往文件中写入；
        // fd文件描述符，offset文件偏移，从文件起始算起
        map = (KVPair<K, V>*) mmap(0, filesize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (map == MAP_FAILED) {
            close(fd);
            perror("Error mmapping the file");
            exit(EXIT_FAILURE);
        }
    }

    // 析构函数
    ~DiskRun<K,V>(){
        fsync(fd); // 同步内存中所有已修改的文件数据到储存设备
        doUnmap();

        // remove()删除给定的文件名
        // c_str()将C++的string转化为C的字符串数组，生成一个const char*指针，指向字符串的首地址
        // perror()用来将上一个函数发生错误的原因输出到标准设备stderr
        // exit(EXIT_FAILURE)代表异常退出
        if (remove(_filename.c_str())){
            perror(("Error removing file " + string(_filename)).c_str());
            exit(EXIT_FAILURE);
        }
    }

    // 设置容量
    void setCapacity(unsigned long newCap){
        _capacity = newCap;
    }

    // 获取容量
    unsigned long getCapacity(){
        return _capacity;
    }

    // 写数据
    void writeData(const KVPair_t *run, const size_t offset, const unsigned long len) {
        // void *memcpy(void *str1, const void *str2, size_t n)从存储区str2复制n个字节到存储区str1，返回一个指向目标存储区str1的指针
        // str1指向用于存储复制内容的目标数组，类型强制转换为void*指针；str2指向要复制的数据源；n是要被复制的字节数
        memcpy(map + offset, run, len * sizeof(KVPair_t));
        _capacity = len;

    }

    // 建立索引
    // 内存页大小为pageSize，一个run的大小是capacity，一个run映射到内存里需要capacicty/pageSize个页
    // fencePointer存着run映射到内存中的每个页的首元素的key
    void constructIndex(){
        // construct fence pointers and write BF
        // _fencePointers.resize(0);
        // reserve() 为容器预留足够的空间，避免不必要的重复分配。预留空间大于等于字符串的长度。
        _fencePointers.reserve(_capacity / pageSize);
        _iMaxFP = -1; // TODO IS THIS SAFE?
        for (int j = 0; j < _capacity; j++) {
            bf.add((K*) &map[j].key, sizeof(K));
            if (j % pageSize == 0){
                _fencePointers.push_back(map[j].key);
                _iMaxFP++;
            }
        }

        // resize()是设置size大小。size()是分配容器的内存大小，而capacity()只是设置容器容量大小，但并没有真正分配内存。
        if (_iMaxFP >= 0){
            _fencePointers.resize(_iMaxFP + 1);
        }

        // 最大key和最小key，这也说明run是有序的，从小到大排列
        minKey = map[0].key;
        maxKey = map[_capacity - 1].key;
        
    }

    // 二分查找
    unsigned long binary_search (const unsigned long offset, const unsigned long n, const K &key, bool &found) {
        if (n == 0){
            found = true;
            return offset;
        }
        unsigned long min = offset, max = offset + n - 1;
        unsigned long middle = (min + max) >> 1;
        while (min <= max) {
            if (key > map[middle].key)
                min = middle + 1;
            else if (key == map[middle].key) {
                found = true;
                return middle;
            }
            else
                max = middle - 1;
            middle = (min + max) >> 1;
            
        }
        return min;
    }

    // 给定一个key，根据fp找到它的start和end位置
    // 这个key在哪一页，这页的起止位置又是哪里
    void get_flanking_FP(const K &key, unsigned long &start, unsigned long &end){
        if (_iMaxFP == 0) {
            start = 0;
            end = _capacity;
        }
        else if (key < _fencePointers[1]){
            start = 0;
            end = pageSize;
        }
        else if (key >= _fencePointers[_iMaxFP]) {
            start = _iMaxFP * pageSize;
            end = _capacity;
        }
        else {
            unsigned min = 0, max = _iMaxFP;
            while (min < max) {
                
                unsigned middle = (min + max) >> 1;
                if (key > _fencePointers[middle]){
                    if (key < _fencePointers[middle + 1]){
                        start = middle * pageSize;
                        end = (middle + 1) * pageSize;
                        return; // TODO THIS IS ALSO GROSS
                    }
                    min = middle + 1;
                }
                else if (key < _fencePointers[middle]) {
                    if (key >= _fencePointers[middle - 1]){
                        start = (middle - 1) * pageSize;
                        end = middle * pageSize;
                        return; // TODO THIS IS ALSO GROSS. THIS WILL BREAK IF YOU DON'T KEEP TRACK OF MIN AND MAX.
                    }
                    
                    max = middle - 1;
                }
                
                else {
                    start = middle * pageSize;
                    end = start;
                    return;
                }
                
            }

        }
    }

    // 找到key的具体位置
    unsigned long get_index(const K &key, bool &found){
        unsigned long start, end;
        // 找到在第几页，然后用二分查找找到具体位置
        get_flanking_FP(key, start, end);
        unsigned long ret = binary_search(start, end - start, key, found);
        return ret;
    }

    // 查找key是否存在
    V lookup(const K &key, bool &found){
         unsigned long idx = get_index(key, found);
         V ret = map[idx].value;
         return found ? ret : (V) NULL;
     }

     // 范围查询，查找key1~key2的索引范围
    void range(const K &key1, const K &key2, unsigned long &i1, unsigned long &i2){
        i1 = 0;
        i2 = 0;
        if (key1 > maxKey || key2 < minKey){
            return;
        }
        if (key1 >= minKey){
            bool found = false;
            i1 = get_index(key1, found);
            
        }
        if (key2 > maxKey){
            i2 = _capacity;
            return;
        }
        else {
            bool found = false;
            i2 = get_index(key2, found);
        }
    }

    // 打印runs
    void printElts(){
        for (int j = 0; j < _capacity; j++){
            cout << map[j].key << " ";
        }
        cout << endl;
    }
    
private:
    unsigned long _capacity;  // 每个
    string _filename;         // 文件名
    int _level;               // 层级
    vector<K> _fencePointers; // 每个pagesize
    unsigned _iMaxFP;         // 最大FencePointer
    unsigned _runID;          // run的id
    double _bf_fp;            // 布隆过滤器的false positive
                            
    void doMap(){
        
        size_t filesize = _capacity * sizeof(KVPair_t);
        
        fd = open(_filename.c_str(), O_RDWR | O_CREAT | O_TRUNC, (mode_t) 0600);
        if (fd == -1) {
            perror("Error opening file for writing");
            exit(EXIT_FAILURE);
        }
        
        
        map = (KVPair<K, V>*) mmap(0, filesize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (map == MAP_FAILED) {
            close(fd);
            perror("Error mmapping the file");
            exit(EXIT_FAILURE);
        }
    }
    
    void doUnmap(){
        size_t filesize = _capacity * sizeof(KVPair_t);

        // munmap()用来取消参数start所指的映射内存起始地址,参数length则是欲取消的内存大小
        if (munmap(map, filesize) == -1) {
            perror("Error un-mmapping the file");
        }
        
        close(fd);
        fd = -5;
    }

    // 这个函数没用到，先不看了
    void doubleSize(){
        unsigned long new_capacity = _capacity * 2;
        
        size_t new_filesize = new_capacity * sizeof(KVPair_t);
        int result = lseek(fd, new_filesize - 1, SEEK_SET);
        if (result == -1) {
            close(fd);
            perror("Error calling lseek() to 'stretch' the file");
            exit(EXIT_FAILURE);
        }
        
        result = write(fd, "", 1);
        if (result != 1) {
            close(fd);
            perror("Error writing last byte of the file");
            exit(EXIT_FAILURE);
        }
        
        map = (KVPair<K, V>*) mmap(0, new_filesize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (map == MAP_FAILED) {
            close(fd);
            perror("Error mmapping the file");
            exit(EXIT_FAILURE);
        }
        
        _capacity = new_capacity;
    }
    
    
    
    
};
#endif /* diskRun_h */

