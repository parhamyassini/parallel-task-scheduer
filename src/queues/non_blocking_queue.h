#include "../common/allocator.h"
#include "../common/utils.h"

using namespace std;

#define LFENCE asm volatile("lfence" : : : "memory")
#define SFENCE asm volatile("sfence" : : : "memory")


const uintptr_t MASK = (0xffff000000000000);


template<class P>
struct pointer_t {
    P* ptr;

    pointer_t(){}
    pointer_t(P* _ptr, uint _count){
        ptr = (P *)(((uintptr_t)_ptr & ~MASK) | ((_count & ~MASK) << 48));
    }
    P* address(){
        //Get the address by getting the 48 least significant bits of ptr
        return  (P *)(((uintptr_t)ptr & ~MASK));

    }
    uint count(){
        //Get the count from the 16 most significant bits of ptr
        return  (uint)(((uintptr_t)ptr & MASK) >> 48);
    }

    inline bool operator==(const pointer_t &pt){
        if((uintptr_t)ptr == (uintptr_t)pt.ptr)
            return true;
        else
            return false;
    }
};

template <class T>
class Node
{
public:
    T value;
    pointer_t<Node<T>> next;
};


template <class T>
class NonBlockingQueue
{
    pointer_t<Node<T>> q_head;
    pointer_t<Node<T>> q_tail;
    CustomAllocator my_allocator_;
public:
    
    NonBlockingQueue() : my_allocator_()
    {
        std::cout << "Using NonBlockingQueue\n";
    }

    void initQueue(long t_my_allocator_size){
        std::cout << "Using Allocator\n";
        my_allocator_.initialize(t_my_allocator_size, sizeof(Node<T>));
        // Initialize the queue head or tail here
        Node<T>* newNode = (Node<T>*)my_allocator_.newNode();
//        my_allocator_.freeNode(newNode);
        newNode->next.ptr = NULL;
        q_head.ptr = q_tail.ptr = newNode;
    }

    void enqueue(T value)
    {
        Node<T>* node = (Node<T>* )my_allocator_.newNode();
        node->value = value;
        node->next.ptr = NULL;
        SFENCE;
        pointer_t<Node<T>> tail;
        pointer_t<Node<T>> next;
        while(true) {
            tail = q_tail;
            LFENCE;
            next = tail.address()->next;
            LFENCE;
            if (tail == q_tail){
                if (next.address() == NULL) {
                    if(CAS(&tail.address()->next, next, pointer_t<Node<T>>(node, next.count()+1)))
                    break;
                }
                else
                    CAS(&q_tail, tail, pointer_t<Node<T>>(next.address(), tail.count()+1));	// ELABEL
            }
        }
        SFENCE;
        CAS(&q_tail, tail, pointer_t<Node<T>>(node, tail.count()+1));
    }

    T dequeue()
    {
        T value;
        pointer_t<Node<T>> head;
        pointer_t<Node<T>> tail;
	    pointer_t<Node<T>> next;
        while(true){
            head = q_head;
            LFENCE;
            tail = q_tail;
            LFENCE;
            next = head.address()->next;
            LFENCE;
            if (head == q_head) {
                if(head.address() == tail.address()) {
                    if(next.address() == NULL)
                        return nullptr;
                    CAS(&q_tail, tail, pointer_t<Node<T>>(next.address(), tail.count()+1));	//DLABEL
                }
                else {
                    value = next.address()->value;
                    if(CAS(&q_head, head, pointer_t<Node<T>>(next.address(), head.count()+1)))
                    break;
                }
            }
        }
        my_allocator_.freeNode(head.address());
        return value;
    }

    bool empty(){
        return q_head == q_tail;
    }

    void cleanup()
    {
        my_allocator_.cleanup();
    }

};


