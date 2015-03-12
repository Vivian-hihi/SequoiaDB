#!/usr/bin/python
# remove macros in source code

import os
import sys

def isMacro(line):
    if line.startswith('#'):
        return True
    return False

def isMacroStart(line):
    if not isMacro(line):
        return False
    if line.find('endif') != -1:
        return False
    if line.find('if') == -1:
        return False
    return True
    
def isMacroEnd(line):
    if not isMacro(line):
        return False
    if line.find('endif') == -1:
        return False
    return True

def removeMacro(fileName, macro):
    if not os.path.exists(fileName):
        raise Exception("file not exists: " + fileName)
    file = open(fileName, 'r')
    isMacroStarted = False
    depth = 0 # macro nested depth
    for rawLine in file:
        line = rawLine.lstrip()
        if not isMacroStarted:
            if isMacroStart(line) and line.find(macro) != -1:
                isMacroStarted = True
                depth += 1
            else:
                sys.stdout.write(rawLine)
        else:
            if isMacroStart(line):
                depth += 1
            elif isMacroEnd(line):
                depth -= 1
                if depth == 0:
                    isMacroStarted = False
    file.close()

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print('<cmd> <file> <macro>')
        sys.exit(1)
    fileName = sys.argv[1].strip()
    macro = sys.argv[2].strip()
    if fileName == '':
        raise Exception("the fileName is empty")
    if macro == '':
        raise Exception("the macro is empty")
    removeMacro(fileName, macro)
