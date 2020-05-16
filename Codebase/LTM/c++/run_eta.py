import os
import time
eta_vec = [0.1, 1e-2, 1e-3, 1e-4, 1e-5, 1e-6, 1e-7]

cit_string_main = """../datasets/cluster_vk/edge_list.txt
../datasets/cluster_vk/base_weights.txt
../datasets/cluster_vk/marg_weights.txt
../datasets/cluster_vk/mem.txt
../datasets/cluster_vk/com.txt
../datasets/cluster_vk/seeds.txt
simpath
35
50
10000
"""
cit_string_mode = """2"""
    


for eta in eta_vec:

    cit_string  = cit_string_main + str(eta) + "\n" + cit_string_mode 
    print("running for eta:", eta)

    with open('newvk_simpath', 'w') as f:
        f.write(cit_string)
    
    os.system("./run.sh newvk_simpath >" + "fixed_eta_" + str(eta) + "_newvk_s20 &")
    time.sleep(60)

    