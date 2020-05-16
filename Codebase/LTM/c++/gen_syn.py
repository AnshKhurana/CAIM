import networkx
import os
import random
import numpy as np
from networkx.generators.random_graphs import powerlaw_cluster_graph

for size in [256, 512, 1024, 2048, 4096, 8192, 16384]:

    print("For size: ", size)

    g = powerlaw_cluster_graph(size,  m=15, p=0.2)
    # if not os.path.exists(os.path.join('../datasets', 'syn_' + str(size))):
    #     os.makedirs(os.path.join('../datasets', 'syn_' + str(size)))

    current_dir = os.path.join('../datasets', 'syn_' + str(size))
    probs = [0.1, 0.01, 0.001]
    thresh = 0.2

    nodes = sorted(g.degree, key=lambda x: x[1], reverse=True)
    print(nodes[:4])
    with open(os.path.join(current_dir, 'seeds.txt'), 'w') as f:
        for node in nodes[:5]:
            f.write(str(node[0]) + '\n')


    with open(os.path.join(current_dir, 'mem.txt'), 'w') as f:
        for i in range(size):
            vec = np.random.uniform(0,1,100)
            my_features = np.where(vec < thresh)
            # print(my_features[0])
            list_f = [str(x) for x in my_features[0]]
            # print(list_f)
            f.write(str(i) + " " + " ".join(list_f) + '\n')

    # with open(os.path.join(current_dir, 'seeds.txt'), 'w') as f:        
    #     vec = np.random.choice(size, 128)
    #     for item in vec:
    #         f.write(str(item) + '\n')
        # f.write(str(i) + " " + " ".join(my_features))

    mems = dict()
    with open(os.path.join(current_dir, 'mem.txt'), 'r') as fin:
        for line in fin.readlines():
            vals = [int(x) for x in line.strip().split()]
            mems[vals[0]] = vals[1:]
    with open(os.path.join(current_dir, 'com.txt'), 'w') as fout:

        for com in range(100):
            fout.write(str(com))
            my_mems = []
            for mem in mems.keys():
                if com in mems[mem]:
                    my_mems.append(mem)
            if not len(my_mems) == 0:
                for mem in my_mems:
                    fout.write(" " + str(mem))
            fout.write('\n')
    
    # with open(os.path)

    with open(os.path.join(current_dir, 'base_weights.txt'), 'w') as bw:
        with open(os.path.join(current_dir, 'marg_weights.txt'), 'w') as mw:
            for u, v in g.edges:
                # edge_list.write(str(u) + " " + str(v) + '\n')
                bw.write(str(u) + " " + str(v) +  " " +str(random.choice(probs))  +'\n' )           
                mw.write(str(u) + " " + str(v) + " " + str(random.choice(probs))  +'\n' )           

