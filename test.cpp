#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <cctype>
#include <vector>
#include <memory>
using namespace std;

// 定义TokenPair结构体，用于存储标识符/关键字和对应的标记
struct TokenPair {
    string lexeme;
    string token;
};

// 全局变量定义
ifstream in;           // 输入文件流
ofstream out;          // 输出文件流
string currentToken;   // 当前识别的单词
string currentValue;   // 当前单词的值
int lineNum = 1;       // 当前行号（用于错误处理）
bool outputSyntaxInfo = true; // 控制是否输出语法信息的标志

// 哈希表存储关键字和符号
unordered_map<string, string> tokenMap;

// 语法树节点类型枚举
enum class NodeType {
    PROGRAM,                // 程序
    CONST_DECL,             // 常量说明
    VAR_DECL,               // 变量说明
    CONST_DEF,              // 常量定义
    VAR_DEF,                // 变量定义
    FUNC_DEF,               // 函数定义
    MAIN_FUNC,              // 主函数
    PARAM_LIST,             // 参数表
    PARAM,                  // 参数
    COMPOUND_STMT,          // 复合语句
    STMT_LIST,              // 语句列
    STMT,                   // 语句
    ASSIGN_STMT,            // 赋值语句
    RETURN_STMT,            // 返回语句
    WRITE_STMT,             // 写语句
    READ_STMT,              // 读语句
    IF_STMT,                // 条件语句
    WHILE_STMT,             // 循环语句
    FOR_STMT,               // 循环语句
    SWITCH_STMT,            // 开关语句
    CASE_STMT,              // 情况语句
    EXPRESSION,             // 表达式
    TERM,                   // 项
    FACTOR,                 // 因子
    UNARY_EXPR,             // 一元表达式
    INTEGER,                // 整数
    UNSIGNED_INTEGER,       // 无符号整数
    CHARACTER,              // 字符
    STRING,                 // 字符串
    IDENTIFIER,             // 标识符
    TOKEN_NODE              // 终结符节点
};

// 语法树节点类
class SyntaxTreeNode {
public:
    NodeType type;
    string value;
    vector<shared_ptr<SyntaxTreeNode>> children;
    
    SyntaxTreeNode(NodeType type, const string& value = "") : type(type), value(value) {}
    
    void addChild(shared_ptr<SyntaxTreeNode> child) {
        children.push_back(child);
    }
    
    // 输出语法树信息（仅用于调试）
    void printTree(int depth = 0) {
        string indent(depth * 2, ' ');
        cout << indent << "Node: " << getNodeTypeName(type) << " Value: " << value << endl;
        for (auto& child : children) {
            child->printTree(depth + 1);
        }
    }
    
    string getNodeTypeName(NodeType type) {
        switch (type) {
            case NodeType::PROGRAM: return "PROGRAM";
            case NodeType::CONST_DECL: return "CONST_DECL";
            case NodeType::VAR_DECL: return "VAR_DECL";
            case NodeType::CONST_DEF: return "CONST_DEF";
            case NodeType::VAR_DEF: return "VAR_DEF";
            case NodeType::FUNC_DEF: return "FUNC_DEF";
            case NodeType::MAIN_FUNC: return "MAIN_FUNC";
            case NodeType::PARAM_LIST: return "PARAM_LIST";
            case NodeType::PARAM: return "PARAM";
            case NodeType::COMPOUND_STMT: return "COMPOUND_STMT";
            case NodeType::STMT_LIST: return "STMT_LIST";
            case NodeType::STMT: return "STMT";
            case NodeType::ASSIGN_STMT: return "ASSIGN_STMT";
            case NodeType::RETURN_STMT: return "RETURN_STMT";
            case NodeType::WRITE_STMT: return "WRITE_STMT";
            case NodeType::READ_STMT: return "READ_STMT";
            case NodeType::IF_STMT: return "IF_STMT";
            case NodeType::WHILE_STMT: return "WHILE_STMT";
            case NodeType::FOR_STMT: return "FOR_STMT";
            case NodeType::SWITCH_STMT: return "SWITCH_STMT";
            case NodeType::CASE_STMT: return "CASE_STMT";
            case NodeType::EXPRESSION: return "EXPRESSION";
            case NodeType::TERM: return "TERM";
            case NodeType::FACTOR: return "FACTOR";
            case NodeType::UNARY_EXPR: return "UNARY_EXPR";
            case NodeType::INTEGER: return "INTEGER";
            case NodeType::UNSIGNED_INTEGER: return "UNSIGNED_INTEGER";
            case NodeType::CHARACTER: return "CHARACTER";
            case NodeType::STRING: return "STRING";
            case NodeType::IDENTIFIER: return "IDENTIFIER";
            case NodeType::TOKEN_NODE: return "TOKEN_NODE";
            default: return "UNKNOWN";
        }
    }
};

