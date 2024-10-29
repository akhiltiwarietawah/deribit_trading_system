



#ifndef AUTH_H
#define AUTH_H

#include <string>
#include <nlohmann/json.hpp>   


std::string getAccessToken(const std::string& client_id, const std::string& client_secret);
std::string parseAccessToken(const std::string& jsonResponse);

#endif // AUTH_H
