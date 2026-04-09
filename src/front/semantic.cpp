#include"front/semantic.h"
#include<iostream>
#include<cassert>
using ir::Instruction;
using ir::Function;
using ir::Operand;
using ir::Operator;

#define TODO assert(0 && "TODO");

// get sepcific child node, and assert
#define GET_CHILD_PTR(node, type, index) auto node = dynamic_cast<type*>(root->children[index]); assert(node); 
// get sepcific child node, and analysis
#define ANALYSIS(node, type, index) auto node = dynamic_cast<type*>(root->children[index]); assert(node); analysis##type(node, buffer);
// transmit the attributes
#define COPY_EXP_NODE(from, to) to->is_computable = from->is_computable; to->v = from->v; to->t = from->t;

map<std::string, ir::Function*>* frontend::get_lib_funcs(){
    static map<std::string, ir::Function*> lib_funcs = {
        {"getint", new Function("getint", Type::Int)},
        {"getch", new Function("getch", Type::Int)},
        {"getfloat", new Function("getfloat", Type::Float)},
        {"getarray", new Function("getarray", {Operand("arr", Type::IntPtr)}, Type::Int)},
        {"getfarray", new Function("getfarray", {Operand("arr", Type::FloatPtr)}, Type::Int)},
        {"putint", new Function("putint", {Operand("i", Type::Int)}, Type::null)},
        {"putch", new Function("putch", {Operand("i", Type::Int)}, Type::null)},
        {"putfloat", new Function("putfloat", {Operand("f", Type::Float)}, Type::null)},
        {"putarray", new Function("putarray", {Operand("n", Type::Int), Operand("arr", Type::IntPtr)}, Type::null)},
        {"putfarray", new Function("putfarray", {Operand("n", Type::Int), Operand("arr", Type::FloatPtr)}, Type::null)},
    };
    return &lib_funcs;
}


// Convert a numeric string to decimal
std::string demical_form(const std::string& s) {
    try {
        if (s.length() > 2) {
            if (s[0] == '0') {
                if (s[1] == 'x' || s[1] == 'X') {
                    // 16
                    return std::to_string(std::stoi(s, nullptr, 16));
                } else if (s[1] == 'b' || s[1] == 'B') {
                    // 2
                    return std::to_string(std::stoi(s.substr(2), nullptr, 2));
                } else {
                    // 8
                    return std::to_string(std::stoi(s, nullptr, 8));
                }
            }
        }
        // 10
        return std::to_string(std::stoi(s, nullptr, 10));
    } catch (...) {
        // failure
        return s;
    }
}

// Type transformation function
void frontend::Analyzer::type_transform(Operand& a, Operand& b, vector<Instruction*>& buffer) {
    // Helper lambda for creating temporary operands
    auto make_temp = [this](Type type) {
        return Operand("t" + std::to_string(tmp_cnt++), type);
    };

    // Helper function for adding conversion instructions
    auto convert_operand = [&](const Operand& op, Type target_type, Operator op_code) -> Operand {
        auto tmp = make_temp(target_type);
        buffer.push_back(new Instruction(op, {}, tmp, op_code));
        return tmp;
    };

    // Helper function specifically for literal conversions
    auto convert_literal = [&](const string& name, Type from_type, Type to_type, Operator op_code) -> Operand {
        Operand literal_op(name, from_type);
        return convert_operand(literal_op, to_type, op_code);
    };

    // Case 1: a is Int
    if (a.type == Type::Int) {
        if (b.type == Type::Float) {
            // Int-Float: convert a to Float
            a = convert_operand(a, Type::Float, Operator::cvt_i2f);
        }
        else if (b.type == Type::FloatLiteral) {
            // Int-FloatLiteral: convert both to Float
            a = convert_operand(a, Type::Float, Operator::cvt_i2f);
            b = convert_literal(b.name, Type::FloatLiteral, Type::Float, Operator::fdef);
        }
        else if (b.type == Type::IntLiteral) {
            // Int-IntLiteral: convert b to Int
            b = convert_literal(b.name, Type::IntLiteral, Type::Int, Operator::def);
        }
    }
    // Case 2: a is IntLiteral
    else if (a.type == Type::IntLiteral) {
        if (b.type == Type::Float) {
            // IntLiteral-Float: convert a to Float
            a = convert_literal(a.name, Type::FloatLiteral, Type::Float, Operator::fdef);
        }
        else if (b.type == Type::Int) {
            // IntLiteral-Int: convert a to Int
            a = convert_literal(a.name, Type::IntLiteral, Type::Int, Operator::def);
        }
        else if (b.type == Type::IntLiteral) {
            // IntLiteral-IntLiteral: convert both to Int
            a = convert_literal(a.name, Type::IntLiteral, Type::Int, Operator::def);
            b = convert_literal(b.name, Type::IntLiteral, Type::Int, Operator::def);
        }
        else if (b.type == Type::FloatLiteral) {
            // IntLiteral-FloatLiteral: convert both to Float
            a = convert_literal(a.name, Type::FloatLiteral, Type::Float, Operator::fdef);
            b = convert_literal(b.name, Type::FloatLiteral, Type::Float, Operator::fdef);
        }
    }
    // Case 3: a is Float
    else if (a.type == Type::Float) {
        if (b.type == Type::Int) {
            // Float-Int: convert b to Float
            b = convert_operand(b, Type::Float, Operator::cvt_i2f);
        }
        else if (b.type == Type::IntLiteral) {
            // Float-IntLiteral: convert b to Float
            b = convert_literal(b.name, Type::FloatLiteral, Type::Float, Operator::fdef);
        }
        else if (b.type == Type::FloatLiteral) {
            // Float-FloatLiteral: convert b to Float
            b = convert_literal(b.name, Type::FloatLiteral, Type::Float, Operator::fdef);
        }
    }
    // Case 4: a is FloatLiteral
    else if (a.type == Type::FloatLiteral) {
        if (b.type == Type::Int) {
            // FloatLiteral-Int: convert both to Float
            a = convert_literal(a.name, Type::FloatLiteral, Type::Float, Operator::fdef);
            b = convert_operand(b, Type::Float, Operator::cvt_i2f);
        }
        else if (b.type == Type::Float) {
            // FloatLiteral-Float: convert a to Float
            a = convert_literal(a.name, Type::FloatLiteral, Type::Float, Operator::fdef);
        }
        else if (b.type == Type::IntLiteral) {
            // FloatLiteral-IntLiteral: convert both to Float
            a = convert_literal(a.name, Type::FloatLiteral, Type::Float, Operator::fdef);
            b = convert_literal(b.name, Type::FloatLiteral, Type::Float, Operator::fdef);
        }
        else if (b.type == Type::FloatLiteral) {
            // FloatLiteral-FloatLiteral: convert both to Float
            a = convert_literal(a.name, Type::FloatLiteral, Type::Float, Operator::fdef);
            b = convert_literal(b.name, Type::FloatLiteral, Type::Float, Operator::fdef);
        }
    }
}


// The scope_cnt can identify the current scope
void frontend::SymbolTable::add_scope(){
    ScopeInfo scope;   // The New scope
    scope.cnt = ++scope_cnt;    // The id of current scope
    scope_stack.push_back(scope);  // push to the stack
}

