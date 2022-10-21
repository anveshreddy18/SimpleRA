# fetch 1st column from file

import sys

def main():
    
    with open('block_accesses', 'r') as f:
        lines = f.readlines()
        counter=0
        for line in lines:
            line = line.split()[0]
            lines[counter]=line
            counter+=1

        with open('block_accesses', 'w') as f:
            for line in lines:
                f.write(line+',\n')

if __name__ == '__main__':
    main()