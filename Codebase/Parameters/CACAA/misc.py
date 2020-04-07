import os, csv
import numpy as np

def check_same():
    with open('../data/topic_features.npy', 'rb') as f:
        tf = np.load(f)

    print("all", tf.shape)

    print("unique rows:", np.unique(tf, axis=0).shape)

if __name__ == "__main__":
    check_same()