# clean results file
import sys
import numpy as np
from matplotlib import pyplot as plt


def clean_time_file(filename):
	# print(filename)
	data = np.array([])
	with open(filename, 'r') as f:
		for line in f.readlines():
			if len(line) > 1: 
				words = line.strip().split()
				if 'F.size()' in words:
					# print(words)
					time = float(words[-1])
					data = np.append(data, [time])										
	return data

def clean_spread_file(filename):
	data = np.array([]) # hard coded init spread value
	with open(filename, 'r') as f:
		for line in f.readlines():
			if len(line) > 1: 
				words = line.strip().split()
				if 'mc_spread' in words:
					# print(words)
					if int(words[2][:-1]) %2 == 0:
						spread = float(words[-1])
						data = np.append(data, [spread])										
	return data


def get_eta_results(filename):
	x_data = [pow(10, -x) for x in range(1, 8)]
	print(x_data)
	time_data = []
	q_data = []
	with open(filename, 'r') as f:
		for line in f.readlines():
			words = line.strip().split()
			print(words)
			time_data.append(float(words[1]))
			q_data.append(float(words[2]))

	print(list(zip(x_data, time_data)))
	print(list(zip(x_data, q_data)))


def print_coordinates(x, y):
	co_string = ""
	print(list(zip(x, y)))


def plot_data(xval, yval, xlabel, ylabel, title):
	plt.plot(xval, yval, '--bo')
	plt.xlabel(xlabel)
	plt.ylabel(ylabel)
	plt.title(title)
	plt.show()

if __name__ == '__main__':
	filename = sys.argv[1]
	file_type = sys.argv[2]

	if file_type == 't':
		data = clean_time_file(filename)
		print_coordinates(np.array(range(1,37)), data)
		# plot_data(np.array(range(1,51)), data, '|F|', 'Time', 'Execution time (in s.) vs number of selected features')
	elif file_type == 's':
		data = clean_spread_file(filename)
		print_coordinates(np.array(range(0, 37, 2)), data)
		# print(np.array(range(0, 51, 2)))
		# plot_data(np.array(range(0, 51, 2)), data, '|F|', 'Spread', 'Spread in the network vs the number of selected features')
	elif file_type == 'e':
		get_eta_results(filename)
	else:
		raise NotImplementedError


	
