#ifndef I_SOUND_ENGINE_IOUTILITY_H
#define I_SOUND_ENGINE_IOUTILITY_H

#include <string>

namespace IO
{

/*!
 * \brief Tests whether or not a file corresponding to a given path exists or not
 */
bool FileExists(std::string const& filepath);
/*!
 * \brief Tests whether or not a file corresponding to a given path exists or not
 */
bool FileExists(char const* filepath);

}

#endif
