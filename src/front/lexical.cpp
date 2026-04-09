#include"front/lexical.h"

#include<map>
#include<cassert>
#include<string>
#include <unordered_map>

#define TODO assert(0 && "todo")

// #define DEBUG_DFA
// #define DEBUG_SCANNER

std::string frontend::toString(State s){
    switch (s){
    case State::Empty: return "Empty";
    case State::Ident: return "Ident";
    case State::IntLiteral: return "IntLiteral";
    case State::FloatLiteral: return "FloatLiteral";
    case State::op: return "op";
    default:
        assert(0 && "invalid State");
    }
    return "";
}


std::set<char> operators = {
    '+', '-', '*', '/', '%', '<', '>', ':', '=', ';', ',', '(', ')', '[', ']', '{', '}', '!', '&', '|'
};

// Check if the character is an operator
bool isoperator(char  c){
    if(operators.find(c) != operators.end())
        return true;
    else
        return false;
}

// Check if the string is a decimal
bool isdecimal(std::string s){
    if (s.size() >= 2)
        return true;
    else if (s.size() == 1 && std::isdigit(s[0]))
        return true;
    else
        return false;
}

// Check if the character is a well-combined operator
bool wellcombined(std::string s, char c){
    if(s == "<" && c == '=' || s == ">" && c == '=' || s == "=" && c == '=' || s == "!" && c == '=' || s == "&" && c == '&' || s == "|" && c == '|')
        return true;
    else
        return false;
}

// The type of the operator
frontend::TokenType get_op_type(std::string  s){
    if(s=="+")
        return frontend::TokenType::PLUS;
    else if(s=="-")
        return frontend::TokenType::MINU;
    else if(s=="*")
        return frontend::TokenType::MULT;
    else if(s=="/")
        return frontend::TokenType::DIV;
    else if(s=="%")
        return frontend::TokenType::MOD;
    else if(s=="<")
        return frontend::TokenType::LSS;
    else if(s==">")
        return frontend::TokenType::GTR;
    else if(s==":")
        return frontend::TokenType::COLON;
    else if(s=="=")
        return frontend::TokenType::ASSIGN;
    else if(s==";")
        return frontend::TokenType::SEMICN;
    else if(s==",")
        return frontend::TokenType::COMMA;
    else if(s=="(")
        return frontend::TokenType::LPARENT;
    else if(s==")")
        return frontend::TokenType::RPARENT;
    else if(s=="[")
        return frontend::TokenType::LBRACK;
    else if(s=="]")
        return frontend::TokenType::RBRACK;
    else if(s=="{")
        return frontend::TokenType::LBRACE;
    else if(s=="}")
        return frontend::TokenType::RBRACE;
    else if(s=="!")
        return frontend::TokenType::NOT;
    else if(s=="<=")
        return frontend::TokenType::LEQ;
    else if(s==">=")
        return frontend::TokenType::GEQ;
    else if(s=="==")
        return frontend::TokenType::EQL;
    else if(s=="!=")
        return frontend::TokenType::NEQ;
    else if(s=="&&")
        return frontend::TokenType::AND;
    else if(s=="||")
        return frontend::TokenType::OR;
}

// Get the type of the keyword
frontend::TokenType get_keyword_type(std::string s){
    if(s=="const")
        return frontend::TokenType::CONSTTK;
    else if(s=="void")
        return frontend::TokenType::VOIDTK;
    else if(s=="int")
        return frontend::TokenType::INTTK;
    else if(s=="float")
        return frontend::TokenType::FLOATTK;
    else if(s=="if")
        return frontend::TokenType::IFTK;
    else if(s=="else")
        return frontend::TokenType::ELSETK;
    else if(s=="while")
        return frontend::TokenType::WHILETK;
    else if(s=="continue")
        return frontend::TokenType::CONTINUETK;
    else if(s=="break")
        return frontend::TokenType::BREAKTK;
    else if(s=="return")
        return frontend::TokenType::RETURNTK;
}

// Set of keywords
std::set<std::string> frontend::keywords= {
    "const", "int", "float", "if", "else", "while", "continue", "break", "return", "void"
};

frontend::DFA::DFA(): cur_state(frontend::State::Empty), cur_str(){}

frontend::DFA::~DFA(){}

