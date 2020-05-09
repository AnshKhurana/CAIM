with open('seeds.txt', 'w') as fout:
	with open('com.txt', 'r') as f:

		# using seed set 35
		for member in f.readlines()[35].strip().split()[1:]:
			fout.write(str(member) + '\n' )
