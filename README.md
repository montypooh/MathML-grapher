# MathML-grapher
graphs long parametric equation written in MathML using openGL

INSTRUCTIONS:

./grapher -r test.mml -> reads from test.mml and writes function to function.h header
./grapher -d          -> draws function written in function.h header using openGL
./grapher -c          -> clears function.h header

for effects to take place after reading or clearing function.h header,
grapher must be re-compiled using make clean && make

TESTED ENVIRONMENT:
Ubuntu Linux 14.04
OpenGL 3.0, freeglut
g++ version 4.8.2
