#include "IO/MemoryMappedFile.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

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
  HANDLE fileHandle = CreateFileA(filepath, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
  HANDLE fileMappingHandle = CreateFileMappingA(fileHandle, nullptr, PAGE_READONLY, 0, 0, nullptr);

  data = reinterpret_cast<char const*>(MapViewOfFile(fileMappingHandle, FILE_MAP_READ, 0, 0, 0));

  LARGE_INTEGER sizeInteger;
  GetFileSizeEx(fileHandle, &sizeInteger);
  size = sizeInteger.QuadPart;

  CloseHandle(fileMappingHandle);
  CloseHandle(fileHandle);
}

MemoryMappedFile::~MemoryMappedFile()
{
  UnmapViewOfFile(data);
}

}
