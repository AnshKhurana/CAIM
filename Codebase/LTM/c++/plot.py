from matplotlib import pyplot as plt 
import sys
import numpy as np

def plot(xData, yData, xAxisLabel, yAxisLabel, plot_title):
    plt.plot(xData, yData)
    plt.xlabel(xAxisLabel)
    plt.ylabel(yAxisLabel)
    plt.title(plot_title)
    # plt.legend()
    plt.savefig('tmp.png')


def read_data(result_file):
    """
    Return data from the results file. Make if format invariant. find?
    """
    with open(result_file, 'r') as f:
        lines = f.readlines()
        # print(lines)
        lines = [np.array(x.strip().split()) for x in lines[2:-1]]
        # print(lines)
        
        matrix = np.array(lines)
        print(matrix.shape)
        xData = matrix[2:-1, 0].astype(float)
        yData = matrix[2:-1, 1].astype(float)
    return xData, yData
    

if __name__ == "__main__":
    
    result_file = sys.argv[1]
    title = "manual" # AI is not there yet
    yAxisLabel = "Spread"
    xAxisLabel = "K"
    xData, yData =  read_data(result_file)
    plot(xData, yData, xAxisLabel, yAxisLabel, title)

