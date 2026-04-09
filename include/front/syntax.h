/**
 * @file syntax.h
 * @author MonrenZheng
 * 
 */

#ifndef SYNTAX_H
#define SYNTAX_H

#include"front/abstract_syntax_tree.h"
#include"front/token.h"

#include<vector>

namespace frontend {

// definition of Parser
// a parser should take a token stream as input, then parsing it, output a AST
struct Parser {
    uint32_t index; // current token index
    const std::vector<Token>& token_stream;

    /**
     * @brief constructor
     * @param tokens: the input token_stream
     */
    Parser(const std::vector<Token>& tokens);

    /**
     * @brief destructor
     */
    ~Parser();
    
    /**
     * @brief creat the abstract syntax tree
     * @return the root of abstract syntax tree
    */
    CompUnit* get_abstract_syntax_tree();
    Term* parseTerm(AstNode* parent, TokenType expected);
    bool parseCompUnit(CompUnit* root);
    bool parseDecl(Decl* root);
    bool parseFuncDef(FuncDef* root);
    bool parseConstDecl(ConstDecl* root);
    bool parseBType(BType* root);
    bool parseVarDecl(VarDecl* root);
    bool parseFuncType(FuncType* root);
    bool parseConstDef(ConstDef* root);
    bool parseBlock(Block* root);
    bool parseVarDef(VarDef* root);
    bool parseFuncFParams(FuncFParams* root);
    bool parseBlockItem(BlockItem* root);
    bool parseConstExp(ConstExp* root);
    bool parseConstInitVal(ConstInitVal* root);
    bool parseInitVal(InitVal* root);
    bool parseFuncFParam(FuncFParam* root);
    bool parseStmt(Stmt* root);
    bool parseAddExp(AddExp* root);
    bool parseExp(Exp* root);
    bool parseLVal(LVal* root);
    bool parseCond(Cond* root);
    bool parseMulExp(MulExp* root);
    bool parseLOrExp(LOrExp* root);
    bool parseUnaryExp(UnaryExp* root);
    bool parseLAndExp(LAndExp* root);
    bool parseFuncRParams(FuncRParams* root);
    bool parsePrimaryExp(PrimaryExp* root);
    bool parseUnaryOp(UnaryOp* root);
    bool parseEqExp(EqExp* root);
    bool parseNumber(Number* root);
    bool parseRelExp(RelExp* root);
    
    bool look2exp();
    bool look2stmt();
    /**
     * @brief for debug, should be called in the beginning of recursive descent functions 
     * @param node: current parsing node 
     */
    void log(AstNode* node);
};

} // namespace frontend

#endif