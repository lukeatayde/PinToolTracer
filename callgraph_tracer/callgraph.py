import typing
import re
import codecs
from collections import defaultdict
from pprint import PrettyPrinter
"""
Script that parses and models call graph of program tracked by pintool.
"""
# ENCODINGS = ["utf8", "cp1252"]

pp = PrettyPrinter(indent=4)

def parse_trace(filename: str):
	file = codecs.open(filename, "r", encoding="cp1252", errors="replace")
	func_name_re = re.compile("\t(?P<FunctionName>.*)$")
	return_re = re.compile("\t(?P<return>ret).*$")
	
	line = file.readline()
	while True:  
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
		# tracks edges in call grpah
		call_graph = defaultdict(set)
		return_graph = defaultdict(set)
		

		call_stack = ["CALL_STACK_START"]

		line = file.readline()
		while line:
			func_match = func_name_re.search(line)
			ret_match = return_re.search(line)
			if func_match:
				name = func_match.group("FunctionName")
				if name not in func_names:
					func_names.add(name)

				call_graph[call_stack[-1]].add(name)
				call_stack.append(name)
			elif ret_match:
				# If it is a return, we need to pop the function name off the stack, and create an edge
				return_from = call_stack.pop()
				return_graph[return_from].add(call_stack[-1])
			else:
				# We got no match. Line is neither a function or a return
				break

			line = file.readline()

		# Hit the bottom of a thread's trace. Repeat the process until the file is exhausted.
		pp.pprint(func_names)
		pp.pprint(call_graph)
		

parse_trace("calltrace_log.txt")