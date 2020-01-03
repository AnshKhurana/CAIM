import sys, os

def clean_data(infile, outfile):
    lines_seen = set() # holds lines already seen
    outfile = open(outfile, "w")
    for line in open(infile, "r"):
        if line not in lines_seen: # not a duplicate
            outfile.write(line)
            lines_seen.add(line)
    outfile.close()


if __name__ == "__main__":
    
    dataset = sys.argv[1]
    infile = os.path.join(dataset, "edge_list.txt") 
    outfile = os.path.join(dataset, "edge_list_cleaned.txt") 

    clean_data(infile, outfile)

