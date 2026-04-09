#include"backend/generator.h"
#include<iostream>
#include<assert.h>
#include<algorithm>
#include<vector>
#include<unordered_map>

using ir::Operand;
using ir::Operator;
using rv::rvREG;
using rv::rvOPCODE;

#define TODO assert(0 && "todo")
#define RTYPE(op, rd, rs1, rs2) \
    strs += "\t" + rv::toString(rvOPCODE::op) + "\t" + \
            rv::toString(rd) + "," + \
            rv::toString(rs1) + "," + \
            rv::toString(rs2) + "\n";

#define ITYPE(op, rd, rs, imm) \
    strs += "\t" + rv::toString(rvOPCODE::op) + "\t" + \
            rv::toString(rd) + "," + \
            rv::toString(rs) + "," + \
            imm + "\n";

#define UNARY(op, rd, rs) \
    strs += "\t" + rv::toString(rvOPCODE::op) + "\t" + \
            rv::toString(rd) + "," + \
            rv::toString(rs) + "\n";

std::unordered_map<int, rvREG> table = {{0, rvREG::X10}, {1, rvREG::X11}, {2, rvREG::X12}, {3, rvREG::X13}, {4, rvREG::X14}, {5, rvREG::X15}, {6, rvREG::X16}, {7, rvREG::X17}};

// 整数寄存器toString
std::string rv::toString(rvREG r) {
    switch (r) {
        case rvREG::X0: return "zero";
        case rvREG::X1: return "ra";
        case rvREG::X2: return "sp";
        case rvREG::X3: return "gp";
        case rvREG::X4: return "tp";

        case rvREG::X5: return "t0";
        case rvREG::X6: return "t1";
        case rvREG::X7: return "t2";

        case rvREG::X8: return "s0";
        case rvREG::X9: return "s1";

        case rvREG::X10: return "a0";
        case rvREG::X11: return "a1";
        case rvREG::X12: return "a2";
        case rvREG::X13: return "a3";
        case rvREG::X14: return "a4";
        case rvREG::X15: return "a5";
        case rvREG::X16: return "a6";
        case rvREG::X17: return "a7";

        case rvREG::X18: return "s2";
        case rvREG::X19: return "s3";
        case rvREG::X20: return "s4";
        case rvREG::X21: return "s5";
        case rvREG::X22: return "s6";
        case rvREG::X23: return "s7";
        case rvREG::X24: return "s8";
        case rvREG::X25: return "s9";
        case rvREG::X26: return "s10";
        case rvREG::X27: return "s11";

        case rvREG::X28: return "t3";
        case rvREG::X29: return "t4";
        case rvREG::X30: return "t5";
        case rvREG::X31: return "t6";

        default:
            assert(0 && "invalid rvREG");
        }

    return "";
}

// opcode toString
std::string rv::toString(rvOPCODE r){
    switch (r) {
        case rvOPCODE::ADD: return "add";
        case rvOPCODE::SUB: return "sub";
        case rvOPCODE::MUL: return "mul";
        case rvOPCODE::DIV: return "div";
        case rvOPCODE::REM: return "rem";
        case rvOPCODE::XOR: return "xor";
        case rvOPCODE::OR:  return "or";
        case rvOPCODE::AND: return "and";
        case rvOPCODE::SLL: return "sll";
        case rvOPCODE::SRL: return "srl";
        case rvOPCODE::SRA: return "sra";
        case rvOPCODE::SLT: return "slt";
        case rvOPCODE::SLTU: return "sltu";
        
        case rvOPCODE::ADDI: return "addi";
        case rvOPCODE::XORI: return "xori";
        case rvOPCODE::ORI:  return "ori";
        case rvOPCODE::ANDI: return "andi";
        case rvOPCODE::SLLI: return "slli";
        case rvOPCODE::SRLI: return "srli";
        case rvOPCODE::SLTI: return "slti";
        case rvOPCODE::SLTIU: return "sltiu";
        case rvOPCODE::NOT: return "not";
        case rvOPCODE::SEQZ: return "seqz";
        case rvOPCODE::SNEZ: return "snez";

        case rvOPCODE::LW: return "LW";
        case rvOPCODE::SW: return "SW";

        case rvOPCODE::BEQ: return "beq";
        case rvOPCODE::BNE: return "bne";
        case rvOPCODE::BLT: return "blt";
        case rvOPCODE::BGE: return "bge";
        case rvOPCODE::BLTU: return "bltu";
        case rvOPCODE::BGEU: return "bgeu";
        case rvOPCODE::BNEZ: return "bnez";
        
        case rvOPCODE::JAL: return "jal";
        case rvOPCODE::JALR: return "jalr";

        case rvOPCODE::NOP: return "nop";

        case rvOPCODE::CALL: return "call";

        case rvOPCODE::LA: return "la";
        case rvOPCODE::LI: return "li"; 
        case rvOPCODE::MOV: return "mv"; 
        case rvOPCODE::J: return "j";
        case rvOPCODE::JR: return "jr";

        default:
            assert(0 && "invalid rvOPCODE");
    }

    return "";
}


