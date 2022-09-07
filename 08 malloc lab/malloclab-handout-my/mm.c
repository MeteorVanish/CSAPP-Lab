// Segregated Free List + Best Fit + Immediate Coalesce

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/**********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please  *
 * provide your team information in the following struct. *
 **********************************************************/
team_t team = {
    /* Team name */
    "Yosoro!",
    /* First member's full name */
    "MeteorVanish",
    /* First member's email address */
    "Zhenyu_Wei@sjtu.edu.cn",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};



/* ========================= Macros ========================= */

#define SEG_NUM     15 // Odd Number to Align

#define FREE        0
#define ALLOCATED   1

#define WSIZE       4
#define DSIZE       8
#define CHUNKSIZE   (1 << 8)

#define MAX(x, y)   ((x) > (y) ? (x) : (y))
#define MIN(x, y)   ((x) < (y) ? (x) : (y))

#define PACK(size, alloc)   ((size) | (alloc))

#define GET(ptr)            (*(unsigned int *)(ptr))
#define PUT(ptr, val)       (*(unsigned int *)(ptr) = (unsigned int)(val))

#define GET_SIZE(ptr)       (GET(ptr) & ~(DSIZE - 1))
#define GET_ALLOC(ptr)      (GET(ptr) & 0x1)

#define HDRP(bp)            ((char *)(bp) - WSIZE)
#define FTRP(bp)            ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

#define PRED_POS(bp)        ((char *)(bp) + WSIZE)
#define SUCC_POS(bp)        ((char *)(bp))
#define PRED(bp)            ((char *)GET(PRED_POS(bp)))
#define SUCC(bp)            ((char *)GET(SUCC_POS(bp)))

#define PREV(bp)       ((char *)(bp) - GET_SIZE((char *)(bp) - DSIZE))
#define NEXT(bp)       ((char *)(bp) + GET_SIZE((char *)(bp) - WSIZE))

/* ========================= Function Declarations ========================= */

static void *Coalesce(void *);
static void *ExtendHeap(size_t);
static void *FindFit(size_t, int);
static void *PlaceBlock(void *, size_t);
static void InsertFreeBlock(void *);
static void DeleteFreeBlock(void *);
static int GetSegIndex(size_t);
static size_t AdjustSize(size_t);

/* ========================= Core Functions ========================= */

/* 
 * mm_init - Initialize the heap and first extend.
 */
int mm_init(void)
{
	char *HeapHeaderPtr;
    if ((HeapHeaderPtr = mem_sbrk((SEG_NUM + 3) * WSIZE)) == (void *)-1)
        return -1;
    
    // Assign Segrgated Free List Header
    for (int i = 0; i < SEG_NUM; ++i)
        PUT(HeapHeaderPtr + i * WSIZE, NULL);

    // Assign Heap Header and Footer Block
    PUT(HeapHeaderPtr + (SEG_NUM + 0) * WSIZE, PACK(DSIZE, ALLOCATED));
    PUT(HeapHeaderPtr + (SEG_NUM + 1) * WSIZE, PACK(DSIZE, ALLOCATED));
    PUT(HeapHeaderPtr + (SEG_NUM + 2) * WSIZE, PACK(0, ALLOCATED));

    // First Extend
    if (ExtendHeap(CHUNKSIZE) == NULL)
        return -1;
    
    return 0;
}

/* 
 * mm_malloc - Get malloc place from FindFit or ExtendHeap, and place bp.
 */
void *mm_malloc(size_t size)
{
    if (!size) return NULL;
    
    size_t asize = AdjustSize(size);
    char *bp;
    
    // Get malloc place
    if ((bp = FindFit(asize, GetSegIndex(asize))) == NULL && (bp = ExtendHeap(MAX(asize, CHUNKSIZE))) == NULL)
        return NULL;
    return PlaceBlock(bp, asize);
}

/*
 * mm_free - Change the info of block and coalesce.
 */
void mm_free(void *ptr)
{
    size_t size = GET_SIZE(HDRP(ptr));
    
    PUT(HDRP(ptr), PACK(size, FREE));
    PUT(FTRP(ptr), PACK(size, FREE));
    Coalesce(ptr);
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
	// Special Condition
    if (ptr == NULL)
        return mm_malloc(size);
    else if (size == 0) {
        mm_free(ptr);
        return NULL;
    }
    
    size_t asize = AdjustSize(size);
    size_t OldSize = GET_SIZE(HDRP(ptr));
    if (asize <= OldSize) 
        return PlaceBlock(ptr, asize);

    char *next = NEXT(ptr);
    size_t NextSize = GET_SIZE(HDRP(next));
    size_t NextAlloc = GET_ALLOC(HDRP(next));
    if (NextAlloc == FREE && OldSize + NextSize >= asize) { // merge next block
        DeleteFreeBlock(next);
        PUT(HDRP(ptr), PACK(OldSize + NextSize, ALLOCATED));
        PUT(FTRP(ptr), PACK(OldSize + NextSize, ALLOCATED));
        PlaceBlock(ptr, asize);
        return ptr;
    }
    else if (!NextSize) { // last block
        if ((int)(mem_sbrk(asize - OldSize)) == -1)
            return NULL;

        PUT(HDRP(ptr), PACK(asize, ALLOCATED));
        PUT(FTRP(ptr), PACK(asize, ALLOCATED));
        PUT(HDRP(NEXT(ptr)), PACK(0, ALLOCATED));
        return ptr;
    }
    else {
        char *newptr;
        if ((newptr = mm_malloc(size)) == NULL)
            return NULL;
        memcpy(newptr, ptr, MIN(size, OldSize));
        mm_free(ptr);
        return newptr;
    }
}

