#include <vector>
#include <cstring>
#include <cstdio>

struct AllocInfo
{
    std::size_t   address;
    std::size_t   size;
    char          file[64];
    std::size_t   line;
};

typedef std::vector<AllocInfo> AllocVector;

AllocVector *allocVector = nullptr;

void AddTrack(std::size_t addr,  std::size_t asize,  const char *fname, std::size_t lnum)
{
    if(allocVector == nullptr)
    {
        allocVector = new AllocVector;
    }
    AllocInfo info;
    info.address = addr;
    strncpy(info.file, fname, 63);
    info.line = lnum;
    info.size = asize;
    allocVector->push_back(info);
}

void RemoveTrack(std::size_t addr)
{
    if(allocVector == nullptr)
    {
        return;
    }
    for(int i=allocVector->size()-1; i>=0; i--)
    {
        if((*allocVector)[i].address == addr)
        {
            allocVector->erase(allocVector->begin() + i);
            break;
        }
    }
}

void DumpUnfreed()
{
    if(allocVector == nullptr)
    {
        return;
    }
    std::size_t totalSize = 0;
    char buf[1024];

    for(int i=allocVector->size()-1; i>=0; i--)
    {
      sprintf(buf, "%-50s:\t\tLINE %lu,\t\tADDRESS %lu\t%lu unfreed\n",
          (*allocVector)[i].file, (*allocVector)[i].line, (*allocVector)[i].address, (*allocVector)[i].size);
      fprintf(stderr, "%s", buf);
      totalSize += (*allocVector)[i].size;
    }
    sprintf(buf, "-----------------------------------------------------------\n");
    fprintf(stderr, "%s", buf);
    sprintf(buf, "Total Unfreed: %lu bytes\n", totalSize);
    fprintf(stderr, "%s", buf);
    allocVector->clear();
    ::operator delete(allocVector);
    allocVector = nullptr;
};
