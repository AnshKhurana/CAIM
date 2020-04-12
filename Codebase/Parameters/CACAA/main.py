# Parameters 
import os
import argparse
from learn_params import get_aux_vk, get_aux_cit, learn_params
from helpers import *
import pprint

parser = argparse.ArgumentParser()


# binary threshold for binary
parser.add_argument("--topic_thr", default=0.034, type =float)

# experiment_name"
parser.add_argument("--exp", help="choose between vk and citation", default="vk", type=str)

# one month
parser.add_argument("--tau", default=2592000, type=int) 

parser.add_argument('--skip_binary', action="store_true", default=False, help="specify this option to refresh the binary features")
parser.add_argument('--nbits', default=250, type=int)

# input files

parser.add_argument('--data_dir', default='../data')

# output dir
if __name__ == "__main__":
    
    args = parser.parse_args()

    

    # input files
    # HARD CODE NAMES
    if args.exp == 'vk':
        args.log_file = os.path.join(args.data_dir, args.exp, 'post_list.csv')
    else:
        args.log_file = os.path.join(args.data_dir, args.exp, 'messages.csv')
    
    args.edge_list = os.path.join(args.data_dir, args.exp, 'edge_list.csv')
    args.user_features_input = os.path.join(args.data_dir, args.exp, 'user_preferences.csv')
    args.topic_features_input = os.path.join(args.data_dir, args.exp, 'topic_features.csv')

    
    if not args.skip_binary:
        create_vecs(args)
    
    args.user_features = os.path.join(args.data_dir, args.exp, 'user_features.npy')
    args.topic_features = os.path.join(args.data_dir, args.exp, 'topic_features.npy')

    print(args)

    if args.exp == 'vk':
        g, n, C1v, C2v, C1vu, C2vu = get_aux_vk(args)
    elif args.exp == 'citation':
        g, n, C1v, C2v, C1vu, C2vu = get_aux_cit(args)
    else:
        raise ValueError("Dataset not available")

    # Actually writes the final values to the output file
    
    learn_params(args, g, n, C1v, C2v, C1vu, C2vu)

