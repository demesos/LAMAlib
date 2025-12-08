#!/usr/bin/env python3
#  
# asdent - Indentation checker and fixer for assembler programs
# Supports LAMAlib structured coding keywords
#
# Author: Wil
# Version: 0.17
# December 2025

import sys
import os
import argparse
import glob

# Define the indentation pairs and structures
code_structures = [
    ('if', [('else', 0, 1, 1)], 'endif'),
    ('do', [], 'loop'),
    ('for', [], 'next'),
    ('do_every', [], 'end_every'),
    ('switch', [('case', 0, 255, 1), ('default', 0, 1, 2)], 'endswitch')
]

code_aliases = [
    ['if', 'longif', 'if_A_in', 'if_X_in', 'if_Y_in']
]

dot_structures = [
    ('.if', [('.elseif', 0, 255, 1), ('.else', 0, 1, 2)], '.endif'),
    ('.repeat', [], '.endrep'),
    ('.proc', [], '.endproc'),
    ('.macro', [], '.endmacro'),
    ('.scope', [], '.endscope')
]

dot_aliases = [
    ['.if', '.ifblank', '.ifconst', '.ifdef', '.ifnblank', '.ifndef', '.ifnref', 
     '.ifp02', '.ifp4510', '.ifp816', '.ifpc02', '.ifpdtv', '.ifpsc02', '.ifref'],
    ['.endrep', '.endrepeat']
]

no_indent_keywords = ['.include', 'def_const', 'install_file', '.zeropage', '.code', '.ident', '.import', '.importzp', '.export', '.exportzp']

verbose = False

def read_file(file_path):
    if not os.path.isfile(file_path):
        raise FileNotFoundError(f"The file '{file_path}' does not exist.")
    
    def convert_tabs_to_spaces(line):
        # Initialize an empty result string
        result = []
        column = 0
        for char in line:
            if char == '\t':
                # Calculate the number of spaces needed to reach the next multiple of 8
                spaces_to_add = 8 - (column % 8)
                result.append(' ' * spaces_to_add)
                column += spaces_to_add
            else:
                result.append(char)
                column += 1
        return ''.join(result)
    
    with open(file_path, 'r') as file:
        code_lines = file.readlines()

    # Convert tabs to spaces and strip spaces at the end of lines
    code_lines = [convert_tabs_to_spaces(line).rstrip() for line in code_lines]

    return code_lines

import os

def save_file(file_path, lines):
    # Back up the original file
    backup_file = file_path + '~'
    if os.path.exists(backup_file):
        os.remove(backup_file)
    if os.path.exists(file_path):
        os.rename(file_path, backup_file)
    # Save the lines to the original file path
    with open(file_path, 'w') as file:
        for line in lines:
            file.write(line + '\n')

def printverbose(message):
    global verbose
    if verbose:
        print(message)

def resolve_aliases(word, aliases):
    word_lower = word.lower()  # Convert to lowercase for case insensitivity
    for alias_group in aliases:
        if word_lower in map(str.lower, alias_group):
            return alias_group[0]  # Return the primary alias (in its original case)
    return word    

        
def is_label(word):
    # Split the word by ';' and take the first part
    word = word.split(';')[0].strip()
    # Return whether it is a label
    return word.endswith(':') or (not word.startswith('.') and '=' in word)    

    
def splitFirstWord(string):
    stripped_string = string.strip()
    if not stripped_string:
        return "", ""
    if not stripped_string.startswith('.') and '=' in stripped_string and not stripped_string.startswith("let "):
        equals_index = stripped_string.index('=')
        semicolon_index = stripped_string.find(';')
        
        if semicolon_index == -1 or equals_index < semicolon_index:
            if semicolon_index != -1:
                firstword = stripped_string[:semicolon_index]
                reststring = stripped_string[semicolon_index:]
            else:
                firstword = stripped_string
                reststring = ""
            return firstword, reststring

    split_string = stripped_string.split(' ', 1)
    
    if len(split_string) == 1:
        return split_string[0], ""

    firstword, reststring = split_string
    return firstword, ' ' + reststring


