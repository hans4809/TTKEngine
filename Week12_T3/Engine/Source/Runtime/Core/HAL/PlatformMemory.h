#pragma once
#include <atomic>
#include <cassert>
#include <iostream>

#include "Core/HAL/PlatformType.h"

enum EAllocationType : uint8
{
    EAT_Object,
    EAT_Container
};

/**
 * 엔진의 Heap 메모리의 할당량을 추적하는 클래스
 *
 * @note new로 생성한 객체는 추적하지 않습니다.
 */
struct FPlatformMemory
{
private:
    static std::atomic<uint64> ObjectAllocationBytes;
    static std::atomic<uint64> ObjectAllocationCount;
    static std::atomic<uint64> ContainerAllocationBytes;
    static std::atomic<uint64> ContainerAllocationCount;

    template <EAllocationType AllocType>
    static void IncrementStats(size_t Size);

    template <EAllocationType AllocType>
    static void DecrementStats(size_t Size);

public:
    static void* Memcpy(void* Dest, const void* Src, uint64 Length)
    {
        return std::memcpy(Dest, Src, Length);
    }

    template <typename T>
    static void Memcpy(T& Dest, const T& Src);

    template <EAllocationType AllocType>
    static void* Malloc(size_t Size);

    template <EAllocationType AllocType>
    static void* AlignedMalloc(size_t Size, size_t Alignment);

    template <EAllocationType AllocType>
    static void Free(void* Address, size_t Size);

    template <EAllocationType AllocType>
    static void AlignedFree(void* Address, size_t Size);

    template <EAllocationType AllocType>
    static uint64 GetAllocationBytes();

    template <EAllocationType AllocType>
    static uint64 GetAllocationCount();

    
    /**  
     * 주어진 메모리 주소의 데이터를 캐시에 미리 로드하도록 힌트를 보냅니다.  
     * CPU 아키텍처별로 적절한 프리페치 명령을 사용합니다.  
     */
    FORCEINLINE static void Prefetch(const void* Ptr)
    {
        _mm_prefetch(static_cast<const char*>(Ptr), _MM_HINT_T0);
    }

    FORCEINLINE static void Prefetch(const void* Ptr, int32 Offset)
    {
        Prefetch(reinterpret_cast<const void*>(reinterpret_cast<std::uintptr_t>(Ptr) + Offset));
    }

    FORCEINLINE static void PrefetchBlock(const void* Ptr)
    {
        Prefetch(Ptr);
    }
};


template <EAllocationType AllocType>
void FPlatformMemory::IncrementStats(size_t Size)
{
    // TotalAllocationBytes += Size;
    // ++TotalAllocationCount;

    if constexpr (AllocType == EAT_Container)
    {
        ContainerAllocationBytes.fetch_add(Size, std::memory_order_relaxed);
        ContainerAllocationCount.fetch_add(1, std::memory_order_relaxed);
    }
    else if constexpr (AllocType == EAT_Object)
    {
        ObjectAllocationBytes.fetch_add(Size, std::memory_order_relaxed);
        ObjectAllocationCount.fetch_add(1, std::memory_order_relaxed);
    }
    else
    {
        //static_assert(false, "Unknown allocation type");
    }
}

template <EAllocationType AllocType>
void FPlatformMemory::DecrementStats(size_t Size)
{
    // TotalAllocationBytes -= Size;
    // --TotalAllocationCount;

    // 멀티스레드 대비
    if constexpr (AllocType == EAT_Container)
    {
        ContainerAllocationBytes.fetch_sub(Size, std::memory_order_relaxed);
        ContainerAllocationCount.fetch_sub(1, std::memory_order_relaxed);
    }
    else if constexpr (AllocType == EAT_Object)
    {
        ObjectAllocationBytes.fetch_sub(Size, std::memory_order_relaxed);
        ObjectAllocationCount.fetch_sub(1, std::memory_order_relaxed);
    }
    else
    {
        //static_assert(false, "Unknown allocation type");
    }
}

template <typename T>
void FPlatformMemory::Memcpy(T& Dest, const T& Src)
{
    Memcpy(&Dest, &Src, sizeof(T));
}

template <EAllocationType AllocType>
void* FPlatformMemory::Malloc(size_t Size)
{
    void* Ptr = std::malloc(Size);
    if (Ptr)
    {
        IncrementStats<AllocType>(Size);
    }
    return Ptr;
}

template <EAllocationType AllocType>
void* FPlatformMemory::AlignedMalloc(size_t Size, size_t Alignment)
{
    void* Ptr = _aligned_malloc(Size, Alignment);
    if (Ptr)
    {
        IncrementStats<AllocType>(Size);
    }
    return Ptr;
}

template <EAllocationType AllocType>
void FPlatformMemory::Free(void* Address, size_t Size)
{
    if (Address)
    {
        DecrementStats<AllocType>(Size);
        std::free(Address);
    }
}

template <EAllocationType AllocType>
void FPlatformMemory::AlignedFree(void* Address, size_t Size)
{
    if (Address)
    {
        DecrementStats<AllocType>(Size);
        _aligned_free(Address);
    }
}

template <EAllocationType AllocType>
uint64 FPlatformMemory::GetAllocationBytes()
{
    if constexpr (AllocType == EAT_Container)
    {
        return ContainerAllocationBytes;
    }
    else if constexpr (AllocType == EAT_Object)
    {
        return ObjectAllocationBytes;
    }
    else
    {
        //static_assert(false, "Unknown AllocationType");
        return -1;
    }
}

template <EAllocationType AllocType>
uint64 FPlatformMemory::GetAllocationCount()
{
    if constexpr (AllocType == EAT_Container)
    {
        return ContainerAllocationCount;
    }
    else if constexpr (AllocType == EAT_Object)
    {
        return ObjectAllocationCount;
    }
    else
    {
        //static_assert(false, "Unknown AllocationType");
        return -1;
    }
}

