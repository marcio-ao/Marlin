#!/usr/bin/python

from __future__ import print_function
import re
import fnmatch
import os

vars = {}

used = {}

substitutions = {
}

string_def_w_comment  = re.compile("(\s*)#define(\s*)(MSG_\w*)(\s*)(.*?)(\s*)(//.*)")
string_def_wo_comment = re.compile("(\s*)#define(\s*)(MSG_\w*)(\s*)(.*)")
message_variable      = re.compile('MSG_[\w#]*', re.S)
uxgt_collapse         = re.compile('_UxGT\("(.*?)"\) _UxGT\("(.*?)"\)')

def do_code_substitutions(line):
  for key in substitutions:
    line = line.replace(key, substitutions[key])
  return line

def flatten_nested(line):
  line = re.sub(r'\bGET_TEXT\(GET_TEXT\((.*?)\)\)',r'GET_TEXT(\1)',line)
  line = re.sub(r'\bPSTR\(GET_TEXT\((.*?)\)\)',r'GET_TEXT(\1)',line)
  line = re.sub(r'\bF\(GET_TEXT\((.*?)\)\)',r'GET_TEXT_F(\1)',line)
  return line

def modify_list(func, m, new_var, new_val):
  new_var = new_var.replace("MSG_MOVE_E_E","MSG_MOVE_E")
  new_m = list(m)
  remove_spaces = len(new_var) - len(new_m[2])
  new_m[2] = new_var
  if remove_spaces > 0:
    new_m[3] = new_m[3][0:-remove_spaces]
  new_m[4] = new_val
  func(new_m)

def process_string_definition(line, func):
  m = re.findall(string_def_w_comment, line)
  if m:
    line = func(m[0], line)
  else:
    m = re.findall(string_def_wo_comment, line)
    if m:
      line = func(m[0], line)
  return line

def learn_string_definitions(m, line):
  vars[m[2]] = m[4]
  return line

def collapse_strings(value):
  return re.sub(uxgt_collapse, lambda m: '_UxGT("' + m.group(1) + m.group(2) + '")', value)

def do_substitutions(value):
  return re.sub(message_variable, lambda m: vars[m.group()] if m.group() in vars else m.group(), value)

def rewrite_string_definition(m, line):
  value = do_substitutions(m[4])
  value = collapse_strings(value)
  if len(m) > 5:
    value = '%sPROGMEM Language_Str %s%s= %s;%s%s\n' % (m[0], m[2], m[3], value, m[5], m[6])
  else:
    value = '%sPROGMEM Language_Str %s%s= %s;\n' % (m[0], m[2], m[3], value)
  return value


def process_code_substitution(m):
  used[m.group()] = 1
  return "GET_TEXT(" + m.group() + ")" if m.group() in vars else m.group()

def process_code_line(line):
  line = do_code_substitutions(line)
  line = re.sub(message_variable, lambda m: process_code_substitution(m), line)
  return line

def process_language_file(filename, func):
  os.rename(filename, filename + ".saved")
  outfile = open(filename, "w")
  for line in open(filename + ".saved"):
    new_line = process_string_definition(line, func)
    print(new_line, end='',file=outfile)
  os.remove(filename + ".saved")

def process_source_file(filename):
  os.rename(filename, filename + ".saved")
  outfile = open(filename, "w")
  for line in open(filename + ".saved"):
    new_line = process_code_line(line)
    new_line = do_code_substitutions(new_line)
    new_line = flatten_nested(new_line)
    print(new_line, end='',file=outfile)
  os.remove(filename + ".saved")

languages = ["de","cz"]

process_language_file('Marlin/src/lcd/language/language_en.h', learn_string_definitions)
for lang in languages:
  process_language_file('Marlin/src/lcd/language/language_' + lang + '.h', rewrite_string_definition)

def walk_dirs(dir):
  for root, dirnames, filenames in os.walk(dir):
      for filename in fnmatch.filter(filenames, '*.cpp'):
          process_source_file(os.path.join(root, filename))

#walk_dirs('Marlin/src')

for key in vars:
  if not key in used:
    print("Warning: " + key + " does not appear to be used anywhere");