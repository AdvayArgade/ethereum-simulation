#include "mpt.hpp"
#include "types.hpp"
#include "../hash/keccak.hpp"
#include <variant>
#include <string>
#include <iomanip>
#include <sstream>
using namespace MPT;
typedef vector<uint8_t> vu8;

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
    in += " ]";
    return in;
}

void print_hex(vu8 vec){
    for (uint8_t vu8 : vec) {
        std::cout << std::hex << std::setw(2) << std::setfill('0') << int(vu8) << " ";
    }
}

std::string to_hex_string(vu8 vec){
    std::ostringstream oss;
    for (uint8_t vu8 : vec) {
        oss << std::hex << std::setw(2) << std::setfill('0') << int(vu8) << " ";
    }
    return oss.str();
}

template <typename T>
std::string Extension<T>:: to_string(){
    std::string in = "[ path: ";
    in += to_hex_string(path);
    
    in += "hash: " + this->hash + " ]";

    return in;
}

template <typename T>
std::string Leaf<T>:: to_string(){
    std::string in = "[ path: ";
    in += to_hex_string(path);
    
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

inline uint8_t get_higher_nibble(uint8_t& val){
    uint8_t mask = 0b11110000;
    return (val & mask)>>4;
}

inline uint8_t get_lower_nibble(uint8_t& val){
    uint8_t mask = 0b00001111;
    return val & mask;
}

inline void higher_nibble_to_ext_even(uint8_t& val){
    uint8_t mask = 0b00010000;
    val |= mask;
    mask = 0b00011111;
    val &= mask;
}

inline void higher_nibble_to_leaf_even(uint8_t& val){
    uint8_t mask = 0b00100000;
    val |= mask;
    mask = 0b00101111;
    val &= mask;
}

inline void higher_nibble_to_leaf_odd(uint8_t& val){
    uint8_t mask = 0b00110000;
    val |= mask;
    mask = 0b00111111;
    val &= mask;
}

enum NodeType{
    LEAF, EXT
};

vu8 pack_nibbles(vu8& path, NodeType type){
    vu8 packed;
    if(path.size()==0) return packed;
    int i = 0;
    switch (type)
    {
        // leaf
        case 0:
            if(path.size()%2==0) packed.push_back(0x20);
            else{
                uint8_t first_vu8 = path[0];
                first_vu8 |= 0x30;
                i = 1;
                packed.push_back(first_vu8);
            }
            break;
        
        // ext
        case 1:
            if(path.size()%2==0) packed.push_back(0x00);
            else{
                uint8_t first_vu8 = path[0];
                first_vu8 |= 0x10;
                i = 1;
                packed.push_back(first_vu8);
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

inline uint8_t get_kth_nibble(vu8& path, int k){
    if(k>=2*path.size()) return 0xff;
    uint8_t key_nibble = (k%2==0) ? get_higher_nibble(path[k/2]) : get_lower_nibble(path[k/2]);
    return key_nibble;
}

vu8 get_suffix(vu8& path, int nibble){
    vu8 suffix;
    if(nibble==2*path.size()) return suffix;

    for(int i = nibble; i<2*path.size(); i++){
        uint8_t key_nibble = get_kth_nibble(path, i);
        suffix.push_back(key_nibble);
    }

    return suffix;    
}

int get_offset(vu8 path){
    if(path.empty()) return 0;
    uint8_t parity = get_higher_nibble(path[0]);
    return (parity % 2 == 0) ? 2 : 1;
}

vector<vu8> find_common_prefix(
    vu8& node_path, 
    vu8& key_path, 
    int start,
    int& key_idx_out,
    int& node_idx_out){
    vu8 path, key_suffix, node_suffix;

    if(node_path.empty()) return {{}, get_suffix(key_path, start), {}};

    // ignore the prefix encoding in the leaf
    uint8_t parity = get_higher_nibble(node_path[0]);
    int offset = (parity % 2 == 0) ? 2 : 1;
    if(key_path.empty()) return {{}, {}, get_suffix(node_path, offset)};

    int min_len = std::min(node_path.size()*2-offset, (key_path.size()*2-start));

    int key_idx = start, node_idx = offset;
    if(start==0) key_idx = get_offset(key_path);

    for(int i = 0; i<min_len; i++, node_idx++, key_idx++){

        uint8_t node_nibble, key_nibble;
        node_nibble = (node_idx%2==0) ? get_higher_nibble(node_path[node_idx/2]) : get_lower_nibble(node_path[node_idx/2]);
        key_nibble = (key_idx%2==0) ? get_higher_nibble(key_path[key_idx/2]) : get_lower_nibble(key_path[key_idx/2]);

        // cout<<"Leaf Nibble: "<<int(node_nibble)<<"\tKey nibble: "<<int(key_nibble)<<endl;
        if(node_nibble==key_nibble) path.push_back(node_nibble);
        else break;
    }

    // fill the unmatched parts in suffix vectors
    for(int j = node_idx; j<node_path.size()*2; j++){
        uint8_t node_nibble = (j%2==0) ? get_higher_nibble(node_path[j/2]) : get_lower_nibble(node_path[j/2]);
        node_suffix.push_back(node_nibble);
    }

    for(int j = key_idx; j<key_path.size()*2; j++){
        uint8_t key_nibble = (j%2==0) ? get_higher_nibble(key_path[j/2]) : get_lower_nibble(key_path[j/2]);
        key_suffix.push_back(key_nibble);
    }

    key_idx_out = key_idx; node_idx_out = node_idx;
    vector<vu8> ret = {path, key_suffix, node_suffix};
    return ret;
}

// pass the path with prefix encoded length in the first nibble: 
// 0: even, 1: odd
template <typename T>
std::shared_ptr<Node<T>> MPTObj<T>::insert(shared_ptr<Node<T>> root, vu8& key, T& value, int nibble){
    if(nibble==2*key.size()) return nullptr;
    uint8_t parity = get_higher_nibble(key[0]);
    int nibble_offset = (nibble==0) ? get_offset(key) : 0;

    // case 1: we stop at null
    if(root==nullptr){
        auto leaf_ptr = std::make_shared<Leaf<T>>();

        vu8 suffix;
        if(nibble%2==1){
            higher_nibble_to_leaf_odd(key[nibble/2]);
        }
        else if(nibble!=0){
            suffix.push_back(0b00100000);
        }
        else if(parity==1){
            higher_nibble_to_leaf_odd(key[0]);
        }
        else{
            higher_nibble_to_leaf_even(key[0]);
        }

        for(int i = nibble/2; i<key.size(); i++) suffix.push_back(key[i]);

        leaf_ptr->path = suffix;
        leaf_ptr->value = std::make_shared<T>(value);

        key_value_db[leaf_ptr->hash_node()] = leaf_ptr;
        return leaf_ptr;
    }

    // case 2: we stop at leaf
    else if(shared_ptr<Leaf<T>> leaf_ptr = dynamic_pointer_cast<Leaf<T>>(root)){
        // cout<<"In leaf.\n";
        // remove the leaf from key value db
        key_value_db.erase(leaf_ptr->hash_node());

        vu8 path, key_suffix, leaf_suffix;

        int key_idx, leaf_idx;
        auto path_suffix = find_common_prefix(leaf_ptr->path, key, nibble, key_idx, leaf_idx);
        path = path_suffix[0];
        key_suffix = path_suffix[1];
        leaf_suffix = path_suffix[2];
        int key_suffix_size = key_suffix.size(), leaf_suffix_size = leaf_suffix.size();

        // pack into byte format
        vu8 packed = pack_nibbles(path, EXT);        

        // if leaf's path and key's suffix are same, change key's value
        if(key_suffix.size()==0 && leaf_suffix.size()==0){
            leaf_ptr->value = std::make_shared<T>(value);
            key_value_db[leaf_ptr->hash_node()] = leaf_ptr;
            return root;
        }

        if(leaf_suffix_size>0) leaf_suffix.erase(leaf_suffix.begin());
        if(key_suffix_size>0) key_suffix.erase(key_suffix.begin());

        vu8 packed_leaf_suffix = pack_nibbles(leaf_suffix, LEAF);
        vu8 packed_key_suffix = pack_nibbles(key_suffix, LEAF);

        // else make an extension node
        auto ext = std::make_shared<Extension<T>>();
        ext->path = packed;
        
        // make a branch node
        auto branch = std::make_shared<Branch<T>>();

        // case 2.1: partial match => branch with 2 leaves
        if(leaf_suffix_size>0 && key_suffix_size>0){
            uint8_t leaf_nibble = (leaf_idx%2==0) ? get_higher_nibble(leaf_ptr->path[leaf_idx/2]) : get_lower_nibble(leaf_ptr->path[leaf_idx/2]);
            uint8_t key_nibble = (key_idx%2==0) ? get_higher_nibble(key[key_idx/2]) : get_lower_nibble(key[key_idx/2]);

            auto leaf_leaf_ptr = std::make_shared<Leaf<T>>(packed_leaf_suffix, leaf_ptr->value);
            auto key_leaf_ptr = std::make_shared<Leaf<T>>(packed_key_suffix, std::make_shared<T>(value));

            branch->branches[leaf_nibble] = leaf_leaf_ptr->hash_node();
            branch->branches[key_nibble] = key_leaf_ptr->hash_node();

            key_value_db[leaf_leaf_ptr->hash_node()] = leaf_leaf_ptr;
            key_value_db[key_leaf_ptr->hash_node()] = key_leaf_ptr;
        }
        
        // case 2.2: leaf path fully matched but unmatched suffix in key
        else if(key_suffix_size>0){
            uint8_t key_nibble = (key_idx%2==0) ? get_higher_nibble(key[key_idx/2]) : get_lower_nibble(key[key_idx/2]);

            auto key_leaf_ptr = std::make_shared<Leaf<T>>(packed_key_suffix, std::make_shared<T>(value));

            branch->branches[key_nibble] = key_leaf_ptr->hash_node();
            branch->branches[16] = make_shared<T>(value);

            key_value_db[key_leaf_ptr->hash_node()] = key_leaf_ptr;
        }

        // case 2.3: key path fully matched but unmatched leaf suffix
        else if(leaf_suffix_size>0){
            uint8_t leaf_nibble = (leaf_idx%2==0) ? get_higher_nibble(leaf_ptr->path[leaf_idx/2]) : get_lower_nibble(leaf_ptr->path[leaf_idx/2]);

            auto leaf_leaf_ptr = std::make_shared<Leaf<T>>(packed_leaf_suffix, leaf_ptr->value);

            branch->branches[leaf_nibble] = leaf_leaf_ptr->hash_node();
            branch->branches[16] = make_shared<T>(value);

            key_value_db[leaf_leaf_ptr->hash_node()] = leaf_leaf_ptr; 
        }

        ext->hash = branch->hash_node();
        key_value_db[branch->hash_node()] = branch;
        key_value_db[ext->hash_node()] = ext;

        return ext;
    }

    // case 3: encountering an extension node
    else if(shared_ptr<Extension<T>> ext_ptr = dynamic_pointer_cast<Extension<T>>(root)){
        // cout<<"In ext\n";
        vu8 path, key_suffix, ext_suffix;

        int key_idx, ext_idx;
        auto path_suffix = find_common_prefix(ext_ptr->path, key, nibble, key_idx, ext_idx);
        path = path_suffix[0];
        key_suffix = path_suffix[1];
        ext_suffix = path_suffix[2];

        // cout<<"Key suffix: "<<to_hex_string(key_suffix)<<endl;
        // cout<<"Ext suffix: "<<to_hex_string(ext_suffix)<<endl;

        // case 3.1: full match with no remaining suffixes => update the value of the node ext points to
        if(key_suffix.size()==0 && ext_suffix.size()==0){
            auto next_ptr = key_value_db[ext_ptr->hash];
            key_value_db.erase(next_ptr->hash_node());

            if(shared_ptr<Leaf<T>> leaf_ptr = dynamic_pointer_cast<Leaf<T>>(next_ptr)){
                leaf_ptr->value = make_shared<T>(value);
                leaf_ptr->path.clear();
                
                key_value_db[leaf_ptr->hash_node()] = leaf_ptr;
            }

            else if(shared_ptr<Branch<T>> branch_ptr = dynamic_pointer_cast<Branch<T>>(next_ptr)){
                branch_ptr->branches[16] = make_shared<T>(value);
                
                key_value_db[branch_ptr->hash_node()] = branch_ptr;
            }

            ext_ptr->hash = next_ptr->hash_node();
            key_value_db[ext_ptr->hash_node()] = ext_ptr;
            return ext_ptr;
        }

        // case 3.2: ext suffix fully matches a substring of key and key suffix is left unmatched
        // => jump to next node and recur
        else if(key_suffix.size()>0 && ext_suffix.size()==0){
            int offset = get_offset(ext_ptr->path);
            int jump = nibble + ext_ptr->path.size()*2 - offset + nibble_offset;
            // cout<<"Jump: "<<jump<<endl;

            auto ret_ptr = insert(key_value_db[ext_ptr->hash], key, value, jump);
            key_value_db.erase(ext_ptr->hash);
            key_value_db[ret_ptr->hash_node()] = ret_ptr;

            key_value_db.erase(ext_ptr->hash_node());
            ext_ptr->hash = ret_ptr->hash_node();
            key_value_db[ext_ptr->hash_node()] = ext_ptr;

            return ext_ptr;
        }

        // case 3.3: partial match of ext path and key
        // => new ext with matched prefix and branch with ext of unmatched suffix of previous ext
        else if(ext_suffix.size()>0){
            // cout<<"\nPartial match in ext\n";
            key_value_db.erase(ext_ptr->hash_node());
            
            uint8_t suffix_ext_pos = ext_suffix[0], leaf_pos;
            ext_suffix.erase(ext_suffix.begin());

            auto branch_ptr = make_shared<Branch<T>>();
            auto suffix_ext_ptr = make_shared<Extension<T>>(pack_nibbles(ext_suffix, EXT), ext_ptr->hash);

            if(!key_suffix.empty()){
                leaf_pos = key_suffix[0];
                key_suffix.erase(key_suffix.begin());

                auto leaf_ptr = make_shared<Leaf<T>>(pack_nibbles(key_suffix, LEAF), make_shared<T>(value));
                branch_ptr->branches[leaf_pos] = leaf_ptr->hash_node();
                key_value_db[leaf_ptr->hash_node()] = leaf_ptr;
            }
            else branch_ptr->branches[16] = make_shared<T>(value);

            branch_ptr->branches[suffix_ext_pos] = suffix_ext_ptr->hash_node(); 

            ext_ptr->path = pack_nibbles(path, EXT);
            ext_ptr->hash = branch_ptr->hash_node();

            key_value_db[ext_ptr->hash_node()] = ext_ptr;
            key_value_db[branch_ptr->hash_node()] = branch_ptr;
            key_value_db[suffix_ext_ptr->hash_node()] = suffix_ext_ptr;

            return ext_ptr;
        }

    }
    
    // case 4: encountering a branch node 
    else if(shared_ptr<Branch<T>> branch_ptr = dynamic_pointer_cast<Branch<T>>(root)){
        // cout<<"In branch.\n";
        key_value_db.erase(branch_ptr->hash_node());

        // case 4.1: halt at branch => change the value
        if(nibble==2*key.size()-1){
            branch_ptr->branches[16] = make_shared<T>(value);
        }

        // case 4.2: either recur or create leaf
        else{
            if(nibble==0) nibble += nibble_offset;
            // cout<<"Nibble: "<<nibble<<endl;
            uint8_t key_nibble = (nibble%2==0) ? get_higher_nibble(key[nibble/2]) : get_lower_nibble(key[nibble/2]);
            // cout<<"Key Nibble: "<<key_nibble<<endl;
            auto child = branch_ptr->branches[key_nibble];
            bool make_leaf = false;

            if(holds_alternative<shared_ptr<Node<T>>>(child)){
                auto next_ptr = get<shared_ptr<Node<T>>>(child);
                
                if(next_ptr!=nullptr){
                    cout<<"Branch holds non-null shared_ptr<Node<T>> at "<<int(key_nibble)<<endl;
                    auto ret_ptr = insert(next_ptr, key, value, nibble+1);
                    branch_ptr->branches[key_nibble] = ret_ptr->hash_node();
                }
                else make_leaf = true;
            } 
            else if(holds_alternative<string>(child)){
                auto next_ptr = get<string>(child);
                if(!next_ptr.empty()){
                    auto ret_ptr = insert(key_value_db[next_ptr], key, value, nibble+1);
                    branch_ptr->branches[key_nibble] = ret_ptr->hash_node();
                }
                else make_leaf = true;
            }
            
            if(make_leaf){
                vu8 suffix = get_suffix(key, nibble+1);
                suffix = pack_nibbles(suffix, LEAF);

                auto leaf_ptr = make_shared<Leaf<T>>(suffix, make_shared<T>(value));
                branch_ptr->branches[key_nibble] = leaf_ptr->hash_node();
                key_value_db[leaf_ptr->hash_node()] = leaf_ptr;
            }
        }
        
        key_value_db[branch_ptr->hash_node()] = branch_ptr;
        return branch_ptr;
    }
    return root;
}

template <typename T>
shared_ptr<T> MPTObj<T>::retrieve(vu8& key){
    if(key.empty()) return nullptr;

    int index = get_offset(key);
    auto curr = root;

    while(index<=key.size()*2){
        if(!curr){
            cout<<"curr is null.\n";
            return nullptr;
        } 

        // cout<<"Curr: "<<curr->to_string()<<endl;
        // case 1: leaf
        if(shared_ptr<Leaf<T>>leaf_ptr = dynamic_pointer_cast<Leaf<T>>(curr)){
            // cout<<"In leaf\n";
            int dummy = 0;
            auto path_suffix = find_common_prefix(leaf_ptr->path, key, index, index, dummy);
            vu8 key_suffix = path_suffix[1];
            vu8 leaf_suffix = path_suffix[2];

            if(key_suffix.size()==0 && leaf_suffix.size()==0) return leaf_ptr->value;
            return nullptr;
        }

        // case 2: ext
        if(shared_ptr<Extension<T>>ext_ptr = dynamic_pointer_cast<Extension<T>>(curr)){
            // cout<<"In ext\n";
            int dummy = 0;
            auto path_suffix = find_common_prefix(ext_ptr->path, key, index, index, dummy);
            vu8 key_suffix = path_suffix[1];
            vu8 ext_suffix = path_suffix[2];

            // cout<<"Key suffix: "<<to_hex_string(key_suffix)<<endl;
            // cout<<"Ext suffix: "<<to_hex_string(ext_suffix)<<endl;
            // case 2.1: unmatched ext suffix => invalid key
            if(ext_suffix.size()>0) return nullptr;

            // case 2.2: key ends at ext's path
            if(key_suffix.size()==0){
                auto next_ptr = key_value_db[ext_ptr->hash];

                // fetch the value from the next node
                if(shared_ptr<Leaf<T>>leaf_ptr = dynamic_pointer_cast<Leaf<T>>(next_ptr)){
                    if(leaf_ptr->path.empty()) return leaf_ptr->value;
                    else return nullptr;
                }

                else if(shared_ptr<Branch<T>>branch_ptr = dynamic_pointer_cast<Branch<T>>(next_ptr)) 
                return get<shared_ptr<T>>(branch_ptr->branches[16]);
            }

            // case 2.3: key longer than ext's path
            // => jump and continue
            else curr = key_value_db[ext_ptr->hash];
        }

        // case 3: branch
        if(shared_ptr<Branch<T>>branch_ptr = dynamic_pointer_cast<Branch<T>>(curr)){
            // cout<<"In branch\n";
            if(index==key.size()*2){
                auto child = branch_ptr->branches[16];
                if(holds_alternative<shared_ptr<T>>(child)){
                    return get<shared_ptr<T>>(child);
                }
                else if(holds_alternative<shared_ptr<Node<T>>>(child)){
                    cout<<"17th child overwritten as shared_ptr<Node<T>>.\n";
                    auto child_ptr = get<shared_ptr<Node<T>>>(child);
                    if(child_ptr==nullptr) cout<<"It is null.\n";
                    return nullptr;
                }
                else if(holds_alternative<string>(child)){
                    cout<<"17th child overwritten as string.\n";
                    return nullptr;
                }
            } 
            else {
                auto child = branch_ptr->branches[get_kth_nibble(key, index++)];

                if(holds_alternative<shared_ptr<Node<T>>>(child)) curr = get<shared_ptr<Node<T>>>(child);
                else if(holds_alternative<string>(child)) curr = key_value_db[get<string>(child)];
            }
        }
    }
    return nullptr;
}

template <typename T>
void MPTObj<T>::print_db(){
    for (const auto& [key, value_ptr] : key_value_db) {
        std::cout << key << " => ";
        if (value_ptr) {
            std::cout << value_ptr->to_string();
        } else {
            std::cout << "null";
        }
        std::cout << std::endl;
    }
}

template class Node<Int>;
template class Branch<Int>;
template class Leaf<Int>;
template class Extension<Int>;
template class MPTObj<Int>;