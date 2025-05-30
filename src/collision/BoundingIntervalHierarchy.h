/*
 * AscEmu Framework based on ArcEmu MMORPG Server
 * Copyright (c) 2014-2025 AscEmu Team <http://www.ascemu.org>
 * Copyright (C) 2005-2010 MaNGOS <http://getmangos.com/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _BIH_H
#define _BIH_H

#include <stdexcept>
#include <vector>
#include <algorithm>
#include <limits>
#include <cmath>

#include "G3D/Vector3.h"
#include "G3D/Ray.h"
#include "G3D/AABox.h"

#define MAX_STACK_SIZE 64

static inline uint32_t floatToRawIntBits(float f)
{
    union
    {
        uint32_t ival;
        float fval;
    } temp;
    temp.fval=f;
    return temp.ival;
}

static inline float intBitsToFloat(uint32_t i)
{
    union
    {
        uint32_t ival;
        float fval;
    } temp;
    temp.ival=i;
    return temp.fval;
}

struct AABound
{
    G3D::Vector3 lo, hi;
};

/** Bounding Interval Hierarchy Class.
    Building and Ray-Intersection functions based on BIH from
    Sunflow, a Java Raytracer, released under MIT/X11 License
    http://sunflow.sourceforge.net/
    Copyright (c) 2003-2007 Christopher Kulla
*/

class BIH
{
    private:
        void init_empty()
        {
            tree.clear();
            objects.clear();
            // create space for the first node
            tree.push_back(3u << 30u); // dummy leaf
            tree.insert(tree.end(), 2, 0);
        }
    public:
        BIH() { init_empty(); }
        template< class BoundsFunc, class PrimArray >
        void build(const PrimArray &primitives, BoundsFunc &getBounds, uint32_t leafSize = 3, bool printStats=false)
        {
            if (primitives.size() == 0)
            {
                init_empty();
                return;
            }

            buildData dat;
            dat.maxPrims = leafSize;
            dat.numPrims = static_cast<uint32_t>(primitives.size());
            dat.indices = new uint32_t[dat.numPrims];
            dat.primBound = new G3D::AABox[dat.numPrims];
            getBounds(primitives[0], bounds);
            for (uint32_t i=0; i<dat.numPrims; ++i)
            {
                dat.indices[i] = i;
                getBounds(primitives[i], dat.primBound[i]);
                bounds.merge(dat.primBound[i]);
            }
            std::vector<uint32_t> tempTree;
            BuildStats stats;
            buildHierarchy(tempTree, dat, stats);
            if (printStats)
                stats.printStats();

            objects.resize(dat.numPrims);
            for (uint32_t i=0; i<dat.numPrims; ++i)
                objects[i] = dat.indices[i];
            //nObjects = dat.numPrims;
            tree = tempTree;
            delete[] dat.primBound;
            delete[] dat.indices;
        }

        uint32_t primCount() const { return static_cast<uint32_t>(objects.size()); }

