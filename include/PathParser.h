#ifndef PATHPARSER_H
#define PATHPARSER_H

#include <string>
#include <vector>

class PathParser {
public:
    // Split path into components
    static std::vector<std::string> split(const std::string& path);
    
    // Check if path is absolute (starts with /)
    static bool isAbsolute(const std::string& path);
    
    // Normalize path (remove trailing slashes, handle . and ..)
    static std::string normalize(const std::string& path);
    
    // Join path components
    static std::string join(const std::vector<std::string>& components);
    
    // Get parent path
    static std::string getParent(const std::string& path);
    
    // Get basename (last component of path)
    static std::string getBasename(const std::string& path);
    
    // Validate path (check for invalid characters)
    static bool isValid(const std::string& path);
};

#endif // PATHPARSER_H
