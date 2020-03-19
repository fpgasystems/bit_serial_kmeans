/*
 * Copyright (c) 2019, Systems Group, ETH Zurich
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <iostream>
#include <random>
#include <stdio.h>
#include <string.h>
#include <chrono>

template <class T, class T2>
class Kmeans {

public:
   Kmeans(T* points, uint32_t size, uint32_t dimensions, uint32_t k);
   ~Kmeans();

   void run(uint32_t iter);
   T2 getSSE();
   double getRuntime();
   void printCentroids();
   
private:
   void assignment();
   void update();
   void initCentroids();
   T2 euclideanDist(T* p1, T* p2);
   
   T* mPoints;
   T* mCentroids;
   T* mAccu;
   uint32_t* mAssigned;
   uint32_t mSize;
   uint32_t mClusters;
   uint32_t mDimensions;
   double   mDurationUs;
};

template <class T, class T2>
Kmeans<T,T2>::Kmeans(T* points, uint32_t size, uint32_t dimensions, uint32_t k)
{
   mPoints = points;
   mSize = size;
   mClusters = k;
   mDimensions = dimensions;

   mCentroids = new T[k*mDimensions];
   mAccu = new T[k*mDimensions];
   mAssigned = new uint32_t[k];
}

template <class T, class T2>
Kmeans<T,T2>::~Kmeans()
{
   delete[] mCentroids;
   delete[] mAccu;
   delete[] mAssigned;
}

template <class T, class T2>
void Kmeans<T,T2>::run(uint32_t iterations)
{
   initCentroids();
   auto start_time = std::chrono::high_resolution_clock::now();
   for (uint32_t it = 0; it < iterations; ++it) {
      memset(mAccu, 0.0, mClusters*mDimensions*sizeof(T));
      memset(mAssigned, 0, mClusters*sizeof(uint32_t));
      assignment();
      update();
   }
   auto end_time = std::chrono::high_resolution_clock::now();
   mDurationUs = std::chrono::duration_cast<std::chrono::microseconds>(end_time-start_time).count();

}

template <class T, class T2>
T2 Kmeans<T,T2>::getSSE()
{
   T2 sse = 0.0;

   for(uint32_t p = 0; p < (mSize*mDimensions); p += mDimensions) {
      T2 minDist = 0.0;
		for (uint32_t c = 0; c < (mClusters*mDimensions); c += mDimensions) {
			T dist = euclideanDist(&mPoints[p], &mCentroids[c]);
			if (c == 0 || dist <= minDist) {
				minDist = dist;
			}
		}

      sse += minDist;
      int ind = p/mDimensions;
     //printf("[%d]sse:%d\n",ind, sse);
  }

   return sse;
}

template <class T, class T2>
double Kmeans<T,T2>::getRuntime()
{
   return mDurationUs;
}

template <class T, class T2>
void Kmeans<T,T2>::printCentroids()
{
   std::cout << "Centroids:" << std::endl;
   for (uint32_t c = 0; c < mClusters; ++c) {
      std::cout << "centroid[" << c << "]: ";
      for (uint32_t d = 0; d < mDimensions; ++d) {
         std::cout << " " << mCentroids[c*mDimensions+d];
      }
      std::cout << std::endl;
   }
}

template <class T, class T2>
void Kmeans<T,T2>::assignment()
{
	for(uint32_t p = 0; p < (mSize*mDimensions); p += mDimensions) {
		T2 minDist = 0.0;
		uint32_t clusterIdx = 0;
		for (uint32_t c = 0; c < mClusters; ++c) {
			T2 dist = euclideanDist(&mPoints[p], &mCentroids[c*mDimensions]);
			if (c == 0 || dist <= minDist) {
				minDist = dist;
				clusterIdx = c;
			}
		}
		int ind = p/mDimensions;
		//printf("[%d]assign:%d\n",ind, clusterIdx);
	//printf("[%d]mindist:%d\n",ind, minDist);
	//Accumulate
		for (uint32_t d = 0; d < mDimensions; ++d) {
			mAccu[clusterIdx*mDimensions + d] += mPoints[p + d];
		}
		mAssigned[clusterIdx]++;
	}
}

template <class T, class T2>
void Kmeans<T,T2>::update()
{
//   printf("updated center:\n");
   for (uint32_t c = 0; c < mClusters; ++c) {
      for (uint32_t d = 0; d < mDimensions; ++d) {
         if (mAssigned[c] != 0) {
            mCentroids[c*mDimensions+d] = mAccu[c*mDimensions+d] / mAssigned[c];
//	    printf("%d ", mCentroids[c*mDimensions+d]);
         }
      }
//	printf("\n");
   }
	
}

template <class T, class T2>
void Kmeans<T,T2>::initCentroids()

{
        int indx_array[8] = {0, 70, 149, 35, 105, 17, 50, 85};

   std::default_random_engine generator;
   std::uniform_int_distribution<int> distribution(0, mSize);
   for (uint32_t c = 0; c < mClusters; ++c) {
     // int idx = distribution(generator);
	int idx = indx_array[c];
      for (uint32_t d = 0; d < mDimensions; ++d) {
         mCentroids[c*mDimensions+d] = mPoints[idx*mDimensions+d];
      }
   }
}

template <class T, class T2>
T2 Kmeans<T,T2>::euclideanDist(T* p1, T* p2)
{
	T2 dist = 0.0;
   for (uint32_t d = 0; d < mDimensions; ++d) {
      T diff = (p1[d] > p2[d]) ? (p1[d] - p2[d]) : (p2[d] - p1[d]);
      dist += (diff * diff);
      //dist += ((p1[d] - p2[d]) * (p1[d] - p2[d]));
   }
   return dist;
}
