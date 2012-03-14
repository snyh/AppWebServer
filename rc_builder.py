#!/usr/bin/python2 -d3
import os
from optparse import OptionParser
from struct import unpack

content_cpp = '\
/*THIS FILE IS AUTO GENERATE BY AppWebServer Resource Builder*/\n\
\
#ifndef AWS_DEBUG\n\
#include <utility>\n\
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



def generate_content(doc_root):
	contents = ["", ""]
	old_path= os.path.abspath('.')
	os.chdir(doc_root)

	counter = 0;
	for root, dirs, files in os.walk("."):
		for file in files:
			if (counter==0):
				c = generate_one("_rc_{0}".format(counter),
						os.path.join(root, file), content_return_first)
			else:
				c = generate_one("_rc_{0}".format(counter),
						os.path.join(root, file))
			contents[0] += c[0]
			contents[1] += c[1]
			counter += 1;

	os.chdir(old_path)
	return contents

def write_content(contents, path):
	f = open(os.path.join(path, "content.cpp"), "w")
	f.write(content_cpp.format(contents[0]+contents[1]))
	f.close()


def main():
	usage = "usage: %prog [options]"
	parser = OptionParser(usage)
	parser.add_option("-o", "--output", dest="output", default="./",
			help="The directory sotrage the content.cpp file. (default:./)")
	parser.add_option("-s", "--source", dest="doc_root", default="doc_root",
			help="The directory to find content file. (default: ./doc_root)")
	(options, args) = parser.parse_args()
	contents = generate_content(options.doc_root)
	write_content(contents, options.output)

if __name__ == "__main__":
	main()
