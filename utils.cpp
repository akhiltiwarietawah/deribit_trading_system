// utils.cpp
#include "utils.h"

using namespace std;

// Function to write data received from cURL into a string
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t totalSize = size * nmemb;  // Calculate the total size of data
    userp->append(static_cast<char*>(contents), totalSize);  // Append data to the string
    return totalSize;  // Return the total size
}
