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

// 哈希表存储关键字和符号
unordered_map<string, string> tokenMap;

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
    
    cout << "哈希表初始化完成" << endl;
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

// 词法分析主函数
void lexical() {
    char c;
    
    while (in.peek() != EOF) {
        skipWhiteSpace();
        
        if (in.peek() == EOF) break;
        
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
        
        // 输出结果
        out << currentToken << " " << currentValue << endl;
        cout << currentToken << " " << currentValue << endl;
    }
}

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
    
    // 执行词法分析
    lexical();
    
    // 关闭文件
    in.close();
    out.close();
    
    cout << "已成功将内容按格式写入output.txt文件" << endl;
    
    return 0;
}
