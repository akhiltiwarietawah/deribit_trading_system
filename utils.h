#ifndef UTILS_H
#define UTILS_H

#include <string>

// Function to write data received from curl into a string
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp);

#endif // UTILS_H
