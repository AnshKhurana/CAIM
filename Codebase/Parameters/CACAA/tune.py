
import numpy as np
from tqdm import tqdm
import os
from learn_params import *
from helpers import create_vecs

def get_qual(args, topic_thresh, sim_bits, test_perc=0.4):


    # get new values on test set based on current thresholds
    args.topic_thr = topic_thresh
    create_vecs(args)
    if args.exp == 'vk':
        g, n, C1, C2, C3, C4 = get_aux_vk(args)
    
    learn_params(args, g, n, C1, C2, C3, C4)
    
    # read the learned parameters
    basefile =  open(os.path.join(args.data_dir, args.exp, 'base_weights.txt'), newline='')
    basefile = csv.reader(basefile, delimiter=' ')

    qfile = open(os.path.join(args.data_dir, args.exp, 'marg_weights.txt'), newline='')
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


    logfile = open(args.log_file, newline='')
    logs = list(csv.reader(logfile, delimiter=' '))
    num_logs = sum(1 for row in logs)
    templogs = logs[:int(num_logs*test_perc)]
    testlogs = []
    for log in templogs: 
        nplog = np.array(log, dtype=np.int)
        testlogs.append(nplog)
    
    testlogs = np.array(testlogs)

    print("Test log in consideration: ", testlogs.shape)
    # testlogs is now a numpy array

    # to estimate auc, taking 100 points on the curve
    for mu in tqdm(np.linspace(0, 1, 100)):    
        print(mu)
        tpx = 0
        fpx = 0
        tnx = 0
        fnx = 0 

        for log in reversed(testlogs):
            
            # [v, a, t_v] = [int(x) for x in log], already converted into an integer
            v, a_v, t_v = [x for x in log]
            # v published a message in the test log

            # set based on similarity
            action_set = [] 
            for u in g.predecessors(v):
                # list of permissible actions performed by u
                a_performed_by_u = get_actions_from_logs(testlogs, u, t_v, sim_bits)

                for au in a_performed_by_u:
                    similar = False
                    for a in action_set:
                        if check_sim(topic_features[au], topic_features[a], nbits=sim_bits):
                            similar = True
                            break
                        if not similar:
                            action_set.append(a)

            # now they are unique actions
            prob = 0.0
            for a in action_set:   
                for u in g.predecessors(v):
                    # get valid predecessors for the actions
                    if checklog(logs, u, a, t_v, sim_bits):
                        prob += b[(u, v)] + q[(u, v)]*get_alpha(user_features[v], topic_features[a])
                
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

    auc = -1 * np.trapz(tpr, fpr)
    print(auc)
    return auc

def local_search_2d(args, num_topics=1000, topic_thresh=0.034, sim_bits=100, delta_thres=0.01, delta_bits=1):

    iter=0
    print("iter: ", iter)    
    current_val = get_qual(args, topic_thresh, sim_bits)
    
    # signs = [(-1,-1,-1), (-1,-1,0), (-1,-1,1), (-1,)]

    while True:
        iter+=1
        print("iter: ", iter)    
        print("Current choice: ", topic_thresh, sim_bits)
        
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
    
    print("best parameters: ", topic_thresh, sim_bits)
    return topic_thresh, sim_bits


def local_search_1d(args, num_topics=100, topic_thresh=0.034, delta_thres=0.01):

    iter=0
    print("iter: ", iter)    
    current_val = 0
    
    # signs = [(-1,-1,-1), (-1,-1,0), (-1,-1,1), (-1,)]
    while True:
        iter+=1
        print("iter: ", iter)    
        print("Current choice: ", topic_thresh)
        
        success = False
        for i in [-1,1]:
            new_val = get_qual_citation(args, topic_thresh + i*delta_thres)
            if new_val > current_val:
                current_val =  new_val
                topic_thresh += i*delta_thres
                success = True
                break
        if not success:
            break
    
    print("best parameters: ", topic_thresh)
    return topic_thresh


