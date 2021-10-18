#include "IO/IOUtility.h"

#define NOMINMAX
#include <Windows.h>

namespace IO
{

bool FileExists(std::string const& filepath)
{
  return FileExists(filepath.c_str());
}

bool FileExists(char const* filepath)
{
  DWORD fileAttributes = GetFileAttributesA(filepath);
  return (fileAttributes != INVALID_FILE_ATTRIBUTES && !(fileAttributes & FILE_ATTRIBUTE_DIRECTORY));
}

}
