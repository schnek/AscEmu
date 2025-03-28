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

#include "BoundingIntervalHierarchy.h"

void BIH::buildHierarchy(std::vector<uint32_t> &tempTree, buildData &dat, BuildStats &stats)
{
    // create space for the first node
    tempTree.push_back(uint32_t(3 << 30)); // dummy leaf
    tempTree.insert(tempTree.end(), 2, 0);
    //tempTree.add(0);

    // seed bbox
    AABound gridBox = { bounds.low(), bounds.high() };
    AABound nodeBox = gridBox;
    // seed subdivide function
    subdivide(0, dat.numPrims - 1, tempTree, dat, gridBox, nodeBox, 0, 1, stats);
}

void BIH::subdivide(int left, int right, std::vector<uint32_t> &tempTree, buildData &dat, AABound &gridBox, AABound &nodeBox, int nodeIndex, int depth, BuildStats &stats)
{
    if ((right - left + 1) <= dat.maxPrims || depth >= MAX_STACK_SIZE)
    {
        // write leaf node
        stats.updateLeaf(depth, right - left + 1);
        createNode(tempTree, nodeIndex, left, right);
        return;
    }
    // calculate extents
    int axis = -1, prevAxis, rightOrig;
    float clipL = G3D::fnan(), clipR = G3D::fnan(), prevClip = G3D::fnan();
    float split = G3D::fnan(), prevSplit;
    bool wasLeft = true;
    while (true)
    {
        prevAxis = axis;
        prevSplit = split;
        // perform quick consistency checks
        G3D::Vector3 d( gridBox.hi - gridBox.lo );
        if (d.x < 0 || d.y < 0 || d.z < 0)
            throw std::logic_error("negative node extents");
        for (int i = 0; i < 3; i++)
        {
            if (nodeBox.hi[i] < gridBox.lo[i] || nodeBox.lo[i] > gridBox.hi[i])
            {
                //UI.printError(Module.ACCEL, "Reached tree area in error - discarding node with: %d objects", right - left + 1);
                throw std::logic_error("invalid node overlap");
            }
        }
        // find longest axis
        axis = d.primaryAxis();
        split = 0.5f * (gridBox.lo[axis] + gridBox.hi[axis]);
        // partition L/R subsets
        clipL = -G3D::finf();
        clipR = G3D::finf();
        rightOrig = right; // save this for later
        float nodeL = G3D::finf();
        float nodeR = -G3D::finf();
        for (int i = left; i <= right;)
        {
            int obj = dat.indices[i];
            float minb = dat.primBound[obj].low()[axis];
            float maxb = dat.primBound[obj].high()[axis];
            float center = (minb + maxb) * 0.5f;
            if (center <= split)
            {
                // stay left
                i++;
                if (clipL < maxb)
                    clipL = maxb;
            }
            else
            {
                // move to the right most
                int t = dat.indices[i];
                dat.indices[i] = dat.indices[right];
                dat.indices[right] = t;
                right--;
                if (clipR > minb)
                    clipR = minb;
            }
            nodeL = std::min(nodeL, minb);
            nodeR = std::max(nodeR, maxb);
        }
        // check for empty space
        if (nodeL > nodeBox.lo[axis] && nodeR < nodeBox.hi[axis])
        {
            float nodeBoxW = nodeBox.hi[axis] - nodeBox.lo[axis];
            float nodeNewW = nodeR - nodeL;
            // node box is too big compare to space occupied by primitives?
            if (1.3f * nodeNewW < nodeBoxW)
            {
                stats.updateBVH2();
                int nextIndex = static_cast<int>(tempTree.size());
                // allocate child
                tempTree.push_back(0);
                tempTree.push_back(0);
                tempTree.push_back(0);
                // write bvh2 clip node
                stats.updateInner();
                tempTree[nodeIndex + 0] = (axis << 30) | (1 << 29) | nextIndex;
                tempTree[nodeIndex + 1] = floatToRawIntBits(nodeL);
                tempTree[nodeIndex + 2] = floatToRawIntBits(nodeR);
                // update nodebox and recurse
                nodeBox.lo[axis] = nodeL;
                nodeBox.hi[axis] = nodeR;
                subdivide(left, rightOrig, tempTree, dat, gridBox, nodeBox, nextIndex, depth + 1, stats);
                return;
            }
        }
        // ensure we are making progress in the subdivision
        if (right == rightOrig)
        {
            // all left
            if (prevAxis == axis && G3D::fuzzyEq(prevSplit, split)) {
                // we are stuck here - create a leaf
                stats.updateLeaf(depth, right - left + 1);
                createNode(tempTree, nodeIndex, left, right);
                return;
            }
            if (clipL <= split) {
                // keep looping on left half
                gridBox.hi[axis] = split;
                prevClip = clipL;
                wasLeft = true;
                continue;
            }
            gridBox.hi[axis] = split;
            prevClip = G3D::fnan();
        }
        else if (left > right)
        {
            // all right
            right = rightOrig;
            if (prevAxis == axis && G3D::fuzzyEq(prevSplit, split)) {
                // we are stuck here - create a leaf
                stats.updateLeaf(depth, right - left + 1);
                createNode(tempTree, nodeIndex, left, right);
                return;
            }
            if (clipR >= split) {
                // keep looping on right half
                gridBox.lo[axis] = split;
                prevClip = clipR;
                wasLeft = false;
                continue;
            }
            gridBox.lo[axis] = split;
            prevClip = G3D::fnan();
        }
        else
        {
            // we are actually splitting stuff
            if (prevAxis != -1 && !std::isnan(prevClip))
            {
                // second time through - lets create the previous split
                // since it produced empty space
                int nextIndex = static_cast<int>(tempTree.size());
                // allocate child node
                tempTree.push_back(0);
                tempTree.push_back(0);
                tempTree.push_back(0);
                if (wasLeft) {
                    // create a node with a left child
                    // write leaf node
                    stats.updateInner();
                    tempTree[nodeIndex + 0] = (prevAxis << 30) | nextIndex;
                    tempTree[nodeIndex + 1] = floatToRawIntBits(prevClip);
                    tempTree[nodeIndex + 2] = floatToRawIntBits(G3D::finf());
                } else {
                    // create a node with a right child
                    // write leaf node
                    stats.updateInner();
                    tempTree[nodeIndex + 0] = (prevAxis << 30) | (nextIndex - 3);
                    tempTree[nodeIndex + 1] = floatToRawIntBits(-G3D::finf());
                    tempTree[nodeIndex + 2] = floatToRawIntBits(prevClip);
                }
                // count stats for the unused leaf
                depth++;
                stats.updateLeaf(depth, 0);
                // now we keep going as we are, with a new nodeIndex:
                nodeIndex = nextIndex;
            }
            break;
        }
    }
    // compute index of child nodes
    int nextIndex = static_cast<int>(tempTree.size());
    // allocate left node
    int nl = right - left + 1;
    int nr = rightOrig - (right + 1) + 1;
    if (nl > 0) {
        tempTree.push_back(0);
        tempTree.push_back(0);
        tempTree.push_back(0);
    } else
        nextIndex -= 3;
    // allocate right node
    if (nr > 0) {
        tempTree.push_back(0);
        tempTree.push_back(0);
        tempTree.push_back(0);
    }
    // write leaf node
    stats.updateInner();
    tempTree[nodeIndex + 0] = (axis << 30) | nextIndex;
    tempTree[nodeIndex + 1] = floatToRawIntBits(clipL);
    tempTree[nodeIndex + 2] = floatToRawIntBits(clipR);
    // prepare L/R child boxes
    AABound gridBoxL(gridBox), gridBoxR(gridBox);
    AABound nodeBoxL(nodeBox), nodeBoxR(nodeBox);
    gridBoxL.hi[axis] = gridBoxR.lo[axis] = split;
    nodeBoxL.hi[axis] = clipL;
    nodeBoxR.lo[axis] = clipR;
    // recurse
    if (nl > 0)
        subdivide(left, right, tempTree, dat, gridBoxL, nodeBoxL, nextIndex, depth + 1, stats);
    else
        stats.updateLeaf(depth + 1, 0);
    if (nr > 0)
        subdivide(right + 1, rightOrig, tempTree, dat, gridBoxR, nodeBoxR, nextIndex + 3, depth + 1, stats);
    else
        stats.updateLeaf(depth + 1, 0);
}