        template<typename RayCallback>
        void intersectRay(const G3D::Ray &r, RayCallback& intersectCallback, float &maxDist, bool stopAtFirst=false) const
        {
            float intervalMin = -1.f;
            float intervalMax = -1.f;
            G3D::Vector3 org = r.origin();
            G3D::Vector3 dir = r.direction();
            G3D::Vector3 invDir;
            for (int i=0; i<3; ++i)
            {
                invDir[i] = 1.f / dir[i];
                if (G3D::fuzzyNe(dir[i], 0.0f))
                {
                    float t1 = (bounds.low()[i]  - org[i]) * invDir[i];
                    float t2 = (bounds.high()[i] - org[i]) * invDir[i];
                    if (t1 > t2)
                        std::swap(t1, t2);
                    if (t1 > intervalMin)
                        intervalMin = t1;
                    if (t2 < intervalMax || intervalMax < 0.f)
                        intervalMax = t2;
                    // intervalMax can only become smaller for other axis,
                    //  and intervalMin only larger respectively, so stop early
                    if (intervalMax <= 0 || intervalMin >= maxDist)
                        return;
                }
            }

            if (intervalMin > intervalMax)
                return;
            intervalMin = std::max(intervalMin, 0.f);
            intervalMax = std::min(intervalMax, maxDist);

            uint32_t offsetFront[3];
            uint32_t offsetBack[3];
            uint32_t offsetFront3[3];
            uint32_t offsetBack3[3];
            // compute custom offsets from direction sign bit

            for (int i=0; i<3; ++i)
            {
                offsetFront[i] = floatToRawIntBits(dir[i]) >> 31;
                offsetBack[i] = offsetFront[i] ^ 1;
                offsetFront3[i] = offsetFront[i] * 3;
                offsetBack3[i] = offsetBack[i] * 3;

                // avoid always adding 1 during the inner loop
                ++offsetFront[i];
                ++offsetBack[i];
            }

            StackNode stack[MAX_STACK_SIZE];
            int stackPos = 0;
            int node = 0;

            while (true) {
                while (true)
                {
                    uint32_t tn = tree[node];
                    uint32_t axis = (tn & (3 << 30)) >> 30;
                    bool BVH2 = (tn & (1 << 29)) != 0;
                    int offset = tn & ~(7 << 29);
                    if (!BVH2)
                    {
                        if (axis < 3)
                        {
                            // "normal" interior node
                            float tf = (intBitsToFloat(tree[node + offsetFront[axis]]) - org[axis]) * invDir[axis];
                            float tb = (intBitsToFloat(tree[node + offsetBack[axis]]) - org[axis]) * invDir[axis];
                            // ray passes between clip zones
                            if (tf < intervalMin && tb > intervalMax)
                                break;
                            int back = offset + offsetBack3[axis];
                            node = back;
                            // ray passes through far node only
                            if (tf < intervalMin) {
                                intervalMin = (tb >= intervalMin) ? tb : intervalMin;
                                continue;
                            }
                            node = offset + offsetFront3[axis]; // front
                            // ray passes through near node only
                            if (tb > intervalMax) {
                                intervalMax = (tf <= intervalMax) ? tf : intervalMax;
                                continue;
                            }
                            // ray passes through both nodes
                            // push back node
                            stack[stackPos].node = back;
                            stack[stackPos].tnear = (tb >= intervalMin) ? tb : intervalMin;
                            stack[stackPos].tfar = intervalMax;
                            stackPos++;
                            // update ray interval for front node
                            intervalMax = (tf <= intervalMax) ? tf : intervalMax;
                            continue;
                        }
                        else
                        {
                            // leaf - test some objects
                            int n = tree[node + 1];
                            while (n > 0) {
                                bool hit = intersectCallback(r, objects[offset], maxDist, stopAtFirst);
                                if (stopAtFirst && hit) return;
                                --n;
                                ++offset;
                            }
                            break;
                        }
                    }
                    else
                    {
                        if (axis>2)
                            return; // should not happen
                        float tf = (intBitsToFloat(tree[node + offsetFront[axis]]) - org[axis]) * invDir[axis];
                        float tb = (intBitsToFloat(tree[node + offsetBack[axis]]) - org[axis]) * invDir[axis];
                        node = offset;
                        intervalMin = (tf >= intervalMin) ? tf : intervalMin;
                        intervalMax = (tb <= intervalMax) ? tb : intervalMax;
                        if (intervalMin > intervalMax)
                            break;
                        continue;
                    }
                } // traversal loop
                do
                {
                    // stack is empty?
                    if (stackPos == 0)
                        return;
                    // move back up the stack
                    stackPos--;
                    intervalMin = stack[stackPos].tnear;
                    if (maxDist < intervalMin)
                        continue;
                    node = stack[stackPos].node;
                    intervalMax = stack[stackPos].tfar;
                    break;
                } while (true);
            }
        }

