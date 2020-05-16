import sys

filename = sys.argv[1]

feature_set = []

# create back-up before doing this

with open(filename, 'r') as f:

    for line in f.readlines():
        if line.startswith("f ="):
            feature_set.append(int(line.strip().split()[2][:-1]))    

print(feature_set)

