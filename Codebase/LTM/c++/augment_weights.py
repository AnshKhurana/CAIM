import sys, os
# delete and rename for correctness

c =20.0

def replace_weights(dataset_dir):
    in_degrees = dict()
    with open(os.path.join(dataset_dir, "degrees.txt")) as fin:
        for line in fin.readlines():
            x = line.strip().split()
            in_degrees[int(x[0])] = int(x[1])

    with open(os.path.join(dataset_dir, "edge_weights_old.txt")) as fin:
        with open(os.path.join(dataset_dir, "edge_weights_aug.txt"), 'w') as fout:
            line = fin.readline()
            while line:
                wt = float(line.strip().split()[2])
                v = int(line.strip().split()[1]) 
                aug_wt = c * wt
                array_text = line.strip().split()
                array_text[2] = str(aug_wt)
                fout.write(" ".join(array_text) + '\n')
                line  = fin.readline()
if __name__ == "__main__":

    dataset_dir = sys.argv[1]

    replace_weights(dataset_dir)