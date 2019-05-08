#!/usr/bin/env python
# -*- coding: utf-8 -*-

# year: 	2008
# author: 	Vallati Carlo <carlo.vallati@iet.unipi.it>
#		Andreozzi Matteo <matteo.andreozzi@iet.unipi.it>
# affiliation:	Dipartimento di ingegneria dell'informazione
#		Universita di Pisa - Italy

#	This script runs simulations according to an xml file 
#	(run option) or generate statistics from already simulated 
#	scenarios (stat option).
#	A simple structure of the xml configuration file is:

#<?xml version="1.0" encoding="UTF-8"?>


#<simulation>
	#<name>Scheduler</name>
	#<description>Scheduler evaluation</description>
	#<check_path>/home/carlo/stat-utils/check</check_path>
	#<recover_path>/home/carlo/stat-utils/recover</recover_path>
	#<ns_path>path/to/ns</ns_path>
	#<ns_output></ns_output>
	#<base_scenario>base.tcl</base_scenario>
	#<min_run>5</min_run>
	#<max_run>5</max_run>
	#<output_dir>savefile</output_dir>
	#<savefile_dir>savefile</savefile_dir>
	#<check_metrics>metrics_to_check</check_metrics>
	#<check_conf_level>0.95</check_conf_level>
	#<check_th>0.05</check_th>
	#<multicell>
		#<param>
			#<name>NMS</name>
			#<description>Number of mobile stations</description>
			#<tclname>nodes</tclname>
			#<value alias="" value="70"></value>
			#<value alias="" value="80"></value>
			#<value alias="" value="90"></value>
		#</param>
		#<param>
			#<name>Scheduler</name>
			#<description>Type of scheduling policy</description>
			#<tclname>psDiscipline</tclname>
			#<value alias="" value="pf">
				#<param>
					#<name>pf-alpha</name>
					#<description>Alpha parameter for PF</description>
					#<tclname>pfAlpha</tclname>
					#<value alias="" value="0.1"></value>
					#<value alias="" value="0.3"></value>
					#<value alias="" value="0.5"></value>
					#<value alias="" value="0.7"></value>
					#<value alias="" value="0.9"></value>
				#</param>
				#<param>
					#<name>expire</name>
					#<description>Packet expire</description>
					#<tclname>pkt-expire</tclname>
					#<value alias="" value="40"></value>
					#<value alias="" value="80"></value>
				#</param>
			#</value>
			#<value alias="" value="drr">
				#<param>
					#<name>quantum</name>
					#<description>Quantum for drr scheduler</description>
					#<tclname>drr-uantum</tclname>
					#<value alias="" value="190"></value>
					#<value alias="" value="1000"></value>
					#<value alias="" value="10000"></value>
				#</param>
				#<param>
					#<name>expire</name>
					#<description>Packet expire</description>
					#<tclname>pkt-expire</tclname>
					#<value alias="" value="40"></value>
					#<value alias="" value="80"></value>
				#</param>
			#</value>
			#<value alias="" value="maxci"></value>
			#<value alias="" value="edf">
				#<param>
					#<name>offset</name>
					#<description>Edf offset</description>
					#<tclname>edf-offset</tclname>
					#<value alias="" value="20"></value>
					#<value alias="" value="40"></value>
				#</param>
			#</value>
		#</param>
	#</multicell>
#</simulation>

import sys
import getopt

import re
import xml.sax.handler

import subprocess

import commands, tokenize

import sys, time, os