// pop_back the last(current) scope 
void frontend::SymbolTable::exit_scope(){
    scope_stack.pop_back();
}

// rename the variable with scope information
string frontend::SymbolTable::get_scoped_name(string id) const {
    return id + "_scope_" + std::to_string(scope_stack.back().cnt);
}


// Given a variable name, search for the nearest variable with the same name in the symbol table
Operand frontend::SymbolTable::get_operand(string id) const {
    for (int i = scope_stack.size() - 1; i >= 0; i--){      // stack search
        auto t = scope_stack[i].table;     // Symbol table of
        if(t.find(id)!=t.end()){
            return t[id].operand;
        }
    }
    return Operand();  // return an empty Operand if not found
}

// Given a variable name, search for the nearest variable with the same name in the symbol table, return the STE
frontend::STE frontend::SymbolTable::get_ste(string id) const {
    for (int i=scope_stack.size()-1; i>=0; i--){      // stack search 
        auto t = scope_stack[i].table;     // Symbol table of
        if(t.find(id)!=t.end()){
            return t[id];
        }
    }
    return frontend::STE();
}


// Initialize the symbol table
frontend::Analyzer::Analyzer(): tmp_cnt(0), symbol_table(){
    symbol_table.scope_stack.push_back({0, "global", map_str_ste()});    // Construct the global scope
}


// Get the ir program
ir::Program frontend::Analyzer::get_ir_program(CompUnit* root){
    ir::Program ir_program = ir::Program();    // Initialize program
    Function* global_func = new Function("global", Type::null); // add global function


    symbol_table.functions.insert({"global", global_func});  // Insert global function
    ir_program.addFunction(*global_func);   // add global function to program

    // add lib functions to symbol table
    auto lib_funcs = *get_lib_funcs();
    for (auto it = lib_funcs.begin(); it != lib_funcs.end(); it++)
        symbol_table.functions[it->first] = it->second;

    analysisCompUnit(root, ir_program);

    auto global_ir = ir_program.functions[0].InstVec;
    for (int i = 0; i < global_ir.size(); i++){   // interval scan global function's IR
        ir_program.globalVal.push_back(ir::GlobalVal(global_ir[i]->des));  // fill to the global variable
    }

    // Global return
    ir_program.functions[0].addInst(new ir::Instruction({Operand("null", Type::null), Operand(), Operand(), Operator::_return}));

    return ir_program;
}


// CompUnit -> (Decl | FuncDef) [CompUnit]
void frontend::Analyzer::analysisCompUnit(CompUnit* root, ir::Program& buffer){
    if(root->children[0]->type == NodeType::DECL){     // Check
        GET_CHILD_PTR(decl, Decl, 0);   // get the Decl node
        analysisDecl(decl, buffer.functions.back().InstVec);    // Analysize Decl node 
    }
    else if(root->children[0]->type == NodeType::FUNCDEF){    // Function definition
        GET_CHILD_PTR(node, FuncDef, 0); // FuncDef node
        auto func = ir::Function();  // Additional ir::function
        analysisFuncDef(node, func);
        buffer.addFunction(func);    // add func to program
    }
    if(root->children.size() > 1){ // recursively process CompUnit
        ANALYSIS(node, CompUnit, 1);
    }
}


// Decl -> ConstDecl | VarDecl
void frontend::Analyzer::analysisDecl(Decl* root, vector<ir::Instruction*>& buffer){
    if(root->children[0]->type == NodeType::CONSTDECL){ // ConstDecl
        ANALYSIS(node, ConstDecl, 0);
    }else if(root->children[0]->type == NodeType::VARDECL){ // VarDecl
        ANALYSIS(node, VarDecl, 0);
    }
    else assert(0 && "Neither ConstDecl nor VarDecl");  // Error
}


// FuncDef -> FuncType Ident '(' [FuncFParams] ')' Block
void frontend::Analyzer::analysisFuncDef(FuncDef* root, ir::Function& function){
    auto token_type = dynamic_cast<Term*>(root->children[0]->children[0])->token.type;  //Return Type
    root->t = token_type == TokenType::VOIDTK ? Type::null : token_type == TokenType::INTTK ? Type::Int :Type::Float;
    root->n = dynamic_cast<Term*>(root->children[1])->token.value;
    function.name = root->n; //func name
    function.returnType = root->t; //return type

    int idx = ++symbol_table.scope_cnt;
    symbol_table.scope_stack.push_back({idx, "fp", map_str_ste()});   //new scope
    symbol_table.functions.insert({root->n, &function});            //add function to symbol table
    curr_func = &function;  // Class member (func Point for the convenience usage in analysisBlock)

    if(function.name == "main"){   // main function
        auto t = Operand("t" + std::to_string(tmp_cnt++), Type::null);
        auto global_call = new ir::CallInst(Operand("global", Type::null), vector<Operand>(), t);  // func call IR
        function.addInst(global_call);
    }

    auto paras = dynamic_cast<FuncFParams*>(root->children[3]); 
    if(paras!=NULL){     // FuncType Ident '(' FuncFParams ')' Block
        analysisFuncFParams(paras, function);
        analysisBlock(dynamic_cast<Block*>(root->children[5]), function.InstVec);
    }
    else{   // FuncType Ident '('  ')' Block
        analysisBlock(dynamic_cast<Block*>(root->children[4]), function.InstVec);
    }

    if(function.returnType == Type::null){     // return null
        function.addInst(new Instruction({Operand("null", Type::null), {}, {}, Operator::_return}));
    }

    symbol_table.exit_scope();  // Exit the current scope
}


// ConstDecl -> 'const' BType ConstDef { ',' ConstDef } ';'
void frontend::Analyzer::analysisConstDecl(ConstDecl* root, vector<ir::Instruction*>& buffer){
    ANALYSIS(node, BType, 1);
    root->t = node->t;   // current node's type is btype->t
    ANALYSIS(node1, ConstDef, 2);    //analysize ConstDef node
    for(int i = 3; i < root->children.size()-1; i+=2){   // { ',' ConstDef } ';'
        if(dynamic_cast<Term*>(root->children[i])->token.type != TokenType::COMMA) // Check if the current node is a comma
            break;
        ANALYSIS(node, ConstDef, i+1);  // analysize ConstDef node
    }
}


// VarDecl -> BType VarDef { ',' VarDef } ';'
void frontend::Analyzer::analysisVarDecl(VarDecl* root, vector<ir::Instruction*>& buffer){
    ANALYSIS(node, BType, 0);      // analysize Btype node
    root->t = node->t;             // update the type of current node
    ANALYSIS(node1, VarDef, 1);    // analysize VarDef
    for(int i = 2; i < root->children.size() - 1; i+=2){   // { ',' VarDef } ';'
        if(dynamic_cast<Term*>(root->children[i])->token.type != TokenType::COMMA) // Check comma
            break;
        ANALYSIS(node, VarDef, i+1);  // anlysize ConstDef
    }
}


// FuncFParams -> FuncFParam { ',' FuncFParam }
void frontend::Analyzer::analysisFuncFParams(FuncFParams* root, ir::Function& buffer){
    ANALYSIS(node, FuncFParam, 0);
    if(root->children.size() > 1){
        for(int i = 1; i < root->children.size(); i+=2){   // { ',' VarDef } ';'
            ANALYSIS(node, FuncFParam, i+1);  // anlysize FuncFParam
        }
    }
}


