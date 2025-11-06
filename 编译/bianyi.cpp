#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include <unordered_map>
#include <cstdlib>
#include <sstream>
#include <string>
#include <cctype>
using namespace std;

class SyntaxAnalyzer {
public:
    vector<string> sourceCode;
    vector<pair<string, string>> tokens;
    int currentPos;
    int currRow;
    int currCol;
    map<string, string> keyword = {
        {"const", "CONSTTK"}, {"int", "INTTK"}, {"char", "CHARTK"}, {"void", "VOIDTK"}, {"main", "MAINTK"}, 
        {"if", "IFTK"}, {"else", "ELSETK"}, {"switch", "SWITCHTK"}, {"case", "CASETK"}, {"default", "DEFAULTTK"},
        {"while", "WHILETK"}, {"for", "FORTK"}, {"scanf", "SCANFTK"}, {"printf", "PRINTFTK"}, {"return", "RETURNTK"}
    };

    fstream in;
    map<string, string> funcResType;
    string inputPath;
    string outputPath;
    
    bool isInt(char c) {
        return c <= '9' && c >= '0';
    }

    bool isLetter(char c) {
        return (c <= 'z' && c >= 'a') || (c <= 'Z' && c >= 'A') || (c == '_');
    }

    string toLower(string str) {
        string temp = str;
        for (int i = 0; i < str.size(); i++) {
            if (temp[i] <= 'Z' && temp[i] >= 'A') {
                temp[i] -= ('A' - 'a');
            }
        }
        return temp;
    }

    SyntaxAnalyzer() {
        inputPath = "testfile.txt";
        outputPath = "output.txt";
        currentPos = 0;
        currRow = 0;
        currCol = 0;
        
        in.open(inputPath, ios::in);
        if (in.is_open()) {
            while (in.peek() != EOF) {
                string input;
                getline(in, input);
                sourceCode.push_back(input);
            }
            in.close();
        }
    }

    void performLexicalAnalysis() {
        tokens.clear();
        unordered_map<string, string> tokenMap;
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

        stringstream ss;
        for (const string& line : sourceCode) {
            ss << line << '\n';
        }

        string content = ss.str();
        size_t pos = 0;
        int lineNum = 1;

        auto skipWhitespace = [&]() {
            while (true) {
                while (pos < content.size() && (content[pos] == ' ' || content[pos] == '\t' || content[pos] == '\r')) {
                    pos++;
                }
                if (pos >= content.size() || content[pos] != '\n') {
                    break;
                }
                lineNum++;
                pos++;
            }
        };

        while (pos < content.size()) {
            skipWhitespace();
            if (pos >= content.size()) break;

            char c = content[pos];
            string token, value;

            if (isalpha(c) || c == '_') {
                while (pos < content.size() && (isalnum(content[pos]) || content[pos] == '_')) {
                    value += content[pos++];
                }
                string lowerValue = toLower(value);
                token = tokenMap.count(lowerValue) ? tokenMap[lowerValue] : "IDENFR";
            }
            else if (isdigit(c)) {
                while (pos < content.size() && isdigit(content[pos])) {
                    value += content[pos++];
                }
                token = "INTCON";
            }
            else if (c == '\'') {
                pos++;
                if (pos < content.size()) {
                    value = content[pos++];
                    if (pos < content.size() && content[pos] == '\'') pos++;
                }
                token = "CHARCON";
            }
            else if (c == '"') {
                pos++;
                while (pos < content.size() && content[pos] != '"') {
                    value += content[pos++];
                }
                if (pos < content.size() && content[pos] == '"') pos++;
                token = "STRCON";
            }
            else {
                value += c;
                pos++;
                if ((c == '<' || c == '>' || c == '=' || c == '!') && pos < content.size() && content[pos] == '=') {
                    value += content[pos++];
                }
                token = tokenMap.count(value) ? tokenMap[value] : "UNKNOWN";
            }

            tokens.emplace_back(token, value);
        }
    }