##  Function to buil tree from XML code ##
def xml2obj(src):
    """
    A simple function to converts XML data into native Python object.
    """

    non_id_char = re.compile('[^_0-9a-zA-Z]')
    def _name_mangle(name):
        return non_id_char.sub('_', name)

    class DataNode(object):
        def __init__(self):
            self._attrs = {}    # XML attributes and child elements
            self.data = None    # child text data
        def __len__(self):
            # treat single element as a list of 1
            return 1
        def __getitem__(self, key):
            if isinstance(key, basestring):
                return self._attrs.get(key,None)
            else:
                return [self][key]
        def __contains__(self, name):
            return self._attrs.has_key(name)
        def __nonzero__(self):
            return bool(self._attrs or self.data)
        def __getattr__(self, name):
            if name.startswith('__'):
                # need to do this for Python special methods???
                raise AttributeError(name)
            return self._attrs.get(name,None)
        def _add_xml_attr(self, name, value):
            if name in self._attrs:
                # multiple attribute of the same name are represented by a list
                children = self._attrs[name]
                if not isinstance(children, list):
                    children = [children]
                    self._attrs[name] = children
                children.append(value)
            else:
                self._attrs[name] = value
        def __str__(self):
            return self.data or ''
        def __repr__(self):
            items = sorted(self._attrs.items())
            if self.data:
                items.append(('data', self.data))
            return u'{%s}' % ', '.join([u'%s:%s' % (k,repr(v)) for k,v in items])

    class TreeBuilder(xml.sax.handler.ContentHandler):
        def __init__(self):
            self.stack = []
            self.root = DataNode()
            self.current = self.root
            self.text_parts = []
        def startElement(self, name, attrs):
            self.stack.append((self.current, self.text_parts))
            self.current = DataNode()
            self.text_parts = []
            # xml attributes --> python attributes
            for k, v in attrs.items():
                self.current._add_xml_attr(_name_mangle(k), v)
        def endElement(self, name):
            text = ''.join(self.text_parts).strip()
            if text:
                self.current.data = text
            if self.current._attrs:
                obj = self.current
            else:
                # a text only node is simply represented by the string
                obj = text or ''
            self.current, self.text_parts = self.stack.pop()
            self.current._add_xml_attr(_name_mangle(name), obj)
        def characters(self, content):
            self.text_parts.append(content)

    builder = TreeBuilder()
    if isinstance(src,basestring):
        xml.sax.parseString(src, builder)
    else:
        xml.sax.parse(src, builder)
    return builder.root._attrs.values()[0]




## Recursive List permuation implementation. ##
def permute(Lists):
    n = len(Lists)
    if n == 0:
        return []
    if n == 1:
        return map(lambda i: (i,), Lists[0])
    # find good splitting point
    prods = []
    prod = 1
    for x in Lists:
        prod = prod * len(x)
        prods.append(prod)
    for i in range(n):
        if prods[i] ** 2 >= prod:
            break
    n = min(i + 1, n - 1)
    a = permute(Lists[:n])
    b = permute(Lists[n:])
    sprayb = []
    lena = len(a)
    for x in b:
        sprayb[len(sprayb):] = [x] * lena
    import operator
    return map(operator.add, a * len(b), sprayb)


## Function to navigate the tree and build the options of scenarios ##
def sim_run(param):
	commands = [ ]
	for val in param.value:
		v = []
		if val.param :
			for par in val.param :
				s = []
				sub_com=sim_run(par)
				for sub in sub_com:
					cmd=sub
					s.append(cmd)
				v.append(s)
			v=permute(v)
			
			for vect in v :
				cmd = ""
				for string in vect:
					cmd+=string
				commands.append("-"+param.tclname+" "+val.value+" "+cmd)
		else :
			cmd="-"+param.tclname+" "+val.value+" "		
			commands.append(cmd)
	return commands
	
## Function to navigate the tree and build the factorial2kr configurations scenarios ##
def sim_fact(param):
	commands = [ ]
	for val in param.value:
		v = []
		if val.param :
			for par in val.param :
				s = []
				sub_com=sim_fact(par)
				for sub in sub_com:
					cmd=sub
					s.append(cmd)
				v.append(s)
			v=permute(v)
			
			for vect in v :
				cmd = ""
				for string in vect:
					cmd+=string
				if val.level == "low":
					commands.append(" "+param.tclname+" 0 "+cmd)
				else:
					commands.append(" "+param.tclname+" 1 "+cmd)
		else :
			if val.level == "low":
				cmd = " "+param.tclname+" 0 "
			else:
				cmd = " "+param.tclname+" 1 "
			commands.append(cmd)
	return commands
		