// Block -> '{' { BlockItem } '}'
void frontend::Analyzer::analysisBlock(Block* root, vector<ir::Instruction*>& buffer){
    if(root->children.size() == 2)return;
    
    symbol_table.add_scope();   // new scope
    for(int i = 1; i < root->children.size() - 1; i++){
        ANALYSIS(node, BlockItem, i);
    }
    symbol_table.exit_scope();  // exit
}


// BType -> 'int' | 'float'，Update synthesized attribute
void frontend::Analyzer::analysisBType(BType* root, vector<ir::Instruction*>& buffer){
    auto u = dynamic_cast<Term*>(root->children[0])->token.type; // type of BType node
    if(u==TokenType::INTTK)
        root->t = Type::Int;
    else if(u==TokenType::FLOATTK)
        root->t = Type::Float;
    else assert(0 && "BType should be int or float");  // Error
}


// ConstDef -> Ident { '[' ConstExp ']' } '=' ConstInitVal
void frontend::Analyzer::analysisConstDef(ConstDef* root, vector<ir::Instruction*>& buffer){
    auto root_type = dynamic_cast<ConstDecl*>(root->parent)->t;   // Get type from parent ConstDecl node
    GET_CHILD_PTR(node, Term, 0);
    string id = node->token.value;    // id
    string true_name = symbol_table.get_scoped_name(id);
    root->arr_name = true_name;  // name under current scope

    if(root->children.size() == 3){
        ANALYSIS(node, ConstInitVal, 2);    // ConstInitVal node
        auto opcode = (root_type == Type::Float || root_type == Type::FloatLiteral) ? Operator::fdef : Operator::def;
        Operand des = Operand(true_name, root_type);     // destination
        Operand op1 = Operand(node->v, node->t); // oprand1
        if(root_type == Type::Float){  // Float
            if(node->t == Type::Int){  // transform: Int->Float
                auto tmp = Operand("t" + std::to_string(tmp_cnt++), Type::Float);
                buffer.push_back(new Instruction(op1, {}, tmp, Operator::cvt_i2f));
                op1 = tmp;  // update op1
            }
            else if(node->t == Type::IntLiteral){ // IntLiteral->FloatLiteral
                op1.type = Type::FloatLiteral;
            }
        }
        else{  // Int
            assert(root_type == Type::Int);
            if(node->t == Type::Float){    // Float->Int
                auto tmp = Operand("t" + std::to_string(tmp_cnt++), Type::Int);
                buffer.push_back(new Instruction(op1, {}, tmp, Operator::cvt_f2i));
                op1 = tmp;
            }else if(node->t == Type::FloatLiteral){    // FloatLiteral->IntLiteral
                op1.name = std::to_string((int)std::stof(op1.name));  // string->float->int->string
                op1.type = Type::IntLiteral;
            }
        }
        buffer.push_back(new Instruction(op1, Operand(), des, opcode)); // push IR
        symbol_table.scope_stack.back().table.emplace(id, STE{op1, {}});
    }
    else{  // array
        STE arr_ste;
        int array_size = 1;
        for(int i = 2; i < root->children.size() - 2; i+=3){
            ANALYSIS(node, ConstExp, i); // ConstExp node
            array_size *= std::stoi(node->v);
            arr_ste.dimension.push_back(std::stoi(node->v));
        }
        auto curr_type = (root_type == ir::Type::Int) ? ir::Type::IntPtr : ir::Type::FloatPtr;  // Determine the type of array
        arr_ste.operand = ir::Operand(true_name, curr_type);
        symbol_table.scope_stack.back().table.emplace(id, arr_ste);  // Insert
        buffer.push_back(new Instruction({Operand(std::to_string(array_size),ir::Type::IntLiteral), {}, Operand(true_name, curr_type), Operator::alloc})); // alloc
        
        GET_CHILD_PTR(node, ConstInitVal, root->children.size() - 1); // ConstInitVal node
        if(node->children.size() == 2){    // {} to Initialize array
            for (int i = 0; i<array_size; i++){
                buffer.push_back(new Instruction({Operand(true_name, Type::IntPtr), Operand(std::to_string(i), Type::IntLiteral), Operand("0", Type::IntLiteral), Operator::store}));
            }
        }
        else{
            for (int i = 1, j = 0; i < node->children.size()-1; i+=2, j++){     //'{' [ ConstInitVal { ',' ConstInitVal } ] '}'
                ConstInitVal* tmp = dynamic_cast<ConstInitVal*>(node->children[i]);
                ConstExp* conexp = dynamic_cast<ConstExp*>(tmp->children[0]);
                analysisConstExp(conexp, buffer); // ConstExp node
                buffer.push_back(new Instruction({Operand(true_name, Type::IntPtr), Operand(std::to_string(j), Type::IntLiteral), Operand(conexp->v, Type::IntLiteral), Operator::store}));
            }
        }
    }
}