    bool isTypeIdentifier(const string& str) {
        return (str == "INTTK" || str == "CHARTK");
    }

    void outputToken(ofstream& out) {
        if (currentPos < tokens.size()) {
            out << tokens[currentPos].first << " " << tokens[currentPos].second << endl;
            currentPos++;
        }
    }

    void parseConstantDeclaration(ofstream& out) {
        outputToken(out);
        parseConstantDefinition(out);
        outputToken(out);
        
        while (currentPos < tokens.size() && tokens[currentPos].first == "CONSTTK") {
            outputToken(out);
            parseConstantDefinition(out);
            outputToken(out);
        }
        out << "<常量说明>" << endl;
    }

    void parseConstantDefinition(ofstream& out) {
        string typeToken = tokens[currentPos].first;
        outputToken(out);
        outputToken(out);
        outputToken(out);
        
        if (typeToken == "INTTK") {
            parseInteger(out);
        } else {
            outputToken(out);
        }
        
        while (currentPos < tokens.size() && tokens[currentPos].first == "COMMA") {
            outputToken(out);
            outputToken(out);
            outputToken(out);
            if (typeToken == "INTTK") {
                parseInteger(out);
            } else {
                outputToken(out);
            }
        }
        out << "<常量定义>" << endl;
    }

    void parseVariableDeclaration(ofstream& out) {
        while (currentPos < tokens.size() && isTypeIdentifier(tokens[currentPos].first) && 
               (currentPos + 2 >= tokens.size() || tokens[currentPos + 2].first != "LPARENT")) {
            string temp;
            do {
                outputToken(out);
                outputToken(out);
                vector<int> dimensions;
                
                while (currentPos < tokens.size() && tokens[currentPos].first == "LBRACK") {
                    outputToken(out);
                    dimensions.push_back(atoi(tokens[currentPos].second.c_str()));
                    parseUnsignedInteger(out);
                    outputToken(out);
                }

                if (currentPos >= tokens.size() || tokens[currentPos].first != "ASSIGN") {
                    temp = "<变量定义无初始化>";
                }
                else {
                    outputToken(out);
                    if (dimensions.empty()) {
                        parseConstant(out);
                    }
                    else {
                        int totalElements = 1;
                        for (int dim : dimensions) {
                            totalElements *= dim;
                        }
                        while (totalElements > 0 && currentPos < tokens.size()) {
                            if (tokens[currentPos].first == "INTCON" || tokens[currentPos].first == "CHARCON") {
                                parseConstant(out);
                                totalElements--;
                            }
                            else {
                                outputToken(out);
                            }
                        }
                        for (size_t i = 0; i < dimensions.size() && currentPos < tokens.size(); i++) {
                            outputToken(out);
                        }
                    }
                    temp = "<变量定义及初始化>";
                }
            } while(currentPos < tokens.size() && tokens[currentPos].first == "COMMA");
            
            out << temp << endl;
            out << "<变量定义>" << endl;
            if (currentPos < tokens.size()) {
                outputToken(out);
            }
        }
        out << "<变量说明>" << endl;
    }

    void parseStatementList(ofstream& out) {
        while (currentPos < tokens.size() && tokens[currentPos].first != "RBRACE") {
            parseStatement(out);
        }
        out << "<语句列>" << endl;
    }