// return the offset of the oprand
int backend::stackVarMap::find_operand(ir::Operand op){
    if(_table.find(op) != _table.end()) return _table[op]; // find the oprand
    return -1;
}


// search for a global operand in the program
bool backend::stackVarMap::global_search(const ir::Program& p, ir::Operand op){
    for(int i = 0; i < p.globalVal.size(); i++)
        if(p.globalVal[i].val.name == op.name)
            return true;

    return false;     // no result
}


// add new oprand and update the offset
void backend::stackVarMap::add_operand(ir::Operand op, uint32_t size = 4){
    _table.insert({op, curr_offset});
    curr_offset += size;    // Update offset
}


// Initialize Generator
backend::Generator::Generator(ir::Program& p, std::ofstream& f): program(p), fout(f){}


// LW Inst
void backend::Generator::LW(Operand op, rvREG reg, std::string& strs){
    if(stackmap.find_operand(op) != -1){     // in stack
        strs += ("\t" + toString(rvOPCODE::LW) + "\t" + toString(reg) + "," + std::to_string(stackmap.find_operand(op)) + "(" + toString(rvREG::X2) + ")\n");
    }else{      // not in the stack
        strs += ("\t" + toString(rvOPCODE::LA) + "\t" + toString(reg) + "," + op.name + "\n"); // load address
        strs += ("\t" + toString(rvOPCODE::LW) + "\t" + toString(reg) + ",0(" + toString(reg) + ")\n"); // get the value
    }
}


// SW Inst
void backend::Generator::SW(Operand op, std::string& strs){
    if(stackmap.find_operand(op) != -1){     // in stack: through stack point and offset
        strs += ("\t" + toString(rvOPCODE::SW) + "\t" + toString(rvREG::X7) + "," + std::to_string(stackmap.find_operand(op)) + "(" + toString(rvREG::X2) + ")\n");
    }
    else if(stackmap.global_search(Generator::program, op)){  // global variable: through global variable name
        strs += ("\t" + toString(rvOPCODE::LA) + "\t" + toString(rvREG::X28) + "," + op.name + "\n");  //  reg X28 (t3)
        strs += ("\t" + toString(rvOPCODE::SW) + "\t" + toString(rvREG::X7) + ",0(" + toString(rvREG::X28) + ")\n");  // storation
    }
    else{      
        // not exist, create a new stack variable
        stackmap.add_operand(op);
        strs += ("\t" + toString(rvOPCODE::SW) + "\t" + toString(rvREG::X7) + "," + std::to_string(stackmap.find_operand(op)) + "(" + toString(rvREG::X2) + ")\n");  // allocate space in stack
    }
}


bool search_oprand(std::vector<ir::Operand> vec, ir::Operand op){
    for(auto u: vec){
        if(u.name == op.name) // check
            return true;    // found
    }
    return false; // not found
}

