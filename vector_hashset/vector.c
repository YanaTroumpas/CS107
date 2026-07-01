#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include <search.h>
#include <string.h>
#include <assert.h>

void VectorNew(vector *v, int elemSize, VectorFreeFunction freeFn, int initialAllocation)
{
    assert(initialAllocation > 0);

    v->curr_len = 0;
    v->elem_size = elemSize;
    v->freefn = freeFn;
    v->init_alloc = initialAllocation != 0 ? initialAllocation : 2;
    v->alloc_len = v->init_alloc;
    v->elems = malloc( v->alloc_len * elemSize);
}

void VectorDispose(vector *v)
{
    if(v->freefn != NULL){
        for(int i = 0; i < v->curr_len; i++){
            v->freefn((char*) v->elems + i * v->elem_size);
        }
    }
    free(v->elems);
}

int VectorLength(const vector *v)
{ return v->curr_len; }

void *VectorNth(const vector *v, int position)
{ 
    assert(position >= 0);
    assert(position < v->curr_len);
    void * elem = (char*) v->elems + position * v->elem_size;
    return elem; }

void VectorReplace(vector *v, const void *elemAddr, int position)
{
    assert(position >= 0);
    assert(position <= v->curr_len);

    void * src = (char*) v->elems + position * v->elem_size;
    if(v->freefn != NULL){
        v->freefn(src);
    }

    memcpy(src, elemAddr, v->elem_size);
}

void VectorInsert(vector *v, const void *elemAddr, int position)
{
    assert(position >= 0);
    assert(position <= v->curr_len);

    if(v->curr_len == v->alloc_len){ // reallocation needed
        v->alloc_len *= v->init_alloc;
        v->elems = realloc(v->elems, v->alloc_len * v->elem_size);
    }
    
    void* src = (char*) v->elems + position * v->elem_size;
    if(v->curr_len != 0){
        void* dest = (char*) v->elems + (position + 1) * v->elem_size;
        memmove(dest, src, (v->curr_len - position) * v->elem_size);
    }
    
    memcpy(src, elemAddr, v->elem_size);
    v->curr_len++;

}

void VectorAppend(vector *v, const void *elemAddr)
{
    if(v->curr_len == v->alloc_len){ // re-allocation needed
        v->alloc_len *= v->init_alloc;
        v->elems = realloc(v->elems, v->alloc_len * v->elem_size);
    }
    
    void* target = (char*) v->elems + v->curr_len * v->elem_size;
    memcpy(target, elemAddr, v->elem_size);
    v->curr_len++;
}

void VectorDelete(vector *v, int position)
{
    assert(position >= 0);
    assert(position <= v->curr_len);

    if(v->freefn != NULL){
        v->freefn((char*) v->elems + position * v->elem_size);
    }

    void* dest = (char*) v->elems + position * v->elem_size;
    void* src = (char*) v->elems + (position + 1) * v->elem_size;
    memmove(dest, src, (v->curr_len - position) * v->elem_size);
    v->curr_len--;
}

void VectorSort(vector *v, VectorCompareFunction compare)
{
    assert(compare != NULL);
    qsort(v->elems, v->curr_len, v->elem_size, compare);
}

void VectorMap(vector *v, VectorMapFunction mapFn, void *auxData)
{
    assert(mapFn != NULL);

    for(int i = 0; i < v->curr_len; i++){
        mapFn((char*) v->elems + i * v->elem_size, auxData);
    }
}

static const int kNotFound = -1;
int VectorSearch(const vector *v, const void *key, VectorCompareFunction searchFn, int startIndex, bool isSorted)
{ 
    assert(startIndex >= 0);
    assert(startIndex <= v->curr_len);
    assert(searchFn != NULL);
    assert(key != NULL);

    void* base = (char*) v->elems + startIndex * v->elem_size;
    size_t len = v->curr_len - startIndex;

    void* found;
    if(isSorted){
        found = bsearch(key, base, len, v->elem_size, searchFn);
    }else{
        found = lfind(key, base, &len, v->elem_size, searchFn);
    }

    if(!found){
        return kNotFound;
    }

    int position = ((char*) found - (char*) v->elems) / v->elem_size;
    return position;
 } 
