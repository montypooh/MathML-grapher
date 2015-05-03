#ifndef _MML_H_
#define _MML_H_

#include <string>
#include <vector>

// mathML tags
enum mathMLTag {
    UNDEF,
    SEMANTICS,
    MTABLE,
    MTR,
    MTD,
    MROW,
    MI,
    MO,
    MN,
    MFRAC,
    GEN
};

// OP, FX, term codes
enum Code {
    FUN,
    EQU,
    MUL,
    DIV,
    ADD,
    SUB,
    PST,
    PEN,
    SIN,
    COS,
    NUMBER,
    FUNCTION,
    VARIABLE,
    OPERATION
};

/* data structures for MathML parsing */
class element {
 public:
    int mathMLTag;
    int op;
    double numf;
    std::string id;
    int fx;
    double numx;
    std::vector<element*> children;
    element();
    element(const int& mathMLTag);
};

class term {
 public:
    Code type;
    std::string str;
};

class func {
 public:
    std::vector<term> terms;
    std::string name;
    int offset;
    func(const std::vector<term>& terms,
         const std::string& name,
         const int& offset);
};

/* openGL draw function */
#define WINDOW_WIDTH 683
#define WINDOW_HEIGHT 384
#define FUNCTION_SCALE 1.5
void display();

/* parser and grapher which calls display() */
class mml {
    bool read;
    std::string file;
    bool clear;
    bool draw;
 public:
    mml(int argc, char* argv[]);
    void parseCmd(int argc, char* argv[]);
    void parseFunctionFile(const std::string& filename);
    void readSource(const std::string& filename, std::string& dst);
    void ridEscapeSequence(const std::string& src, std::string& dst);
    void splitToModules(const std::string& src, std::vector<std::string>& dst);
    void determineTagType(const std::string&, mathMLTag& dst);
    void generateParseTree(const std::vector<std::string>& src, element** root);
    void assembleFunction(const element* node, std::vector<term>& dst);
    void generateFunctionFile(const std::vector<func>& funcs);
    void clearFunctionFile();
    void drawFunction(int argc, char* argv[]);
};

#endif
