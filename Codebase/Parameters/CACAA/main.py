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


def checklog(logs, u, a):

    if (u, a) in logs[:, :2]:
        return True
    else:
        return False


def get_qual(args, topic_thresh, sim_bits, test_perc=0.4):


    # get new values on test set based on current thresholds
    if args.exp == 'vk':
        g, n, C1, C2, C3, C4 = get_aux_vk(args)
    
    learn_params(args, g, n, C1, C2, C3, C4)
    
    # read the learned parameters
    basefile =  open(os.path.join(args.data_fir, args.exp, 'base_weights.txt'), newline='')
    basefile = csv.reader(basefile, delimiter=' ')

    qfile = open(os.path.join(args.data_fir, args.exp, 'marg_weights.txt'), newline='')
    qfile = csv.reader(qfile, delimiter = ' ')

    topic_features = np.load(args.topic_features)
    user_features = np.load(args.user_features)

    b = dict() 
    q = dict()

    for u, v, val in basefile:
        b[int(u), int(v)] = float(val)

    for u, v, val in qfile:
        q[int(u), int(v)] = float(val)

    # calculate tp, fp, tn, fn
    tp = []
    fp = []
    tn = []
    fn = []


    logfile =  open(args.log_file, newline='')
    logs = csv.reader(logfile, delimiter=' ')
    num_logs = len(logs)

    for mu in np.linspace(0, 1, 100):    
        tpx = 0
        fpx = 0
        tnx = 0
        fnx = 0 
        for log in logs[:num_logs*test_perc]:
            
            [u, a, t_u] = [int(x) for x in log]
            # v published a message in the test log

            for v in g.succesors(u):

                prob = 0
                for u1 in g.predecessors(v):
                    prob += b[(u1, v)] + q[(u1, v)]*get_alpha(user_features[v], topic_features[a])
                
                prob = min(1, max(prob, 0))

                prediction = (prob > mu)
                
                gt = checklog(logs, v, a)
                if prediction == True:
                    if gt == True:
                        tpx+=1

                    else:
                        fpx+=1
                else:
                    if gt == True:
                        fnx+=1
                    else:
                        tnx+=1

            tp.append(tpx)
            fp.append(fpx)
            tn.append(tnx)
            fn.append(fnx)

    tp = np.array(tp)
    fp = np.array(fp)
    tn = np.array(tn)
    fn = np.array(fn)

    tpr = tp / (tp + fn)
    fpr = fp / (fp + tn)

    auc = np.trapz(tpr, fpr)

    return auc

def local_search(args, num_topics=1000, topic_thresh=0.034, sim_bits=100, delta_thres=0.01, delta_bits=1):

    
    current_val = get_qual(args, topic_thresh, sim_bits)
    
    # signs = [(-1,-1,-1), (-1,-1,0), (-1,-1,1), (-1,)]

    while True:
        success = False
        for i in [-1,0,1]:
            for j in [-1,0,1]:
                    new_val = get_qual(args, topic_thresh + i*delta_thres, sim_bits +j*delta_bits)

                    if new_val > current_val:
                        current_val =  new_val
                        topic_thresh += i*delta_thres
                        sim_bits += j*delta_bits
                        success = True
                        break
        if not success:
            break
    
    return topic_thresh, sim_bits
        

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
        g, n, C1, C2, C3, C4 = get_aux_vk(args)
    elif args.exp == 'citation':
        g, n, C1, C2, C3, C4 = get_aux_cit(args)
    else:
        raise ValueError("Dataset not available")

    # Actually writes the final values to the output file
    print_all(C1, C2, C3, C4)  
    learn_params(args, g, n, C1, C2, C3, C4)

