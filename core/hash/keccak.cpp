extern "C" {
    #include "Keccak-more-compact.c"
}

#include "keccak.hpp"
#include <string>
using namespace std;

string keccak256(string in_str){
    u64 inLen = in_str.size();
    const u8* in = reinterpret_cast<const unsigned char*>(in_str.data());
    u8 out[32];
    Keccak(1088, 512, in, inLen, 0x01, out, 32);
    string s( reinterpret_cast<const char*>(out), 32 ) ;
    return s;
}