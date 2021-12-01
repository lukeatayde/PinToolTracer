import typing
import re
import codecs

"""
Script that parses and models call graph of program tracked by pintool.
"""
# ENCODINGS = ["utf8", "cp1252"]

def parse_trace(filename: str):
	file = codecs.open(filename, "r", encoding="cp1252", errors="replace")

	line = file.readline()
	 
	# Find the start of each thread's function trace
	while line and not re.match("Thread Function Trace: ", line):
		line = file.readline()

	if not line: 
		return 

	# At this point, we have found the head of a function trace.
	# TODO: Thread numbers may not be unique.
	thread = int(line.split(" ")[-1])
	print(f"Found head for thread { thread }")

	func_names = set()
	func_name_re = re.compile("\t(?P<FunctionName>.*)$")

	line = file.readline()
	while line:
		match = func_name_re.search(line)
		
		# Hit a line that isn't a function call.
		if not match:
			break

		name = match.group("FunctionName")
		if name not in func_names:
			func_names.add(name)

		line = file.readline()

	print(func_names)

parse_trace("calltrace_log.txt")