#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <cctype>
#include <vector>
#include <map>
#include <cstdlib>
using namespace std;

// 全局词法分析相关变量（供语法分析器访问）
ifstream in;           // 输入文件流
ofstream out;          // 输出文件流
string currentToken;   // 当前Token类型（如INTTK、IDENFR等）
string currentValue;   // 当前Token值（如标识符名、常量值等）
int lineNum = 1;       // 当前行号（错误处理用）
unordered_map<string, string> tokenMap;  // 关键字和符号映射表

// 初始化关键字和符号映射表
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

// 跳过空白字符（空格、制表符、换行等）
void skipWhiteSpace() {
    char c;
    while (in.get(c)) {
        if (c == ' ' || c == '\t' || c == '\r') continue;
        else if (c == '\n') lineNum++;
        else { in.putback(c); break; }
    }
}

// 识别标识符或关键字
void identifyIdentifier() {
    char c;
    currentToken.clear();
    while (in.get(c)) {
        if (isalnum(c) || c == '_') currentToken += c;
        else { in.putback(c); break; }
    }
    // 转换为小写查找关键字
    string lowercaseToken = currentToken;
    for (char &ch : lowercaseToken) ch = tolower(ch);
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
void identifyIntConstant(char firstChar) {
    currentToken.clear();
    currentToken += firstChar;
    char c;
    while (in.get(c)) {
        if (isdigit(c)) currentToken += c;
        else { in.putback(c); break; }
    }
    currentValue = currentToken;
    currentToken = "INTCON";
}

// 识别字符常量
void identifyCharConstant() {
    char c;
    currentToken.clear();
    if (in.get(c)) {  // 读取单引号内的字符
        currentToken += c;
        if (in.peek() == '\'') in.get(c);  // 跳过结束单引号
    }
    currentValue = currentToken;
    currentToken = "CHARCON";
}

// 识别字符串常量
void identifyStringConstant() {
    char c;
    currentToken.clear();
    while (in.get(c)) {
        if (c == '"') break;  // 字符串结束
        currentToken += c;
    }
    currentValue = currentToken;
    currentToken = "STRCON";
}

// 识别运算符和界符
void identifyOperator(char firstChar) {
    currentToken.clear();
    currentToken += firstChar;
    // 检查双字符运算符
    if (firstChar == '<' || firstChar == '>' || firstChar == '=' || firstChar == '!') {
        if (in.peek() == '=') {
            in.get(firstChar);
            currentToken += firstChar;
        }
    }
    auto it = tokenMap.find(currentToken);
    if (it != tokenMap.end()) {
        currentValue = currentToken;
        currentToken = it->second;
    } else {
        currentValue = currentToken;
        currentToken = "UNKNOWN";
    }
}

// 获取下一个Token（供语法分析器调用）
bool nextToken() {
    skipWhiteSpace();
    if (in.peek() == EOF) {
        currentToken.clear();
        currentValue.clear();
        return false;
    }
    char c = in.peek();
    if (isalpha(c) || c == '_') {
        identifyIdentifier();
    } else if (isdigit(c)) {
        in.get(c);
        identifyIntConstant(c);
    } else if (c == '\'') {
        in.get(c);  // 跳过开始单引号
        identifyCharConstant();
    } else if (c == '"') {
        in.get(c);  // 跳过开始双引号
        identifyStringConstant();
    } else {
        in.get(c);
        identifyOperator(c);
    }
    return true;
}

// 语法分析器类
class Parser {
public:
    map<string, string> funcResType;  // 函数返回类型映射

    // 检查是否为类型标识符（int/char）
    bool varIDENFR(const string& token) {
        return token == "INTTK" || token == "CHARTK";
    }

    // 输出当前Token并获取下一个
    void outputLexRes() {
        out << currentToken << " " << currentValue << endl;
        nextToken();  // 推进到下一个Token
    }

    // ＜常量说明＞ ::=  const＜常量定义＞;{ const＜常量定义＞;}
    void constDescribe() {
        outputLexRes();  // 输出CONSTTK
        constDefine();
        outputLexRes();  // 输出SEMICN
        while (currentToken == "CONSTTK") {
            outputLexRes();  // 输出CONSTTK
            constDefine();
            outputLexRes();  // 输出SEMICN
        }
        out << "<常量说明>" << endl;
    }

    // ＜常量定义＞ ::= int＜标识符＞＝＜整数＞{,＜标识符＞＝＜整数＞} | char＜标识符＞＝＜字符＞{,＜标识符＞＝＜字符＞}
    void constDefine() {
        string type = currentToken;  // 保存INTTK/CHARTK
        outputLexRes();  // 输出类型
        outputLexRes();  // 输出IDENFR
        outputLexRes();  // 输出ASSIGN
        if (type == "INTTK") {
            intNum();  // 处理整数
        } else {
            outputLexRes();  // 处理CHARCON
        }
        while (currentToken == "COMMA") {
            outputLexRes();  // 输出COMMA
            outputLexRes();  // 输出IDENFR
            outputLexRes();  // 输出ASSIGN
            if (type == "INTTK") intNum();
            else outputLexRes();  // 输出CHARCON
        }
        out << "<常量定义>" << endl;
    }

    // ＜变量说明＞
    void varDecribe() {
        while (varIDENFR(currentToken) && (currentToken != "LPARENT")) {
            string temp;
            do {
                outputLexRes();  // 输出类型（INTTK/CHARTK）
                outputLexRes();  // 输出IDENFR
                vector<int> numRecord;  // 记录数组维度
                while (currentToken == "LBRACK") {
                    outputLexRes();  // 输出LBRACK
                    numRecord.push_back(atoi(currentValue.c_str()));
                    unsignedInteger();  // 处理无符号整数
                    outputLexRes();  // 输出RBRACK
                }
                // 判断是否有初始化
                if (currentToken != "ASSIGN") {
                    temp = "<变量定义无初始化>";
                } else {
                    outputLexRes();  // 输出ASSIGN
                    if (numRecord.empty()) {
                        constNum();  // 简单变量初始化
                    } else {
                        // 数组初始化
                        int totalElem = 1;
                        for (auto x : numRecord) totalElem *= x;
                        while (totalElem > 0) {
                            if (currentToken == "INTCON" || currentToken == "CHARCON") {
                                constNum();
                                totalElem--;
                            } else {
                                outputLexRes();  // 处理'{'
                            }
                        }
                        // 输出数组关闭符
                        for (size_t i = 0; i < numRecord.size(); i++) {
                            outputLexRes();  // 输出RBRACE
                        }
                    }
                    temp = "<变量定义及初始化>";
                }
            } while (currentToken == "COMMA");
            out << temp << endl;
            out << "<变量定义>" << endl;
            outputLexRes();  // 输出SEMICN
        }
        out << "<变量说明>" << endl;
    }

    // ＜语句列＞
    void statementCol() {
        while (currentToken != "RBRACE") {
            statement();
        }
        out << "<语句列>" << endl;
    }

    // ＜语句＞
    void statement() {
        if (currentToken == "SEMICN") {
            outputLexRes();  // 空语句
        } else if (currentToken == "LBRACE") {
            outputLexRes();  // 输出LBRACE
            statementCol();  // 语句列
            outputLexRes();  // 输出RBRACE
        } else if (currentToken == "WHILETK") {
            // 循环语句（while）
            outputLexRes();  // 输出WHILETK
            outputLexRes();  // 输出LPARENT
            condition();     // 条件
            outputLexRes();  // 输出RPARENT
            statement();     // 循环体语句
            out << "<循环语句>" << endl;
        } else if (currentToken == "FORTK") {
            // 循环语句（for）
            outputLexRes();  // 输出FORTK
            outputLexRes();  // 输出LPARENT
            // for初始化：标识符=表达式
            for (int i = 0; i < 3; i++) outputLexRes();  // IDENFR、ASSIGN
            expression();                               // 表达式
            outputLexRes();  // 输出SEMICN
            condition();     // 条件
            outputLexRes();  // 输出SEMICN
            // for更新：标识符=标识符(+/-)步长
            for (int i = 0; i < 4; i++) outputLexRes();  // IDENFR、ASSIGN、IDENFR、PLUS/MINU
            step();          // 步长
            outputLexRes();  // 输出RPARENT
            statement();     // 循环体语句
            out << "<循环语句>" << endl;
        } else if (currentToken == "IFTK") {
            // 条件语句（if）
            outputLexRes();  // 输出IFTK
            outputLexRes();  // 输出LPARENT
            condition();     // 条件
            outputLexRes();  // 输出RPARENT
            statement();     // if体语句
            if (currentToken == "ELSETK") {
                outputLexRes();  // 输出ELSETK
                statement();     // else体语句
            }
            out << "<条件语句>" << endl;
        } else if (funcResType.count(currentValue)) {
            // 函数调用语句
            string type = funcResType[currentValue];
            outputLexRes();  // 输出函数名（IDENFR）
            outputLexRes();  // 输出LPARENT
            valueParameterTable();  // 参数表
            outputLexRes();  // 输出RPARENT
            out << (type == "<无返回值函数定义>" ? "<无返回值函数调用语句>" : "<有返回值函数调用语句>") << endl;
            outputLexRes();  // 输出SEMICN
        } else if (currentToken == "SCANFTK") {
            // 读语句
            for (int i = 0; i < 4; i++) outputLexRes();  // SCANFTK、LPARENT、IDENFR、RPARENT
            out << "<读语句>" << endl;
            outputLexRes();  // 输出SEMICN
        } else if (currentToken == "PRINTFTK") {
            // 写语句
            outputLexRes();  // 输出PRINTFTK
            outputLexRes();  // 输出LPARENT
            if (currentToken == "STRCON") {
                outputLexRes();  // 输出字符串
                out << "<字符串>" << endl;
                if (currentToken == "COMMA") {
                    outputLexRes();  // 输出COMMA
                    expression();    // 表达式
                }
            } else {
                expression();  // 表达式
            }
            outputLexRes();  // 输出RPARENT
            out << "<写语句>" << endl;
            outputLexRes();  // 输出SEMICN
        } else if (currentToken == "SWITCHTK") {
            // 情况语句
            outputLexRes();  // 输出SWITCHTK
            outputLexRes();  // 输出LPARENT
            expression();    // 表达式
            outputLexRes();  // 输出RPARENT
            outputLexRes();  // 输出LBRACE
            situationTable();// 情况表
            defaultRes();    // 缺省
            outputLexRes();  // 输出RBRACE
            out << "<情况语句>" << endl;
        } else if (currentToken == "RETURNTK") {
            // 返回语句
            outputLexRes();  // 输出RETURNTK
            if (currentToken == "LPARENT") {
                outputLexRes();  // 输出LPARENT
                expression();    // 表达式
                outputLexRes();  // 输出RPARENT
            }
            out << "<返回语句>" << endl;
            outputLexRes();  // 输出SEMICN
        } else if (currentToken == "IDENFR") {
            // 赋值语句
            outputLexRes();  // 输出IDENFR
            if (currentToken == "ASSIGN") {
                outputLexRes();  // 输出ASSIGN
                expression();    // 表达式
            } else if (currentToken == "LBRACK") {
                outputLexRes();  // 输出LBRACK
                expression();    // 表达式
                outputLexRes();  // 输出RBRACK
                if (currentToken == "ASSIGN") {
                    outputLexRes();  // 输出ASSIGN
                    expression();    // 表达式
                } else if (currentToken == "LBRACK") {
                    outputLexRes();  // 输出LBRACK
                    expression();    // 表达式
                    outputLexRes();  // 输出RBRACK
                    outputLexRes();  // 输出ASSIGN
                    expression();    // 表达式
                }
            }
            out << "<赋值语句>" << endl;
            outputLexRes();  // 输出SEMICN
        } else {
            cerr << "语法错误（行号：" << lineNum << "）：未知语句类型" << endl;
            exit(1);
        }
        out << "<语句>" << endl;
    }

    // ＜表达式＞::= ［＋｜－］<项>{<加法运算符><项>}
    void expression() {
        if (currentToken == "PLUS" || currentToken == "MINU") {
            outputLexRes();  // 输出正负号
        }
        term();
        while (currentToken == "PLUS" || currentToken == "MINU") {
            outputLexRes();  // 输出加法运算符
            term();
        }
        out << "<表达式>" << endl;
    }

    // ＜项＞ ::= <因子>{<乘法运算符><因子>}
    void term() {
        factor();
        while (currentToken == "MULT" || currentToken == "DIV") {
            outputLexRes();  // 输出乘法运算符
            factor();
        }
        out << "<项>" << endl;
    }

    // ＜因子＞ ::= <标识符>｜<标识符>'['<表达式>']'｜<标识符>'['<表达式>']''['<表达式>']'｜'('<表达式>')'｜<整数>｜<字符>｜<有返回值函数调用语句>
    void factor() {
        if (funcResType.count(currentValue) && funcResType[currentValue] == "<有返回值函数定义>") {
            // 有返回值函数调用
            outputLexRes();  // 输出函数名
            outputLexRes();  // 输出LPARENT
            if (currentToken != "RPARENT") valueParameterTable();
            else out << "<值参数表>" << endl;
            outputLexRes();  // 输出RPARENT
            out << "<有返回值函数调用语句>" << endl;
        } else if (currentToken == "CHARCON") {
            outputLexRes();  // 输出字符常量
        } else if (currentToken == "INTCON" || 
                  (currentToken == "PLUS" && nextToken() && currentToken == "INTCON") ||
                  (currentToken == "MINU" && nextToken() && currentToken == "INTCON")) {
            // 整数（含正负）
            if (currentToken == "PLUS" || currentToken == "MINU") {
                outputLexRes();  // 输出正负号
            }
            outputLexRes();  // 输出INTCON
            out << "<无符号整数>" << endl;
            out << "<整数>" << endl;
        } else if (currentToken == "LPARENT") {
            outputLexRes();  // 输出LPARENT
            expression();    // 表达式
            outputLexRes();  // 输出RPARENT
        } else if (currentToken == "IDENFR") {
            outputLexRes();  // 输出标识符
            if (currentToken == "LBRACK") {
                outputLexRes();  // 输出LBRACK
                expression();    // 表达式
                outputLexRes();  // 输出RBRACK
                if (currentToken == "LBRACK") {
                    outputLexRes();  // 输出LBRACK
                    expression();    // 表达式
                    outputLexRes();  // 输出RBRACK
                }
            }
        } else {
            cerr << "语法错误（行号：" << lineNum << "）：未知因子类型" << endl;
            exit(1);
        }
        out << "<因子>" << endl;
    }

    // ＜值参数表＞ ::= <表达式>{,<表达式>}｜<空>
    void valueParameterTable() {
        if (currentToken == "RPARENT") {
            out << "<值参数表>" << endl;
            return;
        }
        expression();
        while (currentToken == "COMMA") {
            outputLexRes();  // 输出COMMA
            expression();
        }
        out << "<值参数表>" << endl;
    }

    // ＜条件＞ ::= ＜表达式＞＜关系运算符＞＜表达式＞
    void condition() {
        expression();
        outputLexRes();  // 输出关系运算符（LSS/LEQ等）
        expression();
        out << "<条件>" << endl;
    }

    // 函数定义处理
    void func() {
        string funcType;
        if (currentToken == "VOIDTK") {
            funcType = "<无返回值函数定义>";
        } else if (currentToken == "INTTK" || currentToken == "CHARTK") {
            funcType = "<有返回值函数定义>";
        } else if (currentToken == "INTTK" && nextToken() && currentToken == "MAINTK") {
            funcType = "<主函数>";
        } else {
            cerr << "语法错误（行号：" << lineNum << "）：未知函数类型" << endl;
            exit(1);
        }
        outputLexRes();  // 输出函数类型（VOIDTK/INTTK等）
        string funcName = currentValue;
        outputLexRes();  // 输出函数名（IDENFR/MAINTK）
        funcResType[funcName] = funcType;  // 记录函数返回类型

        outputLexRes();  // 输出LPARENT
        if (currentToken != "RPARENT") {
            // 处理参数表
            outputLexRes();  // 输出参数类型
            outputLexRes();  // 输出参数名
            while (currentToken == "COMMA") {
                outputLexRes();  // 输出COMMA
                outputLexRes();  // 输出参数类型
                outputLexRes();  // 输出参数名
            }
            if (funcType != "<主函数>") out << "<参数表>" << endl;
        } else {
            if (funcType != "<主函数>") out << "<参数表>" << endl;
        }
        outputLexRes();  // 输出RPARENT
        outputLexRes();  // 输出LBRACE

        // 处理常量说明
        if (currentToken == "CONSTTK") {
            constDescribe();
        }
        // 处理变量说明
        if (varIDENFR(currentToken)) {
            varDecribe();
        }
        // 处理语句列
        statementCol();
        out << "<复合语句>" << endl;
        outputLexRes();  // 输出RBRACE
        out << funcType << endl;
    }

    // ＜步长＞
    void step() {
        unsignedInteger();
        out << "<步长>" << endl;
    }

    // ＜情况表＞ ::= ＜情况子语句＞{＜情况子语句＞}
    void situationTable() {
        situation();
        while (currentToken == "CASETK") {
            situation();
        }
        out << "<情况表>" << endl;
    }

    // ＜情况子语句＞ ::= case＜常量＞：＜语句＞
    void situation() {
        outputLexRes();  // 输出CASETK
        constNum();      // 常量
        outputLexRes();  // 输出COLON
        statement();     // 语句
        out << "<情况子语句>" << endl;
    }

    // ＜常量＞
    void constNum() {
        if (currentToken == "INTCON" || 
            (currentToken == "PLUS" && nextToken() && currentToken == "INTCON") ||
            (currentToken == "MINU" && nextToken() && currentToken == "INTCON")) {
            intNum();  // 整数常量
        } else if (currentToken == "CHARCON") {
            outputLexRes();  // 字符常量
        }
        out << "<常量>" << endl;
    }

    // ＜整数＞
    void intNum() {
        if (currentToken == "PLUS" || currentToken == "MINU") {
            outputLexRes();  // 正负号
        }
        unsignedInteger();
        out << "<整数>" << endl;
    }

    // ＜无符号整数＞
    void unsignedInteger() {
        outputLexRes();  // 输出INTCON
        out << "<无符号整数>" << endl;
    }

    // ＜缺省＞ ::= default :＜语句＞
    void defaultRes() {
        outputLexRes();  // 输出DEFAULTTK
        outputLexRes();  // 输出COLON
        statement();     // 语句
        out << "<缺省>" << endl;
    }

    // 语法分析入口
    void execute() {
        nextToken();  // 获取第一个Token
        while (!currentToken.empty()) {
            if (currentToken == "CONSTTK") {
                constDescribe();  // 处理常量说明
            } else if ((currentToken == "INTTK" || currentToken == "CHARTK" || currentToken == "VOIDTK") && 
                      nextToken() && (currentToken == "IDENFR" || currentToken == "MAINTK") && 
                      nextToken() && currentToken == "LPARENT") {
                prevToken();  // 回退一个Token（因为nextToken()超前移动了）
                func();       // 处理函数定义
            } else if (varIDENFR(currentToken)) {
                varDecribe();  // 处理变量说明
            } else {
                nextToken();  // 跳过无效Token（错误处理可增强）
            }
        }
        out << "<程序>" << endl;
    }

private:
    // 回退一个Token（临时用，实际词法分析可优化为支持回退）
    void prevToken() {
        // 简化处理：实际实现需保存Token历史，这里假设调用前已获取有效Token
        in.seekg(-currentValue.size(), ios::cur);  // 仅粗略回退（实际需更精确）
        lineNum--;  // 简化处理
    }
};

int main() {
    // 设置控制台编码（Windows）
    system("chcp 65001 > nul");

    // 初始化词法分析
    initTokenMap();
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

    // 执行语法分析
    Parser parser;
    parser.execute();

    // 关闭文件
    in.close();
    out.close();
    cout << "分析完成，结果已写入output.txt" << endl;
    return 0;
}