    void parseStatement(ofstream& out) {
        if (currentPos >= tokens.size()) {
            return;
        }
        
        string tokenType = tokens[currentPos].first;
        
        if (tokenType == "SEMICN") {
            outputToken(out);
        }
        else if (tokenType == "LBRACE") {
            outputToken(out);
            parseStatementList(out);
            if (currentPos < tokens.size()) {
                outputToken(out);
            }
        }
        else if (tokenType == "WHILETK") {
            outputToken(out);
            outputToken(out);
            parseCondition(out);
            outputToken(out);
            parseStatement(out);
            out << "<循环语句>" << endl;
        }
        else if (tokenType == "FORTK") {
            for (int i = 0; i < 4 && currentPos < tokens.size(); i++) {
                outputToken(out);
            }
            parseExpression(out);
            if (currentPos < tokens.size()) {
                outputToken(out);
            }
            parseCondition(out);
            for (int i = 0; i < 5 && currentPos < tokens.size(); i++) {
                outputToken(out);
            }
            parseStep(out);
            if (currentPos < tokens.size()) {
                outputToken(out);
            }
            parseStatement(out);
            out << "<循环语句>" << endl;
        }
        else if (tokenType == "IFTK") {
            outputToken(out);
            outputToken(out);
            parseCondition(out);
            outputToken(out);
            parseStatement(out);
            if (currentPos < tokens.size() && tokens[currentPos].first == "ELSETK") {
                outputToken(out);
                parseStatement(out);
            }
            out << "<条件语句>" << endl;
        }
        else if (funcResType.find(tokens[currentPos].second) != funcResType.end()) {
            string funcCallType = (funcResType[tokens[currentPos].second] == "<无返回值函数定义>") ? 
                                 "<无返回值函数调用语句>" : "<有返回值函数调用语句>";
            outputToken(out);
            outputToken(out);
            parseValueParameterTable(out);
            outputToken(out);
            out << funcCallType << endl;
            outputToken(out);
        }
        else if (tokenType == "SCANFTK") {
            for (int i = 0; i < 4 && currentPos < tokens.size(); i++) {
                outputToken(out);
            }
            out << "<读语句>" << endl;
            outputToken(out);
        }
        else if (tokenType == "PRINTFTK") {
            outputToken(out);
            outputToken(out);
            if (currentPos < tokens.size() && tokens[currentPos].first == "STRCON") {
                outputToken(out);
                out << "<字符串>" << endl;
                if (currentPos < tokens.size() && tokens[currentPos].first == "COMMA") {
                    outputToken(out);
                    parseExpression(out);
                }
            }
            else {
                parseExpression(out);
            }
            outputToken(out);
            out << "<写语句>" << endl;
            outputToken(out);
        }
        else if (tokens[currentPos].second == "switch") {
            outputToken(out);
            outputToken(out);
            parseExpression(out);
            outputToken(out);
            outputToken(out);
            parseSituationTable(out);
            parseDefaultStatement(out);
            outputToken(out);
            out << "<情况语句>" << endl;
        }
        else if (tokenType == "RETURNTK") {
            outputToken(out);
            if (currentPos < tokens.size() && tokens[currentPos].first == "LPARENT") {
                outputToken(out);
                parseExpression(out);
                outputToken(out);
            }
            out << "<返回语句>" << endl;
            outputToken(out);
        }
        else if (tokenType == "IDENFR") {
            outputToken(out);
            if (currentPos < tokens.size() && tokens[currentPos].first == "ASSIGN") {
                outputToken(out);
                parseExpression(out);
            } else if (currentPos < tokens.size()) {
                outputToken(out);
                parseExpression(out);
                outputToken(out);
                if (currentPos < tokens.size() && tokens[currentPos].first == "ASSIGN") {
                    outputToken(out);
                    parseExpression(out);
                }
                else if (currentPos < tokens.size() && tokens[currentPos].first == "LBRACK") {
                    outputToken(out);
                    parseExpression(out);
                    outputToken(out);
                    outputToken(out);
                    parseExpression(out);
                }
            }
            out << "<赋值语句>" << endl;
            outputToken(out);
        }
        out << "<语句>" << endl;
    }

    void parseExpression(ofstream& out) {
        if (currentPos < tokens.size() && 
            (tokens[currentPos].first == "PLUS" || tokens[currentPos].first == "MINU")) {
            outputToken(out);
        }
        parseTerm(out);
        while (currentPos < tokens.size() && 
               (tokens[currentPos].first == "PLUS" || tokens[currentPos].first == "MINU")) {
            outputToken(out);
            parseTerm(out);
        }
        out << "<表达式>" << endl;
    }