def get_qual_citation(args, topic_thresh, test_perc=0.4):

    args.topic_thr = topic_thresh
    create_vecs(args)
    
    # get new values on test set based on current thresholds
    if args.exp == 'citation':
        g, n, C1, C2, C3, C4 = get_aux_cit(args)
    
    
    learn_params(args, g, n, C1, C2, C3, C4)

    

    # read the learned parameters
    basefile =  open(os.path.join(args.data_dir, args.exp, 'base_weights.txt'), newline='')
    basefile = csv.reader(basefile, delimiter=' ')

    qfile = open(os.path.join(args.data_dir, args.exp, 'marg_weights.txt'), newline='')
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


    logfile = open(args.log_file, newline='')
    logs = list(csv.reader(logfile, delimiter=' '))
    num_logs = sum(1 for row in logs)
    templogs = logs[:int(num_logs*test_perc)]
    testlogs = []
    for log in templogs: 
        nplog = np.array(log, dtype=np.int)
        testlogs.append(nplog)
    
    testlogs = np.array(testlogs)

    print("Size of Test log in consideration: ", testlogs.shape)
    # testlogs is now a numpy array

    
    published = dict()
    cited = dict()

    
    for log in testlogs:
        [u, v, c, p] = [int(x) for x in log]

        # if v == 1344: 
        #     print(c)
        if u in published.keys():
            published[u].add(c)
        else:
            published[u] = set()
            published[u].add(c)
        
        if v in published.keys():
            published[v].add(p)
        else:
            published[v] = set()
            published[v].add(p)

        if v in cited.keys():
            cited[v].add(c)
        else:
            cited[v] = set()
            cited[v].add(c)
        
    print("Preprocessed test log.")
    # to estimate auc, taking 100 points on the curve
    for mu in tqdm(np.linspace(0, 1, 100)):    
        
        tpx = 0
        fpx = 0
        tnx = 0
        fnx = 0 

        for v in cited.keys(): # per entry of the log
            for msg in cited[v]:

                # get action set
                for u in g.predecessors(v):
                    if u in published.keys():
                        action_set = set([p for p in published[u]])    
                        
                prob = 0.0

                # prob for an action
                for a in action_set:   
                    for u in g.predecessors(v):
                        # get valid predecessors for the actions
                        if u in published.keys() and a in published[u]:
                            prob += b[(u, v)] + q[(u, v)]*get_alpha(user_features[v], topic_features[a])
                    
                    prob = min(1, max(prob, 0))
                    prediction = (prob > mu)

                    gt = (a == msg)
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

    auc = -1 * np.trapz(tpr, fpr)
    print('auc: ', auc)
    return auc


def tune_citation(args, test_perc=0.4):

    print("entering outer function")
    logfile = open(args.log_file, newline='')
    logs = list(csv.reader(logfile, delimiter=' '))
    num_logs = sum(1 for row in logs)
    testlogs = logs[:int(num_logs*test_perc)]
    # testlogs = []
    # for log in templogs: 
    #     nplog = np.array(log, dtype=np.int)
    #     testlogs.append(nplog)
    
    # testlogs = np.array(testlogs)

    print("Size of Test log in consideration: ", len(testlogs))
    # testlogs is now a numpy array
    
    published = dict()
    cited = dict()

    for log in testlogs:
        [u, v, c, p] = [int(x) for x in log]

        # if v == 1344: 
        #     print(c)
        if u in published.keys():
            published[u].add(c)
        else:
            published[u] = set()
            published[u].add(c)
        
        if v in published.keys():
            published[v].add(p)
        else:
            published[v] = set()
            published[v].add(p)

        if v in cited.keys():
            cited[v].add(c)
        else:
            cited[v] = set()
            cited[v].add(c)
        
    print("Preprocessed test log.")


    def _get_qual_citation(args, topic_thresh, test_perc=0.4):

        print("Calling inner function")
        
        create_vecs(args)    
        # get new values on test set based on current thresholds
        if args.exp == 'citation':
            g, n, C1, C2, C3, C4 = get_aux_cit(args)
        
        b, q = learn_params(args, g, n, C1, C2, C3, C4)

        # need to reload since re-saved when creating the vectors
        topic_features = np.load(args.topic_features)
        user_features = np.load(args.user_features)

        # calculate tp, fp, tn, fn
        tp = []
        fp = []
        tn = []
        fn = []
        
        # to estimate auc, taking 100 points on the curve
        for mu in tqdm(np.linspace(0, 1, 100)):    
            # print(mu)
            tpx = 0
            fpx = 0
            tnx = 0
            fnx = 0 

            for v in cited.keys(): # per entry of the log
                for msg in cited[v]:

                    # get action set
                    for u in g.predecessors(v):
                        if u in published.keys():
                            action_set = set([p for p in published[u]])    
                            
                    prob = 0.0

                    # prob for an action
                    for a in action_set:   
                        for u in g.predecessors(v):
                            # get valid predecessors for the actions
                            if u in published.keys() and a in published[u]:
                                prob += b[(u, v)] + q[(u, v)]*get_alpha(user_features[v], topic_features[a])
                        
                        prob = min(1, max(prob, 0))
                        prediction = (prob > mu)

                        gt = (a == msg)
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

            print("Results for mu = ", mu, "(tp, fp, tn, fn)", tpx, fpx, tnx, fnx)
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

        print("TPR: ", tpr)
        print("FPR: ", fpr)
        auc = -1 * np.trapz(tpr, fpr)
        print('auc: ', auc)
        return auc   

    iter=0
    print("iter: ", iter)    
    current_val = 0
    
    # signs = [(-1,-1,-1), (-1,-1,0), (-1,-1,1), (-1,)]
    while True:
        iter+=1
        print("iter: ", iter)    
        print("Current choice: ", args.topic_thr)
        
        success = False
        for i in [-1,1]:
            new_val = _get_qual_citation(args, args.topic_thr + i*args.delta_thres)
            if new_val > current_val:
                current_val =  new_val
                args.topic_thr += i*args.delta_thres
                success = True
                break
        if not success:
            break
    
    print("best parameters: ", args.topic_thr)
    return args.topic_thr


