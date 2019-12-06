Explore-Update C++ Repository
===================

This folder includes C++ code for algorithms used for PIMUS problem. It uses features of *C++11* and *boost* library.

To run code, first specify input parameters in **setup.txt** in the following order:
- dataset file
- probabilities file 
- memberships file
- communities file
- seeds file
- the number of a group, which will be a seed set (in case, you want seeds to be one of the groups from communities file; keep it anyway, even you provide seeds file)
- number of features required, i.e. K
- number of Monte-Carlo simulations

For example, 

- ../datasets/vk/vk.txt
- ../datasets/vk/vk_wc.txt
- ../datasets/vk/vk_mem.txt
- ../datasets/vk/vk_com.txt
- ../datasets/vk/vk_seeds.txt
- 34662673
- 51
- 10000

The format of files explained in **datasets/** folder.

Second, run ```make``` in the console. This will generate output file **main.o**.

Third, run ```./main.o setup.txt```. This will run algorithms specified in the *main.cpp*. There are 4 algorithms: Greedy, Top-Edges, Top-Nodes, Explore-Update, separated by comments identifying them (comment any of them to avoid running an algorithm). After completion, find 2 output files per algorithm, one specifying found features and second specifying time (running time) and influence spread (each following line contain an influence spread, starting from 1 to K with the step K) . Specify output file names in *main.cpp*. For example, if there are all 4 algorithms uncommented, then running ```./main.o setup.txt``` will produce 8 files: *greedy_features.txt*, *greedy_results.txt*, etc. After any change of *main.cpp*, go to second step to `make` file. Visualize results using *visualise_data.py* file in **Python/** folder.