## Function to navigate the tree and build the mangle of scenarios ##
def sim_mangle(param):
	commands = [ ]
	for val in param.value:
		v = []
		if val.param :
			for par in val.param :
				s = []
				sub_com=sim_mangle(par)
				for sub in sub_com:
					cmd=sub
					s.append(cmd)
				v.append(s)
			v=permute(v)
			
			for vect in v :
				cmd = ""
				for string in vect:
					cmd+=string
				if(val.alias==""):
					commands.append(val.value+"-"+cmd)
				else:
					commands.append(val.alias+"-"+cmd)
		else :
			if(val.alias==""):
				cmd=val.value+"-"
			else:
				cmd=val.alias+"-"
			commands.append(cmd)
	return commands
	
## Function return mangle from cmd ##
def cmd2runmangle(cmd):
	ret=""
	mangle=re.findall('[a-zA-Z0-9_\-.]+', cmd)
	for string in mangle:
		if(string[0]=='-'):
			continue
		else:
			ret+=string+"-"
	ret=ret[0:len(ret)-1]
	return ret
	
## Function return mangle from cmd ##
def cmd2statmangle(cmd):
	ret=""
	mangle=re.findall('[a-zA-Z0-9_\-.]+', cmd)
	for string in mangle:
		if(string[0]=='-'):
			continue
		else:
			ret+=string+","
	ret=ret[0:len(ret)-1]
	return ret

## Function that prints the usage informations and returns
def print_usage():
	# Print the usage informations
	print "Usage: <run|test|stat|csv|fact> sim_desc.xml"
	print "Where:"
	print "	run:	run simulation until the confidence level is reached"
	print "		or the number of replics is beyond maximum"
	print "	test:	print the simulation commands that will be run"
	print "	stat:	collect the measures from the savefiles"
	print "	csv:	print data in CSV format to a single file"
	print "	fact:	create configuration file for factorial2kr"
	exit()

		
