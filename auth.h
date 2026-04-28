#ifndef AUTH_H
#define AUTH_H

#include <string>

// SHA-256 hashing for password storage.
// Returns a 64-char lowercase hex digest.
std::string sha256(const std::string& input);

#endif
