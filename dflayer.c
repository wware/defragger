#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #define COMMENTS_ENABLED
#include "common.h"

struct block {
    struct block *next_by_size;
    struct block *next_by_address;
    unsigned long size;
    char bytes[0];
};

#define HEADER sizeof(struct block)

#define MAX_SIZE   (3UL * 1024 * 1024 * 1024)
static char *field = NULL;
static unsigned long bytes_available;
static struct block *first_by_size;
static struct block *first_by_address;

struct region {
    struct block *previous;
    struct block *current;
};

#define CHARP(p)    ((char *) (p))
#define VOIDP(p)    ((void *) (p))
#define BLOCK(p)    ((struct block *) (p))

typedef int (*test_block_i)(struct block *, unsigned long);
typedef struct block * (*next_block)(struct block *);
typedef void (*set_next_block)(struct block *old, struct block *new);

static struct region first_block_with(struct block **head, test_block_i tb, next_block nb, unsigned long param)
{
    struct block *p, *prev = NULL;
    struct region r;
    for (p = *head; ; prev = p, p = nb(p)) {
        if (p == NULL) {
            r.previous = r.current = NULL;
            return r;
        }
        if (tb(p, param)) {
            r.previous = prev;
            r.current = p;
            return r;
        }
    }
}

/* https://twitter.com/wware/status/850087533388210176 */

int test_by_size(struct block *p, unsigned long param)
{
    return p->size > param;
}

struct block * next_by_size(struct block *p)
{
    return p->next_by_size;
}

int test_by_address_gr(struct block *p, unsigned long param)
{
    return ((unsigned long) p) > param;
}

int test_by_address_eq(struct block *p, unsigned long param)
{
    return ((unsigned long) p) == param;
}

struct block * next_by_address(struct block *p)
{
    return p->next_by_address;
}

static void set_next_by_address(struct block *old, struct block *new)
{
    if (old == NULL)
        first_by_address = new;
    else
        old->next_by_address = new;
}

static void set_next_by_size(struct block *old, struct block *new)
{
    if (old == NULL)
        first_by_size = new;
    else
        old->next_by_size = new;
}

#ifdef COMMENTS_ENABLED
static void print_block_info(struct block *p)
{
    printf("Block at %p\n", p);
    printf("Next by address: %p\n", p->next_by_address);
    printf("Next by size: %p\n", p->next_by_size);
    printf("Size: %lu\n", p->size);
}

static void print_linked_list(char *msg, struct block *head, next_block nb)
{
    printf("Printing linked list: %s\n", msg);
    for ( ; head != NULL; head = nb(head)) {
        print_block_info(head);
    }
}
#endif

static void print_lists(void)
{
#ifdef COMMENTS_ENABLED
    printf("<!--\n");
    print_linked_list("Ordered by address", first_by_address, next_by_address);
    printf("----\n");
    print_linked_list("Ordered by size", first_by_size, next_by_size);
    printf("-->\n");
#endif
}

void * dfmalloc(size_t size)
{
    void dfinit(void);
    long diff;
    struct block *q = NULL;
    struct region r = first_block_with(&first_by_size, test_by_size, next_by_size, size);
    struct region r2;

    if (field == NULL) {
        dfinit();
    }

    COMMENT1("size = %lu", size);
    print_lists();

    if (r.current == NULL) {
        COMMENT("no luck finding anything");
        return NULL;
    }

    COMMENT1("r.current->size = %lu", r.current->size);
    diff = r.current->size - size - HEADER;
    COMMENT1("diff = %lu", diff);
    if (diff > 0) {
        q = BLOCK(CHARP(r.current) + HEADER + size);
        q->size = diff;
        COMMENT2("splitting to create another block at %p with %lu bytes", q, q->size);
        bytes_available -= HEADER;
        assert(bytes_available < MAX_SIZE);
    }

    COMMENT("Remove r.current from the by-size list");
    set_next_by_size(r.previous, r.current->next_by_size);

    COMMENT("Remove r.current from the by-address list");
    r2 = first_block_with(&first_by_address, test_by_address_eq, next_by_address, (unsigned long) r.current);
    set_next_by_address(r2.previous, r2.current->next_by_address);

    print_lists();
    if (q != NULL) {
        COMMENT("Insert q into the by-size list in the right place");
        r2 = first_block_with(&first_by_size, test_by_size, next_by_size, q->size);
        set_next_by_size(q, r2.current);
        set_next_by_size(r2.previous, q);
        COMMENT("Insert q into the by-address list in the right place");
        r2 = first_block_with(&first_by_address, test_by_address_gr, next_by_address, (unsigned long) q);
        set_next_by_address(q, r2.current);
        set_next_by_address(r2.previous, q);
    }

    print_lists();
    r.current->size = size;
    bytes_available -= size;
    assert(bytes_available < MAX_SIZE);
    COMMENT2("returning a block at %p with %lu bytes", VOIDP(CHARP(r.current) + HEADER), r.current->size);
    return VOIDP(CHARP(r.current) + HEADER);
}