def check_balance_and_indentation(code_lines, filter_plus, filter_minus, structures, aliases):
    stacks = {structure[0]: [] for structure in structures}
    issues = []

    for line_number, line in enumerate(code_lines, start=1):
        stripped_line = line.strip()
        firstword,reststring=splitFirstWord(stripped_line)
        
        if filter_plus and not (stripped_line.startswith(filter_plus) and (is_label(firstword) and reststring.startswith(filter_plus))):
            continue
        if filter_minus and stripped_line.startswith(filter_minus):
            continue

        words = stripped_line.split()

        for word in words:
            if word.startswith(';'):
                break

        firstword,reststring=splitFirstWord(stripped_line)
        if is_label(firstword):
            firstword,reststring=splitFirstWord(reststring)
        original_word = firstword
        word = resolve_aliases(firstword, aliases)     
            
        # Handle each structure
        for structure in structures:
            begin_keyword, middle_keywords, end_keyword = structure

            # Handle begin keywords
            if word == begin_keyword:
                stacks[begin_keyword].append({
                    'indentation': line.index(original_word),
                    'line_number': line_number,
                    'word': original_word,
                    'middle_counts': {mk[0]: 0 for mk in middle_keywords},
                    'sequence': 0
                })

            # Handle end keywords
            elif word == end_keyword:
                if not stacks[begin_keyword]:
                    issue = f"Error: Unmatched '{original_word}' on line {line_number}"
                    print(issue)
                    return False
                stack_entry = stacks[begin_keyword].pop()
                if line.index(original_word) != stack_entry['indentation']:
                    issue = f"Warning: '{original_word}' on line {line_number} may not be at the same indentation level as its matching '{stack_entry['word']}' on line {stack_entry['line_number']}"
                    printverbose(issue)
                    issues.append(issue)

            # Handle middle keywords
            else:
                for mk, min_count, max_count, seq in middle_keywords:
                    if word == mk:
                        if not stacks[begin_keyword]:
                            issue = f"Error: '{original_word}' without matching '{begin_keyword}' on line {line_number}"
                            print(issue)
                            return False
                        stack_entry = stacks[begin_keyword][-1]
                        if line.index(original_word) != stack_entry['indentation']:
                            issue = f"Warning: '{original_word}' on line {line_number} is not at the same indentation level as its matching '{stack_entry['word']}' on line {stack_entry['line_number']}"
                            printverbose(issue)
                            issues.append(issue)

                        # Check sequence
                        if stack_entry['sequence'] > seq:
                            issue = f"Error: '{original_word}' is out of order on line {line_number}"
                            print(issue)
                            return False

                        stack_entry['sequence'] = seq
                        stack_entry['middle_counts'][mk] += 1
                        if stack_entry['middle_counts'][mk] > max_count:
                            issue = f"Error: '{original_word}' appears more than allowed on line {line_number}"
                            print(issue)
                            return False

    # Check for any unmatched opening commands
    for begin_keyword, stack in stacks.items():
        for entry in stack:
            issue = f"Error: Unmatched '{entry['word']}' at indentation level {entry['indentation']} on line {entry['line_number']}"
            print(issue)
            return False

    return True


def format_line(label, indentation, firstword, reststring):
    if label=="":
        formattedstring=indentation*' '
    else:
        formattedstring = label + max(indentation - len(label), 1) * ' '
    formattedstring += firstword + reststring
    return formattedstring.rstrip()

    
def fix_indentation(code_lines, filter_plus, filter_minus, structures, aliases, base_indent, indent):
    formatted_lines = []
    current_indent=base_indent
    for line in code_lines:       
        stripped_line = line.strip()

        if (
            (filter_plus and not stripped_line.startswith(filter_plus)) or
            (filter_minus and stripped_line.startswith(filter_minus)) or
            stripped_line.startswith(';')
        ):
            formatted_lines.append(line)
            continue

        label=""
        firstword,reststring=splitFirstWord(stripped_line)
        if is_label(firstword):
            label=firstword
            firstword,reststring=splitFirstWord(reststring)
            
        word = resolve_aliases(firstword, aliases)            
            
        found=False
        # Handle each structure
        for structure in structures:        
            begin_keyword, middle_keywords, end_keyword = structure
            # Handle begin keywords
            if word == begin_keyword:            
                formatted_lines.append(format_line(label,current_indent,firstword,reststring))
                current_indent+=indent
                found=True
                break
            # Handle end keywords
            elif word == end_keyword:
                current_indent-=indent
                formatted_lines.append(format_line(label,current_indent,firstword,reststring))
                found=True
                break                
            # Handle middle keywords
            else:
                for mk, min_count, max_count, seq in middle_keywords:
                    if word == mk:
                        formatted_lines.append(format_line(label,current_indent-indent,firstword,reststring))
                        found=True
                        break
                if found:
                    break                    
        if not found:
            formatted_lines.append(format_line(label,current_indent,firstword,reststring))                    
    return formatted_lines


