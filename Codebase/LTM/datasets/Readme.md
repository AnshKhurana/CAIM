Datasets folder
===================

This folder contains two datasets, Gnutella and VK, probabilities, communities, memberships, and seeds (for VK). 

File *gnutella.txt* or *vk.txt* contains edge list of a corresponding network in the following format: each line has nodes u and v separated by space. For example:

- 0 1
- 0 2
- 0 3

File *gnutella_mv.txt*, *vk_mv.txt*, *gnutella_wc.txt*, *vk_wc.txt* contains edge list of a corresponding network and probability model (MV or WC) in the following format: each line has nodes u and v and probability p, separated by space. For example:

- 3418 5909 0.08
- 3809 6235 0.08
- 5619 2450 0.04

File *gnutella_com.txt*, *vk_com.txt*,  contains communities for a corresponding network in the following format: each line starts with a group id followed by node ids that belong to that group, all separated by spaces. For example:

- 0 0 286 400 498 573 677 690 764 950 970 1185 1361 1485 1844 2079 2123 2218 2279 2676 3018 3132 3599 4098 46

Here, first 0 indicates group id, followed by node id 0 and so on. Obtain such file using *prepare_date.py* in **Python/** folder. 

File *gnutella_mem.txt*, *vk_mem.txt*,  contains memberships for a corresponding network in the following format: each line starts with a node id followed by group ids it belongs to. For example: 

- 0 33913959 18537646 64980878 60140141 49849615 33737722 32277870 17975248

Here, first 0 indicates a node id, followed by a group id 33913959 (don't be confused that it does not contain a group id 0 from previous example, as these two examples are for different networks). 

File *vk_seeds.txt*,  contains seeds only for VK network in the following format: each line starts with a randomly chosen node id. For example:

- 418
- 564
- 569
- 570
- 581
- 632

In *main.cpp* specify (comment/uncomment) how to obtain seeds. To read seeds from the file, use `read_seeds` procedure to read desired number of seeds from the file (eg. VK dataset). To read seeds as a group provide in *setup.txt* of **C++/** folder, which group to use as seeds. 