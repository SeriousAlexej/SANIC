#ifndef DIRTOOLS_HPP_INCLUDED
#define DIRTOOLS_HPP_INCLUDED
#include <vector>
#include <string>

std::vector<std::string> GetFilesOfFormat(std::string folder, std::string format="");
bool DirectoryExists(std::string path);

#endif // DIRTOOLS_HPP_INCLUDED
