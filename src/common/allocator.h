#ifndef CUSTOM_ALLOCATOR_H
#define CUSTOM_ALLOCATOR_H

class CustomAllocator
{
    void* my_custom_allocator;
public:
    CustomAllocator();
    ~CustomAllocator();
    void initialize(long n, long node_size);
    void printStats();
    void* newNode();
    void freeNode(void *object);
    void cleanup();
};
#endif