// VarDef -> Ident { '[' ConstExp ']' } [ '=' InitVal ]
void frontend::Analyzer::analysisVarDef(VarDef* root, vector<ir::Instruction*>& buffer){
    auto root_type = dynamic_cast<VarDecl*>(root->parent)->t;   // parent VarDecl's type

    GET_CHILD_PTR(node, Term, 0);
    string id = node->token.value;
    string true_name = symbol_table.get_scoped_name(id); // name under current scope
    if(root->children.size() == 1){
        auto des = Operand(true_name, root_type);
        if(root_type == Type::Float)    // 个人感觉这两个都是一样的 应该使用一个 IntLiteral 就行不用特判 TODO
            buffer.push_back(new Instruction(Operand("0.0", Type::FloatLiteral), Operand(), des, Operator::fdef));
        else
            buffer.push_back(new Instruction(Operand("0", Type::IntLiteral), Operand(), des, Operator::def));

        symbol_table.scope_stack.back().table.emplace(id, STE{des, {}});
    }
    else{
        GET_CHILD_PTR(term, Term, 1);
        if(term->token.type == TokenType::ASSIGN){   // Assignment
            ANALYSIS(node, InitVal, 2);
            auto des = Operand(true_name, root_type);     // des
            auto opcode = (root_type == Type::Float || root_type == Type::FloatLiteral) ? Operator::fdef : Operator::def;
            auto op1 = Operand(node->v, node->t);    // op1
            if(root_type == Type::Float){ // Float
                if(node->t == Type::Int){  // Int->Float
                    auto tmp = Operand("t" + std::to_string(tmp_cnt++), Type::Float);
                    buffer.push_back(new Instruction(op1, {}, tmp, Operator::cvt_i2f));
                    op1 = tmp;  // op1
                }
                else if(node->t == Type::IntLiteral){     // IntLiteral->FloatLiteral
                    op1.type = Type::FloatLiteral;
                }
            }
            else{  // Int
                assert(root_type == Type::Int && "VarDef should be Int or Float");
                if(node->t == Type::Float){    // Float->Int
                    auto tmp = Operand("t" + std::to_string(tmp_cnt++), Type::Int);
                    buffer.push_back(new Instruction(op1, {}, tmp, Operator::cvt_f2i));
                    op1 = tmp;
                }
                else if(node->t == Type::FloatLiteral){    // FloatLiteral->IntLiteral
                    op1.name = std::to_string((int)std::stof(op1.name));  // string->float->int->string
                    op1.type = Type::IntLiteral;
                }
            }
            buffer.push_back(new Instruction(op1, Operand(), des, opcode));
            symbol_table.scope_stack.back().table.emplace(id, STE{des, {}});
        }
        else if(root->children.back()->type == NodeType::INITVAL){    // array with assignment
            // VarDef -> Ident '[' ConstExp ']' {'[' ConstExp ']'} '=' InitVal
            int array_size = 1;
            ir::Type curr_type = root_type;
            STE arr_ste;    // tmp STE
            for(int i = 2; i < root->children.size() - 2; i+=3){
                ANALYSIS(node, ConstExp, i);
                array_size *= std::stoi(node->v);
                arr_ste.dimension.push_back(std::stoi(node->v));
            }
            curr_type = (root_type == ir::Type::Int) ? ir::Type::IntPtr : ir::Type::FloatPtr;  // type of array
            arr_ste.operand = ir::Operand(true_name, curr_type);
            symbol_table.scope_stack.back().table.emplace(id, arr_ste); // Insert
            buffer.push_back(new Instruction({Operand(std::to_string(array_size),ir::Type::IntLiteral), {}, Operand(true_name, curr_type), Operator::alloc}));
            
            // Initialize array
            GET_CHILD_PTR(node, InitVal, (int)root->children.size() - 1);
            if(node->children.size() == 2){ // {} to Initialize array
                for (int i = 0; i<array_size; i++){
                    buffer.push_back(new Instruction({Operand(true_name, Type::IntPtr), Operand(std::to_string(i), Type::IntLiteral), Operand("0", Type::IntLiteral), Operator::store}));
                }
            }
            else{
                int j = 0;
                for (int i = 1; i< (int)node->children.size()-1; i+=2, j++){     // '{' [ ConstInitVal { ',' ConstInitVal } ] '}'
                    InitVal* u = dynamic_cast<InitVal*>(node->children[i]);
                    auto exp = dynamic_cast<Exp*>(u->children[0]);
                    analysisExp(exp, buffer); // Exp node
                    buffer.push_back(new Instruction({Operand(true_name, Type::IntPtr), Operand(std::to_string(j), Type::IntLiteral), Operand(exp->v, Type::IntLiteral), Operator::store}));
                }
                // a[5]={1,2}, Initialize further
                for (;j<array_size;j++){
                    buffer.push_back(new Instruction({Operand(true_name, Type::IntPtr), Operand(std::to_string(j), Type::IntLiteral), Operand("0", Type::IntLiteral), Operator::store}));
                }
            }
        }
        else{  // array without assignment
            // VarDef -> Ident '[' ConstExp ']' '[' ConstExp ']'
            int array_size = 1;
            ir::Type curr_type = root_type;
            STE arr_ste;
            for(int i = 2; i < root->children.size() - 1; i+=3){
                ANALYSIS(node, ConstExp, i);
                arr_ste.dimension.push_back(std::stoi(node->v));
                array_size *= std::stoi(node->v);
            }
            curr_type = (root_type == ir::Type::Int) ? ir::Type::IntPtr : ir::Type::FloatPtr;  // type of array
            arr_ste.operand = ir::Operand(true_name, curr_type);
            symbol_table.scope_stack.back().table.emplace(id, arr_ste); // Insert
            buffer.push_back(new Instruction({Operand(std::to_string(array_size),ir::Type::IntLiteral), {}, Operand(true_name, curr_type), Operator::alloc}));
            
            // Initialize array
            for (int i = 0; i<array_size; i++){
                buffer.push_back(new Instruction({Operand(true_name, Type::IntPtr), Operand(std::to_string(i), Type::IntLiteral), Operand("0", Type::IntLiteral), Operator::store}));
            }
        }
    }
}


// FuncFParam -> BType Ident ['[' ']' { '[' Exp ']' }]
void frontend::Analyzer::analysisFuncFParam(FuncFParam* root, ir::Function& buffer){
    GET_CHILD_PTR(node, BType, 0); // BType node
    analysisBType(node, buffer.InstVec);
    auto param_name = dynamic_cast<Term*>(root->children[1])->token.value; // parameter name
    Type param_type = node->t; // parameter type
    
    if(root->children.size() > 2) param_type = (node->t == Type::Int) ? Type::IntPtr : Type::FloatPtr; // array parameter type
    buffer.ParameterList.push_back(Operand(param_name, param_type)); // add param
    symbol_table.scope_stack.back().table.insert({param_name, {Operand(param_name, param_type), {}}});
}


// BlockItem -> Decl | Stmt
void frontend::Analyzer::analysisBlockItem(BlockItem* root, vector<ir::Instruction*>& buffer){
    if(root->children[0]->type == NodeType::DECL){ // Decl
        ANALYSIS(node, Decl, 0);
    }else if(root->children[0]->type == NodeType::STMT){ // Stmt
        ANALYSIS(node, Stmt, 0);
    }
    else assert(0 && "BlockItem should be Decl or Stmt");  // Error
}


// ConstExp -> AddExp
void frontend::Analyzer::analysisConstExp(ConstExp* root, vector<ir::Instruction*>& buffer){
    ANALYSIS(node, AddExp, 0);    // Analysize AddExp node
    COPY_EXP_NODE(node, root); // Update
}


// ConstInitVal -> ConstExp | '{' [ ConstInitVal { ',' ConstInitVal } ] '}'
void frontend::Analyzer::analysisConstInitVal(ConstInitVal* root, vector<ir::Instruction*>& buffer){
    if(root->children[0]->type == NodeType::CONSTEXP){
        ANALYSIS(node, ConstExp, 0); //ConstExp node
        root->t = node->t;
        root->v = node->v;
    }
}


// InitVal -> Exp | '{' [ InitVal { ',' InitVal } ] '}'
void frontend::Analyzer::analysisInitVal(InitVal* root, vector<ir::Instruction*>& buffer){
    if(root->children[0]->type == NodeType::EXP){ 
        ANALYSIS(node, Exp, 0);
        COPY_EXP_NODE(node, root); // Update
    }
}


// Exp -> AddExp
void frontend::Analyzer::analysisExp(Exp* root, vector<ir::Instruction*>& buffer){
    ANALYSIS(node, AddExp, 0); // analysize addexp node
    COPY_EXP_NODE(node, root); // Update
}


