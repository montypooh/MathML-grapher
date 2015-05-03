#include "mml.h"

#include <string>
#include <fstream>
#include <cassert>
#include <vector>
#include <iostream>
#include <stack>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <fstream>
#include <cmath>

#include <GL/freeglut.h>

#include "function.h"

/* statement for gdb printing */
template class std::vector<element*>;

element::element(const int& mathMLTag):
    mathMLTag(mathMLTag) {}

func::func(const std::vector<term>& terms, const std::string& name, const int& offset):
    terms(terms), name(name), offset(offset) {}

/* openGL draw function */
void display() {
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    glColor3d(0,0,0);
    glBegin(GL_LINE_LOOP);
    for(double t=0; t<=2*M_PI; t+=0.01) {
        glVertex2d(fx(t)/(FUNCTION_SCALE*WINDOW_WIDTH),
                   fy(t)/(FUNCTION_SCALE*WINDOW_HEIGHT));
    }
    glEnd();
    glFlush();
}

/* parser and grapher which calls display() */
mml::mml(int argc, char* argv[]) {
    if (argc<2) {
        std::cerr<<"Insufficient input parameters"<<std::endl;
        exit(EXIT_FAILURE);
    }
    read = false;
    clear = false;
    draw = false;
    parseCmd(argc, argv);

    /* only one command */
    assert(read^clear^draw);

    if (read) parseFunctionFile(file);
    else if (clear) clearFunctionFile();
    else if (draw) drawFunction(argc, argv);
}

void mml::parseCmd(int argc, char* argv[]) {
    for(int i=1; i<argc; ++i) {
        if (std::string(argv[i])=="-r") {
            read = true;
            file = argv[++i];
        } else if (std::string(argv[i])=="-c") {
            clear = true;
        } else if (std::string(argv[i])=="-d") {
            draw = true;
        } else {
            std::cerr<<"Invalid input"<<std::endl;
        }
    }
}

void mml::parseFunctionFile(const std::string& filename) {
    // do all the jobs required
    std::string source;
    readSource(filename, source);
    std::string ridded;
    ridEscapeSequence(source, ridded);
    std::vector<std::string> split;
    splitToModules(ridded, split);
    element* root;
    generateParseTree(split, &root);
    std::vector<term> fx, fy;
    assembleFunction(root->children[0]->children[0], fx);
    assembleFunction(root->children[0]->children[1], fy);
    func x(fx, "fx", 6); // in test.mml function starts from 6
    func y(fy, "fy", 6);
    std::vector<func> funcs;
    funcs.push_back(x);
    funcs.push_back(y);
    generateFunctionFile(funcs);
}

void mml::readSource(const std::string& filename, std::string& dst) {
    // extract full mml source from mml file to dst
    std::ifstream file(filename);
    if (file.is_open()) {
        std::string line;
        while(getline(file, line)) {
            dst.append(line);
        }
        file.close();
    } else {
        std::cout<<"Unable to open file"<<std::endl;
    }
}

void mml::ridEscapeSequence(const std::string& src, std::string& dst) {
    // get rid of escape sequences and unnecessary spaces in mml source
    assert(&src != &dst);
    for(int i=0; i<(int)src.size()-1; i++) {
        if (src[i]!='\r' && src[i]!=' ') {
            dst+=src[i];
        }
    }
}

void mml::splitToModules(const std::string& src, std::vector<std::string>& dst) { 
    // split mml source into vectors of mml grammar
    std::string buf;
    for(int i=0; i<(int)src.size(); i++) {
        if (src[i]=='<') {
            if (buf.size()!=0) {
                dst.push_back(buf);
            }
            buf="";
            buf+=src[i];
        } else if (src[i]=='>') {
            buf+=src[i];
            dst.push_back(buf);
            buf="";
        } else {
            buf+=src[i];
        }
    }
}

void mml::determineTagType(const std::string& tag, mathMLTag& dst) { 
    // helper function for assigning tab type
    if (tag=="<semantics>" || tag=="</semantics>") dst = SEMANTICS;
    else if (tag=="<mtable>" || tag=="</mtable>") dst = MTABLE;
    else if (tag=="<mtr>" || tag=="</mtr>") dst = MTR;
    else if (tag=="<mtd>" || tag=="</mtd>") dst = MTD;
    else if (tag=="<mrow>" || tag=="</mrow>") dst = MROW;
    else if (tag=="<mi>" || tag=="</mi>") dst = MI;
    else if (tag=="<mo>" || tag=="</mo>") dst = MO;
    else if (tag=="<mn>" || tag=="</mn>") dst = MN;
    else if (tag=="<mfrac>" || tag=="</mfrac>") dst = MFRAC;
}