def tune_vk(args, test_perc=0.2):
    

    logfile = open(args.log_file, newline='')
    logs = list(csv.reader(logfile, delimiter=' '))
    num_logs = sum(1 for row in logs)
    testlogs = logs[:int(num_logs*test_perc)]
    # testlogs = []
    # for log in templogs: 
    #     nplog = np.array(log, dtype=np.int)
    #     testlogs.append(nplog)
    
    # testlogs = np.array(testlogs)

    print("Test log in consideration: ", len(testlogs))

    actions_of = dict()
    action_table = dict()
    action_done_by = dict()

    for log in testlogs:

        u, au, tu = [int(x) for x in log]

        if u in actions_of.keys():
            actions_of[u].add(au)
        else:
            actions_of[u] = set()
            actions_of[u].add(au)
        
        # if au in action_done_by.keys():
        #     action_done_by[au].add(u)
        # else:
        #     action_done_by[au] = set()
        #     action_done_by[au].add(u)
        # action_table[(u, au)] = tu

    print("Log preprocessing done.")

    def _get_qual(args, topic_thresh, sim_bits):
        
        print("Inner qual function")

        # get new values on test set based on current thresholds
        args.topic_thr = topic_thresh
        args.nbits = sim_bits
        create_vecs(args)
        if args.exp == 'vk':
            g, n, C1, C2, C3, C4 = get_aux_vk(args)
        
        b, q = learn_params(args, g, n, C1, C2, C3, C4)
    
        topic_features = np.load(args.topic_features)
        user_features = np.load(args.user_features)

    
        # calculate tp, fp, tn, fn
        tp = []
        fp = []
        tn = []
        fn = []


        # testlogs is now a numpy array

        # to estimate auc, taking 100 points on the curve
        for mu in tqdm(np.linspace(0, 1, 10)):    
            
            tpx = 0
            fpx = 0
            tnx = 0
            fnx = 0 

            for log in reversed(testlogs):
                
                # [v, a, t_v] = [int(x) for x in log], already converted into an integer
                v, a_v, t_v = [int(x) for x in log]
                # v published a message in the test log

                # set based on similarity
                action_list = [] 
                action_set = []

                for u in g.predecessors(v):
                    # list of permissible actions performed by u
                    for au in actions_of[u]:
                        if action_table[(u, au)] > t_v:
                            continue                        
                        action_list.append(au)  

                    

                    for a1 in action_list:
                        similar = False
                        for a2 in action_set:
                            if check_sim(topic_features[a1], topic_features[a2], nbits=sim_bits):
                                similar = True
                                break
                            if not similar:
                                action_set.append(a1)

                print("Formed action set for node: ", v)
                # now they are unique actions
                prob = 0.0
                for a in action_set:   
                    for u in g.predecessors(v):
                        # get valid predecessors for the actions
                        for au in actions_of[u]:
                            if action_table[(u, au)] > t_v:
                                continue
                            if check_sim(topic_features[au], topic_features[a], nbits=sim_bits):
                                prob += b[(u, v)] + q[(u, v)]*get_alpha(user_features[v], topic_features[a])
                    
                    # summed up prob for a by all users and through all their actions

                    prob = min(1, max(prob, 0))
                    prediction = (prob > mu)
                    
                    # gt -> this log entry is similar to the action
                    # gt = checklog(logs, v, a)
                    gt = check_sim(topic_features[a_v], topic_features[a], nbits=sim_bits)

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

            
            print("(tp, fp, tn, fn): ", tpx, fpx, tnx, fnx)
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

        print("TPR: ", tpr)
        print("FPR: ", fpr)

        auc = -1 * np.trapz(tpr, fpr)
        print("auc: ", auc)
        return auc
    
    iter=0
    print("iter: ", iter)    
    
    current_val = 0.0
    
    # signs = [(-1,-1,-1), (-1,-1,0), (-1,-1,1), (-1,)]
    while True:
        iter+=1
        print("iter: ", iter)    
        print("Current choice: ", args.topic_thr, args.nbits)
        
        success = False
        for i in [-1,0,1]:
            for j in [-1,0,1]:
                    new_val = _get_qual(args, args.topic_thr + i*args.delta_thres, args.nbits +j*args.delta_bits)
                    if new_val > current_val:
                        current_val =  new_val
                        args.topic_thr += i*args.delta_thres
                        args.nbits += j*args.delta_bits
                        success = True
                        break
        if not success:
            break
    
    print("best parameters: ", args.topic_thr, args.nbits)
    return args.topic_thr, args.nbits