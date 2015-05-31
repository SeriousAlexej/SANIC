#ifndef MEMORY_H_INCLUDED
#define MEMORY_H_INCLUDED

#include <cstdlib>
void AddTrack(std::size_t addr,  std::size_t asize,  const char *fname, std::size_t lnum);
void RemoveTrack(std::size_t addr);
void DumpUnfreed();

#ifdef SANIC_DEBUG
inline void* operator new(std::size_t size, const char *file, int line)
{
  void *ptr = (void*)std::malloc(size);
  AddTrack((std::size_t)ptr, size, file, line);
  return(ptr);
};
inline void operator delete(void *p)
{
  RemoveTrack((std::size_t)p);
  std::free(p);
};
#define DEBUG_NEW new(__FILE__, __LINE__)
#else
#define DEBUG_NEW new
#endif // SANIC_DEBUG
#define new DEBUG_NEW

#endif // MEMORY_H_INCLUDED
