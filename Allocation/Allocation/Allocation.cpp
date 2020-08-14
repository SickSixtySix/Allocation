#include <iostream>
#include <cstdint>

// Prologue of the buffer
typedef struct prol {
	struct prol *prev; // Pointer to the previous free buffer
	                   // The buffer is the head if NULL
	struct prol *next; // Pointer to the next free buffer
	                   // The buffer is the tail if the pointer is circular
	                   // The buffer is allocated if NULL
	std::size_t  size; // Size of the buffer
} prol_t;

// Epilogue of the buffer
typedef struct epil {
	prol_t      *prol; // Pointer to the prologue of the buffer
} epil_t;

// Converts the universal pointer to a structure pointer using offset in bytes
template<typename T> inline T *strt(void *ptr, std::int64_t offset = 0) {
	return reinterpret_cast<T*>(reinterpret_cast<std::uint8_t*>(ptr) + offset);
}

inline prol_t *to_prol(void *ptr, std::int64_t offset = 0) {
	return reinterpret_cast<prol_t*>(reinterpret_cast<std::uint8_t*>(ptr) + offset);
}

inline epil_t *to_epil(void *ptr, std::int64_t offset = 0) {
	return reinterpret_cast<epil_t*>(reinterpret_cast<std::uint8_t*>(ptr) + offset);
}

/* Initializes a buffer */
inline void *init_buffer(void *ptr, void *prev = NULL, void *next = NULL, std::size_t size = 0, prol_t **out_prol = NULL, epil_t **out_epil = NULL) {
	prol_t *prol = to_prol(ptr);
	prol->prev = to_prol(prev);
	prol->next = to_prol(next);
	prol->size = size;

	if (out_prol)
		*out_prol = prol;

	epil_t *epil = to_epil(prol + 1, size);
	epil->prol = prol;

	if (out_epil)
		*out_epil = epil;

	return epil + 1;
}

// Head of the free buffers list
prol_t *head;

/* Creates the initial memory buffer */
void mysetup(void *buf, std::size_t size)
{
	// Current buffer pointer
	void *ptr = buf;

	// Initializing actual (middle) and border (left, right) buffers
	ptr = init_buffer(ptr);
	ptr = init_buffer(ptr, NULL, ptr, size - sizeof(prol_t) * 2 - sizeof(epil_t) * 2, &head);
	ptr = init_buffer(ptr);
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
				if (curr->prev) {
					if (curr == curr->next) {
						curr->prev->next = curr->prev;
					}
					else {
						curr->prev->next = curr->next;
						curr->next->prev = curr->prev;
					}
				}
				else {
					if (curr == curr->next) {
						head = NULL;
					}
					else {
						head = curr->next;
						head->prev = NULL;
					}
				}

				curr->next = NULL;

				return curr + 1;
			}
			else {
				// Decrease current buffer size
				curr->size -= sizeof(epil_t) + sizeof(prol_t) + size;

				// Initialize the epilogue of the current buffer
				auto epilogue = strt<epil_t>(curr + 1, curr->size);
				epilogue->prol = curr;

				// 
				prol_t *prologue;
				init_buffer(epilogue + 1, NULL, NULL, size, &prologue);

				return prologue + 1;
			}

		prev = curr;
		curr = curr->next;
	} while (curr != prev);

	return NULL;
}

/* Releases the memory buffer located at "p" */
void myfree(void *p)
{
	prol_t
		*curr,
		*prev,
		*next;

	curr = to_prol(p) - 1;
	
	next = to_prol(curr + 1, curr->size + sizeof(epil_t));
	if (next->next) {
		if (next == next->next) {
			curr->next = curr;
		}
		else {
			curr->next = next->next;
		}

		if (next->prev) {
			next->prev->next = curr;
		}
		else {
			head = curr;
		}

		curr->prev = next->prev;
		curr->size += sizeof(prol_t) + next->size + sizeof(epil_t);
	}
	else {
		if (head) {
			curr->prev = NULL;
			curr->next = head;
			head->prev = curr;
			head = curr;
		}
		else {
			head = curr;
			head->next = curr;
		}
	}
}

/* 4 Mb size constant */
#define SIZE 4 * 1024 * 1024

int main()
{
	// Setup the allocator
	mysetup(malloc(SIZE), SIZE);

	void *pointers[5000];

	for (std::size_t j = 3; j-- > 0;) {
		std::size_t allocations = 0;

		for (std::size_t i = 5000 - 1; i-- > 0;) {
			pointers[i] = myalloc(i);
			if (pointers[i])
				++allocations;
		}
	
		std::cout << allocations << std::endl;

		for (std::size_t i = 5000 - 1; i-- > 0;)
			if (pointers[i])
				myfree(pointers[i]);
	}

	std::cin.get();
	return 0;
}