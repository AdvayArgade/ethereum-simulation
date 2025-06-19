#ifndef MPT_HPP
#define MPT_HPP
#include <iostream>
#include <variant>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <iomanip>
using namespace std;

namespace MPT{
    template <typename T>
    struct Node{ 
        virtual string hash_node();
        virtual string to_string();
        virtual ~Node() = default;
    };

    template <typename T>
    struct Branch: public Node<T>{
        vector<variant<shared_ptr<Node<T>>, shared_ptr<T>, string>> branches;
        Branch(): branches(17) {}
        string hash_node() override;
        string to_string() override;
    };

    template <typename T>
    struct Extension: public Node<T>{
        vector<uint8_t> path;
        string hash;
        Extension(){}
        Extension(vector<uint8_t> _path, string _hash): path(_path), hash(_hash) {}
        string hash_node() override;
        string to_string() override;
    };

    template <typename T>
    struct Leaf: public Node<T>{
        vector<uint8_t> path;
        shared_ptr<T> value;
        Leaf(){}
        Leaf(vector<uint8_t> _path, shared_ptr<T> _value): path(_path), value(_value) {}
        string hash_node() override;
        string to_string() override;
    };

    template <typename T>
    std::ostream& operator<<(std::ostream& os, const Extension<T>& ext) {
        os << "[ path: ";
        for (uint8_t byte : ext.path) {
            os << std::hex << std::setw(2) << std::setfill('0') << int(byte) << " ";
        }
        os << std::dec << ", hash: ";
        if (ext.hash) os << ext.hash;
        else os << "null";
        os << " ]";
        return os;
    }

    template <typename T>
    std::ostream& operator<<(std::ostream& os, const Leaf<T>& leaf) {
        os << "[ path: ";
        for (uint8_t byte : leaf.path) {
            os << std::hex << std::setw(2) << std::setfill('0') << int(byte) << " ";
        }
        os << std::dec << ", value: ";
        if (leaf.value) os << *leaf.value;
        else os << "null";
        os << " ]";
        return os;
    }

    template <typename T>
    std::ostream& operator<<(std::ostream& os, const Branch<T>& branch) {
        os << "[ ";
        for (const auto& item : branch.branches) {
            if (std::holds_alternative<std::shared_ptr<Node<T>>>(item)) {
                std::shared_ptr<Node<T>> child = std::get<std::shared_ptr<Node<T>>>(item);
                if (child == nullptr) {
                    os << "null, ";
                } else {
                    os << child->to_string() << ", ";
                }
            }
            else if (std::holds_alternative<std::shared_ptr<T>>(item)) {
                std::shared_ptr<T> val_ptr = std::get<std::shared_ptr<T>>(item);
                os << val_ptr->to_string() << ", ";
            }
            else if (std::holds_alternative<std::string>(item)) {
                os << std::get<std::string>(item) << ", ";
            }
            // You can add more `else if` cases for other types if needed
        }
        os << " ]";
        return os;
    }

    template <typename T>
    class MPTObj{
        unordered_map<string, shared_ptr<Node<T>>> key_value_db;
        
        public:
        shared_ptr<Node<T>> root = nullptr;
        
        shared_ptr<T> retrieve(vector<uint8_t>& key);
        shared_ptr<Node<T>> insert(shared_ptr<Node<T>> root, vector<uint8_t>& key, T& value, int nibble);
        void print_db();
    };

}

#endif //MPT_HPP