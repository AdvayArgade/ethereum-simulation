#include "types.hpp"
#include <string>
using namespace std;

string MPT::Storage::to_string() const {
    if (holds_alternative<string>(value)) {
        return get<string>(value);
    } else {
        const vector<uint8_t>& bytes = get<vector<uint8_t>>(value);
        string hex;
        for (uint8_t byte : bytes) {
            char buf[3];
            sprintf(buf, "%02x", byte);
            hex += buf;
        }
        return hex;
    }
}

string MPT::Int:: to_string() const {
    return std::to_string(this->val);
}
