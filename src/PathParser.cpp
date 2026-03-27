#include "PathParser.h"
#include "Constants.h"
#include <sstream>

using namespace std;

vector<string> PathParser::split(const string& path) {
    vector<string> components;
    if (path.empty()) return components;
    stringstream ss(path);
    string token;
    while (getline(ss, token, '/')) {
        if (!token.empty() && token != ".") components.push_back(token);
    }
    return components;
}

bool PathParser::isAbsolute(const string& path) {
    return !path.empty() && path[0] == '/';
}

string PathParser::normalize(const string& path) {
    if (path.empty()) return "/";
    vector<string> parts = split(path);
    vector<string> norm;
    for (const string& p : parts) {
        if (p == "..") { if (!norm.empty()) norm.pop_back(); }
        else if (p != ".") norm.push_back(p);
    }
    if (norm.empty()) return "/";
    string result;
    for (const string& p : norm) result += "/" + p;
    return result;
}

string PathParser::join(const vector<string>& components) {
    if (components.empty()) return "/";
    string result;
    for (const string& c : components) {
        if (!c.empty()) result += "/" + c;
    }
    return result.empty() ? "/" : result;
}

string PathParser::getParent(const string& path) {
    if (path == "/" || path.empty()) return "/";
    vector<string> parts = split(path);
    if (parts.empty()) return "/";
    parts.pop_back();
    return join(parts);
}

string PathParser::getBasename(const string& path) {
    if (path == "/" || path.empty()) return "/";
    vector<string> parts = split(path);
    return parts.empty() ? "/" : parts.back();
}

bool PathParser::isValid(const string& path) {
    if (path.empty()) return false;
    for (char c : path) { if (c == '\0') return false; }
    return true;
}

bool PathParser::isValidName(const string& name) {
    if (name.empty()) return false;
    if (name.length() > MAX_FILENAME_LENGTH) return false;
    for (char c : name) {
        if (c == '/' || c == '\0') return false;
    }
    return true;
}
