#include "PathParser.h"
#include <sstream>
#include <iostream>

using namespace std;

// Split path into components
vector<string> PathParser::split(const string& path) {
    vector<string> components;
    
    if (path.empty()) {
        return components;
    }
    
    stringstream ss(path);
    string component;
    
    while (getline(ss, component, '/')) {
        if (!component.empty() && component != ".") {
            components.push_back(component);
        }
    }
    
    return components;
}

// Check if path is absolute
bool PathParser::isAbsolute(const string& path) {
    return !path.empty() && path[0] == '/';
}

// Normalize path
string PathParser::normalize(const string& path) {
    if (path.empty()) {
        return "/";
    }
    
    // Split into components
    vector<string> components = split(path);
    vector<string> normalized;
    
    // Handle .. and .
    for (const string& comp : components) {
        if (comp == "..") {
            if (!normalized.empty()) {
                normalized.pop_back();
            }
        } else if (comp != ".") {
            normalized.push_back(comp);
        }
    }
    
    // Rebuild path
    if (normalized.empty()) {
        return "/";
    }
    
    string result = "";
    for (const string& comp : normalized) {
        result += "/" + comp;
    }
    
    return result;
}

// Join path components
string PathParser::join(const vector<string>& components) {
    if (components.empty()) {
        return "/";
    }
    
    string result = "";
    for (const string& comp : components) {
        if (!comp.empty()) {
            result += "/" + comp;
        }
    }
    
    return result.empty() ? "/" : result;
}

// Get parent path
string PathParser::getParent(const string& path) {
    if (path == "/" || path.empty()) {
        return "/";
    }
    
    vector<string> components = split(path);
    
    if (components.empty()) {
        return "/";
    }
    
    components.pop_back();
    return join(components);
}

// Get basename
string PathParser::getBasename(const string& path) {
    if (path == "/" || path.empty()) {
        return "/";
    }
    
    vector<string> components = split(path);
    
    if (components.empty()) {
        return "/";
    }
    
    return components.back();
}

// Validate path
bool PathParser::isValid(const string& path) {
    if (path.empty()) {
        return false;
    }
    
    // Check for invalid characters (for simplicity, just check for nulls)
    for (char c : path) {
        if (c == '\0') {
            return false;
        }
    }
    
    return true;
}
