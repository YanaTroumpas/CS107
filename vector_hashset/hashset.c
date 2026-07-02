#include "hashset.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

void HashSetNew(hashset *h, int elemSize, int numBuckets,
		HashSetHashFunction hashfn, HashSetCompareFunction comparefn, HashSetFreeFunction freefn)
{
	assert(elemSize > 0);
	assert(numBuckets > 0);
	assert(hashfn != 0);
	assert(comparefn != 0);

	h->elem_size = elemSize;
	h->num_buckets = numBuckets;
	h->hashfn = hashfn;
	h->comparefn = comparefn;
	h->freefn = freefn;

	h->buckets = malloc(numBuckets * sizeof(vector));
	for(int i = 0; i < numBuckets; ++i){
		VectorNew(&h->buckets[i], elemSize, freefn, 4);
	}
}

void HashSetDispose(hashset *h)
{
	if(h->freefn != NULL){
		for(int i = 0; i < h->num_buckets; i++){
			VectorDispose(&h->buckets[i]);
		}
	}
	free(h->buckets);
}

int HashSetCount(const hashset *h)
{ 
	int count = 0;
	for(int i = 0; i < h->num_buckets; i++){
		count +=VectorLength(&h->buckets[i]);
	};
	return count;
}

void HashSetMap(hashset *h, HashSetMapFunction mapfn, void *auxData)
{
	assert(mapfn != 0);
	for(int i = 0; i < h->num_buckets; i++){
		VectorMap(&h->buckets[i], mapfn, auxData);
	}
}

void HashSetEnter(hashset *h, const void *elemAddr)
{
	assert(elemAddr != NULL);

	int num_bucket = h->hashfn(elemAddr, h->num_buckets);
	assert(num_bucket >= 0);
	assert(num_bucket < h->num_buckets);

	int found = VectorSearch(&h->buckets[num_bucket], elemAddr, h->comparefn, 0, false);
	if(found < 0){
		VectorAppend(&h->buckets[num_bucket], elemAddr);
	}else{
		VectorReplace(&h->buckets[num_bucket], elemAddr, found);
	}
}

void *HashSetLookup(const hashset *h, const void *elemAddr)
{ 
	assert(elemAddr != NULL);

	int num_bucket = h->hashfn(elemAddr, h->num_buckets);
	assert(num_bucket >= 0);
	assert(num_bucket < h->num_buckets);

	int found = VectorSearch(&h->buckets[num_bucket], elemAddr, h->comparefn, 0, false);
	if(found < 0){
		return NULL;
	}
	
	return VectorNth(&h->buckets[num_bucket], found);
}
