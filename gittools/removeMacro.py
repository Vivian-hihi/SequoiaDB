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
    if line.find('elif') != -1:
        return False
    if line.find('if') == -1:
        return False
    return True

def isMacroElseIf(line):
    if not isMacro(line):
        return False
    if line.find('elif') == -1:
        return False
    return True

def isMacroElse(line):
    if not isMacro(line):
        return False
    if line.find('else') == -1:
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
    inMacroElse = False
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
            elif isMacroElse(line):
                if depth == 1:
                    inMacroElse = True 
            elif isMacroElseIf(line):
                raise Exception("do not support macro '#elif' inside macro " 
                    + macro + " in file: " + fileName)
                pass
            elif isMacroEnd(line):
                depth -= 1
                if depth == 0:
                    isMacroStarted = False
                    inMacroElse = False

            if inMacroElse and not isMacroElse(line):
                sys.stdout.write(rawLine)
    file.close()

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print('Usage: ' + sys.argv[0].strip() + ' <file> <macro>')
        sys.exit(1)
    fileName = sys.argv[1].strip()
    macro = sys.argv[2].strip()
    if fileName == '':
        raise Exception("the fileName is empty")
    if macro == '':
        raise Exception("the macro is empty")
    removeMacro(fileName, macro)
