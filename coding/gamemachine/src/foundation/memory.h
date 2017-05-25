﻿#ifndef __GM_MEMORY_H__
#define __GM_MEMORY_H__
#include "defines.h"
BEGIN_NS

class AlignedMemoryAlloc
{
public:
	static void* gmAlignedAllocInternal(size_t size, GMint alignment);
	static void gmAlignedFreeInternal(void* ptr);
};

typedef void *(gmAlignedAllocFunc)(size_t size, GMint alignment);
typedef void (gmAlignedFreeFunc)(void *memblock);
typedef void *(gmAllocFunc)(size_t size);
typedef void (gmFreeFunc)(void *memblock);
void gmAlignedAllocSetCustom(gmAllocFunc *allocFunc, gmFreeFunc *freeFunc);
void gmAlignedAllocSetCustomAligned(gmAlignedAllocFunc *allocFunc, gmAlignedFreeFunc *freeFunc);

#define gmAlignedAlloc(size,alignment) AlignedMemoryAlloc::gmAlignedAllocInternal(size,alignment)
#define gmAlignedFree(ptr) AlignedMemoryAlloc::gmAlignedFreeInternal(ptr)

template < typename T, unsigned Alignment >
class AlignedAllocator
{
	typedef AlignedAllocator< T, Alignment > my_type;

public:
	AlignedAllocator() {}

public:
	template < typename Other >
	AlignedAllocator(const AlignedAllocator< Other, Alignment > &) {}

	typedef const T* const_pointer;
	typedef const T& const_reference;
	typedef T* pointer;
	typedef T& reference;
	typedef T value_type;

	pointer address(reference ref) const { return &ref; }
	const_pointer address(const_reference  ref) const { return &ref; }
	pointer allocate(GMint n, const_pointer * hint = 0) {
		(void)hint;
		return reinterpret_cast<pointer>(gmAlignedAlloc(sizeof(value_type) * n, Alignment));
	}
	void construct(pointer ptr, const value_type & value) { new (ptr) value_type(value); }
	void deallocate(pointer ptr) {
		gmAlignedFree(reinterpret_cast<void *>(ptr));
	}
	void destroy(pointer ptr) { ptr->~value_type(); }


	template < typename O > struct rebind {
		typedef AlignedAllocator< O, Alignment > other;
	};
	template < typename O >
	my_type & operator=(const AlignedAllocator< O, Alignment > &) { return *this; }

	friend bool operator==(const my_type &, const my_type &) { return true; }
};

// 重在new, delete，对齐分配内存
#if USE_SIMD
#define GM_DECLARE_ALIGNED_ALLOCATOR() \
	public:   \
	inline void* operator new(size_t sizeInBytes){ return gmAlignedAlloc(sizeInBytes, 16); }   \
	inline void  operator delete(void* ptr)         { gmAlignedFree(ptr); }   \
	inline void* operator new(size_t, void* ptr){ return ptr; }   \
	inline void  operator delete(void*, void*)      { }   \
	inline void* operator new[](size_t sizeInBytes)   { return gmAlignedAlloc(sizeInBytes, 16); }   \
	inline void  operator delete[](void* ptr)         { gmAlignedFree(ptr); }   \
	inline void* operator new[](size_t, void* ptr)   { return ptr; }   \
	inline void  operator delete[](void*, void*)      {}
#else
#define GM_DECLARE_ALIGNED_ALLOCATOR()
#endif

class GMAlignmentObject
{
	GM_DECLARE_ALIGNED_ALLOCATOR();

public:
	virtual ~GMAlignmentObject() {}
};

END_NS
#endif