//a lock-free ringbuffer implement
#include <emmintrin.h>
#include "gp.h"


static inline uint32_t
gp_align32pow2(uint32_t x)
{
	x--;
	x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;

	return x;
}

void create_gp_ring(gp_ring **ring, int count)
{
	gp_ring *q = calloc(1, sizeof(*q) + count * sizeof(void *));
	init_gp_ring(q, count);
	
	*ring = q;
}

void init_gp_ring(gp_ring * q, int count)
{
	q->size = gp_align32pow2(count+1);
	q->mask = q->size - 1;
	q->capacity = count;
}

int gp_ring_push(gp_ring *q, int n, void **obj)
{
	uint32_t head = 0;
    uint32_t next = 0;
	int32_t ok;
	uint32_t free_entries = 0;
	int32_t max = n;

	do{
		n = max;

	    head = q->head.first;

		free_entries = q->capacity + q->tail.second - head;

		if(unlikely(n > free_entries))
			n = free_entries;

		if (n == 0)
			return 0;

		next = head + n;
		ok = __sync_bool_compare_and_swap(&q->head.first, head, next);

	}while(!ok);

	unsigned int i;
	unsigned int idx = head & q->mask;
	void **ring = (void **)q->msgs;
	if(likely(idx + n < q->size)){
		for(i = 0; i < (n & ((~(unsigned)0x3))); i+=4, idx+=4) {
			ring[idx] = obj[i];
			ring[idx+1] = obj[i+1];
			ring[idx+2] = obj[i+2];
			ring[idx+3] = obj[i+3];
		}
		switch(n & 0x3) {
		case 3:
			ring[idx++] = obj[i++];
		case 2:
			ring[idx++] = obj[i++];
		case 1:
			ring[idx++] = obj[i++];

		}
	} else {
		for (i = 0; idx < q->size; i++, idx++)
				ring[idx] = obj[i];
		for (idx = 0; i < n; i++, idx++)
				ring[idx] = obj[i];
	}


	asm volatile ("":::"memory");

	while (unlikely(q->head.second != head)){
		_mm_pause();
	}

	q->head.second = next;
	
	return n;
}


int gp_ring_pop(gp_ring *q, int n, void **obj)
{
	uint32_t tail = 0;
    uint32_t next = 0;
	int32_t ok;
	uint32_t entries = 0;
	int32_t max = n;

	do{
		n = max;

	    tail = q->tail.first;

		entries = q->head.second - tail;

		if(n > entries)
			n = entries;

		if (n == 0)
			return 0;

		next = tail + n;
		ok = __sync_bool_compare_and_swap(&q->tail.first, tail, next);

	}while(!ok);

	unsigned int i;
	unsigned int idx = tail & q->mask;
	void **ring = (void **)q->msgs;
	if(likely(idx + n < q->size)){
		for(i = 0; i < (n & ((~(unsigned)0x3))); i+=4, idx+=4) {
			obj[i] = ring[idx];
			obj[i+1] = ring[idx+1];
			obj[i+2] = ring[idx+2];
			obj[i+3] = ring[idx+3];
		}
		switch(n & 0x3) {
		case 3:
			obj[i++] = ring[idx++];
		case 2:
			obj[i++] = ring[idx++];
		case 1:
			obj[i++] = ring[idx++];
		}
	} else {
		for (i = 0; idx < q->size; i++, idx++)
			obj[i] = ring[idx];
		for (idx = 0; i < n; i++, idx++)
			obj[i] = ring[idx];
	}

	asm volatile ("":::"memory");

	while (unlikely(q->tail.second != tail)){
		_mm_pause();
	}

	q->tail.second = next;
	
	return n;
}
