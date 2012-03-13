#!/usr/bin/python2

import sys
import os
from struct import unpack

content_hpp =  '\
/*THIS FILE IS AUTO GENERATE BY AppWebServer Resource Builder*/\n\
\n\
#ifndef CONTENT_HPP_\n \
#define CONTENT_HPP_\n\
#include <utility>\n\
#include <string>\n\
std::pair<unsigned char*, size_t> _RC(std::string path);\n\
#endif\n'

content_cpp = '\
/*THIS FILE IS AUTO GENERATE BY AppWebServer Resource Builder*/\n\
\
#ifndef AWS_DEBUG\n\
#include "content.hpp"\n\
#include <string>\n\
using namespace std;\n\
typedef unsigned char uchar;\n\
	std::pair<uchar*, size_t> _RC(std::string path){{\n\
    static uchar* _rc_empty = 0;\n\
    {0}\n\
    return make_pair(_rc_empty, -1);\n\
}}\n\
#endif'
content_static = 'static uchar {0}[] = {{\n {1} \n}};\n'
content_return_first = '\
if (path == "{0}") {{\n\
    return make_pair({1}, {2});\n\
}}'

content_return = '\
 else if (path == "{0}") {{\n\
   return make_pair({1}, {2});\n\
}}\n'

contents = ["", ""]

doc_root = sys.argv[1] or "doc_root/"
if len(sys.argv) == 2:
    doc_root = sys.argv[1]

def generate_one(name, path, template=content_return):
    f = open(path, 'r')
    data = f.read()
    f.close()
    if (len(data) == 0):
        print "Waring: {0} is zero length file!".format(path)
        c = template.format(path, "_rc_empty", 0)
        return ["",c]

    content = unpack("{0}c".format(len(data)), data)
    tmp = ""
    counter = 0
    for i in content:
        tmp += "0x{0:x}, ".format(ord(i))
        counter += 1
        if (counter % 10 == 0):
            tmp += "\n\t\t"
    s = content_static.format(name, tmp)
    c = template.format(path[1:], name, len(content))
    return [s,c]


old_path= os.path.abspath('.')
os.chdir(doc_root)

counter = 0;
for root, dirs, files in os.walk("."):
    for file in files:
        if (counter==0):
            c = generate_one("_rc_{0}".format(counter),os.path.join(root, file), content_return_first)
        else:
            c = generate_one("_rc_{0}".format(counter),os.path.join(root, file))
        contents[0] += c[0]
        contents[1] += c[1]
        counter += 1;

os.chdir(old_path)

f = open("content.cpp", "w")
f.write(content_cpp.format(contents[0]+contents[1]));
f.close()

f = open("content.hpp", "w")
f.write(content_hpp)
f.close()
