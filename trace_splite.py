#! /bin/python3
import os
import sys

global interrupt
interrupt = False

def main():
    global interrupt
    if len(sys.argv) < 2:
        print("Usage: %s <tracefile>" % sys.argv[0])
        sys.exit(1)
    tracefile = sys.argv[1]
    tracefile_simple = tracefile + ".simple"	
    if not os.path.exists(tracefile):
        print("Error: tracefile %s does not exist" % tracefile)
        sys.exit(1)
    with open(tracefile) as f:
        with open(tracefile_simple,'w+') as f_s:
            for line_number, line in enumerate(f, start=1):
                l = line.strip()
                if is_display_line(l):# and not("core   0: 0x80403d88" in l) and not("core   0: 0x80403d8c" in l):
                    write_line = "["+str(line_number)+"]"+l
                    # print(write_line)
                    f_s.write(write_line + '\n')

def is_display_line(line):
    if "core   1: " in line:
        return True
    else:
        return False      


def is_core(line,id):
    global interrupt
    if "core   "+str(id)+": " in line:
        return True
    else:
        return False      
    
if __name__ == "__main__":
    main()