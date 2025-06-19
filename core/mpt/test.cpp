// #include "mpt.hpp"
// #include <iostream>
// using namespace std;

// // write to_string for Storage and use that instead of int
// int main(){
//     MPT::Node<int>* node_ptr = new MPT::Branch<int>();
//     for(int i = 0; i<6; i++){
//         variant<shared_ptr<MPT::Node<int>>, shared_ptr<int>, string> var = make_shared<MPT::Node<int>>(nullptr);
//         if(MPT::Branch<int>* branch_ptr = dynamic_cast<MPT::Branch<int>*>(node_ptr)){
//             branch_ptr->branches[i] = var;
//         }
//     }

//     for(int i = 6; i<8; i++){
//         MPT::Leaf<int>* leaf_ptr = new MPT::Leaf<int>();
//         leaf_ptr->path = {1, 2, 3};
//         leaf_ptr->value = make_shared<int>(10);
//         variant<shared_ptr<MPT::Node<int>>, shared_ptr<int>, string> var = make_shared<MPT::Node<int>>();
//         if(MPT::Branch<int>* branch_ptr = dynamic_cast<MPT::Branch<int>*>(node_ptr)){
//             branch_ptr->branches[i] = var;
//         }
//     }
// }

#include "types.hpp"
#include "mpt.hpp"
#include <iostream>
#include <memory>
#include <vector>

using namespace std;
using namespace MPT;

void test_mpt_nodes() {
    // Create a sample value of type Int
    auto val = make_shared<Int>();
    val->val = 42;

    // Implement Int::to_string
    struct IntWithToString : public Int {
        string to_string() const override {
            return "Int(" + std::to_string(val) + ")";
        }
    };

    // Create a Leaf node with key path and value
    auto leaf = make_shared<Leaf<Int>>();
    leaf->path = {0x01, 0x02, 0x03};
    leaf->value = val;

    // Create an Extension node
    auto ext = make_shared<Extension<Int>>();
    ext->path = {0xAB, 0xCD};
    ext->hash = "freshmint";

    // Create a Branch node with children
    auto branch = make_shared<Branch<Int>>();
    branch->branches[0] = leaf;
    branch->branches[1] = ext;
    branch->branches[2] = string("test");
    branch->branches[3] = val;

    cout << "--- LEAF ---" << endl;
    cout << "to_string: " << leaf->to_string() << endl;
    cout << "hash_node: " << leaf->hash_node() << endl;

    cout << "--- EXTENSION ---" << endl;
    cout << "to_string: " << ext->to_string() << endl;
    cout << "hash_node: " << ext->hash_node() << endl;

    cout << "--- BRANCH ---" << endl;
    cout << "to_string: " << branch->to_string() << endl;
    cout << "hash_node: " << branch->hash_node() << endl;
}

enum NodeType{
    LEAF, EXT
};

vector<uint8_t> pack(vector<uint8_t>& path, NodeType type){
    vector<uint8_t> packed;
    if(path.size()==0) return packed;
    int i = 0;
    switch (type)
    {
        // leaf
        case 0:
            if(path.size()%2==0) packed.push_back(0x20);
            else{
                uint8_t first_byte = path[0];
                first_byte |= 0x30;
                i = 1;
                packed.push_back(first_byte);
            }
            break;
        
        // ext
        case 1:
            if(path.size()%2==0) packed.push_back(0x00);
            else{
                uint8_t first_byte = path[0];
                first_byte |= 0x10;
                i = 1;
                packed.push_back(first_byte);
            }
            break;
        default:
            break;
    }
    
    for(; i<path.size()-1; i+=2){
        packed.push_back((path[i]<<4) | path[i+1]);
    }
    return packed;
}

int main(){

    MPTObj<Int> trie;
    vector<uint8_t> key = {0x12, 0x01, 0x34, 0x25};
    Int value(1);
    trie.root = trie.insert(trie.root, key, value, 0);
    // if(shared_ptr<Leaf<Int>> leaf_ptr = dynamic_pointer_cast<Leaf<Int>>(trie.root)){
    //     cout<<*leaf_ptr;
    // }
    // cout<<endl;

    // trie.root = make_shared<Extension<Int>>(key, "abcd");
    // trie.key_value_db[trie.root->hash_node()] = trie.root;

    vector<uint8_t> key1 = {0x12, 0x01};
    Int value1(2);
    trie.root = trie.insert(trie.root, key1, value1, 0);

    vector<uint8_t> key2 = {0x12, 0x01, 0x67};
    Int value2(3);
    trie.root = trie.insert(trie.root, key2, value2, 0);
    cout<<"Root: "<<trie.root->to_string()<<endl;

    // vector<uint8_t> key3 = {0x14, 0x56, 0x78};
    // Int value3(4);
    // trie.root = trie.insert(trie.root, key3, value3, 0);
    trie.print_db();

    auto val = trie.retrieve(key);
    if(val) cout<<"Retrieved value: "<<val->to_string();

    return 0;
}