# parsedoc
# short Python program to generate an html file out of a commented cc65 assembler program
#
# Usage: python parsedoc.py filetoparse > outfile.html
#
# Version: 0.2
# Date: 2020-09-16
# Author: Wil Elmenreich (wilfried at gmx dot at)
# License: The Unlicense (public domain)
from __future__ import print_function

def printformatmacroname(m):
   cmds=m.split(";")
   print( "<b>", end='')
   sep=''
   for cmd in cmds:
      w=cmd.lstrip(' ').split(' ')
      if len(w)<=1:
          print( m+"</b>", end='')
      else:
          for c in w:
              if c==w[0]:
                  print(sep+c+"</b>", end='')
              else:
                  print(sep+"<i>"+c+"</i>", end='')
              sep=' '
      sep=' ; '


lines=[]
for l in open("LAMAlib.inc"):
   if l.split(" ")[0] == ".include":
      for l1 in open(l.split(" ")[1].strip().strip('"')):
         lines.append(l1.strip())
   else:
      lines.append(l.strip())

macros=[]
lastline=''
for l in lines:
    if l[0:2]==";;" and lastline=='':
        macros.append(l.lstrip(';').lstrip(' '))
    lastline=l

print("<html>")
print("<body>")
lc=0
while lines[lc][0]==';':
    print(lines[lc].lstrip(';').lstrip(' '), end='')
    if lines[lc][-1]=='>':
        print()
    else:
        print("<br>")
    lc+=1
    if len(lines[lc])==0:
        break

macros.sort()
#for m in macros:
#    printformatmacroname(m)
#    print(", ", end = '')
#print("<br>")

lastline=''
macronameprintmode=False
br="<br>"    #linebreaks will be printed unless preformatted text was enabled with <pre>
for l in lines[lc:]:
    if len(l)==0:
        lastline=l
        continue
    l2=l.lstrip(';').lstrip(' ')
    if l2[0:2]=="<h":
        print(l2)
        lastline=l
        continue
    if l[0:2]==";;":
        if "<pre>" in l[0:2]:
            br=""
        if "</pre>" in l[0:2]:
            br="<br>"            
        if lastline=='':
            print ("<br>")
            printformatmacroname(l2)
            print ("<br>")
            macronameprintmode=True
        elif macronameprintmode==True and "#" in l and len(l)<15:
            printformatmacroname(l2)
            print ("<br>")
            macronameprintmode=False
        else:
            print (l2+br)
            macronameprintmode=False
    lastline=l
print("</body>")
print("</html>")
