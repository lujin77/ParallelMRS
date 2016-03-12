import sys

preWord = ""
preCount = 0

while True:
	line = sys.stdin.readline()
	if not line or line == "\n": 
		break
	items = line.strip("\n").split("\t")
	if preWord != "" and items[0] != preWord:
		print "%s\t%d"%(preWord, preCount)
		sys.stdout.flush()
		preWord = items[0]
		preCount = int(items[1])
	else:
		preWord = items[0]
		preCount = preCount + int(items[1])
# dump last one
if preWord != "" and preCount > 0:
	print "%s\t%d"%(preWord, preCount)
	sys.stdout.flush()

		
	
	
