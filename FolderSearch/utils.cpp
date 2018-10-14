#include "utils.h"
#include "stdafx.h"

bool replace(std::string& str, const std::string& from, const std::string& to){
	size_t start_pos = str.find(from);
	if (start_pos == std::string::npos)
		return false;
	str.replace(start_pos, from.length(), to);
	return true;
}
std::string wsToString(wchar_t* wchar) {
	std::wstring ws(wchar);
	return std::string(ws.begin(), ws.end());
}
LPCWSTR stringToWS(std::string str) {
	std::wstring wsString = std::wstring(str.begin(), str.end());
	return wsString.c_str();
}
template<typename Out>
void split(const std::string &s, char delim, Out result) {
	std::stringstream ss(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		*(result++) = item;
	}
}
std::vector<std::string> split(const std::string &s, char delim) {
	std::vector<std::string> elems;
	split(s, delim, std::back_inserter(elems));
	return elems;
}