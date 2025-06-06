#pragma once
#ifndef KECCAK_HPP
#define KECCAK_HPP
typedef unsigned char u8;
typedef unsigned long long int u64;
#include <string>

std::string keccak256(std::string in_str);

#endif //KECCAK_HPP
