#ifndef TYPES_HPP
#define TYPES_HPP
#include <string>
#include <memory>
#include <variant>
#include <vector>
using namespace std;

namespace MPT{
    template <typename T>
    class MPT;

    struct TreeType {
        virtual string to_string() const = 0;
        virtual ~TreeType() = default;
    };

    struct Int: public TreeType{
        int val;
        string to_string() const override;
    };

    struct Storage: public TreeType{
        variant<string, vector<uint8_t>> value;
        string to_string() const override;
    };

    struct State: public TreeType{
        uint64_t nonce, balance;
        shared_ptr<MPT<Storage>> storage_root;
        string codeHash;
        string to_string() const override;
    };
}

#endif