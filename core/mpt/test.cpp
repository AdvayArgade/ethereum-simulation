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

int main(){
    test_mpt_nodes();
    return 0;
}