// macOS before 10.13 has no aligned operator new/delete in its C++ library
// (the over-aligned engine structures need them). On the Mac the program
// brings its own, so that a build for 10.11 links and starts on old systems;
// on 10.13 and later these replace the library's, which is allowed.
#include <JuceHeader.h>
#if JUCE_MAC
#include <new>
#include <cstdlib>
void* operator new(std::size_t size, std::align_val_t alignment)
{
    void* p = nullptr;
    const std::size_t a = std::max<std::size_t>(static_cast<std::size_t>(alignment), sizeof(void*));
    if (posix_memalign(&p, a, size == 0 ? 1 : size) != 0) throw std::bad_alloc();
    return p;
}
void* operator new[](std::size_t size, std::align_val_t alignment) { return operator new(size, alignment); }
void* operator new(std::size_t size, std::align_val_t alignment, const std::nothrow_t&) noexcept
{
    void* p = nullptr;
    const std::size_t a = std::max<std::size_t>(static_cast<std::size_t>(alignment), sizeof(void*));
    return posix_memalign(&p, a, size == 0 ? 1 : size) == 0 ? p : nullptr;
}
void* operator new[](std::size_t size, std::align_val_t alignment, const std::nothrow_t& tag) noexcept { return operator new(size, alignment, tag); }
void operator delete(void* p, std::align_val_t) noexcept { std::free(p); }
void operator delete[](void* p, std::align_val_t) noexcept { std::free(p); }
void operator delete(void* p, std::size_t, std::align_val_t) noexcept { std::free(p); }
void operator delete[](void* p, std::size_t, std::align_val_t) noexcept { std::free(p); }

// std::hash of strings calls std::__1::__hash_memory, a function Apple added to
// the system's libc++ in macOS 15 - on older systems the loader finds no such
// symbol and refuses to start the program ("DYLD, Symbol missing"). The
// program brings its own under that very symbol name (an assembler label, so
// that the library's own declaration is not redefined); FNV-1a - any
// consistent hash will do. The linker binds it in preference to the library's.
#include <cstddef>
#include <cstdint>
std::size_t libreOrganHashMemory(const void* ptr, std::size_t size) noexcept __asm__("__ZNSt3__113__hash_memoryEPKvm");
std::size_t libreOrganHashMemory(const void* ptr, std::size_t size) noexcept
{
    const unsigned char* p = static_cast<const unsigned char*>(ptr);
    uint64_t h = 1469598103934665603ULL;
    for (std::size_t i = 0; i < size; ++i) { h ^= p[i]; h *= 1099511628211ULL; }
    return static_cast<std::size_t>(h);
}
#endif