// 初始化哈希表
void initTokenMap() {
    // 关键字
    tokenMap["const"] = "CONSTTK";
    tokenMap["int"] = "INTTK";
    tokenMap["char"] = "CHARTK";
    tokenMap["void"] = "VOIDTK";
    tokenMap["main"] = "MAINTK";
    tokenMap["if"] = "IFTK";
    tokenMap["else"] = "ELSETK";
    tokenMap["switch"] = "SWITCHTK";
    tokenMap["case"] = "CASETK";
    tokenMap["default"] = "DEFAULTTK";
    tokenMap["while"] = "WHILETK";
    tokenMap["for"] = "FORTK";
    tokenMap["scanf"] = "SCANFTK";
    tokenMap["printf"] = "PRINTFTK";
    tokenMap["return"] = "RETURNTK";
    
    // 运算符和界符
    tokenMap["+"] = "PLUS";
    tokenMap["-"] = "MINU";
    tokenMap["*"] = "MULT";
    tokenMap["/"] = "DIV";
    tokenMap["<"] = "LSS";
    tokenMap["<="] = "LEQ";
    tokenMap[">"] = "GRE";
    tokenMap[">="] = "GEQ";
    tokenMap["=="] = "EQL";
    tokenMap["!="] = "NEQ";
    tokenMap[":"] = "COLON";
    tokenMap["="] = "ASSIGN";
    tokenMap[";"] = "SEMICN";
    tokenMap[","] = "COMMA";
    tokenMap["("] = "LPARENT";
    tokenMap[")"] = "RPARENT";
    tokenMap["["] = "LBRACK";
    tokenMap["]"] = "RBRACK";
    tokenMap["{"] = "LBRACE";
    tokenMap["}"] = "RBRACE";
}

// 跳过空白字符
void skipWhiteSpace() {
    char c;
    while (in.get(c)) {
        if (c == ' ' || c == '\t' || c == '\r') {
            continue;
        } else if (c == '\n') {
            lineNum++;
        } else {
            in.putback(c);
            break;
        }
    }
}

// 识别标识符或关键字
void identifyIdentifier() {
    char c;
    currentToken.clear();
    
    while (in.get(c)) {
        if (isalnum(c) || c == '_') {
            currentToken += c;
        } else {
            in.putback(c);
            break;
        }
    }
    
    // 转换为小写用于查找
    string lowercaseToken = currentToken;
    for (char &ch : lowercaseToken) {
        ch = tolower(ch);
    }
    
    // 查找是否为关键字
    auto it = tokenMap.find(lowercaseToken);
    if (it != tokenMap.end()) {
        currentValue = currentToken;
        currentToken = it->second;
    } else {
        currentValue = currentToken;
        currentToken = "IDENFR";
    }
}

// 识别整数常量
void identifyIntConstant() {
    char c;
    currentToken.clear();
    // c已经由lexical函数读取
    currentToken += c; 
    
    while (in.get(c)) {
        if (isdigit(c)) {
            currentToken += c;
        } else {
            in.putback(c);
            break;
        }
    }
    
    currentValue = currentToken;
    currentToken = "INTCON";
}

// 识别字符常量
void identifyCharConstant() {
    char c;
    currentToken.clear();
    
    // 读取字符内容（单引号内的字符）
    if (in.get(c)) {
        currentToken += c;
        // 确保有结束的单引号
        if (in.peek() == '\'') {
            in.get(c); // 跳过结束的单引号
        }
    }
    
    currentValue = currentToken;
    currentToken = "CHARCON";
}

