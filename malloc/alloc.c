/**
 * malloc
 * CS 241 - Spring 2021
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#define STRUCT_SIZE (sizeof(data_node))
#define DATA_SIZE_SIZE sizeof(size_t*)

typedef struct data_node {
    size_t size;
    struct data_node *before;
    // Pointer to the after instance of data_node in the list
    struct data_node *after;
    int free;
} data_node;

static data_node *head = NULL;
static void* start = NULL;
// to store the start of heap

void delete_data_node(data_node* be_deleted) {
    if (be_deleted == NULL) {
        return;
    }
    if (!be_deleted->before && !be_deleted->after) {
        head = NULL;
    }
    else {
        if (be_deleted->before && be_deleted->after) {
            be_deleted->after->before = be_deleted->before;
            be_deleted->before->after = be_deleted->after;
        }
        else if (be_deleted->after) {
            be_deleted->after->before = NULL;
            head = be_deleted->after;
        }
        else {
            be_deleted->before->after = NULL;
        }
    }
}

/**
 * Allocate space for array in memory
 *
 * Allocates a get_addr of memory for an array of num elements, each of them size
 * bytes long, and initializes all its bits to zero. The effective result is
 * the allocation of an zero-initialized memory get_addr of (num * size) bytes.
 *
 * @param num
 *    Number of elements to be allocated.
 * @param size
 *    Size of elements.
 *
 * @return
 *    A pointer to the memory get_addr allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested get_addr of memory, a
 *    NULL pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/calloc/
 */
void *calloc(size_t num, size_t size) {
    // implement calloc!
    void* new_alloc = malloc(num * size);
	if (new_alloc == NULL) {
		return NULL;
	}
	memset(new_alloc, 0, num * size);
    return new_alloc;
}

/**
 * Allocate memory get_addr
 *
 * Allocates a get_addr of size bytes of memory, returning a pointer to the
 * beginning of the get_addr.  The content of the newly allocated get_addr of
 * memory is not initialized, remaining with indeterminate values.
 *
 * @param size
 *    Size of the memory get_addr, in bytes.
 *
 * @return
 *    On success, a pointer to the memory get_addr allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested get_addr of memory,
 *    a null pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/malloc/
 */
data_node* find_free_get_addr(size_t size) {
    data_node* current = head;
    // the head of free data_node list
    while (current) {
        if (current->free && current->size >= size) {
            return current;
        }
        current = current->after;
    }
    return NULL;  
    // no suitable get_addr found
}

data_node* expand_heap(size_t size) {
    size_t total_size = STRUCT_SIZE + size + DATA_SIZE_SIZE;
    data_node* new_get_addr = sbrk(total_size);
    // find the break before sbrk
    if (*(int*)new_get_addr == -1) {
        return NULL;
    }
    new_get_addr->size = size;
    new_get_addr->free = 0;
    new_get_addr->before = NULL;
    new_get_addr->after = head;

    if (head) {
        head->before = new_get_addr;
    }
    head = new_get_addr;

    *(size_t*)((char*)new_get_addr + STRUCT_SIZE + size) = size;

    return new_get_addr;
}

void relist_split(data_node* cur, data_node* new_data) {
    if (cur->before && cur->after) {
        cur->before->after = new_data;
        cur->after->before = new_data;
    } else if (cur->before) {
        cur->before->after = new_data;
    } else if (cur->after) {
        cur->after->before = new_data;
        head = new_data;
    } else {
        head = new_data;
    }
}

static void split_metadata(data_node* cur, size_t newsize) {
    data_node* new_data = (data_node*)((char*)cur + STRUCT_SIZE + newsize + DATA_SIZE_SIZE);
    new_data->size = cur->size - newsize - STRUCT_SIZE - DATA_SIZE_SIZE;
    new_data->free = 1;
    new_data->before = cur->before;
    new_data->after = cur->after;

    *(size_t*)((char*)cur + STRUCT_SIZE + newsize) = newsize;
    *(size_t*)((char*)new_data + STRUCT_SIZE + new_data->size) = new_data->size;
    relist_split(cur, new_data);

    cur->size = newsize;
    cur->free = 0;
}

