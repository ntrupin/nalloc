#ifndef nalloc_h
#define nalloc_h

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

// Misc.

#define ABS(x) (x <= 0 ? 0 - x : x)

// Allocator

typedef struct Allocator Allocator;
typedef union MemLeaf MemLeaf;

union MemLeaf {
    struct {
        size_t size;
        MemLeaf *_Nullable next;
    } data;
    long x;
};

struct Allocator {
    MemLeaf blocks;
    MemLeaf *_Nullable free;
    struct {
        size_t allocations;
        size_t extensions;
        size_t frees;
        size_t size;
    };
};

#define MINREQ 1024

void *_Nullable nmalloc(size_t);
void *_Nullable nsbrk(size_t);
void *_Nullable nrealloc(void *_Nullable, size_t);
void nfree(void *_Nullable);
void allocator_init(void);
void allocator_free(void);
void allocator_diagnostic(void);

#endif
