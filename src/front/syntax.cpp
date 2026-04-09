#include"front/syntax.h"

#include<iostream>
#include<cassert>

using frontend::Parser;

// #define DEBUG_PARSER
#define TODO assert(0 && "todo")
#define CUR_TOKEN_IS(tk_type) (token_stream[index].type == TokenType::tk_type)
#define PARSE_TOKEN(tk_type) \
    do { \
        auto* __parse_token_##__COUNTER__ = parseTerm(root, TokenType::tk_type); \
        root->children.push_back(__parse_token_##__COUNTER__); \
    } while(0);
#define PARSE(name, type) \
    do { \
        auto* name##__COUNTER__ = new type(root); \
        assert(parse##type(name##__COUNTER__)); \
        root->children.push_back(name##__COUNTER__); \
    } while(0);


Parser::Parser(const std::vector<frontend::Token>& tokens): index(0), token_stream(tokens){}

Parser::~Parser(){}

frontend::CompUnit* Parser::get_abstract_syntax_tree(){
    // Create the root of AST
    frontend::CompUnit* root = new frontend::CompUnit();
    assert(parseCompUnit(root));
    return root;
}


frontend::Term* Parser::parseTerm(AstNode* parent, TokenType expected){
    if (token_stream[index].type == expected){
        Term* term = new Term(token_stream[index], parent);
        index++;
        return term;
    }
    else assert(0 && "Terminator does not meet expectations");
}


void Parser::log(AstNode* node){
#ifdef DEBUG_PARSER
        std::cout << "in parse" << toString(node->type) << ", cur_token_type::" << toString(token_stream[index].type) << ", token_val::" << token_stream[index].value << '\n';
#endif
}


bool Parser::look2exp(){ // + - ! Ident ( int float
    if(CUR_TOKEN_IS(PLUS) || CUR_TOKEN_IS(MINU) || CUR_TOKEN_IS(NOT) || CUR_TOKEN_IS(IDENFR) || CUR_TOKEN_IS(LPARENT) || CUR_TOKEN_IS(INTLTR) || CUR_TOKEN_IS(FLOATLTR))
        return true;
    else
        return false;
}


bool Parser::look2stmt(){ // First for Stmt
    if(CUR_TOKEN_IS(IDENFR) || CUR_TOKEN_IS(LBRACE) || CUR_TOKEN_IS(IFTK) || CUR_TOKEN_IS(WHILETK) || CUR_TOKEN_IS(BREAKTK) || CUR_TOKEN_IS(CONTINUETK) || CUR_TOKEN_IS(RETURNTK) || CUR_TOKEN_IS(SEMICN) || look2exp())
        return true;
    else
        return false;
}


// CompUnit -> (Decl | FuncDef) [CompUnit]
bool Parser::parseCompUnit(CompUnit* root){
    log(root);

    if(CUR_TOKEN_IS(INTTK) || CUR_TOKEN_IS(FLOATTK)){ // int or float
        // Check declaration or a function definition
        if(token_stream[index+2].type == TokenType::LBRACK || token_stream[index+2].type == TokenType::ASSIGN || token_stream[index+2].type == TokenType::COMMA || token_stream[index+2].type == TokenType::SEMICN){
            /*
                int a
                int a,...
                int a = 1...
                int a[10]...
            */
            PARSE(node, Decl);
        }
        else if(token_stream[index+2].type == TokenType::LPARENT){
            /*
                int f(param1, param2)...
            */
            PARSE(node, FuncDef);
        }
    }
    else if(CUR_TOKEN_IS(CONSTTK)){ /// const int a...
        PARSE(node, Decl);
    }
    else if(CUR_TOKEN_IS(VOIDTK)){ // void f()...
        PARSE(node, FuncDef);
    }
    // Check if there are more declarations or function definitions
    if(CUR_TOKEN_IS(CONSTTK) || CUR_TOKEN_IS(VOIDTK) || CUR_TOKEN_IS(INTTK) || CUR_TOKEN_IS(FLOATTK)){
        PARSE(node, CompUnit);
    }

    return true;
}


// Decl -> ConstDecl | VarDecl
bool Parser::parseDecl(Decl* root){
    log(root);
    
    if(CUR_TOKEN_IS(CONSTTK)){ // const
        PARSE(node, ConstDecl);
    }
    else if(CUR_TOKEN_IS(INTTK) || CUR_TOKEN_IS(FLOATTK)){ // Var
        PARSE(node, VarDecl);
    }
    else assert(0 && "invalid Decl, should be ConstDecl or VarDecl");

    return true;
}


// FuncDef -> FuncType Ident '(' [FuncFParams] ')' Block
bool Parser::parseFuncDef(FuncDef* root){
    log(root);

    PARSE(node, FuncType)
    PARSE_TOKEN(IDENFR);
    PARSE_TOKEN(LPARENT);
    if(CUR_TOKEN_IS(INTTK) || CUR_TOKEN_IS(FLOATTK)){ // Check FuncFParams FuncFParams -> FuncFParam -> BType -> ('int' | 'float')
        PARSE(node, FuncFParams);
    }
    PARSE_TOKEN(RPARENT);
    PARSE(node, Block);

    return true;
}


// ConstDecl -> 'const' BType ConstDef { ',' ConstDef } ';'
bool Parser::parseConstDecl(ConstDecl* root){
    log(root);

    PARSE_TOKEN(CONSTTK);
    PARSE(node, BType);
    PARSE(node, ConstDef);
    while(CUR_TOKEN_IS(COMMA)){ // 0 or * circular
        PARSE_TOKEN(COMMA);
        PARSE(node, ConstDef);
    }
    PARSE_TOKEN(SEMICN); // ;

    return true;
}


// VarDecl -> BType VarDef { ',' VarDef } ';'
bool Parser::parseVarDecl(VarDecl* root){
    log(root);

    PARSE(node, BType);
    PARSE(node, VarDef);
    while(CUR_TOKEN_IS(COMMA)){ // 0 or * circular ---  { ',' VarDef }
        PARSE_TOKEN(COMMA);
        PARSE(node, VarDef);
    }
    PARSE_TOKEN(SEMICN);//;

    return true;
}


// FuncType -> 'void' | 'int' | 'float'
bool Parser::parseFuncType(FuncType* root){
    log(root);
    
    if (CUR_TOKEN_IS(VOIDTK)){
        PARSE_TOKEN(VOIDTK);
    }
    else if (CUR_TOKEN_IS(INTTK)){
        PARSE_TOKEN(INTTK);
    }
    else if (CUR_TOKEN_IS(FLOATTK)){
        PARSE_TOKEN(FLOATTK);
    }
    else assert(0 && "invalid FuncType, should be void, int or float");
    
    return true;
}


// Block -> '{' { BlockItem } '}'
bool Parser::parseBlock(Block* root){
    log(root);

    PARSE_TOKEN(LBRACE);
    while(CUR_TOKEN_IS(CONSTTK) || CUR_TOKEN_IS(INTTK) || CUR_TOKEN_IS(FLOATTK) || look2stmt()){ // 0 or * circular ---- BlockItem -> Decl | Stmt
        PARSE(node, BlockItem);
    }
    PARSE_TOKEN(RBRACE);

    return true;
}


// BType -> 'int' | 'float'
bool Parser::parseBType(BType* root){
    log(root);

    if(CUR_TOKEN_IS(INTTK)){
        PARSE_TOKEN(INTTK);
    }
    else if(CUR_TOKEN_IS(FLOATTK)){
        PARSE_TOKEN(FLOATTK);
    }
    else assert(0 && "invalid BType, should be int or float");

    return true;
}


// ConstDef -> Ident { '[' ConstExp ']' } '=' ConstInitVal
bool Parser::parseConstDef(ConstDef* root){ // const int...
    log(root);

    PARSE_TOKEN(IDENFR);
    while(CUR_TOKEN_IS(LBRACK)){
        PARSE_TOKEN(LBRACK);       //[
        PARSE(node, ConstExp); // ConstExp
        PARSE_TOKEN(RBRACK);       //]
    }
    PARSE_TOKEN(ASSIGN); // =
    PARSE(node, ConstInitVal);

    return true;
}


// VarDef -> Ident { '[' ConstExp ']' } [ '=' InitVal ]
bool Parser::parseVarDef(VarDef* root){
    log(root);

    PARSE_TOKEN(IDENFR);
    while(CUR_TOKEN_IS(LBRACK)){
        PARSE_TOKEN(LBRACK);      // [
        PARSE(node, ConstExp);// ConstExp
        PARSE_TOKEN(RBRACK);      // ]
    }
    if(CUR_TOKEN_IS(ASSIGN)){ // =
        PARSE_TOKEN(ASSIGN);
        PARSE(node, InitVal);
    }

    return true;
}


// FuncFParams -> FuncFParam { ',' FuncFParam }
bool Parser::parseFuncFParams(FuncFParams* root){
    log(root);

    PARSE(node, FuncFParam);
    while(CUR_TOKEN_IS(COMMA)){ // 0 or * circular  { ',' FuncFParam }
        PARSE_TOKEN(COMMA);
        PARSE(node, FuncFParam);
    }

    return true;
}


// BlockItem -> Decl | Stmt
bool Parser::parseBlockItem(BlockItem* root){
    log(root);

    if(CUR_TOKEN_IS(CONSTTK) || CUR_TOKEN_IS(INTTK) || CUR_TOKEN_IS(FLOATTK)){
        PARSE(node, Decl);
    }
    else if(look2stmt()){
        PARSE(node, Stmt);
    }
    else assert(0 && "invalid BlockItem, should be Decl or Stmt");

    return true;
}


// ConstExp -> AddExp
bool Parser::parseConstExp(ConstExp* root){
    log(root);

    PARSE(node, AddExp);

    return true;
}


// ConstInitVal -> ConstExp | '{' [ ConstInitVal { ',' ConstInitVal } ] '}'
bool Parser::parseConstInitVal(ConstInitVal* root){
    log(root);

    if(look2exp()){
        PARSE(node, ConstExp);
    }
    else if(CUR_TOKEN_IS(LBRACE)){
        PARSE_TOKEN(LBRACE);
        if(look2exp() || CUR_TOKEN_IS(LBRACE)){ // look ahead to check if it is a ConstExp or a ConstInitVal
            PARSE(node, ConstInitVal);
            while(CUR_TOKEN_IS(COMMA)){
                PARSE_TOKEN(COMMA);
                PARSE(node, ConstInitVal);
            }
        }
        PARSE_TOKEN(RBRACE);
    }
    else assert(0 && "invalid ConstInitVal, should be ConstExp or { ConstInitVal { , ConstInitVal } }");

    return true;
}


// InitVal -> Exp | '{' [ InitVal { ',' InitVal } ] '}'
bool Parser::parseInitVal(InitVal* root){
    log(root);

    if(look2exp()){ // Look for Exp------ (Exp -> AddExp  AddExp -> MulExp { ('+' | '-') MulExp } ....)
        PARSE(node, Exp);
    }
    else if(CUR_TOKEN_IS(LBRACE)){
        PARSE_TOKEN(LBRACE);
        if(look2exp() || CUR_TOKEN_IS(LBRACE)){ // look ahead to check if there is an InitVal    InitVal-> Exp | '{' [ InitVal { , InitVal } ] '}'
            PARSE(node, InitVal);
            while(CUR_TOKEN_IS(COMMA)){ // { ',' InitVal }
                PARSE_TOKEN(COMMA);
                PARSE(node, InitVal);
            }
        }
        PARSE_TOKEN(RBRACE);
    }
    else assert(0 && "invalid InitVal, should be Exp or { InitVal { , InitVal } }");

    return true;
}


// FuncFParam -> BType Ident ['[' ']' { '[' Exp ']' }]
bool Parser::parseFuncFParam(FuncFParam* root){
    log(root);

    PARSE(node, BType);
    PARSE_TOKEN(IDENFR);
    if(CUR_TOKEN_IS(LBRACK)){
        PARSE_TOKEN(LBRACK);
        PARSE_TOKEN(RBRACK);
        while(CUR_TOKEN_IS(LBRACK)){ // { '[' Exp ']' }
            PARSE_TOKEN(LBRACK);
            PARSE(node, Exp);
            PARSE_TOKEN(RBRACK);
        }
    }

    return true;
}


// Stmt -> LVal '=' Exp ';' | Block | 'if' '(' Cond ')' Stmt [ 'else' Stmt ] | 'while' '(' Cond ')' Stmt | 'break' ';' | 'continue' ';' | 'return' [Exp] ';' | [Exp] ';'
bool Parser::parseStmt(Stmt* root){
    log(root);

    if(CUR_TOKEN_IS(IDENFR)){ // LVal -> Ident {'[' Exp ']'} or Exp ...
        if(token_stream[index+1].type == TokenType::ASSIGN || token_stream[index+1].type == TokenType::LBRACK){ ///Ident {'[' Exp ']'} or Ident '='
            PARSE(node, LVal);
            PARSE_TOKEN(ASSIGN);
            PARSE(node, Exp);
            PARSE_TOKEN(SEMICN);
        }
        else{ // Exp ;
            PARSE(node, Exp);
            PARSE_TOKEN(SEMICN);
        }
    }
    else if(CUR_TOKEN_IS(LBRACE)){ // Block -> '{' { BlockItem } '}'
        PARSE(node, Block);
    }
    else if(CUR_TOKEN_IS(IFTK)){ // 'if' '(' Cond ')' Stmt [ 'else' Stmt ]
        PARSE_TOKEN(IFTK);
        PARSE_TOKEN(LPARENT);
        PARSE(node, Cond);
        PARSE_TOKEN(RPARENT);
        PARSE(node, Stmt);
        if(CUR_TOKEN_IS(ELSETK)){ // [ 'else' Stmt ]
            PARSE_TOKEN(ELSETK);
            PARSE(node, Stmt);
        }
    }
    else if(CUR_TOKEN_IS(WHILETK)){ // while
        PARSE_TOKEN(WHILETK);
        PARSE_TOKEN(LPARENT);
        PARSE(node, Cond);
        PARSE_TOKEN(RPARENT);
        PARSE(node, Stmt);
    }
    else if(CUR_TOKEN_IS(BREAKTK)){ // break
        PARSE_TOKEN(BREAKTK);
        PARSE_TOKEN(SEMICN);
    }
    else if(CUR_TOKEN_IS(CONTINUETK)){ //continue
        PARSE_TOKEN(CONTINUETK);
        PARSE_TOKEN(SEMICN);
    }
    else if(CUR_TOKEN_IS(RETURNTK)){ // return
        PARSE_TOKEN(RETURNTK);
        if(look2exp()){ // [Exp]
            PARSE(node, Exp);
        }
        PARSE_TOKEN(SEMICN);
    }
    else if(look2exp() || CUR_TOKEN_IS(SEMICN)){ // [Exp] ;
        // [Exp] ;
        if(CUR_TOKEN_IS(SEMICN)){
            PARSE_TOKEN(SEMICN);
        }
        else{
            PARSE(node, Exp);
            PARSE_TOKEN(SEMICN);
        }
    }
    else assert(0 && "invalid Stmt, should be LVal = Exp ; | Block | if( Cond ) Stmt [ else Stmt ] | while( Cond ) Stmt | break ; | continue ; | return [Exp] ; | [Exp] ;");

    return true;
}


// AddExp -> MulExp { ('+' | '-') MulExp }
bool Parser::parseAddExp(AddExp* root){
    log(root);

    PARSE(node, MulExp);
    while(CUR_TOKEN_IS(PLUS) || CUR_TOKEN_IS(MINU)){
        if(CUR_TOKEN_IS(PLUS)){
            PARSE_TOKEN(PLUS);
        }
        else{
            PARSE_TOKEN(MINU);
        }
        PARSE(node, MulExp);
    }

    return true;
}


// Exp -> AddExp
bool Parser::parseExp(Exp* root){
    log(root);

    PARSE(node, AddExp);

    return true;
}


// LVal -> Ident {'[' Exp ']'}
bool Parser::parseLVal(LVal* root){
    log(root);

    PARSE_TOKEN(IDENFR);
    while(CUR_TOKEN_IS(LBRACK)){
        PARSE_TOKEN(LBRACK);
        PARSE(node, Exp);
        PARSE_TOKEN(RBRACK);
    }

    return true;
}


// Cond -> LOrExp
bool Parser::parseCond(Cond* root){
    log(root);

    PARSE(node, LOrExp);

    return true;
}


// MulExp -> UnaryExp { ('*' | '/' | '%') UnaryExp }
bool Parser::parseMulExp(MulExp* root){
    log(root);

    PARSE(node, UnaryExp);
    while(CUR_TOKEN_IS(MULT) || CUR_TOKEN_IS(DIV) || CUR_TOKEN_IS(MOD)){ // 0 or * circular
        if(CUR_TOKEN_IS(MULT)){
            PARSE_TOKEN(MULT);
        }
        else if(CUR_TOKEN_IS(DIV)){
            PARSE_TOKEN(DIV);
        }
        else{
            PARSE_TOKEN(MOD);
        }
        PARSE(node, UnaryExp);
    }

    return true;
}


// LOrExp -> LAndExp [ '||' LOrExp ]
bool Parser::parseLOrExp(LOrExp* root){
    log(root);

    PARSE(node, LAndExp);
    if(CUR_TOKEN_IS(OR)){ // [ '||' LOrExp ]
        PARSE_TOKEN(OR);
        PARSE(node, LOrExp);
    }

    return true;
}


// UnaryExp -> PrimaryExp | Ident '(' [FuncRParams] ')' | UnaryOp UnaryExp
bool Parser::parseUnaryExp(UnaryExp* root){
    log(root);

    if(CUR_TOKEN_IS(LPARENT) || CUR_TOKEN_IS(INTLTR) || CUR_TOKEN_IS(FLOATLTR)){ // PrimaryExp -> '(' Exp ')' | Number
        PARSE(node, PrimaryExp);
    }
    else if(CUR_TOKEN_IS(IDENFR)){ // PrimaryExp | Ident '(' [FuncRParams] ')'
        if(token_stream[index+1].type == TokenType::LPARENT){ // Ident '(' [FuncRParams] ')'
            PARSE_TOKEN(IDENFR);
            PARSE_TOKEN(LPARENT);
            if(look2exp()){
                PARSE(node, FuncRParams);
            }
            PARSE_TOKEN(RPARENT);
        }
        else{ // PrimaryExp
            PARSE(node, PrimaryExp);
        }
    }
    else if(CUR_TOKEN_IS(PLUS) || CUR_TOKEN_IS(MINU) || CUR_TOKEN_IS(NOT)){ // UnaryOp -> '+' | '-' | '!'
        PARSE(node, UnaryOp);
        PARSE(node, UnaryExp);
    }
    else assert(0 && "invalid UnaryExp, should be PrimaryExp | Ident ( [FuncRParams] ) | UnaryOp UnaryExp");

    return true;
}


// LAndExp -> EqExp [ '&&' LAndExp ]
bool Parser::parseLAndExp(LAndExp* root){
    log(root);

    PARSE(node, EqExp);
    if(CUR_TOKEN_IS(AND)){
        PARSE_TOKEN(AND);// '&&'
        PARSE(node, LAndExp);
    }

    return true;
}


// FuncRParams -> Exp { ',' Exp }
bool Parser::parseFuncRParams(FuncRParams* root){
    log(root);

    PARSE(node, Exp);
    while(CUR_TOKEN_IS(COMMA)){ // {',' Exp}
        PARSE_TOKEN(COMMA);
        PARSE(node, Exp);
    }

    return true;
}


// PrimaryExp -> '(' Exp ')' | LVal | Number
bool Parser::parsePrimaryExp(PrimaryExp* root){
    log(root);

    if(CUR_TOKEN_IS(LPARENT)){ // '(' Exp ')'
        PARSE_TOKEN(LPARENT);
        PARSE(node, Exp);
        PARSE_TOKEN(RPARENT);
    }
    else if(CUR_TOKEN_IS(IDENFR)){ // LVal -> Ident {'[' Exp ']'}
        PARSE(node, LVal);
    }
    else if(CUR_TOKEN_IS(INTLTR) || CUR_TOKEN_IS(FLOATLTR)){ // Number -> IntConst | floatConst
        PARSE(node, Number);
    }
    else assert(0 && "invalid PrimaryExp, should be ( Exp ) | LVal | Number");

    return true;
}


// UnaryOp -> '+' | '-' | '!'
bool Parser::parseUnaryOp(UnaryOp* root){
    log(root);

    if(CUR_TOKEN_IS(PLUS)){
        PARSE_TOKEN(PLUS);
    }
    else if(CUR_TOKEN_IS(MINU)){
        PARSE_TOKEN(MINU);
    }
    else if(CUR_TOKEN_IS(NOT)){
        PARSE_TOKEN(NOT);
    }
    else assert(0 && "invalid UnaryOp, should be + | - | !");

    return true;
}


// EqExp -> RelExp { ('==' | '!=') RelExp }
bool Parser::parseEqExp(EqExp* root){
    log(root);

    PARSE(node, RelExp);
    while (CUR_TOKEN_IS(EQL) || CUR_TOKEN_IS(NEQ)){
        if (CUR_TOKEN_IS(EQL)){
            PARSE_TOKEN(EQL);
        }
        else PARSE_TOKEN(NEQ);
        PARSE(node, RelExp);
    }

    return true;
}


// Number -> IntConst | floatConst
bool Parser::parseNumber(Number* root){
    log(root);

    if(CUR_TOKEN_IS(INTLTR)){
        PARSE_TOKEN(INTLTR);
    }
    else if(CUR_TOKEN_IS(FLOATLTR)){
        PARSE_TOKEN(FLOATLTR);
    }
    else assert(0 && "invalid Number, should be IntConst | floatConst");

    return true;
}


// RelExp -> AddExp { ('<' | '>' | '<=' | '>=') AddExp }
bool Parser::parseRelExp(RelExp* root){
    log(root);

    PARSE(node, AddExp);
    while(CUR_TOKEN_IS(LSS) || CUR_TOKEN_IS(GTR) || CUR_TOKEN_IS(LEQ) || CUR_TOKEN_IS(GEQ)){
        if (CUR_TOKEN_IS(LSS)){
            PARSE_TOKEN(LSS);
        }
        else if (CUR_TOKEN_IS(GTR)){
            PARSE_TOKEN(GTR);
        }
        else if (CUR_TOKEN_IS(LEQ)){
            PARSE_TOKEN(LEQ);
        }
        else PARSE_TOKEN(GEQ);
        PARSE(node, AddExp);
    }

    return true;
}
