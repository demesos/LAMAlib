# parsedoc
# short Python program to generate an HTML file out of a commented cc65 assembler program
#
# Usage: python parsedoc.py filetoparse1 [filetoparse2 ...] > outfile.html
#
# Version: 0.3
# Date: 2024-11-24
# Author: Wil Elmenreich (wilfried at gmx dot at)
# License: The Unlicense (public domain)

from __future__ import print_function
import sys

def printformatmacroname(m):
    cmds = m.split(";")
    print("<b>", end='')
    sep = ''
    for cmd in cmds:
        w = cmd.lstrip(' ').split(' ')
        if len(w) <= 1:
            print(m + "</b>", end='')
        else:
            for c in w:
                if c == w[0]:
                    print(sep + c + "</b>", end='')
                else:
                    print(sep + "<i>" + c + "</i>", end='')
                sep = ' '
        sep = ' ; '

def process_file(filename):
    lines = []
    for l in open(filename):
        if l.split(" ")[0] == ".include":
            include_file = l.split(" ")[1].strip().strip('"')
            for l1 in open(include_file):
                lines.append(l1.strip())
        else:
            lines.append(l.strip())
    return lines

if len(sys.argv) < 2:
    print("Usage: python parsedoc.py filetoparse1 [filetoparse2 ...] > outfile.html", file=sys.stderr)
    sys.exit(1)

all_lines = []
for filename in sys.argv[1:]:
    all_lines.extend(process_file(filename))

macros = []
lastline = ''
for l in all_lines:
    if l.startswith(";;") and lastline == '':
        macros.append(l.lstrip(';').lstrip(' '))
    lastline = l

print("<html>")
print("<body>")

lc = 0
while all_lines[lc][0] == ';':
    print(all_lines[lc].lstrip(';').lstrip(' '), end='')
    if all_lines[lc][-1] == '>':
        print()
    else:
        print("<br>")
    lc += 1
    if len(all_lines[lc]) == 0:
        break

macros.sort()

lastline = ''
macronameprintmode = False
br = "<br>"  # linebreaks will be printed unless preformatted text was enabled with <pre>
for l in all_lines[lc:]:
    if len(l) == 0:
        lastline = l
        continue
    l2 = l.lstrip(';').lstrip(' ')
    if l2.startswith("<h"):
        print(l2)
        lastline = l
        continue
    if l.startswith(";;"):
        if "<pre>" in l:
            br = ""
        if "</pre>" in l:
            br = "<br>"
        if lastline == '':
            print("<br>")
            printformatmacroname(l2)
            print("<br>")
            macronameprintmode = True
        elif macronameprintmode and "#" in l and len(l) < 15:
            printformatmacroname(l2)
            print("<br>")
            macronameprintmode = False
        else:
            print(l2 + br)
            macronameprintmode = False
    lastline = l

print("</body>")
print("</html>")