// Stmt -> LVal '=' Exp ';' | Block | 'if' '(' Cond ')' Stmt [ 'else' Stmt ] | 'while' '(' Cond ')' Stmt | 'break' ';' | 'continue' ';' | 'return' [Exp] ';' | [Exp] ';'
void frontend::Analyzer::analysisStmt(Stmt* root, vector<ir::Instruction*>& buffer){
    auto u = dynamic_cast<Term*>(root->children[0]);
    if(root->children[0]->type == NodeType::LVAL){
        ANALYSIS(node, Exp, 2);  // Exp node
        ANALYSIS(node1, LVal, 0);
    }
    else if(root->children[0]->type == NodeType::BLOCK){ // Block
        ANALYSIS(node, Block, 0);
    }
    else if(root->children[0]->type == NodeType::EXP){    // Exp
        ANALYSIS(node, Exp, 0);
    }
    else if(u->token.type == TokenType::IFTK){  // if block
        // Stmt -> 'if' '(' Cond ')' Stmt [ 'else' Stmt ]
        auto IR1 = vector<Instruction*>();
        GET_CHILD_PTR(cond, Cond, 2);
        analysisCond(cond, IR1);    // cond node
        buffer.insert(buffer.end(), IR1.begin(), IR1.end());    // Insert cond IR
        // if successfully jump
        buffer.push_back(new Instruction(Operand(cond->v, cond->t), Operand(), Operand("2",Type::IntLiteral), Operator::_goto));

        // Stmt
        auto IRS2 = vector<Instruction*>();  // stmt IR
        GET_CHILD_PTR(stmt, Stmt, 4);   // stmt
        analysisStmt(stmt, IRS2);   // stmt node
        auto IRS3 = vector<Instruction*>();     // else stmt's IRs
        if(root->children.size() >= 7){
            GET_CHILD_PTR(stmt2, Stmt, 6);
            analysisStmt(stmt2, IRS3);   // else stmt
        }

        buffer.push_back(new Instruction({Operand(), Operand(), Operand(std::to_string(IRS2.size()+2), Type::IntLiteral), Operator::_goto}));
        buffer.insert(buffer.end(), IRS2.begin(), IRS2.end());
        buffer.push_back(new Instruction({Operand(), Operand(), Operand(std::to_string(IRS3.size()+1), Type::IntLiteral), Operator::_goto}));
        buffer.insert(buffer.end(), IRS3.begin(), IRS3.end());
        buffer.push_back(new Instruction({Operand(), Operand(), Operand(), Operator::__unuse__}));
    }
    else if(u->token.type == TokenType::WHILETK){   // while
        // Stmt -> 'while' '(' Cond ')' Stmt 
        GET_CHILD_PTR(cond, Cond, 2);
        auto IRS1 = vector<Instruction*>();  // cond's IR
        analysisCond(cond, IRS1);

        GET_CHILD_PTR(stmt, Stmt, 4);
        auto IRS2 = vector<Instruction*>();  // while stmt's IR
        analysisStmt(stmt, IRS2);
        IRS2.push_back(new Instruction({Operand("continue", Type::null), Operand(), Operand(), Operator::__unuse__})); // nop usage
        // check the continue and break
        for (int i = 0; i < IRS2.size(); i++){
            if(IRS2[i]->op == Operator::__unuse__ && IRS2[i]->op1.type == Type::null){
                if(IRS2[i]->op1.name == "break"){
                    IRS2[i] = new Instruction({Operand(), Operand(), Operand(std::to_string((int)IRS2.size()-i),Type::IntLiteral), Operator::_goto});
                }
                else if(IRS2[i]->op1.name == "continue"){
                    IRS2[i] = new Instruction({Operand(), Operand(), Operand(std::to_string(-(2+i+(int)IRS1.size())), Type::IntLiteral), Operator::_goto});
                }
            }
        }
        buffer.insert(buffer.end(), IRS1.begin(), IRS1.end());
        buffer.push_back(new Instruction({Operand(cond->v,cond->t), Operand(), Operand("2",Type::IntLiteral), Operator::_goto}));
        buffer.push_back(new Instruction({Operand(), Operand(), Operand(std::to_string(IRS2.size()+1), Type::IntLiteral), Operator::_goto}));
        buffer.insert(buffer.end(), IRS2.begin(), IRS2.end());

        buffer.push_back(new Instruction(Operand(), Operand(), Operand(), Operator::__unuse__));
    }
    else if(u->token.type == TokenType::BREAKTK){   // break
        buffer.push_back(new Instruction({Operand("break", Type::null), Operand(), Operand(), Operator::__unuse__}));
    }
    else if(u->token.type == TokenType::CONTINUETK){    // continue
        buffer.push_back(new Instruction({Operand("continue", Type::null), Operand(), Operand(), Operator::__unuse__}));
    }
    else if(u->token.type == TokenType::RETURNTK){  // return
        // stmt -> 'return' [Exp] ';'
        if((int)root->children.size() == 2){
            buffer.push_back(new Instruction({Operand("null", Type::null), Operand(), Operand(), Operator::_return}));
        }
        else{
            // stmt -> 'return' Exp ';'
            auto IRS = vector<Instruction*>();
            GET_CHILD_PTR(exp, Exp, 1);
            analysisExp(exp, IRS);
            buffer.insert(buffer.end(), IRS.begin(), IRS.end());     // exp IR

            if(curr_func->returnType == Type::Int){
                // Int or IntLiteral
                if(exp->t == Type::Int || exp->t == Type::IntLiteral){
                    buffer.push_back(new Instruction({Operand(exp->v, exp->t), Operand(), Operand(), Operator::_return}));  
                }
                // Float or FloatLiteral
                else if(exp->t == Type::FloatLiteral){
                    buffer.push_back(new Instruction({Operand(std::to_string((int)std::stof(exp->v)), Type::IntLiteral), Operand(), Operand(), Operator::_return}));
                }
                else if(exp->t == Type::Float){
                    Operand tmp = Operand("t" + std::to_string(tmp_cnt++), Type::Int);
                    buffer.push_back(new Instruction(Operand(exp->v,Type::Float), Operand(), tmp, Operator::cvt_f2i));
                    buffer.push_back(new Instruction(tmp, Operand(), Operand(), Operator::_return));
                }
            }
            else if(curr_func->returnType == Type::Float){
                // Float or FloatLiteral
                if(exp->t == Type::Float || exp->t == Type::FloatLiteral){
                    buffer.push_back(new Instruction(Operand(exp->v,exp->t), Operand(), Operand(), Operator::_return));
                }
                // Int or IntLiteral
                else if(exp->t == Type::IntLiteral){
                    buffer.push_back(new Instruction(Operand(std::to_string((float)std::stoi(exp->v)),Type::FloatLiteral), Operand(), Operand(), Operator::_return));
                }
                else if(exp->t == Type::Int){
                    Operand tmp = Operand("t" + std::to_string(tmp_cnt++), Type::Float);
                    buffer.push_back(new Instruction(Operand(exp->v, exp->t), Operand(), tmp, Operator::cvt_i2f));
                    buffer.push_back(new Instruction(tmp, Operand(), Operand(), Operator::_return));
                }
            }
        }
    }
}


