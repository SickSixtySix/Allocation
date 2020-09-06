#include <iostream>
#include <cstdint>
#include <vector>

#define MIN_BUF_SIZE 16

typedef struct mcb {
	struct mcb *prev;
	struct mcb *next;
	std::size_t size;
	bool free;
} mcb_t;

mcb_t *hb;

inline void remove(mcb_t *b) {
	if (b->prev) {
		b->prev->next = b->next;
		if (b->next) {
			b->next->prev = b->prev;
		}
	}
	else {
		hb = b->next;
		if (b->next) {
			b->next->prev = NULL;
		}
	}

	b->free = false;
}

inline mcb_t* beginning(void *p, mcb_t *prev = NULL, mcb_t *next = NULL, std::size_t size = 0, bool free = false) {
	auto b = reinterpret_cast<mcb_t*>(p);
	b->prev = prev;
	b->next = next;
	b->size = size;
	b->free = free;
	return b;
}

inline mcb_t** ending(mcb_t *b) {
	auto e = reinterpret_cast<mcb_t**>(
		reinterpret_cast<std::uint8_t*>(
			b + 1
			) + b->size
		);
	*e = b;
	return e;
}

inline void merge(mcb_t *pb, mcb_t *nb) {
	// Update the beginning block of the merged buffer
	pb->size += sizeof(mcb_t*) + sizeof(mcb_t) + nb->size;

	// Update the ending block of the merged buffer
	ending(pb);
}

void mysetup(void *buf, std::size_t size)
{
	// Left heap border
	auto lb = beginning(buf);
	auto le = ending(lb);

	// Usable heap region
	hb = beginning(le + 1, NULL, NULL,
		size - sizeof(mcb_t) * 3 - sizeof(mcb_t*) * 3, true);
	auto he = ending(hb);

	// Right heap border
	auto rb = beginning(he + 1);
	auto re = ending(rb);
}

void *myalloc(std::size_t size)
{
	auto cb = hb;

	while (cb) {
		if (cb->size >= size) {
			if (cb->size <= MIN_BUF_SIZE + sizeof(mcb_t*) + sizeof(mcb_t) + size) {
				remove(cb);
				return cb + 1;
			}
			else {
				cb->size -= sizeof(mcb_t*) + sizeof(mcb_t) + size;
				auto ce = ending(cb);

				auto nb = beginning(ce + 1, NULL, NULL, size);
				ending(nb);
				return nb + 1;
			}
		}

		cb = cb->next;
	}

	return NULL;
}

void myfree(void *p)
{
	auto cb = reinterpret_cast<mcb_t*>(p) - 1;
	auto pb = (*(reinterpret_cast<mcb_t**>(cb) - 1));
	auto nb = reinterpret_cast<mcb_t*>(
		reinterpret_cast<std::uint8_t*>(
			cb + 1
			) + cb->size + sizeof(mcb_t*)
		);

	if (pb->free) {
		// Remove the previous buffer from the free buffers list
		remove(pb);

		// Merge the previous and the current buffers
		merge(pb, cb);

		// Repeat operation on a new buffer
		myfree(pb + 1);
	}
	else if (nb->free) {
		// Remove the next buffer from the free buffers list
		remove(nb);

		// Merge the current and the next buffers
		merge(cb, nb);

		// Repeat operation on a new buffer
		myfree(cb + 1);
	}
	else {
		cb->prev = NULL;
		cb->next = hb;
		cb->free = true;

		if (hb) {
			hb->prev = cb;
		}

		hb = cb;
	}
}

#define HEAP_SIZE 524288

int main()
{
	srand(1);

	mysetup(malloc(HEAP_SIZE), HEAP_SIZE);

	std::vector<void*> pointers;
	std::size_t removals = 0;

	while (true) {
		for (std::size_t i = rand() % 3 + 2; i-- > 0;) {
			auto pointer = myalloc(16 + rand() % 2048);
			if (pointer)
				pointers.push_back(pointer);
		}

		if (pointers.size() > 0) {
			const auto id = rand() % pointers.size();
			auto pointer = pointers[id];
			pointers.erase(pointers.begin() + id);
			myfree(pointer);

			std::cout << "removed " << removals << std::endl;

			removals++;
		}
	}

	std::cin.get();
	return 0;
}