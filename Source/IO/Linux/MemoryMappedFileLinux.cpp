#include "IO/MemoryMappedFile.h"

#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "IO/IOUtility.h"
#include <cassert>

namespace IO
{

MemoryMappedFile::MemoryMappedFile(std::string const& filepath)
  : MemoryMappedFile(filepath.c_str())
{}

MemoryMappedFile::MemoryMappedFile(char const* filepath)
{
  assert(IO::FileExists(filepath));

  int fd = open(filepath, O_RDONLY, 0);

  struct stat st;
  fstat(fd, &st);
  size = st.st_size;

  data = reinterpret_cast<char*>(
    mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0)
    );

  close(fd);
}

MemoryMappedFile::~MemoryMappedFile()
{
  munmap((void*)data, size);
}

}
