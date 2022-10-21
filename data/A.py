import random
import os

def main():

        with open('INSERT_1_100', 'w') as f:
            for i in range(100):
                f.write('INSERT '+str(random.randint(1, 100))+'\n')

if __name__ == '__main__':
    main()


# generate a permutation of numbers between 1 and 100
# Path: data/B.py
import random
import os

def main():
    
            with open('INSERT_1_100', 'w') as f:
                for i in range(100):
                    f.write('INSERT '+str(i+1)+'\n')

            with open('INSERT_1_100', 'r') as f:
                lines = f.readlines()
                random.shuffle(lines)

            with open('INSERT_1_100', 'w') as f:
                for line in lines:
                    f.write(line)
            

if __name__ == '__main__':
    main()

