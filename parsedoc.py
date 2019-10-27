# parsedoc
# short Python program to generate an html file out of a commented cc65 assembler program
#
# Usage: python parsedoc.py filetoparse > outfile.html
#
# Version: 0.1 
# Date: 2019-10-26
# Author: Wil Elmenreich (wilfried at gmx dot at)
# License: The Unlicense (public domain)

def printformatmacroname(m):
   cmds=m.split(";")
   sep=''
   for cmd in cmds:
      w=cmd.lstrip(' ').split(' ')
      if len(w)<=1:
          print( m, end='')
      else:
          for c in w:
              if c==w[0]:
                  print(sep+c, end='')
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
for l in lines[lc:]:
    if len(l)==0:
        lastline=l
        continue
    l2=l.lstrip(';').lstrip(' ')
    if l2[0:1]=="<":
        print(l2)
        continue
    if l[0:2]==";;":
        if lastline=='':
            print ()
            print ("<h3>", end = '')
            printformatmacroname(l2)
            print ("</h3>")
        else:
            print (l2+"<br>")
    lastline=l
print("</body>")
print("</html>")