// 识别字符串常量
void identifyStringConstant() {
    char c;
    currentToken.clear();
    
    while (in.get(c)) {
        if (c == '"') {
            break; // 字符串结束
        }
        currentToken += c;
    }
    
    currentValue = currentToken;
    currentToken = "STRCON";
}

// 识别运算符和界符
void identifyOperator(char c) {
    currentToken.clear();
    
    // c已经是当前字符，不需要再读取
    currentToken += c;
    
    // 检查是否是双字符运算符
    if (c == '<' || c == '>' || c == '=' || c == '!') {
        if (in.peek() == '=') {
            in.get(c);
            currentToken += c;
        }
    }
    
    // 查找对应的标记
    auto it = tokenMap.find(currentToken);
    if (it != tokenMap.end()) {
        currentValue = currentToken;
        currentToken = it->second;
    } else {
        // 未知符号处理
        currentValue = currentToken;
        currentToken = "UNKNOWN";
    }
}

// 读取下一个单词
void getToken() {
    char c;
    
    skipWhiteSpace();
    
    if (in.peek() == EOF) return;
    
    c = in.peek();
    
    if (isalpha(c) || c == '_') {
        // 标识符或关键字
        identifyIdentifier();
    } else if (isdigit(c)) {
        // 整数常量
        in.get(c); // 读取第一个数字
        currentToken.clear();
        currentToken += c;
        
        while (in.get(c)) {
            if (isdigit(c)) {
                currentToken += c;
            } else {
                in.putback(c);
                break;
            }
        }
        
        currentValue = currentToken;
        currentToken = "INTCON";
    } else if (c == '\'') {
        // 字符常量
        in.get(c); // 跳过开始的单引号
        identifyCharConstant();
    } else if (c == '"') {
        // 字符串常量
        in.get(c); // 跳过开始的双引号
        identifyStringConstant();
    } else {
        // 运算符或界符
        in.get(c); // 读取运算符字符
        identifyOperator(c);
    }
    
    // 输出单词信息
    out << currentToken << " " << currentValue << endl;
    cout << currentToken << " " << currentValue << endl;
}

// 创建终结符节点
shared_ptr<SyntaxTreeNode> createTokenNode() {
    auto node = make_shared<SyntaxTreeNode>(NodeType::TOKEN_NODE, currentToken + " " + currentValue);
    return node;
}

// 打印语法成分信息
void printSyntax(const string& syntaxName) {
    if (outputSyntaxInfo) {
        out << syntaxName << endl;
        cout << syntaxName << endl;
    }
}

// 语法分析函数：程序
shared_ptr<SyntaxTreeNode> Program();

// 语法分析函数：常量说明
shared_ptr<SyntaxTreeNode> ConstDecl();

// 语法分析函数：变量说明
shared_ptr<SyntaxTreeNode> VarDecl();

// 语法分析函数：常量定义
shared_ptr<SyntaxTreeNode> ConstDef();

// 语法分析函数：变量定义
shared_ptr<SyntaxTreeNode> VarDef();

// 语法分析函数：函数定义
shared_ptr<SyntaxTreeNode> FuncDef();

// 语法分析函数：主函数
shared_ptr<SyntaxTreeNode> MainFunc();

// 语法分析函数：参数表
shared_ptr<SyntaxTreeNode> ParamList();

// 语法分析函数：参数
shared_ptr<SyntaxTreeNode> Param();

// 语法分析函数：复合语句
shared_ptr<SyntaxTreeNode> CompoundStmt();

// 语法分析函数：语句列
shared_ptr<SyntaxTreeNode> StmtList();

// 语法分析函数：语句
shared_ptr<SyntaxTreeNode> Stmt();

// 语法分析函数：赋值语句
shared_ptr<SyntaxTreeNode> AssignmentStmt();

// 语法分析函数：返回语句
shared_ptr<SyntaxTreeNode> ReturnStmt();

// 语法分析函数：写语句
shared_ptr<SyntaxTreeNode> WriteStmt();

// 语法分析函数：表达式
shared_ptr<SyntaxTreeNode> Expression();

// 语法分析函数：项
shared_ptr<SyntaxTreeNode> Term();

// 语法分析函数：因子
shared_ptr<SyntaxTreeNode> Factor();