static int contiguous(struct block *block1, struct block *block2)
{
    char *p = (char *) block1;
    p += HEADER + block1->size;
    return p == (char*) block2;
}

void dffree(void *ptr)
{
    struct block *p, *pnext;
    struct region r, r2;
    if (ptr == NULL)
        return;
    COMMENT1("dffree(%p)", ptr);
    char *q = (char *) ptr;
    p = (struct block *) (q - HEADER);
    bytes_available += p->size;
    assert(bytes_available < MAX_SIZE);
    COMMENT("Insert p into the by-size list in the right place");
    r2 = first_block_with(&first_by_size, test_by_size, next_by_size, p->size);
    set_next_by_size(p, r2.current);
    set_next_by_size(r2.previous, p);
    COMMENT("Insert p into the by-address list in the right place");
    r2 = first_block_with(&first_by_address, test_by_address_gr, next_by_address, (unsigned long) p);
    set_next_by_address(p, r2.current);
    set_next_by_address(r2.previous, p);
    COMMENT("Step thru the by-address list, re-combining contiguous blocks");
    for (p = first_by_address; p != NULL; p = next_by_address(p)) {
        while (1) {
            pnext = p->next_by_address;
            if (!contiguous(p, pnext)) break;
            COMMENT("Blocks p and pnext are contiguous, so assimilate pnext's bytes into p");
            p->size += HEADER + pnext->size;
            bytes_available += HEADER;
            COMMENT("Remove pnext from the by-address list (easy)");
            p->next_by_address = pnext->next_by_address;
            COMMENT("Remove pnext from the by-size list");
            r = first_block_with(&first_by_size, test_by_address_eq, next_by_size, (unsigned long) pnext);
            set_next_by_size(r.previous, r.current->next_by_size);
            COMMENT("Remove p from the by-size list");
            r = first_block_with(&first_by_size, test_by_address_eq, next_by_size, (unsigned long) p);
            set_next_by_size(r.previous, r.current->next_by_size);
            COMMENT("Insert p into the by-size list in the right place");
            r = first_block_with(&first_by_size, test_by_size, next_by_size, p->size);
            set_next_by_size(p, r.current);
            set_next_by_size(r.previous, p);
        }
    }
}

void *dfrealloc(void *ptr, size_t size)
{
    if (ptr == NULL)
        return dfmalloc(size);
    struct block *old = (struct block *) (((char*) ptr) - HEADER);
    size_t oldsize = old->size;
    void *new;
    if (oldsize > size)
        return NULL;
    if (old->size == size)
        return ptr;
    new = dfmalloc(size);
    if (new == NULL)
        return NULL;
    memmove(new, old, oldsize);
    if (new != old)
        dffree(ptr);
    return new;
}

double dfusage(void)
{
    return (sizeof(field) - HEADER - bytes_available) / (1024. * 1024);
}

void dfinit(void)
{
    field = malloc(MAX_SIZE);
    if (field == NULL) {
        fprintf(stderr, "Can't allocate this memory\n");
        exit(1);
    }
    COMMENT1("Set up a field of bytes at %p", field);
    COMMENT1("HEADER = %lu", HEADER);
    memset(field, 0, MAX_SIZE);
    COMMENT("Initialize the field of bytes to zero");
    struct block *p = (struct block *) &field;
    p->next_by_size = NULL;
    p->next_by_address = NULL;
    bytes_available = sizeof(field) - HEADER;
    p->size = bytes_available;
    first_by_size = p;
    first_by_address = p;
}
