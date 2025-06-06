#include "mpt.hpp"
#include "types.hpp"
#include "../hash/keccak.hpp"
#include <variant>
#include <string>
#include <iomanip>
#include <sstream>
using namespace MPT;

// overload ostream and to_string in T's namespace
template <typename T>
std::string Branch<T>:: to_string(){
    std::string in = "[ ";
    for(auto& item: this->branches){
        if(std::holds_alternative<shared_ptr<Node<T>>>(item)){
            shared_ptr<Node<T>> child = std::get<shared_ptr<Node<T>>>(item);
            if(child==nullptr){
                in += "null, ";
                continue;
            }

            in += child->to_string() + ", ";
        }

        else if(std::holds_alternative<shared_ptr<T>>(item)){
            shared_ptr<T> val_ptr = std::get<shared_ptr<T>>(item);
            in += val_ptr->to_string() + ", ";
        }

        else if(std::holds_alternative<std::string>(item)){
            in += std::get<std::string>(item) + ", ";
        }

    }
    in += "]";
    return in;
}

template <typename T>
std::string Extension<T>:: to_string(){
    std::string in = "[path: ";
    for(auto& item: this->path){
        in += std::to_string(item) + ", ";
    }
    
    in += "hash: " + this->hash + " ]";

    return in;
}

template <typename T>
std::string Leaf<T>:: to_string(){
    std::string in = "[path: ";
    for(auto& item: this->path){
        in += std::to_string(item) + ", ";
    }
    
    shared_ptr<T> val_ptr = this->value;
    if(val_ptr) in += " value: " + val_ptr->to_string() + " ]";

    return in;
}

std::string to_hex(const std::string& input) {
    std::ostringstream oss;
    for (unsigned char c : input) {
        oss << std::hex << std::setw(2) << std::setfill('0') << (int)c;
    }
    return oss.str();
}

template <typename T>
std::string Branch<T>:: hash_node(){
    std::string in = "";
    for(auto& item: this->branches){
        if(std::holds_alternative<shared_ptr<Node<T>>>(item)){
            shared_ptr<Node<T>> child = std::get<shared_ptr<Node<T>>>(item);
            if(child==nullptr) continue;

            in += child->hash_node();
        }

        else if(std::holds_alternative<shared_ptr<T>>(item)){
            shared_ptr<T> val_ptr = std::get<shared_ptr<T>>(item);
            in += val_ptr->to_string();
        }

        else if(std::holds_alternative<std::string>(item)){
            in += std::get<std::string>(item);
        }
    
    }

    return to_hex(keccak256(in));
}

template <typename T>
std::string Extension<T>:: hash_node(){
    return to_hex(keccak256(this->to_string()));
}

template <typename T>
std::string Leaf<T>::hash_node(){
    return to_hex(keccak256(this->to_string()));
}

template <typename T>
std::string Node<T>::to_string() {
    return "Node<T>";
}

template <typename T>
std::string Node<T>::hash_node() {
    return "Node<T> hash";
}

template class Node<Int>;
template class Branch<Int>;
template class Leaf<Int>;
template class Extension<Int>;