// 语法分析函数：整数
shared_ptr<SyntaxTreeNode> Integer();

// 语法分析函数：无符号整数
shared_ptr<SyntaxTreeNode> UnsignedInteger();

// 语法分析函数：字符串
shared_ptr<SyntaxTreeNode> String();

// 实现语法分析函数
shared_ptr<SyntaxTreeNode> Program() {
    auto node = make_shared<SyntaxTreeNode>(NodeType::PROGRAM);
    
    // 处理所有可能的程序元素，直到文件结束
    while (currentToken != "" && in.peek() != EOF) {
        if (currentToken == "CONSTTK") {
            // 处理常量说明
            node->addChild(ConstDecl());
        } else if (currentToken == "INTTK" || currentToken == "CHARTK") {
            // 处理变量说明或函数定义
            // 检查是否是函数定义（后面跟着标识符）
            bool isFunction = false;
            char peekChar;
            in >> ws;
            peekChar = in.peek();
            in.putback(peekChar);
            
            // 如果下一个字符是字母或下划线，可能是函数名
            if (isalpha(peekChar) || peekChar == '_') {
                string tempToken = currentToken;
                string tempValue = currentValue;
                getToken(); // 获取标识符
                if (currentToken == "IDENFR") {
                    // 再看下一个是否是左括号
                    in >> ws;
                    peekChar = in.peek();
                    in.putback(peekChar);
                    if (peekChar == '(') {
                        isFunction = true;
                    }
                }
                // 回退
                for (char ch : currentValue) {
                    in.putback(ch);
                }
                currentToken = tempToken;
                currentValue = tempValue;
            }
            
            if (isFunction) {
                // 函数定义
                node->addChild(FuncDef());
            } else {
                // 变量说明
                node->addChild(VarDecl());
            }
        } else if (currentToken == "VOIDTK") {
            // 处理void函数（可能是主函数）
            // 先保存当前token
            string tempToken = currentToken;
            string tempValue = currentValue;
            
            // 获取下一个token看是否是main
            getToken();
            if (currentToken == "MAINTK") {
                // 是主函数，回退并处理
                in.putback('m');
                currentToken = tempToken;
                currentValue = tempValue;
                node->addChild(MainFunc());
            } else {
                // 不是主函数，回退并作为普通函数处理
                for (char ch : currentValue) {
                    in.putback(ch);
                }
                currentToken = tempToken;
                currentValue = tempValue;
                node->addChild(FuncDef());
            }
        }
    }
    
    printSyntax("<程序>");
    return node;
}

shared_ptr<SyntaxTreeNode> ConstDecl() {
    auto node = make_shared<SyntaxTreeNode>(NodeType::CONST_DECL);
    
    if (currentToken == "CONSTTK") {
        node->addChild(createTokenNode());
        getToken();
        
        if (currentToken == "INTTK" || currentToken == "CHARTK") {
            node->addChild(createTokenNode());
            getToken();
            
            node->addChild(ConstDef());
            while (currentToken == "COMMA") {
                node->addChild(createTokenNode());
                getToken();
                node->addChild(ConstDef());
            }
            
            if (currentToken == "SEMICN") {
                node->addChild(createTokenNode());
                getToken();
            }
        }
    }
    
    printSyntax("<常量说明>");
    return node;
}

shared_ptr<SyntaxTreeNode> VarDecl() {
    auto node = make_shared<SyntaxTreeNode>(NodeType::VAR_DECL);
    
    if (currentToken == "INTTK" || currentToken == "CHARTK") {
        node->addChild(createTokenNode());
        getToken();
        
        node->addChild(VarDef());
        while (currentToken == "COMMA") {
            node->addChild(createTokenNode());
            getToken();
            node->addChild(VarDef());
        }
        
        if (currentToken == "SEMICN") {
            node->addChild(createTokenNode());
            getToken();
        }
    }
    
    printSyntax("<变量说明>");
    return node;
}