bool BIH::writeToFile(FILE* wf) const
{
    uint32_t treeSize = static_cast<uint32_t>(tree.size());
    size_t check = 0;
    check += fwrite(&bounds.low(), sizeof(float), 3, wf);
    check += fwrite(&bounds.high(), sizeof(float), 3, wf);
    check += fwrite(&treeSize, sizeof(uint32_t), 1, wf);
    check += fwrite(&tree[0], sizeof(uint32_t), treeSize, wf);
    uint32_t count = static_cast<uint32_t>(objects.size());
    check += fwrite(&count, sizeof(uint32_t), 1, wf);
    check += fwrite(&objects[0], sizeof(uint32_t), count, wf);
    return (check == (3 + 3 + 2 + treeSize + count));
}

bool BIH::readFromFile(FILE* rf)
{
    uint32_t treeSize;
    G3D::Vector3 lo, hi;
    uint32_t check = 0, count = 0;
    check += static_cast<uint32_t>(fread(&lo, sizeof(float), 3, rf));
    check += static_cast<uint32_t>(fread(&hi, sizeof(float), 3, rf));
    bounds = G3D::AABox(lo, hi);
    check += static_cast<uint32_t>(fread(&treeSize, sizeof(uint32_t), 1, rf));
    tree.resize(treeSize);
    check += static_cast<uint32_t>(fread(&tree[0], sizeof(uint32_t), treeSize, rf));
    check += static_cast<uint32_t>(fread(&count, sizeof(uint32_t), 1, rf));
    objects.resize(count); // = new uint32_t[nObjects];
    check += static_cast<uint32_t>(fread(&objects[0], sizeof(uint32_t), count, rf));
    return (uint64_t(check) == uint64_t(3 + 3 + 1 + 1 + uint64_t(treeSize) + uint64_t(count)));
}