    void parseTerm(ofstream& out) {
        parseFactor(out);
        while (currentPos < tokens.size() && 
               (tokens[currentPos].first == "MULT" || tokens[currentPos].first == "DIV")) {
            outputToken(out);
            parseFactor(out);
        }
        out << "<项>" << endl;
    }

    void parseFactor(ofstream& out) {
        if (currentPos >= tokens.size()) {
            return;
        }
        
        if (funcResType.find(tokens[currentPos].second) != funcResType.end()) {
            outputToken(out);
            outputToken(out);
            if (currentPos < tokens.size() && tokens[currentPos].first != "RPARENT") {
                parseValueParameterTable(out);
            } else {
                out << "<值参数表>" << endl;
            }
            outputToken(out);
            out << "<有返回值函数调用语句>" << endl;
        }
        else if (tokens[currentPos].first == "CHARCON") {
            outputToken(out);
        }
        else if (tokens[currentPos].first == "INTCON") {
            outputToken(out);
            out << "<无符号整数>" << endl;
            out << "<整数>" << endl;
        }
        else if (currentPos + 1 < tokens.size() && 
                 (tokens[currentPos].first == "PLUS" || tokens[currentPos].first == "MINU") && 
                 tokens[currentPos + 1].first == "INTCON") {
            outputToken(out);
            outputToken(out);
            out << "<无符号整数>" << endl;
            out << "<整数>" << endl;
        }
        else if (tokens[currentPos].first == "LPARENT") {
            outputToken(out);
            parseExpression(out);
            outputToken(out);
        }
        else {
            outputToken(out);
            if (currentPos < tokens.size() && tokens[currentPos].first == "LBRACK") {
                outputToken(out);
                parseExpression(out);
                outputToken(out);
                if (currentPos < tokens.size() && tokens[currentPos].first == "LBRACK") {
                    outputToken(out);
                    parseExpression(out);
                    outputToken(out);
                }
            }
        }
        out << "<因子>" << endl;
    }

    void parseValueParameterTable(ofstream& out) {
        if (currentPos < tokens.size() && tokens[currentPos].first == "RPARENT") {
            out << "<值参数表>" << endl;
            return;
        }
        parseExpression(out);
        while (currentPos < tokens.size() && tokens[currentPos].first == "COMMA") {
            outputToken(out);
            parseExpression(out);
        }
        out << "<值参数表>" << endl;
    }

    void parseCondition(ofstream& out) {
        parseExpression(out);
        if (currentPos < tokens.size()) {
            outputToken(out);
        }
        parseExpression(out);
        out << "<条件>" << endl;
    }

    void parseFunction(ofstream& out) {
        string funcType;
        if (currentPos + 1 < tokens.size() && tokens[currentPos + 1].first == "MAINTK") {
            funcType = "<主函数>";
        }
        else if (tokens[currentPos].first == "VOIDTK") {
            funcType = "<无返回值函数定义>";
        }
        else {
            funcType = "<有返回值函数定义>";
        }
        outputToken(out);
        
        if (currentPos < tokens.size()) {
            funcResType[tokens[currentPos].second] = funcType;
            outputToken(out);
        }
        
        if (currentPos + 1 < tokens.size() && tokens[currentPos + 1].first == "RPARENT") {
            if (funcType == "<有返回值函数定义>") {
                out << "<声明头部>" << endl;
            }
            outputToken(out);
            if (funcType != "<主函数>") {
                out << "<参数表>" << endl;
            }
        }
        else {
            if (funcType == "<有返回值函数定义>") {
                out << "<声明头部>" << endl;
            }
            outputToken(out);
            outputToken(out);
            outputToken(out);
            while (currentPos < tokens.size() && tokens[currentPos].first == "COMMA") {
                outputToken(out);
                outputToken(out);
                outputToken(out);
            }
            if (funcType != "<主函数>") {
                out << "<参数表>" << endl;
            }
        }
        
        outputToken(out);
        outputToken(out);
        
        if (currentPos < tokens.size() && tokens[currentPos].first == "CONSTTK") {
            parseConstantDeclaration(out);
        }
        if (currentPos < tokens.size() && isTypeIdentifier(tokens[currentPos].first) && 
            (currentPos + 2 >= tokens.size() || tokens[currentPos + 2].first != "LPARENT")) {
            parseVariableDeclaration(out);
        }
        parseStatementList(out);
        out << "<复合语句>" << endl;
        outputToken(out);
        out << funcType << endl;
        currentPos--;
    }