shared_ptr<SyntaxTreeNode> ConstDef() {
    auto node = make_shared<SyntaxTreeNode>(NodeType::CONST_DEF);
    
    if (currentToken == "IDENFR") {
        node->addChild(createTokenNode());
        getToken();
        
        if (currentToken == "ASSIGN") {
            node->addChild(createTokenNode());
            getToken();
            
            // 处理初始值
            if (currentToken == "INTCON") {
                node->addChild(UnsignedInteger());
                node->addChild(Integer());
            } else if (currentToken == "MINU") {
                // 处理负整数
                node->addChild(createTokenNode());
                getToken();
                if (currentToken == "INTCON") {
                    node->addChild(UnsignedInteger());
                    node->addChild(Integer());
                }
            } else if (currentToken == "CHARCON") {
                node->addChild(createTokenNode());
                getToken();
            }
        }
    }
    
    printSyntax("<常量定义>");
    return node;
}

shared_ptr<SyntaxTreeNode> VarDef() {
    auto node = make_shared<SyntaxTreeNode>(NodeType::VAR_DEF);
    
    if (currentToken == "IDENFR") {
        node->addChild(createTokenNode());
        getToken();
        
        if (currentToken == "ASSIGN") {
            node->addChild(createTokenNode());
            getToken();
            node->addChild(Expression());
        } else {
            printSyntax("<变量定义无初始化>");
        }
    }
    
    printSyntax("<变量定义>");
    return node;
}

shared_ptr<SyntaxTreeNode> FuncDef() {
    auto node = make_shared<SyntaxTreeNode>(NodeType::FUNC_DEF);
    
    // 处理返回类型
    if (currentToken == "INTTK" || currentToken == "CHARTK") {
        node->addChild(createTokenNode());
        getToken();
    }
    
    // 处理函数名
    if (currentToken == "IDENFR") {
        node->addChild(createTokenNode());
        getToken();
        printSyntax("<声明头部>");
    }
    
    // 处理参数表
    if (currentToken == "LPARENT") {
        node->addChild(createTokenNode());
        getToken();
        
        // 处理参数列表
        if (currentToken == "INTTK" || currentToken == "CHARTK") {
            node->addChild(ParamList());
        }
        
        if (currentToken == "RPARENT") {
            node->addChild(createTokenNode());
            getToken();
        }
    }
    
    // 处理函数体复合语句
    node->addChild(CompoundStmt());
    
    printSyntax("<有返回值函数定义>");
    return node;
}

shared_ptr<SyntaxTreeNode> MainFunc() {
    auto node = make_shared<SyntaxTreeNode>(NodeType::MAIN_FUNC);
    
    // 处理void关键字
    if (currentToken == "VOIDTK") {
        node->addChild(createTokenNode());
        getToken();
    }
    
    // 处理main函数名
    if (currentToken == "MAINTK") {
        node->addChild(createTokenNode());
        getToken();
    }
    
    // 处理参数列表括号
    if (currentToken == "LPARENT") {
        node->addChild(createTokenNode());
        getToken();
    }
    
    if (currentToken == "RPARENT") {
        node->addChild(createTokenNode());
        getToken();
    }
    
    // 处理函数体复合语句
    node->addChild(CompoundStmt());
    
    printSyntax("<主函数>");
    return node;
}

shared_ptr<SyntaxTreeNode> ParamList() {
    auto node = make_shared<SyntaxTreeNode>(NodeType::PARAM_LIST);
    node->addChild(createTokenNode()); // 类型
    getToken();
    node->addChild(createTokenNode()); // 参数名
    getToken();
    
    while (currentToken == "COMMA") {
        node->addChild(createTokenNode());
        getToken();
        node->addChild(createTokenNode()); // 类型
        getToken();
        node->addChild(createTokenNode()); // 参数名
        getToken();
    }
    
    printSyntax("<参数表>");
    return node;
}

shared_ptr<SyntaxTreeNode> CompoundStmt() {
    auto node = make_shared<SyntaxTreeNode>(NodeType::COMPOUND_STMT);
    
    // 处理左大括号
    if (currentToken == "LBRACE") {
        node->addChild(createTokenNode());
        getToken();
        
        // 处理变量说明
        while (currentToken == "INTTK" || currentToken == "CHARTK") {
            node->addChild(VarDecl());
        }
        
        // 处理语句列
        if (currentToken != "RBRACE") {
            node->addChild(StmtList());
        }
        
        // 处理右大括号
        if (currentToken == "RBRACE") {
            node->addChild(createTokenNode());
            getToken();
        }
    }
    
    printSyntax("<复合语句>");
    return node;
}