def custom_indentation(code_lines, dot_structures, indent, no_indent_keywords):
    def is_dot_command(word):
        if not word.startswith('.'):
            return False
        
        word = resolve_aliases(word, dot_aliases)
        
        for structure in dot_structures:
            start_command, middle_commands, end_command = structure
            if word == start_command or word == end_command:
                return False
            for middle_command in middle_commands:
                if word == middle_command[0]:
                    return False
        
        return True
    
    formatted_lines = []
    for line in code_lines:       
        stripped_line = line.strip()        
        label=""
        firstword,reststring=splitFirstWord(stripped_line)     
        if is_label(firstword):
            label=firstword
            firstword,reststring=splitFirstWord(reststring)        
        if firstword.startswith(tuple(no_indent_keywords)):
            formatted_lines.append(format_line(label,0,firstword,reststring))
        elif is_dot_command(firstword):        
            formatted_lines.append(format_line(label,indent,firstword,reststring))            
        else:
            formatted_lines.append(line)
    return formatted_lines

def fix_lamalib_include_case(code_lines):
    fixed = False
    new_lines = []
    
    # Target filename to check for, used for case-insensitive search
    lamalib_inc_name = 'lamalib.inc'
    # Correct filename for replacement
    correct_filename = 'LAMAlib.inc'

    for line in code_lines:
        line_lower = line.lower()
        
        # Check if it starts with .include and contains the target filename (case-insensitive)
        if line_lower.strip().startswith('.include') and lamalib_inc_name in line_lower:
            
            # Find the starting index of the case-insensitive filename match
            start_index = line_lower.find(lamalib_inc_name)
            
            # Extract the original substring
            original_substring = line[start_index:start_index + len(lamalib_inc_name)]
            
            # Only fix if the original substring is NOT already correctly cased
            if original_substring != correct_filename:
                # Perform the replacement on the original line
                new_line_content = line[:start_index] + correct_filename + line[start_index + len(lamalib_inc_name):]
                new_lines.append(new_line_content)
                fixed = True
                continue
        
        new_lines.append(line)
        
    if fixed:
        print("Fixed case of LAMAlib include.")
        
    return new_lines


def print_usage():
    usage_text = """
Usage: python asdent.py [-c | -v] <file> [<file2> ... <fileN>]

Options:
    -c       : Check only and show issues
    -v       : Verbose output (shows all misindentations, etc.)
    -? | -h  : Show this usage information
"""
    print(usage_text)

def process_file(file_path):
    global args
    
    # Read the file
    code_lines = read_file(file_path)

    # Perform checks if -c is specified
    if verbose:
        print(f"Processing file: {file_path}")

    code_balanced = check_balance_and_indentation(code_lines, "", ".", code_structures, code_aliases)
    if code_balanced:
        printverbose("Code structures are balanced.")
    else:
        print("Error: Code structures are not balanced.")
        sys.exit(1)

    dot_balanced = check_balance_and_indentation(code_lines, ".", "", dot_structures, dot_aliases)
    if dot_balanced:
        printverbose("Assembler preprocessor instructions are balanced.")
    else:
        print("Error: Assembler preprocessor instructions are not balanced.")
        sys.exit(1)        
        
    if not args.c:
        code_lines = fix_lamalib_include_case(code_lines)        
        # Indentation fixes
        code_lines = fix_indentation(code_lines, "", ".", code_structures, code_aliases,8,4)
        code_lines = fix_indentation(code_lines, ".", "", dot_structures, dot_aliases,0,2)        
        code_lines = custom_indentation(code_lines, dot_structures, 8, no_indent_keywords)                
        save_file(file_path, code_lines)
    return

def main():
    global verbose, args
    
    parser = argparse.ArgumentParser(description="Process and check assembly source files for correct indentation and structure.")
    
    parser.add_argument("file_paths", nargs='+', help="Path(s) to the assembly source file(s). Wildcards are supported.")
    
    parser.add_argument("-c", action="store_true", help="Check only (default with verbose).")
    parser.add_argument("-v", action="store_true", help="Verbose output (shows all misindentations, default with -c).")
    
    args = parser.parse_args()

    verbose = args.c or args.v

    # Expand wildcards for Windows
    all_files = []
    for file_path in args.file_paths:
        matched_files = glob.glob(file_path)
        if not matched_files:
            print(f"Error: No files found matching '{file_path}'")
            sys.exit(1)
        all_files.extend(matched_files)

    # Process each file
    for file_path in all_files:
        if os.path.isfile(file_path):
            process_file(file_path)
        else:
            print(f"Error: {file_path} is not a valid file.")
            sys.exit(1)

if __name__ == "__main__":
    main()