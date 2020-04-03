# Parameters 
import os
import argparse
from learn_params import get_aux, learn_params
from helpers import *
import pprint

parser = argparse.ArgumentParser()


# binary threshold for binary
parser.add_argument("--topic_thr", default=0.034, type =float)

parser.add_argument('--get_binary', action="store_true", default=False, help="specify this option to refresh the binary features")
# input files

parser.add_argument('--data_dir', default='../data')

# output dir
if __name__ == "__main__":
    
    args = parser.parse_args()

    # input files
    # HARD CODE NAMES
    args.log_file = os.path.join(args.data_dir, 'post_list.csv')
    args.edge_list = os.path.join(args.data_dir, 'vk_graph_lt.csv')
    args.user_features_input = os.path.join(args.data_dir, 'user_preferences.csv')
    args.topic_features_input = os.path.join(args.data_dir, 'reduced_tf_idf.csv')

    
    if not args.get_binary:
        create_vecs(args)
    
    args.user_features = os.path.join(args.data_dir, 'user_features.npy')
    args.topic_features = os.path.join(args.data_dir, 'topic_features.npy')

    print(args)

    g, n, C1v, C2v, C1vu, C2vu = get_aux(args)

    # Actually writes the final values to the output file
    learn_params(args, g, n, C1v, C2v, C1vu, C2vu)

