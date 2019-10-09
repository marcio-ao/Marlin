#!/usr/bin/python

from __future__ import print_function
import re
import os

string_def_w_comment  = re.compile("(\s*)PROGMEM Language_Str (MSG_\w*)(\s*)= (.*?);(\s*)(//.*)")
string_def_wo_comment = re.compile("(\s*)PROGMEM Language_Str (MSG_\w*)(\s*)= (.*);")

variable = "MSG_FILAMENTCHANGE"
#suffixes = ["A","B","C","E","E1","E2","E3","E4","E5","E6"]
suffixes = ["", "E1","E2","E3","E4","E5","E6"]
#suffixes = ["E1","E2","E3","E4","E5","E6"]
#suffixes = ["0","1", "2","3","4","5"]
#suffixes = ["0","1", "2","3","4","5"]
#suffixes = ["H1","H2", "H3", "H4", "H5", "H6"]

def rewrite_string_definition(m, line, suffix1, suffix2):
  spaces = m[2]
  spaces = spaces[:-len(suffix1)-1]
  value  = m[3]
  if len(m) > 4:
    line = '%sPROGMEM Language_Str %s%s= %s;%s%s\n' % (m[0], m[1] + "_" + suffix1, spaces, value + ' " " ' + "MSG_" + suffix2, m[4], m[5])
  else:
    line = '%sPROGMEM Language_Str %s%s= %s;\n'     % (m[0], m[1] + "_" + suffix1, spaces, value + ' " " ' + "MSG_" + suffix2)
  return line

def add_prefix(m, line):
  if m[1] == variable:
    if not "" in suffixes:
      line = ""
    for s in suffixes:
        if s:
          line = line + rewrite_string_definition(m, line, s, s)
  return line

def process_string_definition(line):
  m = re.findall(string_def_w_comment, line)
  if m:
    line = add_prefix(m[0], line)
  else:
    m = re.findall(string_def_wo_comment, line)
    if m:
      line = add_prefix(m[0], line)
  return line

def process_language_file(filename, func):
  os.rename(filename, filename + ".saved")
  outfile = open(filename, "w")
  for line in open(filename + ".saved"):
    new_line = process_string_definition(line)
    print(new_line, end='',file=outfile)
  os.remove(filename + ".saved")


languages = ["an","bg","ca","cz","da","de","en","el-gr","el","es","eu","fi","fr","gl","hr","it","jp-kana","ko_KR","nl","pl","pt-br","pt","ru","sk","test","tr","uk","vi","zh_CN","zh_TW"]

for lang in languages:
  process_language_file('Marlin/src/lcd/language/language_' + lang + '.h', rewrite_string_definition)