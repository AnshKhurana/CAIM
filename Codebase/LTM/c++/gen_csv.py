import os, csv
import sys

# data_1 = [(1, 199.591), (2, 396.854), (3, 597.572), (4, 796.886), (5, 996.248), (6, 1193.4), (7, 1387.6), (8, 1576.86), (9, 1764.48), (10, 1949.66), (11, 2132.85), (12, 2313.83), (13, 2492.48), (14, 2669.94), (15, 2844.29), (16, 3016.33), (17, 3186.27), (18, 3354.3), (19, 3522.8), (20, 3686.63), (21, 3848.51), (22, 4008.38), (23, 4166.43), (24, 4322.39), (25, 4476.24), (26, 4627.97), (27, 4777.48), (28, 4924.98), (29, 5070.67), (30, 5214.21), (31, 5355.68), (32, 5495.47), (33, 5633.01), (34, 5768.47), (35, 5902.49), (36, 6033.99), (37, 6163.52), (38, 6291.18), (39, 6417.47), (40, 6541.88), (41, 6664.35), (42, 6784.8), (43, 6903.0), (44, 7019.18), (45, 7133.28), (46, 7245.34), (47, 7355.3), (48, 7463.36), (49, 7569.34), (50, 7673.58)]
# data_2 = [(1, 399.013), (2, 785.374), (3, 1171.85), (4, 1554.99), (5, 1935.71), (6, 2313.92), (7, 2687.26), (8, 3057.13), (9, 3423.82), (10, 3793.17), (11, 4161.27), (12, 4527.08), (13, 4887.84), (14, 5243.24), (15, 5596.12), (16, 5945.33), (17, 6292.64), (18, 6634.9), (19, 6971.5), (20, 7305.36), (21, 7633.08), (22, 7958.36), (23, 8277.66), (24, 8592.82), (25, 8901.51), (26, 9202.48), (27, 9499.52), (28, 9792.98), (29, 10082.3), (30, 10367.0), (31, 10648.5), (32, 10925.9), (33, 11200.9), (34, 11471.5), (35, 11738.5), (36, 12002.7), (37, 12261.8), (38, 12516.4), (39, 12767.2), (40, 13014.7), (41, 13256.8), (42, 13494.8), (43, 13729.2), (44, 13959.6), (45, 14186.7), (46, 14408.9), (47, 14627.2), (48, 14841.0), (49, 15051.2), (50, 15257.3)]


def get_spread_data(src):
    data = []
    with open(src, 'r') as f:
        for line in f.readlines():
            if line.startswith("f"):
                data.append(float(line.strip().split()[-1]))
    return data

data_1 = get_spread_data('spr_vs_f_cit_simpath_data_s20')
data_2 = get_spread_data('t_vs_f_cit_greedy_data')

print(data_1)
print(data_2)


with open(sys.argv[1], 'w', newline='') as f:
    writer = csv.writer(f)
    writer.writerow(['iter', 'spr_simpath', 'spr_greedy'])
    for i in range(50):
        writer.writerow([i+1, data_1[i], data_2[i]])

    