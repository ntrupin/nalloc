//
//  allocator.c
//
//  Created by Noah Trupin on 10/13/20.
//

#include "nalloc.h"

Allocator *_Nonnull allocator;

void *_Nullable nmalloc(size_t bytes) {
#define a allocator
    size_t units = (bytes + sizeof(MemLeaf) - 1) / sizeof(MemLeaf) + 1;
    a->allocations += 1;
    a->size += units;
    MemLeaf *p;
    MemLeaf *prev = a->free;
    if (!prev) {
        a->blocks = (MemLeaf) {
            .data = {
                .size = 0,
                .next = NULL
            }
        };
        a->free = &(a->blocks);
        prev = &(a->blocks);
        a->blocks.data.next = &(a->blocks);
    }
    for (p=prev->data.next;;prev=p,p=p->data.next) {
        if (p->data.size >= units) {
            if (p->data.size == units) {
                prev->data.next = p->data.next;
            } else {
                p->data.size -= units;
                p += p->data.size;
                p->data.size = units;
            }
            a->free = prev;
            return (void *)(p+1);
        }
        if (p == a->free) {
            p = nsbrk(units);
            if (!p) {
                return NULL;
            }
        }
    }
#undef a
}

void *_Nullable nsbrk(size_t bytes) {
#define a allocator
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    a->extensions += 1;
    void *res;
    MemLeaf *ext;
    size_t units = bytes < MINREQ ? MINREQ : bytes;
    res = sbrk((int)(units * sizeof(MemLeaf)));
    if (res == (void *) -1) {
        return NULL;
    }
    ext = res;
    ext->data.size = units;
    nfree((void *)(ext+1));
    return a->free;
#pragma clang diagnostic pop
#undef a
}

void nfree(void *ap) {
#define a allocator
    a->frees += 1;
    MemLeaf *p;
    MemLeaf *bp = (MemLeaf *)ap-1;
    for (p=a->free;!(bp>p&&bp<p->data.next);p=p->data.next) {
        if (p >= p->data.next && (bp > p || bp < p->data.next)) {
            break;
        }
    }
    if (bp + bp->data.size == p->data.next) {
        bp->data.size += p->data.next->data.size;
        bp->data.next = p->data.next->data.next;
    } else {
        bp->data.next = p->data.next;
    }
    if (p + p->data.size == bp) {
        p->data.size += bp->data.size;
        p->data.next = bp->data.next;
    } else {
        p->data.next = bp;
    }
    a->free = p;
#undef a
}

void *_Nullable nrealloc(void *_Nullable ap, size_t n) {
    void *new;
    if (!ap) {
        new = nmalloc(n);
    } else {
        MemLeaf *bp = (MemLeaf *)ap-1;
        if (bp->data.size < n) {
            new = nmalloc(n);
            memcpy(new, ap, bp->data.size * sizeof(ap));
            nfree(ap);
        } else {
            new = ap;
        }
    }
    return new;
}

void allocator_init() {
    Allocator *a = malloc(sizeof(Allocator));
    a->free = NULL;
    a->allocations = 0;
    a->frees = 0;
    a->extensions = 0;
    a->size = 0;
    allocator = a;
}

void allocator_free() {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
    if (!allocator) return;
    void *res;
    res = sbrk((int)((allocator->size < MINREQ ? MINREQ : allocator->size) * -1));
    if (res == (void *) -1) {
        fprintf(stderr, "Error freeing Allocator");
    }
    free(allocator);
#pragma clang diagnostic pop
}

void allocator_diagnostic() {
#define a allocator
    printf(
        "The Allocator performed\n- %zu allocation%c\n- %zu extension%c\n- %zu free%c\nMax size: %zu byte%c\n",
           a->allocations, ABS(a->allocations) == 1 ? ' ' : 's',
           a->extensions, ABS(a->extensions) == 1 ? ' ' : 's',
           a->frees, ABS(a->frees) == 1 ? ' ' : 's',
           a->size, ABS(a->size) == 1 ? ' ' : 's'
    );
#undef a
}