/* ========================= Auxiliary Functions ========================= */

// merge the block with neighbor if possible, and insert to the corresponding list
static void *Coalesce(void *bp)
{
    char *prev = PREV(bp), *next = NEXT(bp);
    size_t size = GET_SIZE(HDRP(bp));
    size_t PrevAlloc = GET_ALLOC(HDRP(prev));
    size_t NextAlloc = GET_ALLOC(HDRP(next));

    if (PrevAlloc == FREE && NextAlloc == ALLOCATED) {
        size += GET_SIZE(HDRP(prev));
        DeleteFreeBlock(prev);
        bp = prev;
        PUT(HDRP(bp), PACK(size, FREE));
        PUT(FTRP(bp), PACK(size, FREE));
    }
    else if (PrevAlloc == ALLOCATED && NextAlloc == FREE) {
        size += GET_SIZE(HDRP(next));
        DeleteFreeBlock(next);
        PUT(HDRP(bp), PACK(size, FREE));
        PUT(FTRP(bp), PACK(size, FREE));
    }
    else if (PrevAlloc == FREE && NextAlloc == FREE) {
        size += GET_SIZE(HDRP(prev)) + GET_SIZE(HDRP(next));
        DeleteFreeBlock(prev);
        DeleteFreeBlock(next);
        bp = prev;
        PUT(HDRP(bp), PACK(size, FREE));
        PUT(FTRP(bp), PACK(size, FREE));
    }
    PUT(PRED_POS(bp), NULL);
    PUT(SUCC_POS(bp), NULL);
    InsertFreeBlock(bp);
    return bp;
}

// get extra heap space
static void *ExtendHeap(size_t size)
{
    char *bp;

    if ((int)(bp = mem_sbrk(size)) == -1) return NULL;
    PUT(HDRP(bp), PACK(size, FREE));
    PUT(FTRP(bp), PACK(size, FREE));
    PUT(HDRP(NEXT(bp)), PACK(0, ALLOCATED));

    return Coalesce(bp);
}

// return best legal bp in list, or NULL if doesn't exist
static void *FindFit(size_t size, int SegInd)
{
    while (SegInd < SEG_NUM) {
        size_t min = 0x7fffffffu;
        char *bp = SUCC(mem_heap_lo() + SegInd * WSIZE);
        char *ret = NULL;
        while (bp) {
            size_t bpsize = GET_SIZE(HDRP(bp));
            if (bpsize >= size && bpsize < min) {
                min = bpsize;
                ret = bp;
            }
            bp = SUCC(bp);
        }
        if (ret) return ret;
        else ++SegInd;
    }
    return NULL;
}

// delete bp from list, change the block info and maybe split
static void *PlaceBlock(void *bp, size_t size)
{
    size_t BlockSize = GET_SIZE(HDRP(bp));
    size_t RemainSize = BlockSize - size;
    
    if (GET_ALLOC(HDRP(bp)) == FREE)
        DeleteFreeBlock(bp);
    if (RemainSize >= 2 * DSIZE) { // place with split block
        PUT(HDRP(bp), PACK(RemainSize, FREE));
        PUT(FTRP(bp), PACK(RemainSize, FREE));
        PUT(HDRP(NEXT(bp)), PACK(size, ALLOCATED));
        PUT(FTRP(NEXT(bp)), PACK(size, ALLOCATED));
        Coalesce(bp);
        return NEXT(bp);
    }
    else { // simply place
        PUT(HDRP(bp), PACK(BlockSize, ALLOCATED));
        PUT(FTRP(bp), PACK(BlockSize, ALLOCATED));
        return bp;
    }
}

// insert bp to certain list
static void InsertFreeBlock(void *bp)
{
    int SegInd = GetSegIndex(GET_SIZE(HDRP(bp)));
    char *head = mem_heap_lo() + SegInd * WSIZE;
    char *succ = SUCC(head);
    PUT(SUCC_POS(bp), succ);
    PUT(PRED_POS(bp), head);
    PUT(SUCC_POS(head), bp);
    if (succ != NULL) // the list is not empty
        PUT(PRED_POS(succ), bp);
}

// delete bp from certain list
static void DeleteFreeBlock(void *bp)
{
    char *pred = PRED(bp), *succ = SUCC(bp);
    PUT(SUCC_POS(pred), succ);
    if (succ != NULL) // not the last block of list
        PUT(PRED_POS(succ), pred);
}

// return the segregated list index
static int GetSegIndex(size_t size)
{
    size_t log2 = 0, shift;

    // get log2(size) with O(1) method
    shift = (size > 0xFFFF) << 4; size >>= shift; log2 |= shift;
    shift = (size > 0xFF)   << 3; size >>= shift; log2 |= shift;
    shift = (size > 0b1111) << 2; size >>= shift; log2 |= shift;
    shift = (size > 0b11)   << 1; size >>= shift; log2 |= shift;
    log2 |= (size >> 1);
    
    // least block size = 16 (1 << 4)
    int ret = (int)log2 - 4;
    if (ret < 0)
        ret = 0;
    if (ret >= SEG_NUM)
        ret = SEG_NUM - 1;
    return ret;
}

// align size to DSIZE and plus DSIZE (block header and footer)
static size_t AdjustSize(size_t size)
{
	return DSIZE + ((size + DSIZE - 1) & ~(DSIZE - 1));
}
