#include <iostream>
#include <cstdint>

// Structure of a memory control block
typedef struct mcb {
	struct mcb   *prev; // Previous buffer head memory control block
	struct mcb   *next; // Next buffer tail memory control block
	std::size_t   size; // Current buffer size
	bool          free; // True for free buffers
} mcb_t;

#define MCB_SIZE sizeof(mcb_t)

inline mcb_t* get_mcb(void *ptr, std::size_t offset = 0)
{
	return reinterpret_cast<mcb_t*>(
		reinterpret_cast<std::uint8_t*>(ptr) + offset
		);
}

inline mcb_t* get_tail(mcb_t *head)
{
	return reinterpret_cast<mcb_t*>(
		reinterpret_cast<std::uint8_t*>(head) + head->size - MCB_SIZE
		);
}

mcb_t *head;

/* Creates the initial memory buffer */
void mysetup(void *buf, std::size_t size)
{
	auto head = get_mcb(buf);
	auto tail = get_mcb(buf, size - MCB_SIZE);
	head->prev = tail->prev = NULL;
	head->next = tail->next = NULL;
	head->size = tail->size = size - MCB_SIZE * 2;
	::head = head;
}

/* Allocates a memory buffer of "size" bytes */
void *myalloc(std::size_t size)
{
	auto curr_h = head;
	auto curr_t = get_tail(head);

	while (curr_h)
	{
		if (curr_h->size < size)
		{
			curr_h = curr_h->next;
		}
		else if (curr_h->size == size)
		{

		}
		else
		{

		}
	}
}

/* Releases the memory buffer located at "p" */
void myfree(void *p)
{
	
}

/* 4 Mb size constant */
#define SIZE 4 * 1024 * 1024

int main()
{
	// Setup the allocator
	mysetup(malloc(SIZE), SIZE);

	std::cin.get();
	return 0;
}