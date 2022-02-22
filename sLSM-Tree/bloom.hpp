//
//  bloom.hpp
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

#ifndef bloom_h
#define bloom_h

#include <stdio.h>
#include <cstdint>
#include <vector>
#include <array>
#include <math.h>

#include "MurmurHash.h"

using namespace std;

template<class Key>
class BloomFilter {
public:
    BloomFilter(uint64_t n, double fp) {

        // 分母
        double denom = 0.480453013918201; // (ln(2))^2

        double size = -1 * (double) n * (log(fp) / denom);
        
        m_bits = vector<bool>((int) size);
        
        double ln2 = 0.693147180559945;
        // ceil()返回大于或者指定表达式的最小整数
        m_numHashes = (int) ceil( (size / n) * ln2);  // ln(2)
    }

    // 将元素通过Murmur3哈希函数映射生成hash值
    array<uint64_t, 2> hash(const Key *data, size_t len) {

        // 生成两个hash值
        array<uint64_t, 2> hashValue;

        // array.data()返回指向数组对象中的一个元素的指针
        MurmurHash3_x64_128(data, (int) len, 0, hashValue.data());
        
        return hashValue;
    }

    // 根据hash值找到filter数组的对应位置
    uint64_t nthHash(uint32_t n, uint64_t hashA, uint64_t hashB, uint64_t filterSize) {
        return (hashA + n * hashB) % filterSize;
    }

    // 将hash值在filter数组上对应的位置位 置为true
    void add(const Key *data, size_t len) {
        auto hashValues = hash(data, len);
        
        for (int n = 0; n < m_numHashes; n++) {
            m_bits[nthHash(n, hashValues[0], hashValues[1], m_bits.size())] = true;
        }
    }

    // 检查是否元素存在
    bool mayContain(const Key *data, size_t len) {
        auto hashValues = hash(data, len);
        
        for (int n = 0; n < m_numHashes; n++) {
            if (!m_bits[nthHash(n, hashValues[0], hashValues[1], m_bits.size())]) {
                return false;
            }
        }
        
        return true;
    }
    
private:
    uint8_t m_numHashes;
    vector<bool> m_bits;
};




#endif /* bloom_h */