shared_ptr<SyntaxTreeNode> StmtList() {
    auto node = make_shared<SyntaxTreeNode>(NodeType::STMT_LIST);
    
    node->addChild(Stmt());
    while (currentToken != "RBRACE") {
        node->addChild(Stmt());
    }
    
    printSyntax("<语句列>");
    return node;
}

shared_ptr<SyntaxTreeNode> Stmt() {
    auto node = make_shared<SyntaxTreeNode>(NodeType::STMT);
    
    if (currentToken == "IDENFR") {
        // 赋值语句
        node->addChild(AssignmentStmt());
    } else if (currentToken == "RETURNTK") {
        // 返回语句
        node->addChild(ReturnStmt());
    } else if (currentToken == "PRINTFTK") {
        // 写语句
        node->addChild(WriteStmt());
    } else if (currentToken == "SCANFTK") {
        // 读语句（简化实现）
        node->addChild(createTokenNode());
        getToken();
        if (currentToken == "LPARENT") {
            node->addChild(createTokenNode());
            getToken();
            if (currentToken == "RPARENT") {
                node->addChild(createTokenNode());
                getToken();
            }
        }
        if (currentToken == "SEMICN") {
            node->addChild(createTokenNode());
            getToken();
        }
    } else if (currentToken == "IFTK") {
        // 条件语句（简化实现）
        node->addChild(createTokenNode());
        getToken();
        if (currentToken == "LPARENT") {
            node->addChild(createTokenNode());
            getToken();
            node->addChild(Expression());
            if (currentToken == "RPARENT") {
                node->addChild(createTokenNode());
                getToken();
            }
        }
        node->addChild(Stmt());
    } else if (currentToken == "LBRACE") {
        // 复合语句
        node->addChild(CompoundStmt());
    }
    
    printSyntax("<语句>");
    return node;
}

shared_ptr<SyntaxTreeNode> AssignmentStmt() {
    auto node = make_shared<SyntaxTreeNode>(NodeType::ASSIGN_STMT);
    node->addChild(createTokenNode()); // IDENFR
    getToken();
    
    if (currentToken == "ASSIGN") {
        node->addChild(createTokenNode());
        getToken();
        node->addChild(Expression());
    }
    
    printSyntax("<赋值语句>");
    if (currentToken == "SEMICN") {
        node->addChild(createTokenNode());
        getToken();
    }
    return node;
}

shared_ptr<SyntaxTreeNode> ReturnStmt() {
    auto node = make_shared<SyntaxTreeNode>(NodeType::RETURN_STMT);
    node->addChild(createTokenNode()); // RETURNTK
    getToken();
    
    if (currentToken == "LPARENT") {
        node->addChild(createTokenNode());
        getToken();
        node->addChild(Expression());
        if (currentToken == "RPARENT") {
            node->addChild(createTokenNode());
            getToken();
        }
    }
    
    printSyntax("<返回语句>");
    if (currentToken == "SEMICN") {
        node->addChild(createTokenNode());
        getToken();
    }
    return node;
}

shared_ptr<SyntaxTreeNode> WriteStmt() {
    auto node = make_shared<SyntaxTreeNode>(NodeType::WRITE_STMT);
    node->addChild(createTokenNode()); // PRINTFTK
    getToken();
    
    if (currentToken == "LPARENT") {
        node->addChild(createTokenNode());
        getToken();
        
        if (currentToken == "STRCON") {
            node->addChild(String());
        } else if (currentToken == "IDENFR") {
            // 函数调用
            node->addChild(createTokenNode());
            getToken();
            if (currentToken == "LPARENT") {
                node->addChild(createTokenNode());
                getToken();
                
                // 处理值参数表
                node->addChild(Expression());
                while (currentToken == "COMMA") {
                    node->addChild(createTokenNode());
                    getToken();
                    node->addChild(Expression());
                }
                printSyntax("<值参数表>");
                
                if (currentToken == "RPARENT") {
                    node->addChild(createTokenNode());
                    getToken();
                }
            }
            printSyntax("<有返回值函数调用语句>");
            node->addChild(Factor());
            node->addChild(Term());
            node->addChild(Expression());
        }
        
        if (currentToken == "RPARENT") {
            node->addChild(createTokenNode());
            getToken();
        }
    }
    
    printSyntax("<写语句>");
    if (currentToken == "SEMICN") {
        node->addChild(createTokenNode());
        getToken();
    }
    return node;
}

