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
				if words[0] == 'it':
					# print(words)
					time = float(words[-1])
					data = np.append(data, [time])										
	return data[1:]

def clean_spread_file(filename):
	data = np.array([21]) # hard coded init spread value
	with open(filename, 'r') as f:
		for line in f.readlines():
			if len(line) > 1: 
				words = line.strip().split()
				if 'mc_spread' in words:
					# print(words)
					spread = float(words[-1])
					data = np.append(data, [spread])										
	return data


def print_coordinates(x, y):
	co_string = ""
	print(list(zip(x, y)))


def plot_data(xval, yval, xlabel, ylabel, title):
	plt.plot(xval, yval, '--bo')
	plt.xlabel(xlabel)
	plt.ylabel(ylabel)
	plt.title(title)
	plt.savefig('cit_t_sim.png')


def spread_k_gnu():

	data_greedy = [(0, 24.8623), (2, 28.642), (4, 29.6624), (6, 30.5756), (8, 31.3923), (10, 32.0986), (12, 32.7519), (14, 33.3654), (16, 33.9164), (18, 34.3967), (20, 34.8718), (22, 35.3167), (24, 35.6515), (26, 36.0449), (28, 36.4401), (30, 36.7503), (32, 37.0303), (34, 37.3628), (36, 37.6209), (38, 37.9655), (40, 38.2206), (42, 38.4256), (44, 38.6783), (46, 38.8322), (48, 38.9647), (50, 39.1039)]

	x = [point[0] for point in data_greedy]
	y = [point[1] for point in data_greedy]
	plt.plot(x, y, '-ro')


	data_simpath = [(0, 24.8623), (2, 28.6247), (4, 29.5759), (6, 30.5458), (8, 31.3343), (10, 32.1546), (12, 32.7763), (14, 33.0439), (16, 33.7628), (18, 34.3229), (20, 34.6693), (22, 35.0951), (24, 35.474), (26, 35.8677), (28, 36.3221), (30, 36.5791), (32, 36.9588), (34, 37.2798), (36, 37.5714), (38, 37.7783), (40, 38.0252), (42, 38.1056), (44, 38.451), (46, 38.4988), (48, 38.6791), (50, 38.858)] 


	x = [point[0] for point in data_simpath]
	y = [point[1] for point in data_simpath]
	
	plt.plot(x, y, '-bo')
	plt.xlabel('|F|')
	plt.ylabel('Spread')

	plt.savefig('gnu_spr_vs_k.png')


if __name__ == '__main__':
	filename = sys.argv[1]
	file_type = sys.argv[2]




	# auc_vals = np.array([ 0.5727698194988937, 0.5923203969882388, 0.5838123726437189, 0.5762415010285576, 0.5554355483357896, 0.5167516519011217, 0.4713535962063935 ])
	# plot_data(np.linspace(-0.02, 0.1, 7), auc_vals, r'$\theta$', 'AUC', '')

	# spread_k_gnu()
	if file_type == 't':
		data = clean_time_file(filename)
		print_coordinates(np.array(range(1,51)), data)
		plot_data(np.array(range(1,51)), data, '|F|', 'Time', 'Execution time (in s.) vs number of selected features')
	elif file_type == 's':
		data = clean_spread_file(filename)
		print_coordinates(np.array(range(0, 51, 2)), data)
		# print(np.array(range(0, 51, 2)))
		plot_data(np.array(range(0, 51, 2)), data, '|F|', 'Spread', 'Spread in the network vs the number of selected features')
	else:
		raise NotImplementedError


	