        template<typename IsectCallback>
        void intersectPoint(const G3D::Vector3 &p, IsectCallback& intersectCallback) const
        {
            if (!bounds.contains(p))
                return;

            StackNode stack[MAX_STACK_SIZE];
            int stackPos = 0;
            int node = 0;

            while (true) {
                while (true)
                {
                    uint32_t tn = tree[node];
                    uint32_t axis = (tn & (3 << 30)) >> 30;
                    bool BVH2 = (tn & (1 << 29)) != 0;
                    int offset = tn & ~(7 << 29);
                    if (!BVH2)
                    {
                        if (axis < 3)
                        {
                            // "normal" interior node
                            float tl = intBitsToFloat(tree[node + 1]);
                            float tr = intBitsToFloat(tree[node + 2]);
                            // point is between clip zones
                            if (tl < p[axis] && tr > p[axis])
                                break;
                            int right = offset + 3;
                            node = right;
                            // point is in right node only
                            if (tl < p[axis]) {
                                continue;
                            }
                            node = offset; // left
                            // point is in left node only
                            if (tr > p[axis]) {
                                continue;
                            }
                            // point is in both nodes
                            // push back right node
                            stack[stackPos].node = right;
                            stackPos++;
                            continue;
                        }
                        else
                        {
                            // leaf - test some objects
                            int n = tree[node + 1];
                            while (n > 0) {
                                intersectCallback(p, objects[offset]); // !!!
                                --n;
                                ++offset;
                            }
                            break;
                        }
                    }
                    else // BVH2 node (empty space cut off left and right)
                    {
                        if (axis>2)
                            return; // should not happen
                        float tl = intBitsToFloat(tree[node + 1]);
                        float tr = intBitsToFloat(tree[node + 2]);
                        node = offset;
                        if (tl > p[axis] || tr < p[axis])
                            break;
                        continue;
                    }
                } // traversal loop

                // stack is empty?
                if (stackPos == 0)
                    return;
                // move back up the stack
                stackPos--;
                node = stack[stackPos].node;
            }
        }

        bool writeToFile(FILE* wf) const;
        bool readFromFile(FILE* rf);

    protected:
        std::vector<uint32_t> tree;
        std::vector<uint32_t> objects;
        G3D::AABox bounds;

        struct buildData
        {
            uint32_t *indices;
            G3D::AABox *primBound;
            uint32_t numPrims;
            int maxPrims;
        };
        struct StackNode
        {
            uint32_t node;
            float tnear;
            float tfar;
        };

        class BuildStats
        {
            private:
                int numNodes;
                int numLeaves;
                int sumObjects;
                int minObjects;
                int maxObjects;
                int sumDepth;
                int minDepth;
                int maxDepth;
                int numLeavesN[6];
                int numBVH2;

            public:
            BuildStats():
                numNodes(0), numLeaves(0), sumObjects(0), minObjects(0x0FFFFFFF),
                maxObjects(0xFFFFFFFF), sumDepth(0), minDepth(0x0FFFFFFF),
                maxDepth(0xFFFFFFFF), numBVH2(0)
            {
                for (int i=0; i<6; ++i) numLeavesN[i] = 0;
            }

            void updateInner() { numNodes++; }
            void updateBVH2() { numBVH2++; }
            void updateLeaf(int depth, int n);
            void printStats();
        };

        void buildHierarchy(std::vector<uint32_t> &tempTree, buildData &dat, BuildStats &stats);

        void createNode(std::vector<uint32_t> &tempTree, int nodeIndex, uint32_t left, uint32_t right) const
        {
            // write leaf node
            tempTree[nodeIndex + 0] = (3 << 30) | left;
            tempTree[nodeIndex + 1] = right - left + 1;
        }

        void subdivide(int left, int right, std::vector<uint32_t> &tempTree, buildData &dat, AABound &gridBox, AABound &nodeBox, int nodeIndex, int depth, BuildStats &stats);
};

#endif // _BIH_H