## Main program ##
def main():
	global command
	main_params = 0
	args = sys.argv

	if len(args) is 1:
		print_usage()

	# No args, delete lock file
	action = args[1]
  
	if len(args) is 3:
		infile = open(args[2],"r")
		# Read the XML file
		desc = infile.read()
		infile.close();
		# Parse the XML file
		simulation = xml2obj(desc)
	
		res = []
		final_run = []
		
		# Get the parameters
		for par in simulation.multicell.param :
			res.append(sim_run(par))
			main_params = main_params + 1
			
		# Permute the parameters
		res=permute(res)
		
		# Get the sub-parameters recursively and permute them
		for vect in res :
			cmd = ""
			for string in vect:
				cmd+=string
			final_run.append(cmd)
			
		res = []
		final_mangle = []
			
		for par in simulation.multicell.param :
			res.append(sim_mangle(par))
			
		res=permute(res)
		
		# Build the scenarios
		for vect in res :
			cmd = ""
			for string in vect:
				cmd+=string
			cmd=cmd[0:len(cmd)-1]
			final_mangle.append(cmd)
		
		res = []
		factorial = []
		
		for par in simulation.multicell.param :
			res.append(sim_fact(par))
			
		res=permute(res)
		
		# Build the scenarios
		for vect in res :
			cmd = ""
			for string in vect:
				cmd+=string
			cmd=cmd[0:len(cmd)-1]
			factorial.append(cmd)
		
	if action == "help":
		# Print the usage informations
		print_usage()
	elif action == "test":
		# Some arguments are missing
		if len(args) < 3:
			print "Missing arguments..."
			return
		# Print all the simulation scenarios
		for item in final_run :
			print item
	elif action == "run":
		# Some arguments are missing
		if len(args) < 3:
			print "Missing arguments..."
			return
		index=0
		# Run all the scenarios
		for item in final_run :
			# Execute a scenario
			mangle=final_mangle[index]
			index=index+1
			# Get the number of runs already saved
			run = commands.getoutput(simulation.check_path+" -n "+simulation.savefile_dir+"/"+mangle)
			if run == "err" or run == "no" :
				run = "0"
			run=int(run)
			fifo=simulation.name+".fifo"
			commands.getoutput("rm -f "+fifo)	
			# Create fifo
			commands.getoutput("mkfifo "+fifo)				
			# Create the directory to save files
			commands.getoutput("mkdir "+simulation.savefile_dir)
			print mangle
			start_time=time.time()
			# Execute the runs
			while int(run) < int(simulation.max_run) :
				# Check the results
				status=commands.getoutput( simulation.check_path+" "+simulation.savefile_dir+"/"+mangle+" -c "+simulation.check_conf_level+" -t "+simulation.check_th+" "+simulation.check_metrics )
				if status == "ok" and int(run) >= int(simulation.min_run) :
					break
				elif int(run) > 0 and status == "err" :
					print "Error in ",simulation.savefile_dir
					return
				run=run+1
				pid = os.fork()
				# create the fifo to store data
				if pid == 0:
					commands.getoutput( "cat "+fifo+" >> "+simulation.savefile_dir+"/"+mangle )
					sys.exit (0)
				print "\t run ",run
				# Launch the simulator
				ns_cmd = simulation.ns_path+" "+simulation.base_scenario+" "+item+" -out "+fifo+" -run "+str(run)
				if simulation.ns_output <> "":
					ns_cmd += " > "+simulation.ns_output+"-out."+str(run)+" 2> "+simulation.ns_output+"-err."+str(run)
				commands.getoutput (ns_cmd)
				os.waitpid (pid, 0)
			commands.getoutput("rm -f "+fifo)				
			print "Total scenario sim time ",time.time()-start_time," sec"
	elif action == "stat":
		# Some arguments are missing
		if len(args) < 3:
			print "Missing arguments..."
			return
		# Generate statistics for each scenario
		for item in final_mangle :
			commands.getoutput("mkdir "+simulation.output_dir)
			mangle=item.replace("-",",")
			# Extract statistics
			commands.getoutput( simulation.recover_path+" "+simulation.savefile_dir+"/"+item+" -c "+simulation.check_conf_level+" -s "+simulation.output_dir+"/ -n "+mangle+" -d ")
	elif action == "csv":
		# Some arguments are missing
		if len(args) < 3:
			print "Missing arguments..."
			return
		# Generate statistics for each scenario
		commands.getoutput("/bin/rm "+simulation.csv_data)
		for item in final_mangle :
			commands.getoutput("mkdir "+simulation.output_dir)
			mangle=item.replace("-",",")
			# Extract statistics
			commands.getoutput(simulation.csv_save_path+" "+simulation.savefile_dir+"/"+item+" -n "+mangle+" >> "+simulation.csv_data)
	elif action == "fact":
		# Some arguments are missing
		if len(args) < 3:
			print "Missing arguments..."
			return
		name = "conf";
		# Parse the XML file
		simulation = xml2obj(desc)
		# Generate the list of responses to be analyzed
		responses=re.split(' ',simulation.factorial_response)
		# Consider all the scenarios
		out_file = open(name,"w")
		out_file.write("savefile_dir "+simulation.savefile_dir+"\n")
		out_file.write("response_var "+responses[0]+"\n")
		out_file.write("num_pr_factors "+str(main_params)+"\n")
		for par in simulation.multicell.param :
			out_file.write(par.tclname+"\n")
		i=0
		for item in factorial :
			out_file.write(final_mangle[i]+" "+item+"\n")
			i = i + 1
		out_file.close()
		commands.getoutput("rm -rf "+simulation.factorial2kr_save)
		commands.getoutput("mkdir "+simulation.factorial2kr_save)
		# Call the factorial2kr program for each response variable
		for response in responses :
			commands.getoutput(simulation.factorial2kr_path+" "+name+" -o "+response+" -r residual_"+response+".dat -q quantile_"+response+".dat >> "+response+".dat")
		commands.getoutput("mv *.dat "+simulation.factorial2kr_save)
		commands.getoutput("rm "+name)
		
try:
	main()

except KeyboardInterrupt:
	# Interrupted by the user
	print "Interrupted by user"

except xml.sax._exceptions.SAXParseException:
	# The syntax of the xml file is not correct
	print "XML file is not correct"
	
except TypeError:
	# The simulation environment misses some parameters
	print "A base simulation parameter is missing in XML file"

