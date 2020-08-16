import argparse
import os

parser = argparse.ArgumentParser()

parser.add_argument('--dataset', type=str, default='cit')
parser.add_argument('--exp_mode', type=int, default=0)


exp_dict = {0:  'spread vs K', 1:'time vs K', 2:'spread vs eta', 3:'time vs eta'}

def gen_file(dataset, exp_mode, eta=0.01):
    pass

if __name__ == "__main__":
    args = parser.parse_args()

    exp_mode = args.exp_mode 
    dataset = args.dataset

    if exp_mode == 0: 
        