void mml::generateParseTree(const std::vector<std::string>& src, element** root) { 
    // organize mml elements hierarchy using parse tree
    std::stack<element*> elements;
    element* parent = NULL;
    mathMLTag tag = UNDEF;

    for(int i=0; i<(int)src.size(); i++) {
        std::string curr = src[i];
        if (curr[0]=='<') { // is a mathML tag
            if (curr[1]!='/') { // is a start tag
                determineTagType(curr, tag);
                if (tag == UNDEF) {
                    // skip until parent SEMANTICS
                } else if (tag == SEMANTICS) {
                    *root = new element(tag);
                    parent = *root;
                    elements.push(parent);
                } else {
                    element* child = new element(tag);
                    parent->children.push_back(child);
                    elements.push(child);
                    parent = child;
                }
            } else { // is an end tag
                determineTagType(curr, tag);
                assert(parent->mathMLTag==tag);
                elements.pop();
                if (tag!=SEMANTICS) parent = elements.top();
            }
        } else { // is a mathematical value
            if (parent->mathMLTag==MI) {
                if (curr=="sin") parent->fx = SIN;
                else if (curr=="cos") parent->fx = COS;
                else parent->id = curr;
            } else if (parent->mathMLTag==MO) {
                if (curr=="&#8289;") parent->op = FUN;
                else if (curr=="&#8290;") parent->op = MUL;
                else if (curr=="&#63449;") parent->op = EQU;
                else if (curr=="+") parent->op = ADD;
                else if (curr=="-") parent->op = SUB;
                else if (curr=="(") parent->op = PST;
                else if (curr==")") parent->op = PEN;
            } else if (parent->mathMLTag==MN) {
                parent->numf = atof(curr.c_str());
            }
        }
    }
}

void mml::assembleFunction(const element* node, std::vector<term>& dst) { 
    // use depth-first search to simplify mml hierarchy from
    // children to parent into C++ mathematical equation syntax
    std::vector<element*> children = node->children;
    int szChildren = (int)children.size();

    term t;
    for(int i=0; i<szChildren; i++) {
        if (node->mathMLTag==MFRAC) {
            if (i==0) {
                t.type = OPERATION;
                t.str = "(";
                dst.push_back(t);
            } else if (i==1) {
                t.type = OPERATION;
                t.str = "/";
                dst.push_back(t);
            }
        }
        element* e = children[i];
        if (e->mathMLTag==MI) {
            if (e->fx==SIN) {
                t.type = FUNCTION;
                t.str = "sin";
                dst.push_back(t);
            } else if (e->fx==COS) {
                t.type = FUNCTION;
                t.str = "cos";
                dst.push_back(t);
            } else {
                t.type = VARIABLE;
                t.str = e->id;
                dst.push_back(t);
            }
        } else if (e->mathMLTag==MN) {
            t.type = NUMBER;
            std::stringstream ss;
            ss<<e->numf;
            t.str = ss.str();
            dst.push_back(t);
        } else if (e->mathMLTag==MO) {
            t.type = OPERATION;
            if (e->op==FUN) t.str = "";
            else if (e->op==MUL) t.str = "*";
            else if (e->op==EQU) t.str = "=";
            else if (e->op==ADD) t.str = "+";
            else if (e->op==DIV) t.str = "/";
            else if (e->op==SUB) t.str = "-";
            else if (e->op==PST) t.str = "(";
            else if (e->op==PEN) t.str = ")";
            dst.push_back(t);
        } else {
            assembleFunction(e, dst);
        }
    }
    if (node->mathMLTag==MFRAC) {
        t.type = OPERATION;
        t.str = ")";
        dst.push_back(t);
    }

    for(int i=0; i<szChildren; i++) {
        delete children[i];
    }
    children.clear();
}

void mml::generateFunctionFile(const std::vector<func>& funcs) { 
    // write generated C++ mathematical equation to header function.h
    std::ofstream file;
    file.open("function.h", std::ios::trunc | std::ios::out);
    if (file.is_open()) {
        file<<"#ifndef _FUNCTION_H_"<<std::endl;
        file<<"#define _FUNCTION_H_"<<std::endl;
        file<<std::endl;
        file<<"#include <cmath>"<<std::endl;
        for(int i=0; i<(int)funcs.size(); i++) {
            func f = funcs[i];
            file<<std::endl;
            file<<"double "<<funcs[i].name<<"(double t) {"<<std::endl;
            file<<"return ";
            for(int j=f.offset; j<(int)f.terms.size(); j++) {
                file<<f.terms[j].str;
            }
            file<<";"<<std::endl;
            file<<"}"<<std::endl;
        }
        file<<"#endif"<<std::endl;
    }
    file.close();
}

void mml::clearFunctionFile() {
    // clear function record
    std::ofstream file;
    file.open("function.h", std::ios::trunc | std::ios::out);
    if (file.is_open()) {
        file<<"#ifndef _FUNCTION_H_"<<std::endl;
        file<<"#define _FUNCTION_H_"<<std::endl;
        file<<std::endl;
        file<<"#include <cmath>"<<std::endl;
        file<<std::endl;
        file<<"double fx(double t) {"<<std::endl;
        file<<"return 0.0";
        file<<";"<<std::endl;
        file<<"}"<<std::endl;
        file<<std::endl;
        file<<"double fy(double t) {"<<std::endl;
        file<<"return 0.0";
        file<<";"<<std::endl;
        file<<"}"<<std::endl;
        file<<"#endif"<<std::endl;
    }
    file.close();
}

void mml::drawFunction(int argc, char* argv[]) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("Grapher");
    glutDisplayFunc(display);
    glutMainLoop();
}