    void parseStep(ofstream& out) {
        parseUnsignedInteger(out);
        out << "<步长>" << endl;
    }

    void parseSituationTable(ofstream& out) {
        parseCaseStatement(out);
        while (currentPos < tokens.size() && tokens[currentPos].first == "CASETK") {
            parseCaseStatement(out);
        }
        out << "<情况表>" << endl;
    }

    void parseCaseStatement(ofstream& out) {
        outputToken(out);
        parseConstant(out);
        outputToken(out);
        parseStatement(out);
        out << "<情况子语句>" << endl;
    }

    void parseConstant(ofstream& out) {
        if (currentPos >= tokens.size()) {
            return;
        }
        
        if (tokens[currentPos].first == "INTCON" ||
            (currentPos + 1 < tokens.size() && tokens[currentPos + 1].first == "INTCON" && 
             (tokens[currentPos].first == "PLUS" || tokens[currentPos].first == "MINU"))) {
            parseInteger(out);
        }
        else if (tokens[currentPos].first == "CHARCON") {
            outputToken(out);
        }
        out << "<常量>" << endl;
    }

    void parseInteger(ofstream& out) {
        if (currentPos < tokens.size() && 
            (tokens[currentPos].first == "PLUS" || tokens[currentPos].first == "MINU")) {
            outputToken(out);
        }
        parseUnsignedInteger(out);
        out << "<整数>" << endl;
    }

    void parseUnsignedInteger(ofstream& out) {
        outputToken(out);
        while (currentPos < tokens.size() && tokens[currentPos].first == "INTTK") {
            outputToken(out);
        }
        out << "<无符号整数>" << endl;
    }

    void parseDefaultStatement(ofstream& out) {
        if (currentPos < tokens.size() && tokens[currentPos].first == "DEFAULTTK") {
            outputToken(out);
            outputToken(out);
            parseStatement(out);
            out << "<缺省>" << endl;
        }
    }

    void analyze() {
        performLexicalAnalysis();
        ofstream out(outputPath);
        
        if (!out.is_open()) {
            return;
        }
        
        for (currentPos = 0; currentPos < tokens.size(); currentPos++) {
            if (tokens[currentPos].first == "CONSTTK") {
                parseConstantDeclaration(out);
                currentPos--;
            }
            else if (currentPos + 5 < tokens.size() && 
                     (tokens[currentPos].first == "CHARTK" || tokens[currentPos].first == "INTTK" || tokens[currentPos].first == "VOIDTK") &&
                     (tokens[currentPos+1].first == "IDENFR" || tokens[currentPos+1].first == "MAINTK") &&
                     tokens[currentPos+2].first == "LPARENT") {
                parseFunction(out);
            }
            else if (isTypeIdentifier(tokens[currentPos].first) && 
                     (currentPos + 2 >= tokens.size() || tokens[currentPos + 2].first != "LPARENT")) {
                parseVariableDeclaration(out);
                currentPos--;
            }
        }
        out << "<程序>" << endl;
        out.close();
    }
};

int main() {
    SyntaxAnalyzer analyzer;
    analyzer.analyze();
    return 0;
}
