import sys
import time

def is_alpha(str):
	upper_str = str.upper()
	for char in upper_str:
		ascii = ord(char)
		if ascii < 65 or ascii > 90:
			return False
	return True 
		

while True:
	line = sys.stdin.readline()
	if not line: 
		break
	words = line.strip().replace('\t',' ').split(" ")
	for word in words:
		if word == "" or not is_alpha(word):
			continue
		print "%s\t1"%(word)  
		sys.stdout.flush()
#while True:
#	time.sleep(5)
#print "exit mapper\t1"
