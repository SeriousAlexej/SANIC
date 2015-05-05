#include <algorithm>
#include <boost/filesystem.hpp>
#include "dirtools.h"

using namespace boost::filesystem;

bool DirectoryExists(std::string _path)
{
    path folder(_path);
    return exists(folder);
}

std::vector<std::string> GetFilesOfFormat(std::string _folder, std::string format)
{
    std::vector<std::string> names;
    path folder(_folder);
    directory_iterator end;

    for(directory_iterator it(folder); it != end; ++it)
    {
        path file = it->path();

        if( is_directory(*it) && format == "") {
            names.push_back(file.filename().string());
            continue;
        }

        if( file.extension() == "."+format) {
            names.push_back(file.filename().string());
            continue;
        }
    }
	std::sort(names.begin(), names.end());
    return names;
}
