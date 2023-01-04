#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define HDR_SIZE sizeof(struct block_hdr)

struct block_hdr    {
    size_t size;
};

struct free_list   {
    size_t size;
    struct free_list *next;
};

struct free_list *head = NULL;
void *block_end = NULL;

inline size_t align(size_t n) {
    return (n + sizeof(long) - 1) & ~(sizeof(long) - 1);
}

void add_free_block(struct block_hdr *block)    {
    struct free_list *free_block = (struct free_list*) block;
    if(!head)   {
        free_block->next = NULL;
        head = free_block;
    }
    else    {
        free_block->next = head;
        head = free_block;
    }
}

void *get_free_block(size_t size)  {
    struct free_list *temp = head;
    while(temp) {
        if(temp->size >= size)  {
            return temp;
        }
        temp = temp->next;
    }
    return NULL;
}

void print_freelist()   {
    struct free_list *temp = head;
    int i = 0;
    while(temp) {
        fprintf(stderr, "free block %d, size %ld\n", i, temp->size);
        temp = temp->next;
    }
}

void remove_free_block(struct free_list *block)    {
    struct free_list *temp = head;
    if(head == block)   {
        head = head->next;
    }
    else    {
        while(temp->next) {
            if(temp->next == block) {
                temp->next = temp->next->next;
                return;
            }
            temp = temp->next;
        }
    }
}

void *alloc_block(size_t size) {
    size_t capacity = size > 8000 ? size : 8000;
    void *block = NULL;
    if(allocated <= 0)  {
        block = sbrk(capacity);
        block_end = !block_end ? block + size : block_end + size;
        allocated += capacity - size;
    }
    else if(size <= allocated)  {
        block = block_end;
        block_end += size;
        allocated -= size;
    }
    else    {
        sbrk(capacity);
        block = block_end;
        block_end += size;
        allocated += capacity - size;
    }
    return block;
}

void *mymalloc(size_t size) {
    if(!size)   {
        return NULL;
    }
    size_t total_size = align(size + HDR_SIZE);
    void *block = get_free_block(total_size);
    if(!block)  {
        block = alloc_block(total_size);
        struct block_hdr *h = (struct block_hdr*) block;
        h->size = total_size;
    }
    else    {
        struct free_list *free_block = (struct free_list*) block;
        remove_free_block(free_block);
    }
    return block + HDR_SIZE;
}

void *mycalloc(size_t nmemb, size_t size)   {
    if(!size || !nmemb) {
        return NULL;
    }
    size_t total_size = align(nmemb * size);
    void *block = mymalloc(total_size);
    if(!block)  {
        return NULL;
    }
    memset(block, 0, total_size);
    return block;
}

void myfree(void *ptr)  {
    if(!ptr)    {
        return;
    }
    struct block_hdr *h = (struct block_hdr*) ptr - 1;
    add_free_block(h);
}

void *myrealloc(void *ptr, size_t size) {
    if(!size && ptr)    {
        myfree(ptr);
        return NULL;
    }
    if(!ptr)    {
        return mymalloc(size);
    }
    struct block_hdr *h = (struct block_hdr*) ptr-1;
    if(h->size >= size) {
        return ptr;
    }
    size_t total_size = align(size + HDR_SIZE);
    size_t prev_size = h->size;
    void *block = mymalloc(total_size);
    memcpy(block, h+1, prev_size);
    return block;
}


/*
 * Enable the code below to enable system allocator support for your allocator.
 * Doing so will make debugging much harder (e.g., using printf may result in
 * infinite loops).
 */
#if 1
void *malloc(size_t size) { return mymalloc(size); }
void *calloc(size_t nmemb, size_t size) { return mycalloc(nmemb, size); }
void *realloc(void *ptr, size_t size) { return myrealloc(ptr, size); }
void free(void *ptr) { myfree(ptr); }
#endif