bool frontend::DFA::next(char input, Token& buf){
#ifdef DEBUG_DFA
#include<iostream>
    std::cout << "in state [" << toString(cur_state) << "], input = \'" << input << "\', str = " << cur_str << "\t";
#endif
    bool fg = false;
    switch (cur_state){
        case State::Empty:
            if(input==' ' || input=='\t' || input=='\n' || input=='\r')
                cur_state = State::Empty;
            else if(input>='0' && input<='9'){
                cur_state = State::IntLiteral;
                cur_str += input;
            } else if(isoperator(input)){
                cur_state = State::op;
                cur_str += input;
            }
            else if(input=='_' || std::isalpha(input)){
                cur_state = State::Ident;
                cur_str += input;
            }
            else if(input=='.'){
                cur_state = State::FloatLiteral;
                cur_str += input;
            }
            else assert(0 && "invalid input character in DFA");
            break;
        
        case State::Ident:
            if(input=='_' || std::isalpha(input) || (input >='0' && input<='9')){
                cur_state = State::Ident;
                cur_str += input;
            }
            else{
                if(keywords.find(cur_str) != keywords.end()){ // keyword
                    buf.type = get_keyword_type(cur_str);
                    buf.value = cur_str;
                }
                else{ // identifier
                    buf.type = frontend::TokenType::IDENFR;
                    buf.value = cur_str;
                }
                if(input==' ' || input=='\t' || input=='\n' || input=='\r'){
                    cur_state = State::Empty;
                    cur_str = "";
                }
                else if(isoperator(input)){
                    cur_state = State::op;
                    cur_str = input;
                }
                fg = true;
            }
            break;

        case State::IntLiteral:
            if(cur_str=="0" && (input=='x' || input=='b' || input=='X' || input=='B'))
                cur_str += input;
            else if(cur_str.size()>=2 && (cur_str[1]=='x' || cur_str[1]=='X') && ((input>='A' && input<='F') || (input>='a' && input<='f')))
                cur_str += input;
            else if(input>='0' && input<='9')
                cur_str += input;
            else if(isdecimal(cur_str) && input=='.'){
                cur_state = State::FloatLiteral;
                cur_str += '.';
            }
            else{
                buf.type = frontend::TokenType::INTLTR;
                buf.value = cur_str;
                if(isoperator(input)){
                    cur_state = State::op;
                    cur_str = input;
                }
                else if(input==' ' || input=='\n' || input=='\t' || input=='\r'){
                    cur_state = State::Empty;
                    cur_str = "";
                }
                fg = true;
            }
            break;

        case State::FloatLiteral:
            if(input>='0' && input<='9')
                cur_str += input;
            else{
                buf.type = frontend::TokenType::FLOATLTR;
                buf.value = cur_str;

                if(isoperator(input)){
                    cur_state = State::op;
                    cur_str = input;
                }
                else if(input==' ' || input=='\n' || input=='\t' || input=='\r'){
                    cur_state = State::Empty;
                    cur_str = "";
                }
                fg = true;
            }
            break;

        case State::op:
            if(wellcombined(cur_str, input))
                cur_str += input;
            else{
                buf.type = get_op_type(cur_str);
                buf.value = cur_str;

                if(isoperator(input)){
                    cur_state = State::op;
                    cur_str = input;
                }
                else if(input==' ' || input=='\t' || input=='\n' || input=='\r'){
                    cur_state = State::Empty;
                    cur_str = "";
                }
                else if(input>='0' && input<='9'){
                    cur_state = State::IntLiteral;
                    cur_str = input;
                }
                else if(input=='.'){
                    cur_state = State::FloatLiteral;
                    cur_str = input;
                }
                else if(input=='_' || std::isalpha(input)){
                    cur_state = State::Ident;
                    cur_str = input;
                }
                fg = true;
            }
            break;

        default:
            break;
    }

#ifdef DEBUG_DFA
    std::cout << "next state is [" << toString(cur_state) << "], next str = " << cur_str << std::endl;
#endif
    return fg;
}

void frontend::DFA::reset(){
    cur_state = State::Empty;
    cur_str = "";
}

frontend::Scanner::Scanner(std::string filename): fin(filename){
    if(!fin.is_open()){
        assert(0 && "in Scanner constructor, input file cannot open");
    }
}

frontend::Scanner::~Scanner(){
    fin.close();
}

std::vector<frontend::Token> frontend::Scanner::run(){
    // preprocess the input file
    std::vector<frontend::Token> streams;  //token sequence
    std::string str, cur;
    int inx1, inx2;
    while(std::getline(fin, cur)){
        // remove the comments
        inx1 = cur.find("//");
        cur = cur.substr(0, inx1);
        str += cur;
        str += "\n";
        //remove the multi-line comments
        if(str.find("*/") != -1){
            inx1 = str.find("/*");
            inx2 = str.rfind("*/");
            std::string post = str.substr(inx2 + 2);
            if(inx2 - inx1 >= 2){ //  /**/
                str = str.substr(0, inx1);
                str += post;  // add the part after the multi-line comment
            }
        }
    }
    str += ' ';  // add a character to the end of the string
    
    // Initialize DFA
    DFA  dfa;
    Token  t;
    for(int i=0; i < str.size(); i++){
        if(dfa.next(str[i],  t)){
            streams.push_back(t);
            #ifdef DEBUG_SCANNER
            #include<iostream>
                std::cout << "token: " << toString(t.type) << "\t" << t.value << std::endl;
            #endif
        }
    }

    return streams;
}