shared_ptr<SyntaxTreeNode> Expression() {
    auto node = make_shared<SyntaxTreeNode>(NodeType::EXPRESSION);
    
    if (currentToken == "MINU") {
        // 处理负表达式
        node->addChild(createTokenNode());
        getToken();
        node->addChild(Term());
    } else {
        node->addChild(Term());
    }
    
    while (currentToken == "PLUS" || currentToken == "MINU") {
        node->addChild(createTokenNode());
        getToken();
        node->addChild(Term());
    }
    
    printSyntax("<表达式>");
    return node;
}

shared_ptr<SyntaxTreeNode> Term() {
    auto node = make_shared<SyntaxTreeNode>(NodeType::TERM);
    node->addChild(Factor());
    
    while (currentToken == "MULT" || currentToken == "DIV") {
        node->addChild(createTokenNode());
        getToken();
        node->addChild(Factor());
    }
    
    printSyntax("<项>");
    return node;
}

shared_ptr<SyntaxTreeNode> Factor() {
    auto node = make_shared<SyntaxTreeNode>(NodeType::FACTOR);
    
    if (currentToken == "IDENFR") {
        node->addChild(createTokenNode());
        getToken();
    } else if (currentToken == "LPARENT") {
        node->addChild(createTokenNode());
        getToken();
        node->addChild(Expression());
        if (currentToken == "RPARENT") {
            node->addChild(createTokenNode());
            getToken();
        }
    } else if (currentToken == "INTCON") {
        // 处理整数常量
        node->addChild(createTokenNode());
        getToken();
    } else if (currentToken == "MINU") {
        // 处理负号
        node->addChild(createTokenNode());
        getToken();
        node->addChild(Factor());
    } else if (currentToken == "CHARCON") {
        node->addChild(createTokenNode());
        getToken();
    }
    
    printSyntax("<因子>");
    return node;
}

shared_ptr<SyntaxTreeNode> Integer() {
    auto node = make_shared<SyntaxTreeNode>(NodeType::INTEGER);
    
    if (currentToken == "MINU") {
        node->addChild(createTokenNode());
        getToken();
    }
    node->addChild(UnsignedInteger());
    
    printSyntax("<整数>");
    return node;
}

shared_ptr<SyntaxTreeNode> UnsignedInteger() {
    auto node = make_shared<SyntaxTreeNode>(NodeType::UNSIGNED_INTEGER);
    node->addChild(createTokenNode()); // INTCON
    getToken();
    printSyntax("<无符号整数>");
    return node;
}

shared_ptr<SyntaxTreeNode> String() {
    auto node = make_shared<SyntaxTreeNode>(NodeType::STRING);
    node->addChild(createTokenNode()); // STRCON
    getToken();
    printSyntax("<字符串>");
    return node;
}

// 修改后的主函数
int main() {
    // 设置控制台编码（Windows）
    system("chcp 65001 > nul");
    
    // 初始化哈希表
    initTokenMap();
    
    // 打开文件
    in.open("testfile.txt");
    if (!in.is_open()) {
        cerr << "错误：无法打开testfile.txt文件" << endl;
        return 1;
    }
    
    out.open("output.txt");
    if (!out.is_open()) {
        cerr << "错误：无法打开output.txt文件" << endl;
        in.close();
        return 1;
    }
    
    cout << "从testfile.txt读取的内容：" << endl;
    // 读取并显示文件内容（用于调试）
    string line;
    while (getline(in, line)) {
        cout << line << endl;
    }
    in.clear();
    in.seekg(0, ios::beg);
    
    // 获取第一个单词
    getToken();
    
    // 执行语法分析
    shared_ptr<SyntaxTreeNode> syntaxTree = Program();
    
    // 关闭文件
    in.close();
    out.close();
    
    cout << "已成功将内容按格式写入output.txt文件" << endl;
    
    return 0;
}