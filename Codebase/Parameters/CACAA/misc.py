import os, csv
import numpy as np

def check_same():
    with open('../data/topic_features.npy', 'rb') as f:
        tf = np.load(f)

    print("all", tf.shape)

    print("unique rows:", np.unique(tf, axis=0).shape)


def check_num_users():

    print("From graph")
    user_set = set()
    edge_list = open('../data/vk_graph_lt.csv', newline='')
    edges = csv.reader(edge_list, delimiter=' ')

    for edge in edges:
        [u, v] = edge
        user_set.add(u)
        user_set.add(v)
    
    print(len(user_set))

    print("From user features ")

    with open('../data/user_preferences.csv', 'r') as f:
        user_list = f.readlines()

    print("Total number of users", len(user_list))    
    


def fix_users_file(path="../data/vk/user_preferences_raw.csv"):

    with open(path, 'r') as f:
        user_pref_list = f.readlines()

    user_pref_list = [x.strip().split() for x in user_pref_list]

    # now a list of list

    fixed_prefs = dict()

    for pref in user_pref_list:
        # pref = [float(x) for x in pref]
        fixed_prefs[int(float(pref[0]))] = pref[1:]
        
    newpath = os.path.join('/'.join(path.split('/')[:-1]), 'user_preferences.csv')
    print(newpath)
    with open(newpath, 'w') as f:
        for i in range(len(fixed_prefs.keys())):
            f.write(" ".join(fixed_prefs[i]) + '\n')

# NEED TO CHANGE CWD FIRST
# convert sparse to dense vector notation
def convert_dense_topic():
	
	
	num_features = 100
	with open('lda_vectors_sparse.csv', 'r') as f:
		sparse_vectors = f.readlines()

	sparse_vectors = [x.strip().split() for x in sparse_vectors]

	dense_vectors = dict()

	for line in sparse_vectors:
		key, feature, value = line
		key = int(key)
		feature = int(feature)
		value = float(value)

		if key in dense_vectors.keys():
			dense_vectors[key][feature] = value
		else:
			dense_vectors[key] = np.zeros(num_features)
			dense_vectors[key][feature] = value

	with open('topic_features.csv', 'w') as f:
		for i in range(len(dense_vectors.keys())):
			f.write(" ".join([str(x) for x in dense_vectors[i]]) + '\n')
	print(i)


def clean_edge_list():
	
	with open('citation_social_graph_gcc.csv', 'r') as f:
		with open('edge_list.csv', 'w') as fout:
			for line in f.readlines():
				read = line.strip().split()
				write = read[:2]

				fout.write(" ".join(write) + '\n')


if __name__ == "__main__":
    # check_same()
    fix_users_file()