// LVal -> Ident {'[' Exp ']'}
void frontend::Analyzer::analysisLVal(LVal* root, vector<ir::Instruction*>& buffer){
    auto tk_type = dynamic_cast<Term*>(root->children[0])->token.value; // Term token
    auto op = symbol_table.get_operand(tk_type);
    root->t = op.type;

    if((int)root->children.size() == 1){ // LVal -> Ident
        // root->v = tk_type;
        root->v = op.name;
        root->is_computable = (root->t == Type::IntLiteral || root->t == Type::FloatLiteral);
        root->i = 0;

        if(root->parent->type == NodeType::STMT){ // lval=exp;
            frontend::Exp* node = dynamic_cast<Exp*>(root->parent->children[2]); // exp node
            Operand op1 = Operand(node->v, node->t);
            Operand des = Operand(root->v, root->t);
            Operator op_type = (root->t == Type::Int) ? Operator::mov : Operator::fmov; // mov or fmov
            buffer.push_back(new Instruction({op1, Operand(), des, op_type}));
        }
    }
    else{      // LVal -> Ident {'[' Exp ']'}
        auto arr = symbol_table.get_ste(tk_type);
        auto dimensions = arr.dimension;  // dim

        // Ident '[' Exp ']'
        if(root->children.size() == 4){ // one dimension array
            ANALYSIS(node, Exp, 2);
            Type type = (root->t == Type::IntPtr) ? Type::Int : Type::Float;
            root->t = type;
            Operand idx = Operand(node->v, node->t);    // index of array
            if(root->parent->type == NodeType::STMT){  // Store operation
                auto node = dynamic_cast<Exp*>(root->parent->children[2]);   // Through the parent node to get the exp node
                auto des = Operand(node->v, node->t);
                Instruction* store_inst = new Instruction({arr.operand, idx, des, Operator::store}); // store operation
                buffer.push_back(store_inst);
                root->v = des.name;
            }
            else{  // fetch operation
                Operand des = Operand("t" + std::to_string(tmp_cnt++), type);
                Instruction* fetch_inst = new Instruction({arr.operand, idx, des, Operator::load}); // load operation
                buffer.push_back(fetch_inst);  // load operation
                root->v = des.name;
            }
        }
        else{      // two dimension array
            // Ident '[' Exp ']' '[' Exp ']'
            ANALYSIS(exp1, Exp, 2);
            ANALYSIS(exp2, Exp, 5);
            Type type = (root->t == Type::IntPtr) ? Type::Int : Type::Float;
            root->t = type;
            if(exp1->is_computable && exp2->is_computable){    // is_computable
                std::string s = std::to_string(std::stoi(exp1->v) * dimensions[1] + std::stoi(exp2->v));
                Operand idx = Operand(s, Type::IntLiteral);    // get index of array
                if(root->parent->type == NodeType::STMT){   // Store operation
                    auto node = dynamic_cast<Exp*>(root->parent->children[2]);   // offset
                    auto des = Operand(node->v, node->t);
                    Instruction* store_inst = new Instruction({arr.operand, idx, des, Operator::store});
                    buffer.push_back(store_inst);
                    root->v = des.name;
                }
                else{
                    Operand des = Operand("t" + std::to_string(tmp_cnt++), type); // load des
                    Instruction* fetch_inst = new Instruction({arr.operand, idx, des, Operator::load}); 
                    buffer.push_back(fetch_inst);
                    root->v = des.name;
                }
            }
            else{
                Operand op1 = Operand(exp1->v, exp1->t);
                Operand op2 = Operand(exp2->v, exp2->t);
                Operand op3 = Operand(std::to_string(dimensions[1]), Type::IntLiteral);
                type_transform(op1, op2, buffer);
                Operand t1 = Operand("t" + std::to_string(tmp_cnt++), Type::Int);
                Operand t2 = Operand("t" + std::to_string(tmp_cnt++), Type::Int);
                buffer.push_back(new Instruction({op1, op3, t1, Operator::mul}));
                buffer.push_back(new Instruction({t1, op2, t2, Operator::add}));
                if(root->parent->type == NodeType::STMT){   // assignment operation
                    auto node = dynamic_cast<Exp*>(root->parent->children[2]);   // get exp node
                    auto des = Operand(node->v, node->t);
                    Instruction* store_inst = new Instruction({arr.operand, t2, des, Operator::store});
                    buffer.push_back(store_inst);
                    root->v = des.name;
                }
                else{
                    Operand des = Operand("t" + std::to_string(tmp_cnt++), type);
                    Instruction* fetch_inst = new Instruction({arr.operand, t2, des, Operator::load});
                    buffer.push_back(fetch_inst);
                    root->v = des.name;
                }
            }
        }
    }
}


// PrimaryExp -> '(' Exp ')' | LVal | Number
void frontend::Analyzer::analysisPrimaryExp(PrimaryExp* root, vector<ir::Instruction*>& buffer){
    if(root->children[0]->type == NodeType::TERMINAL){ // '(' Exp ')'
        ANALYSIS(node, Exp, 1);  // Exp node
        COPY_EXP_NODE(node, root);
    }
    else if(root->children[0]->type == NodeType::LVAL){ // LVal
        ANALYSIS(node, LVal, 0);    // Lval node
        COPY_EXP_NODE(node, root);
    }
    else if(root->children[0]->type == NodeType::NUMBER){ // Number
        root->is_computable = true;
        Token tk = dynamic_cast<Term*>(root->children[0]->children[0])->token;  // terminal token
        root->t = (tk.type==TokenType::INTLTR) ? Type::IntLiteral : Type::FloatLiteral;
        if(root->t == Type::IntLiteral){
            root->v = demical_form(tk.value);
        }
        else{
            root->v = tk.value;
        }
    }
    else assert(0 && "PrimaryExp should be Terminal, LVal or Number");  // Error
}


// Cond -> LOrExp
void frontend::Analyzer::analysisCond(Cond* root, vector<ir::Instruction*>& buffer){
    ANALYSIS(node, LOrExp, 0); // LOrExp node
    COPY_EXP_NODE(node, root);
}


// LOrExp -> LAndExp [ '||' LOrExp ]
void frontend::Analyzer::analysisLOrExp(LOrExp* root, vector<ir::Instruction*>& buffer){
    if (root->children.size() == 1) { // LOrExp -> LAndExp
        ANALYSIS(node, LAndExp, 0);
        COPY_EXP_NODE(node, root);
        return;
    }
    // LOrExp -> LAndExp '||' LOrExp
    root->t = Type::Int;

    ANALYSIS(lhs, LAndExp, 0);
    vector<ir::Instruction*> rhs_ir;
    GET_CHILD_PTR(rhs, LOrExp, 2);
    analysisLOrExp(rhs, rhs_ir);

    Operand lhs_op(lhs->v, lhs->t);
    Operand rhs_op(rhs->v, rhs->t);
    Operand tmp1("t" + std::to_string(tmp_cnt++), root->t);
    Operand result("t" + std::to_string(tmp_cnt++), root->t);

    // if lhs is true, set result to 1, else run rhs's IR
    buffer.push_back(new Instruction({lhs_op, {}, tmp1, Operator::mov}));
    buffer.push_back(new Instruction({tmp1, {}, {"2", Type::IntLiteral}, Operator::_goto}));
    buffer.push_back(new Instruction({{}, {}, {"3", Type::IntLiteral}, Operator::_goto})); // run rhs's IR
    buffer.push_back(new Instruction({"1", Type::IntLiteral}, {}, result, Operator::mov)); // set 1
    buffer.push_back(new Instruction({{}, {}, {std::to_string(rhs_ir.size() + 2), Type::IntLiteral}, Operator::_goto})); // jump
    buffer.insert(buffer.end(), rhs_ir.begin(), rhs_ir.end());
    buffer.push_back(new Instruction({rhs_op, {}, result, Operator::mov}));
    buffer.push_back(new Instruction({{}, {}, {}, Operator::__unuse__}));

    root->v = result.name;
}


