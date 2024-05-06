# This program generates C code for menu navigation from a human readable menu file
# input file need to use tabs for menu depth (not spaces)
# input files is assumed to have "back" element first at a new level
# input file can contain "|" and position for fixed adresses
# v0.8 2021-06-21 Johan von Konow
#
# example menu:
# settings
#   back
#	about|131
#	calibrate|132
#
# ==> generates:
# const char *menuTxt[] = {"settings", "back", "about", "calibrate"};
# uint8_t menuPre[] = {0, 1, 1, 2};
# uint8_t menuNxt[] = {0, 2, 3, 3};
# uint8_t menuSel[] = {2, 0, 131, 132};


filepath = 'menu.txt'   # <== file to parse
# open file and create lvl (menu depth / tabCount) txt and jmp array
id = 0
txt = []
jmp = []
lvl = []
define = []
for line in open(filepath, "r"):
    lvl.append(0)
    # avoid removing last char at eof
    if line[-1:] != "\n":
        line += ("\n")
    i = 0
    while line[i] == '\t':
        lvl[id] += 1
        i += 1
    # create txt array (name of the menu items)
    jmpPos = line.find('|')
    txt.append(line[i: jmpPos])
    # create jmp array (fixed jump for functions)
    if jmpPos > 0:
        jmp.append(int(line[jmpPos + 1: -1]))
        define.append(line[i: jmpPos].replace(" ","_") +" " + line[jmpPos + 1: -1])
    else:
        jmp.append(0)
    id += 1

# create id array (for print / debugging assistance)
ida = []
for x in range(len(lvl)):
    ida.append(x)

# create prev array
pre = []
for id in range(len(lvl)):
    i = 1
    pre.append(id)
    if(id - i >= 0):
        while lvl[id] < lvl[id-i]:
            i += 1
            if(id - i == 0):
                break
    if(id-i < len(lvl)):
        if (lvl[id] == lvl[id-i]):
            pre[id] = id-i

# create next array
nxt = []
for id in range(len(lvl)):
    i = 1
    nxt.append(id)
    if(id + i < len(lvl)):
        while lvl[id] < lvl[id+i]:
            i += 1
            if(id + i >= len(lvl)):
                break
    if(id+i < len(lvl)):
        if (lvl[id] == lvl[id+i]):
            nxt[id] = id+i

# create select array
sel = []
for id in range(len(lvl)):
    sel.append(id)
    # enter next lvl?(skips first element (back))
    if(id+2 < len(lvl)):
        if (lvl[id] + 1 == lvl[id+1] & lvl[id] + 1 == lvl[id+2]):
            sel[id] = id+2
    # back to previous lvl?
    if(id > 0):
        if (lvl[id] - 1 == lvl[id-1]):
            sel[id] = id-1
    # jump to fixed position?
    if jmp[id] > 0:
        sel[id] = jmp[id]

#print("row: " + str(ida))
#print("lev: " + str(lvl))
#print("jmp: " + str(jmp))
#print("jmp: " + str(txt))
#print("define: " + str(define))
#print()
print("const char *menuTxt[] = {\"" + '\", \"'.join(str(x) for x in txt) + "\"};")
print("uint8_t menuPre[] = {" + ', '.join(str(x) for x in pre) + "};")
print("uint8_t menuNxt[] = {" + ', '.join(str(x) for x in nxt) + "};")
print("uint8_t menuSel[] = {" + ', '.join(str(x) for x in sel) + "};")
print("#define mode_" + "\n#define mode_".join(str(x) for x in define))