// Program
void backend::Generator::gen(){
    // header
    std::string strs = "";
    strs += "\t.option nopic\n";

    auto global_f = program.functions[0];    // global function
    if(global_f.InstVec.size() > 1)    // have global variable
        strs += "\t.data\n";

    //  process global variables
    for(int i = 0; i < global_f.InstVec.size(); i++){
        auto inst = global_f.InstVec[i];
        // define global variable
        if(inst->op == Operator::def){
            strs += ("\t.globl\t" + inst->des.name + "\n");     // global variable
            strs += ("\t.type\t" + inst->des.name + ", @object\n"); // set type
            strs += ("\t.size\t" + inst->des.name + ", 4\n"); // 4 bytes size
            strs += "\t.align\t"; strs += "4\n";   // 4 bytes alignment
            strs += (inst->des.name + ":\n"); // create label
            strs += ("\t.word\t" + inst->op1.name + "\n"); // initialize with value
        }
        // alloc global variable
        else if(inst->op == Operator::alloc){
            strs += ("\t.globl\t" + inst->des.name + "\n");     // global variable
            strs += ("\t.type\t" + inst->des.name + ", @object\n"); // set type
            strs += ("\t.size\t" + inst->des.name + ", " + std::to_string(stoi(inst->op1.name) * 4) + "\n"); // size
            strs += "\t.align\t"; strs += "4\n";   // 4 bytes alignment
            strs += (inst->des.name + ":\n"); // create label
            strs += "\t.word\t";
        }
        // store
        else if(inst->op == Operator::store){
            if(global_f.InstVec[i-1]->op != Operator::alloc){
                strs += ",";
            }
            strs += inst->des.name; // store value

            if(global_f.InstVec[i+1]->op != Operator::store){
                strs += "\n"; // end of store
            }
        } else if (inst->op == Operator::_return) { // function definition
            
        }
        else {
            std::cout << "Invalid global instruction: " << ir::toString(inst->op) << std::endl;
            assert(0 && "invalid global instruction"); // invalid instruction
        }
    }
    
    strs += "\t.text\n"; // text segment
    fout << strs; // write to file

    // process local functions
    for(int i = 1; i < program.functions.size(); i++){  // skip global function
        auto f = program.functions[i];
        gen_func(f);
    }
}


// generate Function assembly
void backend::Generator::gen_func(const ir::Function& func){
    // add global function header
    std::string strs = "";
    strs += "\t.globl\t" + func.name + "\n"; // global function
    strs += "\t.type\t" + func.name + ", @function\n"; // set type
    strs += func.name + ":\n"; // create label

    label_map.clear();  // clear label map
    // Process goto IR
    for(int i = 0; i < func.InstVec.size(); i++){
        auto inst = func.InstVec[i];    // current IR
        if(inst->op == ir::Operator::_goto){   // goto IR
            int goto_idx = i + stoi(inst->des.name);  // goto IR address
            if(label_map.find(goto_idx) == label_map.end()){   // not exist
                label_map[goto_idx] = "label_" + std::to_string(label_idx++);
            }
        }
    }

    fout << strs;   // write to file
    stackmap.curr_offset = 0;   // offset is zero
    strs = "";   // clear string

    // Table for parameter registers
    for(int i = 0; i < func.ParameterList.size(); i++){  // reverse parameter list
        auto op = func.ParameterList[i];    // IR Operand
        if(op.type == ir::Type::Int || op.type == ir::Type::IntPtr){  // Int
            stackmap.add_operand(op);    // add oprand
            strs += ("\t" + toString(rvOPCODE::SW) + "\t" + toString(table[i]) + "," + std::to_string(stackmap.find_operand(op)) + "(" + toString(rvREG::X2) + ")\n");     // save to stack
        }
    }

    curr_idx = 0;
    for(int i = 0; i < func.InstVec.size(); i++){     //reverse every instruction
        if(label_map.find(i) != label_map.end())   // add label
            strs += (label_map[i] + ":\n");
    
        auto inst = func.InstVec[i];    // IR
        gen_instr(*inst, strs, func);      // IR transform to risc-v
        curr_idx += 1;
    }

    // process function enter
    int inx;
    while ((inx = strs.find("%X")) != (int)std::string::npos){  // replace with offset
        strs.replace(inx, 2, std::to_string(stackmap.curr_offset));
    }
    strs = ("\t" + toString(rvOPCODE::SW) + "\t" + toString(rvREG::X1) + "," + std::to_string(stackmap.curr_offset) + "(" + toString(rvREG::X2) + ")\n") + strs;  // save return address(ra) to stack
    
    stackmap.curr_offset += 4;   // (ra) need 4 bytes in stack
    while ((inx = strs.find("%Y")) != (int)std::string::npos){  // replace with offset
        strs.replace(inx, 2, std::to_string(stackmap.curr_offset));
    }
    strs = ("\t" + toString(rvOPCODE::ADDI) + "\t" + toString(rvREG::X2) + "," + toString(rvREG::X2) + ",-" + std::to_string(stackmap.curr_offset) + "\n") + strs;    // adjust stack pointer(sp) to current offset

    // function exit
    strs += ("\t.size\t" + func.name + ", .-" + func.name + "\n");
    if(func.name == "main"){
        strs += ("\t.ident\t\"GCC: (GNU) 9.2.0\"\n");
        strs += ("\t.section\t.note.GNU-stack,\"\",@progbits\n");
    }
    
    // fout
    fout<<strs;
}


