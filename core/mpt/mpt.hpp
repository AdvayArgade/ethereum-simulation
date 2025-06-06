#ifndef MPT_HPP
#define MPT_HPP
#include <iostream>
#include <variant>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
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
        string hash_node() override;
        string to_string() override;
    };

    template <typename T>
    struct Leaf: public Node<T>{
        vector<uint8_t> path;
        shared_ptr<T> value;
        string hash_node() override;
        string to_string() override;
    };

    template <typename T>
    class MPT{
        unordered_map<string, shared_ptr<Node<T>>> key_value_db;
        public:
        shared_ptr<Node<T>> root;
        
        T retrieve(vector<uint8_t> key);
        void insert(vector<uint8_t> key, T value);

    };

}

#endif //MPT_HPP