// LAndExp -> EqExp [ '&&' LAndExp ]
void frontend::Analyzer::analysisLAndExp(LAndExp* root, vector<ir::Instruction*>& buffer) {
    if (root->children.size() == 1) {
        ANALYSIS(node, EqExp, 0);
        COPY_EXP_NODE(node, root);
        return;
    }
    // LAndExp -> EqExp '&&' LAndExp
    ANALYSIS(eqexp, EqExp, 0);

    vector<ir::Instruction*> rhs_ir;
    GET_CHILD_PTR(rhs, LAndExp, 2);
    analysisLAndExp(rhs, rhs_ir);

    root->t = Type::Int;

    Operand lhs_op(eqexp->v, eqexp->t);
    Operand rhs_op(rhs->v, rhs->t);
    Operand tmp1("t" + std::to_string(tmp_cnt++), root->t);
    Operand result("t" + std::to_string(tmp_cnt++), root->t);

    // if lhs is false, set result to 0, else execute rhs's IR
    buffer.push_back(new Instruction({lhs_op, {}, tmp1, Operator::mov})); // tmp
    buffer.push_back(new Instruction({tmp1, {}, {std::to_string(3), Type::IntLiteral}, Operator::_goto})); // if tmp is true, execute rhs's IR
    buffer.push_back(new Instruction({"0", Type::IntLiteral}, {}, result, Operator::mov)); // if tmp is false, set result to 0
    buffer.push_back(new Instruction({{}, {}, {std::to_string(rhs_ir.size() + 2), Type::IntLiteral}, Operator::_goto})); // unconditionally jump
    buffer.insert(buffer.end(), rhs_ir.begin(), rhs_ir.end());
    buffer.push_back(new Instruction({rhs_op, {}, result, Operator::mov}));
    buffer.push_back(new Instruction({{}, {}, {}, Operator::__unuse__})); // nop usage

    root->v = result.name;
}


// EqExp -> RelExp { ('==' | '!=') RelExp }
void frontend::Analyzer::analysisEqExp(EqExp* root,vector<ir::Instruction*>& buffer){
    if((int)root->children.size() == 1){    // EqExp -> RelExp
        ANALYSIS(node, RelExp, 0);     // Analysize RelExp
        COPY_EXP_NODE(node, root);
        return;
    }
    // EqExp -> RelExp ('==' | '!=') RelExp
    ANALYSIS(node, RelExp, 0);     // RelExp node
    root->is_computable = node->is_computable;
    root->v = node->v;
    root->t = node->t;
    for(int i = 1; i < root->children.size(); i+=2){
        ANALYSIS(node, RelExp, i+1);     // RelExp node
        auto tk_type = dynamic_cast<Term*>(root->children[i])->token.type;
        
        root->is_computable = false;

        auto op1 = Operand(root->v, root->t);
        auto op2 = Operand(node->v, node->t);
        type_transform(op1, op2, buffer);

        auto des_type = op1.type == Type::Int ? Type::Int : Type::Float;
        auto des = Operand("t" + std::to_string(tmp_cnt++), des_type);
        auto op_type = (tk_type == TokenType::EQL) ? \
                        ((op1.type == Type::Int)? Operator::eq : Operator::feq) : \
                        ((op1.type == Type::Int)? Operator::neq : Operator::fneq);

        buffer.push_back(new Instruction({op1, op2, des, op_type})); // push IR

        root->v = des.name;
        root->t = Type::Int;
    }
}


// RelExp -> AddExp { ('<' | '>' | '<=' | '>=') AddExp }
void frontend::Analyzer::analysisRelExp(RelExp* root,vector<ir::Instruction*>& buffer){
    if((int)root->children.size() == 1){    // RelExp -> AddExp
        ANALYSIS(node, AddExp, 0);
        COPY_EXP_NODE(node, root);
        return;
    }   
    // RelExp -> AddExp {('<' | '>' | '<=' | '>=') AddExp}
    ANALYSIS(node, AddExp, 0);
    root->is_computable = node->is_computable;
    root->t = node->t;
    root->v = node->v;

    for(int i = 1; i < root->children.size(); i+=2){
        ANALYSIS(node, AddExp, i+1); // AddExp node
        auto tk_type = dynamic_cast<Term*>(root->children[i])->token.type;   // op
        root->is_computable = false;
        auto op1 = Operand(root->v, root->t);
        auto op2 = Operand(node->v, node->t);
        type_transform(op1, op2, buffer);
        auto des_type = (op1.type == Type::Int) ? Type::Int : Type::Float;
        auto des = Operand("t" + std::to_string(tmp_cnt++), des_type);

        auto op_type = (tk_type == TokenType::LSS) ? \
                        ((op1.type == Type::Int)? Operator::lss : Operator::flss) : \
                        (tk_type == TokenType::GTR) ? \
                        ((op1.type == Type::Int)? Operator::gtr : Operator::fgtr) : \
                        (tk_type == TokenType::LEQ) ? \
                        ((op1.type == Type::Int)? Operator::leq : Operator::fleq) : \
                        ((op1.type == Type::Int)? Operator::geq : Operator::fgeq);

        buffer.push_back(new Instruction({op1, op2, des, op_type})); // push IR
        
        root->v = des.name;
        root->t = Type::Int;
    }
}


// AddExp -> MulExp { ('+' | '-') MulExp }
void frontend::Analyzer::analysisAddExp(AddExp* root, vector<ir::Instruction*>& buffer){
    if((int)root->children.size() == 1){
        ANALYSIS(node, MulExp, 0);
        COPY_EXP_NODE(node, root); // pushup
        return;
    }
    // AddExp -> MulExp { ('+' | '-') MulExp }
    ANALYSIS(node, MulExp, 0);    // mulexp node
    root->is_computable = node->is_computable;
    root->t = node->t;
    root->v = node->v;

    for(int i = 1; i < root->children.size(); i+=2){
        auto tk_type = dynamic_cast<Term*>(root->children[i])->token.type;
        ANALYSIS(node, MulExp, i+1);
        if(root->is_computable && node->is_computable){ // both are computable!
            root->is_computable = true;
            if(root->t != node->t)
                root->t = Type::FloatLiteral;

            if(root->t == Type::IntLiteral && tk_type == TokenType::PLUS)
                root->v = std::to_string(std::stoi(root->v) + std::stoi(node->v));
            else if(root->t == Type::IntLiteral && tk_type == TokenType::MINU)
                root->v = std::to_string(std::stoi(root->v) - std::stoi(node->v));
            else if(root->t == Type::FloatLiteral && tk_type == TokenType::PLUS)
                root->v = std::to_string(std::stof(root->v) + std::stof(node->v));
            else
                root->v = std::to_string(std::stof(root->v) - std::stof(node->v));
        }
        else{
            root->is_computable = false;
            auto op1 = Operand(root->v, root->t);
            auto op2 = Operand(node->v, node->t);
            auto des = Operand("t" + std::to_string(tmp_cnt++), Type::Int);
            auto op_type = (tk_type == TokenType::PLUS) ? Operator::addi : Operator::subi; // addi or subi
            if(root->t == Type::Int && node->t == Type::IntLiteral)
                buffer.push_back(new Instruction({op1, op2, des, op_type}));  // addi | subi
            else if(root->t == Type::IntLiteral && node->t == Type::Int && tk_type == TokenType::PLUS) // 1-a x
                buffer.push_back(new Instruction({op2, op1, des, op_type}));  // addi
            else{
                type_transform(op1, op2, buffer);
                des.type = op1.type; // des type
                op_type = (tk_type == TokenType::PLUS) ? \
                                (op1.type == Type::Int) ? Operator::add : Operator::fadd : \
                                (op1.type == Type::Int) ? Operator::sub : Operator::fsub;

                buffer.push_back(new Instruction({op1, op2, des, op_type}));
            }
            root->v = des.name;
            root->t = des.type;
        }
    }
}


