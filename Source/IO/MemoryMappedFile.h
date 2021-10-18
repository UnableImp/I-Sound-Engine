#ifndef I_SOUND_ENGINE_MEMORYMAPPEDFILE_H
#define I_SOUND_ENGINE_MEMORYMAPPEDFILE_H

#include <string>

namespace IO
{

/*!
 * \brief A file that is read as if it is a contiguous region of memory.
 * To use it, simply construct it with a filepath, then read from the
 * region of memory beginning at Data(). There will be Size() bytes of
 * memory there. This file cannot currently be written to.
 * Upon destruction, the memory region is invalidated.
 * Do not store pointers to the data if they may outlive the file object.
 */
class MemoryMappedFile
{
public:
  MemoryMappedFile(std::string const& filepath);
  MemoryMappedFile(char const* filepath);
  ~MemoryMappedFile();

  size_t Size()
  {
    return size;
  }
  char const* Data()
  {
    return data;
  }
private:
  size_t size;
  char const* data;
};

}

#endif // I_SOUND_ENGINE_MEMORYMAPPEDFILE_H
