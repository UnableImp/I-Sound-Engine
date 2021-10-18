#include "IO/IOUtility.h"

#include <unistd.h>

namespace IO
{

bool FileExists(std::string const& filepath)
{
  return FileExists(filepath.c_str());
}

bool FileExists(char const* filepath)
{
  return (access(filepath, F_OK) == 0);
}

}