// MulExp -> UnaryExp { ('*' | '/' | '%') UnaryExp }
void frontend::Analyzer::analysisMulExp(MulExp* root, vector<ir::Instruction*>& buffer){
    if(root->children.size() == 1){
        ANALYSIS(node, UnaryExp, 0);
        COPY_EXP_NODE(node, root); // pushup
        return;
    }
    ANALYSIS(node, UnaryExp, 0);    // unaryexp node
    root->is_computable = node->is_computable;
    root->t = node->t;
    root->v = node->v;
    for(int i = 1; i < root->children.size(); i+=2){
        auto tk_type = dynamic_cast<Term*>(root->children[i])->token.type;
        ANALYSIS(node, UnaryExp, i+1);     // unaryexp node
        if(root->is_computable && node->is_computable){ // both are computable!
            root->is_computable = true;
            if(root->t != node->t) // Align
                root->t = Type::FloatLiteral;

            if(root->t == Type::IntLiteral && tk_type == TokenType::MULT)
                root->v = std::to_string(std::stoi(root->v) * std::stoi(node->v));
            else if(root->t == Type::IntLiteral && tk_type == TokenType::DIV)
                root->v = std::to_string(std::stoi(root->v) / std::stoi(node->v));
            else if(root->t == Type::IntLiteral && tk_type == TokenType::MOD)
                root->v = std::to_string(std::stoi(root->v) % std::stoi(node->v));
            else if(root->t == Type::FloatLiteral && tk_type == TokenType::MULT)
                root->v = std::to_string(std::stof(root->v) * std::stof(node->v));
            else if(root->t == Type::FloatLiteral && tk_type == TokenType::DIV)
                root->v = std::to_string(std::stof(root->v) / std::stof(node->v));
        }
        else{
            root->is_computable = false;
            Operand op1 = Operand(root->v, root->t);
            Operand op2 = Operand(node->v, node->t);
            Operand tmp = Operand("t" + std::to_string(tmp_cnt++), root->t);
            type_transform(op1, op2, buffer);
            auto des_type = (op1.type == Type::Int) ? Type::Int : Type::Float;
            auto des = Operand("t" + std::to_string(tmp_cnt++), des_type);

            auto op_type = (tk_type == TokenType::MULT) ? \
                            ((op1.type == Type::Int)? Operator::mul : Operator::fmul) : \
                           (tk_type == TokenType::DIV) ? \
                            ((op1.type == Type::Int)? Operator::div : Operator::fdiv) : \
                            Operator::mod; // mod

            buffer.push_back(new Instruction({op1, op2, des, op_type})); // push IR

            root->v = des.name;
            root->t = des.type;
        }
    }
}


// UnaryExp -> PrimaryExp | Ident '(' [FuncRParams] ')' | UnaryOp UnaryExp
void frontend::Analyzer::analysisUnaryExp(UnaryExp* root, vector<ir::Instruction*>& buffer){
    if(root->children[0]->type == NodeType::PRIMARYEXP){   // UnaryExp -> PrimaryExp
        ANALYSIS(node, PrimaryExp, 0);
        COPY_EXP_NODE(node, root); // pushup
    }
    else if(root->children[0]->type == NodeType::TERMINAL){   // UnaryExp -> Ident '(' [FuncRParams] ')'
        auto func_name = dynamic_cast<Term*>(root->children[0])->token.value;   // func_name
        auto t = symbol_table.functions[func_name]->returnType;  // get the return type
        auto op1 = Operand(func_name, Type::null);  // op1 as the function name
        auto des = Operand("t" + std::to_string(tmp_cnt++), t);
        if(root->children.size() == 3)
            buffer.push_back(new ir::CallInst(op1, des)); // no params
        else{
            GET_CHILD_PTR(node, FuncRParams, 2);     // FuncRParams node
            auto call_inst = new ir::CallInst(op1, vector<Operand>(), des);  //  Call IR
            analysisFuncRParams(node, buffer, *call_inst);
            buffer.push_back(call_inst);   //  push IR
        }
        root->v = des.name;
        root->t = t;
    }
    else if(root->children[0]->type == NodeType::UNARYOP){ // UnaryExp -> UnaryOp UnaryExp
        auto tk_type = dynamic_cast<Term*>(root->children[0]->children[0])->token.type;
        ANALYSIS(node, UnaryExp, 1);    // UnaryExp node
        if(tk_type == TokenType::PLUS){     // itself
            COPY_EXP_NODE(node, root);
        }
        else{
            // "-" "!"
            root->is_computable = node->is_computable;
            root->t = node->t;
            if(node->is_computable){ // is_computable
                if(node->t == Type::IntLiteral && tk_type == TokenType::MINU)
                    root->v = std::to_string(- std::stoi(node->v));
                else if(node->t == Type::IntLiteral && tk_type == TokenType::NOT)
                    root->v = std::to_string(! std::stoi(node->v));
                else if(node->t == Type::FloatLiteral && tk_type == TokenType::MINU)
                    root->v = std::to_string(- std::stof(node->v));
                else
                    root->v = std::to_string(! std::stof(node->v));
            }
            else{
                auto op1 = Operand(node->v, node->t);
                auto des = Operand("t" + std::to_string(tmp_cnt++), Type::Int);
                if(tk_type == TokenType::NOT){
                    buffer.push_back(new Instruction(op1, Operand(), des, Operator::_not));
                    root->v = des.name;
                }
                else{
                    bool fg = node->t == Type::Int;
                    auto ty1 = fg ? Type::IntLiteral: Type::FloatLiteral;
                    auto ty2 = fg ? Operator::def: Operator::fdef;
                    auto ty3 = fg ? Operator::sub: Operator::fsub;
                    des.type = node->t;
                    auto op2 = Operand("t" + std::to_string(tmp_cnt++), des.type);
                    buffer.push_back(new Instruction(Operand("0", ty1), Operand(), op2, ty2)); // 0
                    buffer.push_back(new Instruction(op2, op1, des, ty3)); // des = 0 - op1
                    root->v = des.name;
                }
            }
        }
    }
    else assert(0 && "UnaryExp should be PrimaryExp, Ident or UnaryOp");  // Error
}


// FuncRParams -> Exp { ',' Exp }
void frontend::Analyzer::analysisFuncRParams(FuncRParams* root, vector<ir::Instruction*>& buffer, ir::CallInst& callinst){
    ANALYSIS(node, Exp, 0);  // Exp node // TODO
    callinst.argumentList.push_back(Operand(node->v, node->t));
    for(int i = 1; i < root->children.size(); i+=2){
        ANALYSIS(node, Exp, i+1);  // Exp node
        callinst.argumentList.push_back(Operand(node->v, node->t));
    }
}

