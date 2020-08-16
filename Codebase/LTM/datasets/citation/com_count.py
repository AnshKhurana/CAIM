min_len = 100000

with open('com_size.txt', 'w') as fout:
	with open('com.txt') as f:
		for line in f.readlines():
			vals = line.strip().split()
			size = len(vals[1:])
			newline = vals[0] + " " + str(size) + "\n"
			if size < min_len:
				min_len = size
				best = vals[0]

print(best, min_len)