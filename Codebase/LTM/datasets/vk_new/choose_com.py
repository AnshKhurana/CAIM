import numpy as np

seed = 5 

with open('com.txt', 'r') as f:
    lines = f.readlines()
    newlines = np.random.RandomState(seed).choice(lines, 200, replace=False)
    with open('new_com.txt', 'w') as fout:
        for line in newlines:
            fout.write(line)
