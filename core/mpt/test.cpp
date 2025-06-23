#include "types.hpp"
#include "mpt.hpp"
#include <iostream>
#include <memory>
#include <vector>
#include <random>
#include <sstream>
#include <chrono>
#include <fstream>

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

std::string to_hex_vector(vector<uint8_t> vec){
    std::ostringstream oss;
    for (uint8_t vu8 : vec) {
        oss << std::hex << std::setw(2) << std::setfill('0') << int(vu8) << " ";
    }
    return oss.str();
}

void save_vecvec(const std::vector<std::vector<uint8_t>>& data, const std::string& filename) {
    std::ofstream out(filename, std::ios::binary);
    size_t outer_size = data.size();
    out.write(reinterpret_cast<const char*>(&outer_size), sizeof(outer_size));
    for (const auto& inner : data) {
        size_t inner_size = inner.size();
        out.write(reinterpret_cast<const char*>(&inner_size), sizeof(inner_size));
        out.write(reinterpret_cast<const char*>(inner.data()), inner_size);
    }
    out.close();
}

std::vector<std::vector<uint8_t>> load_vecvec(const std::string& filename) {
    std::ifstream in(filename, std::ios::binary);
    size_t outer_size;
    in.read(reinterpret_cast<char*>(&outer_size), sizeof(outer_size));

    std::vector<std::vector<uint8_t>> data(outer_size);
    for (size_t i = 0; i < outer_size; ++i) {
        size_t inner_size;
        in.read(reinterpret_cast<char*>(&inner_size), sizeof(inner_size));
        data[i].resize(inner_size);
        in.read(reinterpret_cast<char*>(data[i].data()), inner_size);
    }
    in.close();
    return data;
}

int main(){

    MPTObj<Int> trie;
    // vector<uint8_t> key = {0x12, 0x01, 0x34, 0x25};
    // Int value(1);
    // trie.root = trie.insert(trie.root, key, value, 0);
    // // if(shared_ptr<Leaf<Int>> leaf_ptr = dynamic_pointer_cast<Leaf<Int>>(trie.root)){
    // //     cout<<*leaf_ptr;
    // // }
    // // cout<<endl;

    // // trie.root = make_shared<Extension<Int>>(key, "abcd");
    // // trie.key_value_db[trie.root->hash_node()] = trie.root;

    // vector<uint8_t> key1 = {0x12, 0x01};
    // Int value1(2);
    // trie.root = trie.insert(trie.root, key1, value1, 0);

    // vector<uint8_t> key2 = {0x12, 0x01, 0x67};
    // Int value2(3);
    // trie.root = trie.insert(trie.root, key2, value2, 0);

    // vector<uint8_t> key3 = {0x14, 0x56, 0x78};
    // Int value3(4);
    // vector<uint8_t> key4 = {0x00, 0x58, 0x96};
    // Int value4(5);
    // trie.root = trie.insert(trie.root, key3, value3, 0);
    // trie.root = trie.insert(trie.root, key4, value4, 0);

    // vector<vector<uint8_t>> keys = {{0x13, 0x35, 0x56, 0x57, 0x28, 0x90}, {0x12, 0x01, 0x10, 0x43}, {0x00, 0x83, 0x51, 0x95}, {0x13, 0x33, 0x47}, {0x00, 0x58, 0x96}};
    vector<Int> values;

    std::mt19937 gen(std::chrono::steady_clock::now().time_since_epoch().count());
    std::uniform_int_distribution<> len_dist(5, 25);     // reasonable length
    std::uniform_int_distribution<> val_dist(0, 15);     // values in [0, 15]

    std::vector<std::vector<uint8_t>> keys;
    for (int i = 0; i < 1000; ++i) {
        int len = len_dist(gen);
        std::vector<uint8_t> key;
        for (int j = 0; j < len; ++j) {
            key.push_back(static_cast<uint8_t>(val_dist(gen)));
        }
        keys.push_back(pack(key, EXT));
        values.push_back(Int(i+1));
    }

    save_vecvec(keys, "keys.bin");
    // auto keys = load_vecvec("keys.bin");
    // for(int i = 0; i<keys.size(); i++) values.push_back(Int(i+1));
    
    // Optional: print generated keys
    // for (const auto& key : keys) {
    //     cout<<to_hex_vector(key)<<endl;
    // }

    for(int i = 0; i<keys.size(); i++)
    {
        trie.root = trie.insert(trie.root, keys[i], values[i], 0);
    }
    cout<<"----------------Insertion finished.------------------\n";

    // cout<<"Root: "<<trie.root->to_string()<<endl;
    // trie.print_db();

    int num_passed = 0, num_failed = 0;
    for(int i = 0; i<keys.size(); i++){
        auto val = trie.retrieve(keys[i]);
        // if(val) cout<<"Retrieved value: "<<val->to_string()<<endl;
        // else cout<<"Key does not exist.\n";
        if(val){
            if(val->to_string()!=to_string(i+1)){
                cout<<"Failed for "<<to_hex_vector(keys[i])<<endl;
                num_failed++;
            }
            else num_passed++;
        }
        else{
            cout<<"Key does not exist for "<<to_hex_vector(keys[i])<<endl;
            cout<<"Index: "<<i<<endl<<endl;
            num_failed++;
        }
    }

    cout<<num_passed<<" cases passed.\n";
    
    return 0;
}