void BIH::BuildStats::updateLeaf(int depth, int n)
{
    numLeaves++;
    minDepth = std::min(depth, minDepth);
    maxDepth = std::max(depth, maxDepth);
    sumDepth += depth;
    minObjects = std::min(n, minObjects);
    maxObjects = std::max(n, maxObjects);
    sumObjects += n;
    int nl = std::min(n, 5);
    ++numLeavesN[nl];
}

void BIH::BuildStats::printStats()
{
    printf("Tree stats:\n");
    printf("  * Nodes:          %d\n", numNodes);
    printf("  * Leaves:         %d\n", numLeaves);
    printf("  * Objects: min    %d\n", minObjects);
    printf("             avg    %.2f\n", (float) sumObjects / numLeaves);
    printf("           avg(n>0) %.2f\n", (float) sumObjects / (numLeaves - numLeavesN[0]));
    printf("             max    %d\n", maxObjects);
    printf("  * Depth:   min    %d\n", minDepth);
    printf("             avg    %.2f\n", (float) sumDepth / numLeaves);
    printf("             max    %d\n", maxDepth);
    printf("  * Leaves w/: N=0  %3d%%\n", 100 * numLeavesN[0] / numLeaves);
    printf("               N=1  %3d%%\n", 100 * numLeavesN[1] / numLeaves);
    printf("               N=2  %3d%%\n", 100 * numLeavesN[2] / numLeaves);
    printf("               N=3  %3d%%\n", 100 * numLeavesN[3] / numLeaves);
    printf("               N=4  %3d%%\n", 100 * numLeavesN[4] / numLeaves);
    printf("               N>4  %3d%%\n", 100 * numLeavesN[5] / numLeaves);
    printf("  * BVH2 nodes:     %d (%3d%%)\n", numBVH2, 100 * numBVH2 / (numNodes + numLeaves - 2 * numBVH2));
}
