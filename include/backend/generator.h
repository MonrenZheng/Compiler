#ifndef GENERARATOR_H
#define GENERARATOR_H

#include "ir/ir.h"
#include "backend/rv_def.h"

#include<map>
#include<unordered_map>
#include<string>
#include<vector>
#include<fstream>

namespace backend {

// it is a map bewteen variable and its mem addr, the mem addr of a local variable can be identified by ($sp + off)
struct stackVarMap {
    std::map<ir::Operand, int> _table;
    int curr_offset;    // stack offset of current function
    /**
     * @brief find the addr of a ir::Operand
     * @return the offset
    */
    int find_operand(ir::Operand);
    bool global_search(const ir::Program&, ir::Operand);
    /**
     * @brief add a ir::Operand into current map, alloc space for this variable in memory 
     * @param[in] size: the space needed(in byte)
     * @return the offset
    */
    void add_operand(ir::Operand, uint32_t);
};


struct Generator {
    const ir::Program& program;         // the program to gen
    std::ofstream& fout;                 // output file
    stackVarMap stackmap;
    int curr_idx;  // The order of the current instruction in the function
    int label_idx=0;  // label counter
    std::map<int, std::string> label_map;   // hash map of label index to label name

    Generator(ir::Program&, std::ofstream&);

    // generate wrapper function
    void gen();
    void gen_func(const ir::Function&);
    void gen_instr(ir::Instruction&, std::string&, const ir::Function&);

    void SW(ir::Operand, std::string&);  // store the reg value to the memory address
    void LW(ir::Operand, rv::rvREG, std::string&);  // load the value from the memory address to the reg
};



} // namespace backend


#endif