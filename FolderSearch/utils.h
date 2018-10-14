#pragma once
bool replace(std::string& str, const std::string& from, const std::string& to);
std::string wsToString(wchar_t* wchar);
LPCWSTR stringToWS(std::string str);
template<typename Out>
void split(const std::string &s, char delim, Out result);
std::vector<std::string> split(const std::string &s, char delim);