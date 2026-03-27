#ifndef PATHPARSER_H
#define PATHPARSER_H

#include <string>
#include <vector>

class PathParser {
public:
    // Split a path string into its components (ignoring empty segments)
    static std::vector<std::string> split(const std::string& path);

    // Return true if path starts with '/'
    static bool isAbsolute(const std::string& path);

    // Remove trailing slashes and redundant separators
    static std::string normalize(const std::string& path);

    // Join a list of components with '/' separators
    static std::string join(const std::vector<std::string>& components);

    // Return everything up to the last '/'
    static std::string getParent(const std::string& path);

    // Return the last component of a path
    static std::string getBasename(const std::string& path);

    // Return true if path contains only legal characters
    static bool isValid(const std::string& path);

    // Return true if 'name' is a valid single filename component:
    //   - non-empty
    //   - no '/' or null bytes
    //   - length <= MAX_FILENAME_LENGTH
    static bool isValidName(const std::string& name);
};

#endif // PATHPARSER_H
