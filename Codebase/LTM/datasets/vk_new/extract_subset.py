# Code to extract an induced subgraph with N nodes 
import os

src = "./"
dst = "../small_greedy/"
N = 500
c=1./10

def gen_mem():
    with open(os.path.join(src, "vk_mem.txt"), 'r') as fin:
        with open(os.path.join(dst, "mem.txt"), 'w') as fout:
            line = fin.readline()
            while line:
                x = int(line.strip().split()[0]) 
                if x in range(N):
                    fout.write(line)
                line = fin.readline()

def gen_com():
    with open(os.path.join(src, "vk_com.txt")) as fin:
        with open(os.path.join(dst, "com.txt"), 'w') as fout:
            line = fin.readline()
            while line:
                x = line.strip().split()
                comm_num = x[0]
                new_mem = [a for a in x[1:] if int(a) in range(N)]
                if new_mem:
                    fout.write(comm_num + " " + " ".join(new_mem) + "\n")
                line = fin.readline()

def gen_edge_list():
    with open(os.path.join(src, "vk.txt")) as fin:
        with open(os.path.join(dst, "edge_list.txt"), 'w') as fout:
            line = fin.readline()
            while line:
                x = [int(a) for a in line.strip().split()[:2]]
                if (x[0]  in range(N) and (x[1] in range(N))):
                    fout.write(line)
                line = fin.readline()


def gen_wts(model = "wc"):
    with open(os.path.join(src, "vk_" + model + ".txt")) as fin:
        with open(os.path.join(dst, "edge_weights.txt"), 'w') as fout:
            line = fin.readline()
            while line:
                x = [int(a) for a in line.strip().split()[:2]]
                if (x[0]  in range(N) and (x[1] in range(N))):
                    fout.write(line)
                line = fin.readline()
def gen_seeds():
    with open(os.path.join(src, "vk_seeds.txt")) as fin:
        with open(os.path.join(dst, "seeds.txt"), 'w') as fout:
            line = fin.readline()
            while line:
                x = int(line.strip())
                if x in range(N):
                    fout.write(line)
                line = fin.readline() 

if __name__ == "__main__":
    gen_edge_list()
    gen_wts()
    gen_com()
    gen_mem()

