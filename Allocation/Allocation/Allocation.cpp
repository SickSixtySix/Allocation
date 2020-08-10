#include <iostream>
#include <cstdint>

// Pointer to the first free memory buffer
void *head;

/* Creates the initial memory buffer */
void mysetup(void *buf, std::size_t size)
{
	// Assign the pointer to the first free memory buffer
	head = buf;
}

/* Allocates a memory buffer of "size" bytes */
void *myalloc(std::size_t size)
{
	return NULL;
}

/* Releases the memory buffer located at "p" */
void myfree(void *p)
{
	
}

/* 4 Mb size constant */
#define SIZE 1 * 1024 * 1024

int main()
{
	// Setup the allocator
	mysetup(malloc(SIZE), SIZE);

	std::cin.get();
	return 0;
}