#include "keccak.hpp"
#include <iostream>
#include <string>
using namespace std;

int main(){
    string in = "Ethereum";
    string out = keccak256(in);
    
    cout<<out;
    return 0;
}