void *malloc(size_t size) {
    // implement malloc!
    if (start == NULL) {
        start = sbrk(0);
    }
    if (size == 0) {
        return NULL;
    }
    data_node* find_node = NULL;
    find_node = find_free_get_addr(size);

    if (find_node == NULL) {
        find_node = expand_heap(size);
        if (!find_node) {
            return NULL;
        }
    } else {
        find_node->free = 0;
        if (find_node->size - size > STRUCT_SIZE + DATA_SIZE_SIZE) {
            split_metadata(find_node, size);
            return (void*) (find_node + 1);
        }
    }
	delete_data_node(find_node);

    return (void*) (find_node + 1);
}

/**
 * Deallocate space in memory
 *
 * A get_addr of memory beforeiously allocated using a call to malloc(),
 * calloc() or realloc() is deallocated, making it available again for
 * further allocations.
 *
 * Notice that this function leaves the value of ptr unchanged, hence
 * it still points to the same (now invalid) location, and not to the
 * null pointer.
 *
 * @param ptr
 *    Pointer to a memory get_addr beforeiously allocated with malloc(),
 *    calloc() or realloc() to be deallocated.  If a null pointer is
 *    passed as argument, no action occurs.
 */

//merge before, cur, after get_addrs as one free get_addr
static void merge_both(data_node* cur) {
    cur->before->size += cur->size + cur->after->size + 2 * STRUCT_SIZE + 2 * DATA_SIZE_SIZE;
    *(size_t*)((char*)cur->after + STRUCT_SIZE + cur->after->size) = cur->before->size;
    data_node* both_node = cur->after;
    delete_data_node(both_node);

    cur->free = 0;
    both_node->free = 0;
}

//merge cur, after get_addrs as one free get_addr
static void merge_with_after(data_node* cur) {
    //update the cur metadata and the size mark to store the new size information
    cur->size += cur->after->size + STRUCT_SIZE + DATA_SIZE_SIZE;
    *(size_t*)((char*)cur->after + STRUCT_SIZE + cur->after->size) = cur->size;
    data_node* after_data = cur->after;
    cur->before = after_data->before;
	cur->after = after_data->after;
    relist_split(after_data, cur);
    after_data->free = 0;
}

static void merge_with_before(data_node* cur) {
    cur->before->size += cur->size + STRUCT_SIZE + DATA_SIZE_SIZE;
    *(size_t*)((char*)cur + STRUCT_SIZE + cur->size) = cur->before->size;
    cur->free = 0;
}

void merge_get_addrs(data_node* cur, data_node* before_start, data_node* after_end) {
    if (!((void*)after_end <= sbrk(0) && cur->after->free) && !((void*)before_start >= start && cur->before->free)) {
        if (!head) {
            head = cur;
            cur->before = cur->after = NULL;
        } else {
            cur->after = head;
            cur->before = NULL;
            head->before = cur;
            head = cur;
        }
    }
    else {
        if ((void*)after_end <= sbrk(0) && cur->after->free && (void*)before_start >= start && cur->before->free) {
            merge_both(cur);
        } else if ((void*)after_end <= sbrk(0) && cur->after->free) {
            merge_with_after(cur);
        } else if ((void*)before_start >= start && cur->before->free) {
            merge_with_before(cur);
        }
    }
}    

void free(void *ptr) {
    // implement free!
    if (!ptr) {
        return;
    }
    data_node* cur = (data_node*)((char*)ptr - STRUCT_SIZE);
    // the address of cur node
    cur->free = 1;
    size_t* node_tail_before = (void*)cur - DATA_SIZE_SIZE;
    cur->before = (data_node*)((char*)node_tail_before - (*node_tail_before) - sizeof(data_node));
    // the address of prev node
    cur->after = (data_node*)((char*)cur + STRUCT_SIZE + cur->size + DATA_SIZE_SIZE);
    // the address of next node
    data_node* before_start = cur->before;
    data_node* after_end = cur->after + STRUCT_SIZE;
    
    merge_get_addrs(cur, before_start, after_end);
    cur->free = 1;
}

