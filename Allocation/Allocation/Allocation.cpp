#include <iostream>
#include <cstdint>

// Prologue of the buffer
typedef struct prol {
	struct prol *next; // Pointer to the next free buffer. The buffer is allocated if NULL
	std::size_t  size; // Size of the buffer
} prol_t;

// Epilogue of the buffer
typedef struct epil {
	prol_t      *prologue; // Pointer to the prologue of the buffer
} epil_t;

// Converts the universal pointer to a structure pointer using offset in bytes
template<typename T> inline T *strt(void *ptr, std::int64_t offset = 0) {
	return reinterpret_cast<T*>(reinterpret_cast<std::uint8_t*>(ptr) + offset);
}

// Head of the free buffers list
prol_t *head;

// Tail of the free buffers list
prol_t *tail;

/* Creates the initial memory buffer */
void mysetup(void *buf, std::size_t size)
{
	// Initialize the prologue of the initial buffer
	auto prologue = strt<prol_t>(buf);
	prologue->next = prologue;
	prologue->size = size - sizeof(prol_t) - sizeof(epil_t);

	// Initialize the epilogue of the initial buffer
	strt<epil_t>(prologue + 1, prologue->size)->prologue = prologue;

	// Initialize the free buffers list
	head = tail = prologue;
}

/* Allocates a memory buffer of "size" bytes */
void *myalloc(std::size_t size)
{
	if (!size || !head)
		return NULL;

	prol_t
		*prev = NULL,
		*curr = head;

	do {
		if (curr->size >= size)
			if (curr->size <= sizeof(epil_t) + sizeof(prol_t) + size) {
				if (curr == head)
					if (curr == tail)
						head = tail = NULL;
					else
						head = curr->next;
				else
					prev->next = curr->next;

				curr->next = NULL;

				return curr + 1;
			}
			else {
				// Decrease current buffer size
				curr->size -= sizeof(epil_t) + sizeof(prol_t) + size;

				// Initialize the epilogue of the current buffer
				auto epilogue = strt<epil_t>(curr + 1, curr->size);
				epilogue->prologue = curr;

				// Initialize the prologue of the allocated buffer
				auto prologue = strt<prol_t>(epilogue + 1);
				prologue->next = NULL;
				prologue->size = size;
				
				// Initialize the epilogue of the allocated buffer
				strt<epil_t>(prologue + 1, size)->prologue = prologue;

				return prologue + 1;
			}

		prev = curr;
		curr = curr->next;
	} while (prev != tail);

	return NULL;
}

/* Releases the memory buffer located at "p" */
void myfree(void *p)
{
	auto curr = strt<prol_t>(p, -static_cast<std::int64_t>(sizeof(prol_t)));
	auto prev = strt<epil_t>(curr, -static_cast<std::int64_t>(sizeof(epil_t)))->prologue;
	auto next = strt<prol_t>(curr + 1, curr->size);

	if (prev->next) {
		prev->size += sizeof(epil_t) + sizeof(prol_t) + curr->size;
		strt<epil_t>(prev + 1, prev->size)->prologue = prev;
		curr = prev;
	}

	if (curr != tail && next->next) {
		if (next == tail) {
			curr->next = curr;
			tail = curr;
		} else
			curr->next = next->next;

		curr->size += sizeof(epil_t) + sizeof(prol_t) + next->size;
		strt<epil_t>(curr + 1, curr->size)->prologue = curr;
	}
}

/* 4 Mb size constant */
#define SIZE 4 * 1024 * 1024

int main()
{
	// Setup the allocator
	mysetup(malloc(SIZE), SIZE);

	void *pointers[5000];

	for (std::size_t j = 2; j-- > 0;) {
		std::size_t last = 0;

		for (std::size_t i = 1; i < 5000; i++)
			if (!(pointers[i] = myalloc(i))) {
				last = i;
				std::cout << i << std::endl;
				break;
			}
	
		for (std::size_t i = 1; i < last; i++)
			myfree(pointers[i]);
	}

	std::cin.get();
	return 0;
}