std::unordered_map<Operator, rvOPCODE> op_mapping = {
    {Operator::add, rvOPCODE::ADD},
    {Operator::sub, rvOPCODE::SUB},
    {Operator::mul, rvOPCODE::MUL},
    {Operator::div, rvOPCODE::DIV},
    {Operator::mod, rvOPCODE::REM},
    {Operator::lss, rvOPCODE::SLT},
    {Operator::gtr, rvOPCODE::SLT},  // gtr is handled as slt
};


// Instruction assembly
void backend::Generator::gen_instr(ir::Instruction& inst, std::string& strs, const ir::Function& func){
    auto op1 = inst.op1;  // first operand
    auto op2 = inst.op2;  // second operand
    auto des = inst.des;  // destination operand
    if(inst.op == Operator::_return){   // return
        if(op1.type == ir::Type::Int){      // return const variable
            LW(op1, rvREG::X5, strs);  // t0 load return value
            UNARY(MOV, rvREG::X10, rvREG::X5);  // a0 = t0
        }
        else if(op1.type == ir::Type::IntLiteral){     // return literal
            strs += ("\t" + toString(rvOPCODE::LI) + "\t" + toString(rvREG::X10) + "," + op1.name + "\n");     // a0置为立即数
        }

        // jump to the caller
        strs += ("\t" + toString(rvOPCODE::LW) + "\t" + toString(rvREG::X1) + ",%X(" + toString(rvREG::X2) + ")\n");   // return address using X1 and load from stack
        strs += ("\t" + toString(rvOPCODE::ADDI) + "\t" + toString(rvREG::X2) + "," + toString(rvREG::X2) + ",%Y\n");      // pop stack pointer
        strs += ("\t" + toString(rvOPCODE::JR) + "\t" + toString(rvREG::X1) + "\n");   // Jump to return address
    }
    else if(inst.op == Operator::_goto){     // Jump
        if(op1.type == ir::Type::IntLiteral){   // Literal jump condition
            strs += ("\t" + toString(rvOPCODE::LI) + "\t" + toString(rvREG::X5) + "," + op1.name + "\n");
            strs += ("\t" + toString(rvOPCODE::BNEZ) + "\t" + toString(rvREG::X5) + "," + label_map[curr_idx + stoi(des.name)] + "\n");   // cond = 1
        }
        else if(op1.type == ir::Type::null){   // unconditionally jump
            strs += ("\t" + toString(rvOPCODE::J) + "\t" + label_map[curr_idx + stoi(des.name)] + "\n");  // jump to label
        }
        else{
            LW(op1, rvREG::X5, strs);   // t0 load cond value
            strs += ("\t" + toString(rvOPCODE::BNEZ) + "\t" + toString(rvREG::X5) + "," + label_map[curr_idx + stoi(des.name)] + "\n");   // cond = 1
        }
    }
    else if(inst.op == Operator::call){      // call function
        auto call_inst = dynamic_cast<ir::CallInst*>(&inst);     // get the class object
        if(call_inst->op1.name == "global")return;   // no need to call global function
        for(int i = 0; i < call_inst->argumentList.size(); i++){  // reverse argument list
            auto op = call_inst->argumentList[i];
            if(op.type == ir::Type::Int)     // Variable argument
                LW(op, table[i], strs);    // i-th register to load op
            else if(op.type == ir::Type::IntLiteral)   // Literal argument     
                strs += ("\t" + toString(rvOPCODE::LI) + "\t" + toString(table[i]) + "," + op.name + "\n");
            else if(op.type == ir::Type::IntPtr){     // array pointer argument
                if(stackmap.find_operand(op) != -1){  // local array pointer
                    if(search_oprand(func.ParameterList, op))
                        LW(op, table[i], strs);   // the first element of the array is the address of the array

                    else  
                        strs += ("\t" + toString(rvOPCODE::ADDI) + "\t" + toString(table[i]) + "," + toString(rvREG::X2) + "," + std::to_string(stackmap.find_operand(op)) + "\n");
                }
                else if(stackmap.global_search(program, op)){    // global array pointer
                    strs += ("\t" + toString(rvOPCODE::LA) + "\t" + toString(table[i]) + "," + op.name + "\n");
                }
            }
            else assert(0 && "invalid call argument type");  // invalid argument type
        }
        strs += ("\t" + toString(rvOPCODE::CALL) + "\t" + call_inst->op1.name + "\n");
        stackmap.add_operand(des);  // add tmp variable to stack
        strs += ("\t" + toString(rvOPCODE::SW) + "\t" + toString(rvREG::X10) + "," + std::to_string(stackmap.find_operand(des)) + "(" + toString(rvREG::X2) + ")\n");  // store return value
    }
    else if(inst.op == Operator::alloc){
        if(stackmap.global_search(program, des))return;
        stackmap._table.insert({des, stackmap.curr_offset});     // insert
        stackmap.curr_offset += (stoi(op1.name) * 4);  // offset
    }
    else if(inst.op == Operator::store){     // store IR
        if(op2.type == ir::Type::IntLiteral){     // offset is literal
            if(stackmap.find_operand(op1) != -1){   // local variable array
                if(search_oprand(func.ParameterList, op1)){      // array parameter
                    LW(op1, rvREG::X5, strs);
                    std::string offset = std::to_string(stoi(op2.name) * 4);
                    if(offset!="0"){
                        strs += ("\t" + toString(rvOPCODE::LI) + "\t" + toString(rvREG::X6) + "," + offset + "\n");
                        RTYPE(ADD, rvREG::X5, rvREG::X5, rvREG::X6);  // t0 = t0 + offset
                    }    
                    if(des.type == ir::Type::IntLiteral)
                        strs += ("\t" + toString(rvOPCODE::LI) + "\t" + toString(rvREG::X7) + "," + des.name + "\n");
                    else
                        LW(des, rvREG::X7, strs);   // t1 load variable
                   
                    strs += ("\t" + toString(rvOPCODE::SW) + "\t" + toString(rvREG::X7) + ",0(" + toString(rvREG::X5) + ")\n");
                }
                else{
                    std::string offset = std::to_string(stackmap.find_operand(op1) + stoi(op2.name) * 4);     // array offset + stack offset
                    if(des.type == ir::Type::IntLiteral)
                        strs += ("\t" + toString(rvOPCODE::LI) + "\t" + toString(rvREG::X7) + "," + des.name + "\n");
                    else
                        LW(des, rvREG::X7, strs);   // t1 load variable

                    strs += ("\t" + toString(rvOPCODE::SW) + "\t" + toString(rvREG::X7) + "," + offset + "(" + toString(rvREG::X2) + ")\n");   // sp + offset
                }
            }
            else{      // global variable array
                std::string offset = std::to_string(stoi(op2.name) * 4);   // offset
                if(des.type == ir::Type::IntLiteral)
                    strs += ("\t" + toString(rvOPCODE::LI) + "\t" + toString(rvREG::X7) + "," + des.name + "\n");
                else
                    LW(des, rvREG::X7, strs);   // t1 load variable

                strs += ("\t" + toString(rvOPCODE::LA) + "\t" + toString(rvREG::X6) + "," + op1.name + "\n");     // t1 load symbol 
                strs += ("\t" + toString(rvOPCODE::SW) + "\t" + toString(rvREG::X7) + "," + offset + "(" + toString(rvREG::X6) + ")\n");   // 地址=基址+偏移量
            }
        }
        else{      // offset is variable
            if(stackmap.find_operand(op1) != -1){
                if(search_oprand(func.ParameterList, op1)){
                    LW(op1, rvREG::X5, strs);   // t0 load arr addr
                    LW(op2, rvREG::X6, strs);   // t1 load offset
                    ITYPE(SLLI, rvREG::X6, rvREG::X6, "2");  // t1 = t1 * 4
                    RTYPE(ADD, rvREG::X5, rvREG::X5, rvREG::X6);  // t0 = t0 + t1
                    if(des.type == ir::Type::IntLiteral)
                        strs += ("\t" + toString(rvOPCODE::LI) + "\t" + toString(rvREG::X7) + "," + des.name + "\n");
                    else
                        LW(des, rvREG::X7, strs);

                    strs += ("\t" + toString(rvOPCODE::SW) + "\t" + toString(rvREG::X7) + ",0(" + toString(rvREG::X5) + ")\n");
                }
                else{
                    LW(op2, rvREG::X5, strs);    // t0 load offset
                    ITYPE(SLLI, rvREG::X5, rvREG::X5, "2");  // t0 = offset * 4
                    strs += ("\t" + toString(rvOPCODE::LI) + "\t" + toString(rvREG::X6) + "," + std::to_string(stackmap.find_operand(op1)) + "\n");  // t1 = array stack offset
                    RTYPE(ADD, rvREG::X5, rvREG::X5, rvREG::X6);  // t0 = t0 + t1
                    RTYPE(ADD, rvREG::X5, rvREG::X5, rvREG::X2);  // t0 = t0 + sp   (sp + offset)
                    
                    if(des.type == ir::Type::IntLiteral)
                        strs += ("\t" + toString(rvOPCODE::LI) + "\t" + toString(rvREG::X7) + "," + des.name + "\n");
                    else
                        LW(des, rvREG::X7, strs);

                    strs += ("\t" + toString(rvOPCODE::SW) + "\t" + toString(rvREG::X7) + ",0(" + toString(rvREG::X5) + ")\n");
                }
            }
            else{
                LW(op2, rvREG::X5, strs);    // t0 load offset
                ITYPE(SLLI, rvREG::X5, rvREG::X5, "2");  // t0 = offset * 4
                strs += ("\t" + toString(rvOPCODE::LA) + "\t" + toString(rvREG::X6) + "," + op1.name + "\n");     // t1 load symbol
                RTYPE(ADD, rvREG::X5, rvREG::X5, rvREG::X6);  // t0 = t0 + t1
                
                if(des.type == ir::Type::IntLiteral)
                    strs += ("\t" + toString(rvOPCODE::LI) + "\t" + toString(rvREG::X7) + "," + des.name + "\n");
                else
                    LW(des, rvREG::X7, strs);

                strs += ("\t" + toString(rvOPCODE::SW) + "\t" + toString(rvREG::X7) + ",0(" + toString(rvREG::X5) + ")\n");
            }
        }
    }
    else if(inst.op == Operator::mov || inst.op == Operator::def){   // def
        if(op1.type == ir::Type::IntLiteral){     // literal operand
            strs += ("\t" + toString(rvOPCODE::LI) + "\t" + toString(rvREG::X7) + "," + op1.name + "\n");
            SW(des, strs);
        }
        else{      // variable operand
            LW(op1, rvREG::X5, strs);   // t0 load op1
            UNARY(MOV, rvREG::X7, rvREG::X5);  // t2 = t0
            SW(des, strs);  // des save t2
        }
    }
    else if(inst.op == Operator::addi){  // add with immediate
        LW(op1, rvREG::X5, strs);   // t0 load op1
        ITYPE(ADDI, rvREG::X7, rvREG::X5, op2.name);  // t1 load op2
        SW(des, strs);
    }
    else if(inst.op == Operator::subi){     // sub with immediate
        LW(op1, rvREG::X5, strs);   // t0 load op1
        ITYPE(ADDI, rvREG::X7, rvREG::X5, "-" + op2.name);  // t1 load op2
        SW(des, strs);
    }
    else if(inst.op == Operator::leq){ // <
        LW(op2, rvREG::X5, strs);   // t0 load op2
        LW(op1, rvREG::X6, strs);   // t1 load op1
        RTYPE(SLT, rvREG::X5, rvREG::X5, rvREG::X6);
        UNARY(SEQZ, rvREG::X7, rvREG::X5); // t2 = (t0 < t1) ? 1 : 0
        SW(des, strs);   // des save t2
    }
    else if(inst.op == Operator::geq){ // >=
        LW(op1, rvREG::X5, strs);   // t0 load op1
        LW(op2, rvREG::X6, strs);   // t1 load op2
        RTYPE(SLT, rvREG::X5, rvREG::X5, rvREG::X6); // t2 = (t0 >= t1) ? 1 : 0
        UNARY(SEQZ, rvREG::X7, rvREG::X5);
        SW(des, strs);  // des save t2
    }
    else if(inst.op == Operator::eq){   // ==
        LW(op1, rvREG::X5, strs);   // t0 load op1
        LW(op2, rvREG::X6, strs);   // t1 load op2
        RTYPE(XOR, rvREG::X5, rvREG::X5, rvREG::X6);
        UNARY(SEQZ, rvREG::X7, rvREG::X5);
        SW(des, strs);  // des save t2
    }
    else if(inst.op == Operator::neq){   // !=
        LW(op1, rvREG::X5, strs);   // t0 load op1
        LW(op2, rvREG::X6, strs);   // t1 load op2
        RTYPE(XOR, rvREG::X6, rvREG::X5, rvREG::X6);
        UNARY(SNEZ, rvREG::X7, rvREG::X6); // t2 = (t0 != t1) ? 1 : 0
        SW(des, strs);  // des save t2
    }
    else if(inst.op == Operator::_not){  // !
        LW(op1, rvREG::X5, strs);   // t0 load op1
        UNARY(SEQZ, rvREG::X7, rvREG::X5);
        SW(des, strs);  // des save t2
    }
    else if(inst.op == Operator::__unuse__){     // 
        strs += ("\t" + toString(rvOPCODE::NOP) + "\n");
    }
    else if(inst.op == Operator::load){   // load IR
        if(op2.type == ir::Type::IntLiteral){     // offset is literal
            if(stackmap.find_operand(op1) != -1){   // local variable
                if(search_oprand(func.ParameterList, op1)){  // array is parameter
                    // load array address
                    LW(op1, rvREG::X5, strs);
                    std::string offset = std::to_string(stoi(op2.name) * 4);
                    if(offset!="0"){
                        strs += ("\t" + toString(rvOPCODE::LI) + "\t" + toString(rvREG::X6) + "," + offset + "\n");
                        RTYPE(ADD, rvREG::X5, rvREG::X5, rvREG::X6);  // t0 = t0 + offset
                    }
                    strs += ("\t" + toString(rvOPCODE::LW) + "\t" + toString(rvREG::X7) + ",0(" + toString(rvREG::X5) + ")\n");
                    SW(des, strs);
                }
                else{  // array within function
                    std::string offset = std::to_string(stackmap.find_operand(op1) + stoi(op2.name) * 4);   // offset
                    strs += ("\t" + toString(rvOPCODE::LW) + "\t" + toString(rvREG::X7) + "," + offset + "(" + toString(rvREG::X2) + ")\n");   // sp + offset
                    SW(des, strs);
                }
            }
            else if(stackmap.global_search(program, op1)){      // global variable array
                std::string offset = std::to_string(stoi(op2.name) * 4);   //offset
                strs += ("\t" + toString(rvOPCODE::LA) + "\t" + toString(rvREG::X6) + "," + op1.name + "\n");     // t1 load symbol 
                strs += ("\t" + toString(rvOPCODE::LW) + "\t" + toString(rvREG::X7) + "," + offset + "(" + toString(rvREG::X6) + ")\n");   // base + offset
                SW(des, strs);
            }
        }
        else{
            if(stackmap.find_operand(op1) != -1){   // local variable array   
                if(search_oprand(func.ParameterList, op1)){ 
                    LW(op1, rvREG::X5, strs);   // t0 load arr addr
                    LW(op2, rvREG::X6, strs);   // t1 load offset
                    ITYPE(SLLI, rvREG::X6, rvREG::X6, "2");  // t1 = t1 * 4
                    RTYPE(ADD, rvREG::X5, rvREG::X5, rvREG::X6);  // t0 = t0 + t1
                    strs += ("\t" + toString(rvOPCODE::LW) + "\t" + toString(rvREG::X7) + ",0(" + toString(rvREG::X5) + ")\n");
                    SW(des, strs);
                }
                else{      // array within function
                    LW(op2, rvREG::X5, strs);    // t0 load offset
                    ITYPE(SLLI, rvREG::X5, rvREG::X5, "2");  // t0 = offset * 4
                    strs += ("\t" + toString(rvOPCODE::LI) + "\t" + toString(rvREG::X6) + "," + std::to_string(stackmap.find_operand(op1)) + "\n");  // 数组栈内偏移，置于t1
                    RTYPE(ADD, rvREG::X5, rvREG::X5, rvREG::X6);  // t0 = t0 + t1  
                    RTYPE(ADD, rvREG::X5, rvREG::X5, rvREG::X2);  // t0 = t0 + sp   (sp + offset)
                    strs += ("\t" + toString(rvOPCODE::LW) + "\t" + toString(rvREG::X7) + ",0(" + toString(rvREG::X5) + ")\n");
                    SW(des, strs);
                }
            }
            else{  // global variable array
                LW(op2, rvREG::X5, strs);    // t0 load offset
                ITYPE(SLLI, rvREG::X5, rvREG::X5, "2");  // t0 = offset * 4
                strs += ("\t" + toString(rvOPCODE::LA) + "\t" + toString(rvREG::X6) + "," + op1.name + "\n");     // t1 load symbol
                RTYPE(ADD, rvREG::X5, rvREG::X5, rvREG::X6);  // t0 = t0 + t1
                strs += ("\t" + toString(rvOPCODE::LW) + "\t" + toString(rvREG::X7) + ",0(" + toString(rvREG::X5) + ")\n");
                SW(des, strs);
            }
        }
    }
    // There are no 'and' 'or' operation
    else{                                     // sub, mul, div, mod, lss, gtr
        rvOPCODE op = op_mapping[inst.op];    // fetch the opcode
        if(inst.op == Operator::mod){
            if(op1.type == ir::Type::IntLiteral){     // Literal operand
                strs += ("\t" + toString(rvOPCODE::LI) + "\t" + toString(rvREG::X5) + "," + op1.name + "\n");
            }
            else{
                LW(op1, rvREG::X5, strs);
            }
            
            if(op2.type == ir::Type::IntLiteral){     // op2 is constant value, use LI
                strs += ("\t" + toString(rvOPCODE::LI) + "\t" + toString(rvREG::X6) + "," + op2.name + "\n");
            }
            else{
                LW(op2, rvREG::X6, strs);
            }
        }
        else if(inst.op == Operator::gtr){
            LW(op2, rvREG::X5, strs);
            LW(op1, rvREG::X6, strs);
        }
        else{
            LW(op1, rvREG::X5, strs);
            LW(op2, rvREG::X6, strs);
        }

        strs += ("\t" + toString(op) + "\t" + toString(rvREG::X7) + "," + toString(rvREG::X5) + "," + toString(rvREG::X6) + "\n");
        SW(des, strs);
    }
}