/**
 * Reallocate memory get_addr
 *
 * The size of the memory get_addr pointed to by the ptr parameter is changed
 * to the size bytes, expanding or reducing the amount of memory available
 * in the get_addr.
 *
 * The function may move the memory get_addr to a new location, in which case
 * the new location is returned. The content of the memory get_addr is preserved
 * up to the lesser of the new and old sizes, even if the get_addr is moved. If
 * the new size is larger, the value of the newly allocated portion is
 * indeterminate.
 *
 * In case that ptr is NULL, the function behaves exactly as malloc, assigning
 * a new get_addr of size bytes and returning a pointer to the beginning of it.
 *
 * In case that the size is 0, the memory beforeiously allocated in ptr is
 * deallocated as if a call to free was made, and a NULL pointer is returned.
 *
 * @param ptr
 *    Pointer to a memory get_addr beforeiously allocated with malloc(), calloc()
 *    or realloc() to be reallocated.
 *
 *    If this is NULL, a new get_addr is allocated and a pointer to it is
 *    returned by the function.
 *
 * @param size
 *    New size for the memory get_addr, in bytes.
 *
 *    If it is 0 and ptr points to an existing get_addr of memory, the memory
 *    get_addr pointed by ptr is deallocated and a NULL pointer is returned.
 *
 * @return
 *    A pointer to the reallocated memory get_addr, which may be either the
 *    same as the ptr argument or a new location.
 *
 *    The type of this pointer is void*, which can be cast to the desired
 *    type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested get_addr of memory,
 *    a NULL pointer is returned, and the memory get_addr pointed to by
 *    argument ptr is left unchanged.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/realloc/
 */

data_node* split_get_addr_for_reduction(data_node* get_addr, size_t new_size) {
    if (get_addr->size <= (1024 * 1024 * 256)) {
        return get_addr;
    }
    if (get_addr->size - new_size > sizeof(data_node) + DATA_SIZE_SIZE) {
        data_node* new_node = (data_node*)((char*)get_addr + sizeof(data_node) + new_size + DATA_SIZE_SIZE);
        new_node->size = get_addr->size - new_size - sizeof(data_node) - DATA_SIZE_SIZE;
        get_addr->size = new_size;

        *((size_t*)((char*)get_addr + sizeof(data_node) + new_size)) = new_size;
        *((size_t*)((char*)new_node + sizeof(data_node) + new_node->size)) = new_node->size;

        if (!head) {
            head = new_node;
            new_node->before = new_node->after = NULL;
        } else {
            new_node->after = head;
            new_node->before = NULL;
            head->before = new_node;
            head = new_node;
        }
        new_node->free = 1;
        get_addr->free = 0;
    }
    return get_addr;
}

void* reduce_size(void* ptr, size_t new_size) {
    data_node* get_addr = (data_node*)ptr - 1;
    return split_get_addr_for_reduction(get_addr, new_size) + 1;
}

void* increase_size(void* ptr, size_t old_size, size_t new_size) {
    void* new_ptr = malloc(new_size);
    if (!new_ptr) {
        return NULL;
    }
    memcpy(new_ptr, ptr, old_size);
    free(ptr);
    return new_ptr;
}

void* realloc(void* ptr, size_t size) {
    if (!ptr) return malloc(size);
    if (size == 0) {
        free(ptr);
        return NULL;
    }
    
    data_node* get_addr = (data_node*)ptr - 1;
    size_t old_size = get_addr->size;

    if (old_size >= size) {
        return reduce_size(ptr, size);
    } else {
        return increase_size(ptr, old_size, size);
    }
}