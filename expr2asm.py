#!/usr/bin/env python3
#
# expr2asm - Expression to Assembly Translator
# Compiles high-level expressions to 6502 assembly using LAMAlib macros
#
# Author: Wil Elmenreich
# Version: 0.3
# Dezember 2025

import ply.lex as lex
import ply.yacc as yacc
import argparse
import sys
import re
import shutil
from pathlib import Path

__version__ = "0.3"

# ============================================================================
# CODE GENERATOR
# ============================================================================

class CodeGenerator:
    """Manages code generation and variable tracking"""
    
    def __init__(self, temp_start=10, verbose=False):
        self.variables = set()  # All variables
        self.assigned_vars = set()  # Variables assigned to (left side of let)
        self.referenced_vars = set()  # Variables only referenced (right side)
        self.temp_vars = set()
        self.temp_counter = temp_start
        self.temp_start = temp_start
        self.verbose = verbose
    
    def reset_temps(self):
        """Reset temporary counter for new statement"""
        self.temp_counter = self.temp_start
    
    def get_temp(self):
        """Generate a temporary variable name"""
        tmp = f"tmp{self.temp_counter}"
        self.temp_vars.add(tmp)
        self.temp_counter += 1
        return tmp
    
    def add_variable(self, name):
        """Register a variable as assigned"""
        self.variables.add(name)
        self.assigned_vars.add(name)
    
    def reference_variable(self, name):
        """Register a variable as referenced"""
        self.variables.add(name)
        self.referenced_vars.add(name)
    
    def get_warnings(self):
        """Get warnings about variables referenced but never assigned"""
        warnings = []
        for var in sorted(self.referenced_vars - self.assigned_vars):
            warnings.append(f"Warning, variable {var} is referenced but never assigned a value to it.")
        return warnings
    
    def generate_variable_declarations(self):
        """Generate assembly declarations for all variables"""
        lines = []
        lines.append("")
        lines.append("; +++ Variable declarations from expr2asm, all 16-bit")
        for var in sorted(self.variables):
            lines.append(f".ifndef {var}")
            lines.append(f"{var}:\t.res 2")
            lines.append(".endif")
        
        if self.temp_vars:
            lines.append("")
            lines.append("; Temporary variables")
            for tmp in sorted(self.temp_vars):
                lines.append(f".ifndef {tmp}")
                lines.append(f"{tmp}:\t.res 2")
                lines.append(".endif")
        
        lines.append("; --- End of variable declarations from expr2asm")
        
        return lines

# ============================================================================
# LEXER
# ============================================================================

tokens = (
    'NUMBER', 'VARIABLE', 'LET',
    'PLUS', 'MINUS', 'TIMES', 'DIVIDE', 'MODULO',
    'AND', 'OR', 'XOR',
    'LSHIFT', 'RSHIFT',
    'LPAREN', 'RPAREN', 'EQUAL', 'COMMA',
    'PLUSEQ', 'MINUSEQ', 'TIMESEQ', 'DIVEQ', 'MODEQ',
    'ANDEQ', 'OREQ', 'XOREQ',
    'LSHIFTEQ', 'RSHIFTEQ',
    'PEEK', 'PEEKW'
)

# Token rules (compound operators must come before simple ones)
t_PLUSEQ = r'\+='
t_MINUSEQ = r'-='
t_TIMESEQ = r'\*='
t_DIVEQ = r'/='
t_MODEQ = r'%='
t_ANDEQ = r'&='
t_OREQ = r'\|='
t_XOREQ = r'\^='
t_LSHIFTEQ = r'<<='
t_RSHIFTEQ = r'>>='
t_PLUS = r'\+'
t_MINUS = r'-'
t_TIMES = r'\*'
t_DIVIDE = r'/'
t_MODULO = r'%'
t_AND = r'&'
t_OR = r'\|'
t_XOR = r'\^'
t_LSHIFT = r'<<'
t_RSHIFT = r'>>'
t_LPAREN = r'\('
t_RPAREN = r'\)'
t_EQUAL = r'='
t_COMMA = r','
t_ignore = ' \t'

def t_LET(t):
    r'let'
    return t

def t_NUMBER(t):
    r'(0[xX][0-9a-fA-F]+|\d+)'
    if t.value.startswith(('0x', '0X')):
        t.value = int(t.value, 16)
    else:
        t.value = int(t.value)
    return t

def t_VARIABLE(t):
    r'[a-zA-Z_][a-zA-Z0-9_]*'
    # Check if it's a reserved word
    lower = t.value.lower()
    if lower == 'let':
        t.type = 'LET'
    elif lower == 'peekw':
        t.type = 'PEEKW'
    elif lower == 'peek':
        t.type = 'PEEK'
    return t

def t_newline(t):
    r'\n+'
    t.lexer.lineno += len(t.value)

def t_error(t):
    print(f"Illegal character '{t.value[0]}'", file=sys.stderr)
    t.lexer.skip(1)

# ============================================================================
# PARSER
# ============================================================================

class Expression:
    """Represents an expression with generated assembly code"""
    
    def __init__(self, code, is_immediate=False, value=None):
        self.code = code if isinstance(code, list) else [code]
        self.is_immediate = is_immediate  # True if this is a literal number
        self.value = value                # The numeric value if immediate

# Parser precedence rules (lowest to highest)
precedence = (
    ('left', 'OR'),
    ('left', 'XOR'),
    ('left', 'AND'),
    ('left', 'LSHIFT', 'RSHIFT'),
    ('left', 'PLUS', 'MINUS'),
    ('left', 'TIMES', 'DIVIDE', 'MODULO'),
)

# Global codegen instance
codegen = None

# ============================================================================
# GRAMMAR RULES
# ============================================================================

def p_statement_let(p):
    """statement : LET VARIABLE EQUAL expression"""
    var_name = p[2]
    expr = p[4]
    
    codegen.add_variable(var_name)
    p[0] = expr.code + [f"stax {var_name}"]
    codegen.reset_temps()

def p_statement_compound(p):
    """statement : LET VARIABLE PLUSEQ expression
                 | LET VARIABLE MINUSEQ expression
                 | LET VARIABLE TIMESEQ expression
                 | LET VARIABLE DIVEQ expression
                 | LET VARIABLE MODEQ expression
                 | LET VARIABLE ANDEQ expression
                 | LET VARIABLE OREQ expression
                 | LET VARIABLE XOREQ expression
                 | LET VARIABLE LSHIFTEQ expression
                 | LET VARIABLE RSHIFTEQ expression"""
    var_name = p[2]
    op = p[3]
    expr = p[4]
    
    codegen.add_variable(var_name)
    
    # Map compound operators to their base operations
    op_map = {
        '+=': 'addax',
        '-=': 'subax',
        '*=': 'mul16',
        '/=': 'div16',
        '%=': 'mod16',
        '&=': 'andax',
        '|=': 'orax',
        '^=': 'eorax',
    }
    
    # Start by loading the variable
    code = [f"ldax {var_name}"]
    
    if op in ['<<=', '>>=']:
        # Shift operations - need immediate value
        if not expr.is_immediate:
            print(f"Error: Shift compound assignment only supports immediate values", 
                  file=sys.stderr)
            p[0] = []
            return
        
        shift_amount = expr.value
        if shift_amount < 0 or shift_amount > 15:
            print(f"Error: Shift amount must be between 0 and 15 (got {shift_amount})", 
                  file=sys.stderr)
            p[0] = []
            return
        
        # Generate repeated shift instructions
        shift_op = "aslax" if op == '<<=' else "lsrax"
        for _ in range(shift_amount):
            code.append(shift_op)
    else:
        # Regular operations
        base_op = op_map[op]
        
        # Check if expression is immediate
        if expr.is_immediate:
            code.append(f"{base_op} #{expr.value}")
        else:
            # For non-commutative operations (-, /, %), need special handling
            if op in ['-=', '/=', '%=']:
                # Need to save expression result, then load variable, then operate
                code = expr.code[:]
                tmp = codegen.get_temp()
                code.append(f"stax {tmp}")
                code.append(f"ldax {var_name}")
                code.append(f"{base_op} {tmp}")
            else:
                # Commutative or operations that work correctly
                is_simple_var = (len(expr.code) == 1 and 
                                expr.code[0].startswith('ldax ') and 
                                not expr.code[0].startswith('ldax #'))
                
                if is_simple_var:
                    var_ref = expr.code[0].split()[1]
                    code.append(f"{base_op} {var_ref}")
                else:
                    # Complex expression - need temp
                    code.extend(expr.code)
                    tmp = codegen.get_temp()
                    code.append(f"stax {tmp}")
                    code.append(f"ldax {var_name}")
                    code.append(f"{base_op} {tmp}")
    
    # Store result back to variable
    code.append(f"stax {var_name}")
    
    p[0] = code
    codegen.reset_temps()

def p_expression_binop(p):
    """expression : expression PLUS shift_expr
                  | expression MINUS shift_expr"""
    left = p[1]
    right = p[3]
    
    if right.is_immediate:
        op = "addax" if p[2] == '+' else "subax"
        p[0] = Expression(left.code + [f"{op} #{right.value}"])
    else:
        is_simple_var = (len(right.code) == 1 and 
                        right.code[0].startswith('ldax ') and 
                        not right.code[0].startswith('ldax #'))
        
        if is_simple_var:
            var_name = right.code[0].split()[1]
            op = "addax" if p[2] == '+' else "subax"
            p[0] = Expression(left.code + [f"{op} {var_name}"])
        elif p[2] == '-':
            tmp = codegen.get_temp()
            p[0] = Expression(right.code + [f"stax {tmp}"] + 
                             left.code + [f"subax {tmp}"])
        else:
            tmp = codegen.get_temp()
            p[0] = Expression(left.code + [f"stax {tmp}"] + 
                             right.code + [f"addax {tmp}"])

def p_expression_shift(p):
    """expression : shift_expr"""
    p[0] = p[1]

def p_shift_binop(p):
    """shift_expr : shift_expr LSHIFT and_expr
                  | shift_expr RSHIFT and_expr"""
    left = p[1]
    right = p[3]
    
    if not right.is_immediate:
        print(f"Error: Shift operations only support immediate values", 
              file=sys.stderr)
        p[0] = left
        return
    
    shift_amount = right.value
    if shift_amount < 0 or shift_amount > 15:
        print(f"Error: Shift amount must be between 0 and 15 (got {shift_amount})", 
              file=sys.stderr)
        p[0] = left
        return
    
    shift_op = "aslax" if p[2] == '<<' else "lsrax"
    code = left.code[:]
    for _ in range(shift_amount):
        code.append(shift_op)
    
    p[0] = Expression(code)

def p_shift_and(p):
    """shift_expr : and_expr"""
    p[0] = p[1]

def p_and_binop(p):
    """and_expr : and_expr AND xor_expr"""
    left = p[1]
    right = p[3]
    
    if right.is_immediate:
        p[0] = Expression(left.code + [f"andax #{right.value}"])
    else:
        is_simple_var = (len(right.code) == 1 and 
                        right.code[0].startswith('ldax ') and 
                        not right.code[0].startswith('ldax #'))
        if is_simple_var:
            var_name = right.code[0].split()[1]
            p[0] = Expression(left.code + [f"andax {var_name}"])
        else:
            tmp = codegen.get_temp()
            p[0] = Expression(left.code + [f"stax {tmp}"] + 
                             right.code + [f"andax {tmp}"])

def p_and_xor(p):
    """and_expr : xor_expr"""
    p[0] = p[1]

def p_xor_binop(p):
    """xor_expr : xor_expr XOR or_expr"""
    left = p[1]
    right = p[3]
    
    if right.is_immediate:
        p[0] = Expression(left.code + [f"eorax #{right.value}"])
    else:
        is_simple_var = (len(right.code) == 1 and 
                        right.code[0].startswith('ldax ') and 
                        not right.code[0].startswith('ldax #'))
        if is_simple_var:
            var_name = right.code[0].split()[1]
            p[0] = Expression(left.code + [f"eorax {var_name}"])
        else:
            tmp = codegen.get_temp()
            p[0] = Expression(left.code + [f"stax {tmp}"] + 
                             right.code + [f"eorax {tmp}"])

def p_xor_or(p):
    """xor_expr : or_expr"""
    p[0] = p[1]

def p_or_binop(p):
    """or_expr : or_expr OR term"""
    left = p[1]
    right = p[3]
    
    if right.is_immediate:
        p[0] = Expression(left.code + [f"orax #{right.value}"])
    else:
        is_simple_var = (len(right.code) == 1 and 
                        right.code[0].startswith('ldax ') and 
                        not right.code[0].startswith('ldax #'))
        if is_simple_var:
            var_name = right.code[0].split()[1]
            p[0] = Expression(left.code + [f"orax {var_name}"])
        else:
            tmp = codegen.get_temp()
            p[0] = Expression(left.code + [f"stax {tmp}"] + 
                             right.code + [f"orax {tmp}"])

def p_or_term(p):
    """or_expr : term"""
    p[0] = p[1]

def p_term_binop(p):
    """term : term TIMES factor
            | term DIVIDE factor
            | term MODULO factor"""
    left = p[1]
    right = p[3]
    
    op_map = {'*': 'mul16', '/': 'div16', '%': 'mod16'}
    op = op_map[p[2]]
    
    if right.is_immediate:
        p[0] = Expression(left.code + [f"{op} #{right.value}"])
    else:
        is_simple_var = (len(right.code) == 1 and 
                        right.code[0].startswith('ldax ') and 
                        not right.code[0].startswith('ldax #'))
        
        if is_simple_var:
            var_name = right.code[0].split()[1]
            p[0] = Expression(left.code + [f"{op} {var_name}"])
        elif p[2] in ['/', '%']:
            tmp = codegen.get_temp()
            p[0] = Expression(right.code + [f"stax {tmp}"] + 
                             left.code + [f"{op} {tmp}"])
        else:
            tmp = codegen.get_temp()
            p[0] = Expression(left.code + [f"stax {tmp}"] + 
                             right.code + [f"{op} {tmp}"])

def p_term_factor(p):
    """term : factor"""
    p[0] = p[1]

def p_factor_number(p):
    """factor : NUMBER"""
    p[0] = Expression([f"ldax #{p[1]}"], is_immediate=True, value=p[1])

def p_factor_variable(p):
    """factor : VARIABLE"""
    codegen.reference_variable(p[1])
    p[0] = Expression([f"ldax {p[1]}"])

def p_factor_peek(p):
    """factor : PEEK LPAREN expression RPAREN
              | PEEK LPAREN expression COMMA VARIABLE RPAREN"""
    addr_expr = p[3]
    
    # Check if it's a simple constant/variable or complex expression
    is_simple = (addr_expr.is_immediate or 
                 (len(addr_expr.code) == 1 and 
                  addr_expr.code[0].startswith('ldax ') and 
                  not addr_expr.code[0].startswith('ldax #')))
    
    if is_simple:
        # Simple address - use direct peek
        if addr_expr.is_immediate:
            addr = f"#{addr_expr.value}"
        else:
            # Extract variable name from "ldax variable"
            addr = addr_expr.code[0].split()[1]
        
        if len(p) == 5:
            # peek(addr) - result in A, extend to AX
            code = [f"lda {addr}", "ldx #0"]
        else:
            # peek(addr, reg) - specified register
            reg = p[5]
            code = [f"peek {addr},{reg}", "ldx #0"]
    else:
        # Complex expression - address in AX
        code = addr_expr.code[:]
        if len(p) == 5:
            # peek(ax) - result in A, extend to AX
            code.append("peek ax")
            code.append("ldx #0")
        else:
            # peek(ax, reg)
            reg = p[5]
            code.append(f"peek ax,{reg}")
            code.append("ldx #0")
    
    p[0] = Expression(code)

def p_factor_peekw(p):
    """factor : PEEKW LPAREN expression RPAREN"""
    addr_expr = p[3]
    
    # Check if it's a simple constant/variable or complex expression
    is_simple = (addr_expr.is_immediate or 
                 (len(addr_expr.code) == 1 and 
                  addr_expr.code[0].startswith('ldax ') and 
                  not addr_expr.code[0].startswith('ldax #')))
    
    if is_simple:
        # Simple address - use direct peekw
        if addr_expr.is_immediate:
            addr = f"#{addr_expr.value}"
        else:
            # Extract variable name from "ldax variable"
            addr = addr_expr.code[0].split()[1]
        
        # peekw returns 16-bit value in AX directly
        code = [f"peekw {addr}"]
    else:
        # Complex expression - address in AX
        code = addr_expr.code[:]
        code.append("peekw ax")
    
    p[0] = Expression(code)

def p_factor_paren(p):
    """factor : LPAREN expression RPAREN"""
    p[0] = p[2]

def p_error(p):
    if p:
        print(f"Syntax error at '{p.value}'", file=sys.stderr)
    else:
        print("Syntax error at EOF", file=sys.stderr)

# ============================================================================
# COMPILATION FUNCTIONS
# ============================================================================

def compile_line(line, lexer, parser, add_comments=True):
    """Compile a single let statement"""
    try:
        parsed = parser.parse(line, lexer=lexer)
        
        if parsed:
            result = []
            if add_comments:
                result.append(f"; +++ {line}")
            result.extend(parsed)
            if add_comments:
                result.append(f"; --- {line}")
            result.append("")
            return result
        return []
    except Exception as e:
        print(f"Error compiling '{line}': {e}", file=sys.stderr)
        return []

def find_compiled_blocks(lines):
    """Find all compiled blocks marked with +++ and --- comments"""
    blocks = []
    i = 0
    while i < len(lines):
        line = lines[i].rstrip()
        
        # Check for variable declarations block first (more specific)
        if line.startswith('; +++ Variable declarations from expr2asm'):
            start_line = i
            i += 1
            
            # Collect until end marker
            while i < len(lines):
                line = lines[i].rstrip()
                if line.startswith('; --- End of variable declarations from expr2asm'):
                    end_line = i
                    blocks.append({
                        'start': start_line,
                        'end': end_line,
                        'type': 'variables'
                    })
                    break
                i += 1
        elif line.startswith('; +++'):
            # Found start of expression block
            start_line = i
            original_let = line[5:].strip()  # Extract "let ..." from "; +++ let ..."
            i += 1
            code_lines = []
            
            # Collect code until we find the end marker
            while i < len(lines):
                line = lines[i].rstrip()
                if line.startswith('; ---'):
                    end_line = i
                    blocks.append({
                        'start': start_line,
                        'end': end_line,
                        'let_statement': original_let,
                        'code': code_lines,
                        'type': 'expression'
                    })
                    break
                else:
                    code_lines.append(line)
                i += 1
        i += 1
    return blocks

def undo_compilation(lines):
    """Remove compiled code, restore original let statements"""
    blocks = find_compiled_blocks(lines)
    if not blocks:
        return lines, 0
    
    # Process blocks in reverse order to maintain line numbers
    result = lines[:]
    removed_count = 0
    
    for block in reversed(blocks):
        if block['type'] == 'expression':
            # Remove everything from start to end (inclusive)
            # Replace with the original let statement
            result[block['start']:block['end']+1] = [block['let_statement'] + '\n']
            removed_count += 1
        elif block['type'] == 'variables':
            # Remove entire variable declaration block
            del result[block['start']:block['end']+1]
    
    return result, removed_count

def has_uncommented_let_statements(filepath):
    """Check if a file contains uncommented let statements"""
    try:
        with open(filepath, 'r') as f:
            for line in f:
                stripped = line.strip()
                # Check if line starts with 'let ' (after optional whitespace)
                if re.match(r'^let\s', stripped):
                    return True
                # Check if line has whitespace then 'let '
                if re.match(r'^\s+let\s', line):
                    return True
    except Exception:
        return False
    return False

def redo_compilation(lines, lexer, parser, add_comments=True):
    """Recompile existing blocks"""
    blocks = find_compiled_blocks(lines)
    if not blocks:
        return lines, 0, []
    
    result = lines[:]
    recompiled_count = 0
    warnings = []
    
    # Process blocks in reverse order to maintain line numbers
    for block in reversed(blocks):
        if block['type'] == 'expression':
            # Check if end marker has different let statement
            end_line_text = lines[block['end']].rstrip()
            if end_line_text.startswith('; ---'):
                end_let = end_line_text[5:].strip()
                if end_let != block['let_statement']:
                    warnings.append(f"Line {block['start']+1}: End marker has different let statement, will be corrected")
            
            # Recompile
            compiled = compile_line(block['let_statement'], lexer, parser, add_comments)
            if compiled:
                # Add newlines to compiled lines
                compiled_with_newlines = [line + '\n' for line in compiled]
                # Replace entire block with recompiled version
                result[block['start']:block['end']+1] = compiled_with_newlines
                recompiled_count += 1
        # Skip variable blocks - they will be regenerated
    
    return result, recompiled_count, warnings

def process_includes(input_file, lines, output_file, args):
    """Process .include directives and compile included files"""
    modified_lines = []
    compiled_includes = []
    
    for line in lines:
        stripped = line.strip()
        
        # Check for .include "file" or .include 'file'
        match = re.match(r'\.include\s+["\']([^"\']+)["\']', stripped, re.IGNORECASE)
        if match:
            include_file = match.group(1)
            
            # Check if it's a local file (has one of our target extensions)
            local_extensions = ['.s', '.asm', '.i', '.inc']
            include_ext = Path(include_file).suffix.lower()
            
            if include_ext in local_extensions:
                # Try to find the include file
                include_path = None
                base_dir = Path(input_file).parent
                
                # Try relative to input file
                candidate = base_dir / include_file
                if candidate.exists():
                    include_path = candidate
                else:
                    # Try current directory
                    candidate = Path(include_file)
                    if candidate.exists():
                        include_path = candidate
                
                if include_path:
                    # Check if the file contains uncommented let statements
                    if has_uncommented_let_statements(str(include_path)):
                        # Compile the included file
                        output_include = include_path.with_suffix('.asm')
                        try:
                            compile_file(str(include_path), str(output_include), args, is_include=True)
                            
                            # Preserve the directory structure from the original include path
                            include_file_path = Path(include_file)
                            new_include_path = include_file_path.parent / (include_file_path.stem + '.asm')
                            
                            # Change .include to point to .asm file
                            modified_lines.append(f'.include "{new_include_path}"\n')
                            compiled_includes.append(str(include_path))
                        except Exception as e:
                            print(f"Warning: Could not compile include '{include_file}': {e}", file=sys.stderr)
                            modified_lines.append(line)
                    else:
                        # No let statements found, keep original include
                        modified_lines.append(line)
                else:
                    # Include file not found, leave as is
                    modified_lines.append(line)
            else:
                # Not a local file extension, leave as is
                modified_lines.append(line)
        else:
            modified_lines.append(line)
    
    return modified_lines, compiled_includes

def compile_file(input_file, output_file=None, args=None, is_include=False):
    """Main compilation function"""
    global codegen
    
    # Initialize code generator
    temp_start = getattr(args, 'temp_start', 10) if args else 10
    verbose = getattr(args, 'verbose', False) if args else False
    quiet = getattr(args, 'quiet', False) if args else False
    
    codegen = CodeGenerator(temp_start=temp_start, verbose=verbose)
    
    # Build lexer and parser
    lexer = lex.lex()
    parser = yacc.yacc(debug=False, write_tables=False)
    
    # Read input file
    with open(input_file, 'r') as f:
        lines = f.readlines()
    
    # Handle different modes
    if args and args.undo:
        # Undo mode: remove compiled code
        result_lines, count = undo_compilation(lines)
        if not quiet and not is_include:
            print(f"Removed {count} compiled block(s)")
        
        if output_file and not args.dry_run:
            with open(output_file, 'w') as f:
                f.writelines(result_lines)
        elif args.dry_run and not quiet:
            print("".join(result_lines))
        
        return True
    
    if args and args.redo:
        # Redo mode: recompile existing blocks
        result_lines, count, warnings = redo_compilation(lines, lexer, parser, not args.no_comments if args else True)
        if warnings and not quiet:
            for warning in warnings:
                print(f"Warning: {warning}", file=sys.stderr)
        if not quiet and not is_include:
            print(f"Recompiled {count} block(s)")
        
        if output_file and not args.dry_run:
            with open(output_file, 'w') as f:
                f.writelines(result_lines)
        elif args.dry_run and not quiet:
            print("".join(result_lines))
        
        return True
    
    # Normal compilation mode
    # Generate assembly header
    result = []
    result.append("; Generated by expr2asm - Expression to Assembly Translator\n")
    result.append(f"; Source: {Path(input_file).name}\n")
    result.append("\n")
    
    # Process each line
    line_num = 0
    error_count = 0
    add_comments = not (args and hasattr(args, 'no_comments') and args.no_comments)
    
    for line in lines:
        line_num += 1
        stripped = line.strip()
        
        # Skip empty lines
        if not stripped:
            result.append(line)
            continue
        
        # Pass through assembly comments unchanged
        if stripped.startswith(';'):
            result.append(line)
            continue
        
        # Check if this line is a high-level expression (starts with let)
        is_expression = stripped.startswith('let ')
        
        if not is_expression:
            # Pass through raw assembly code unchanged
            result.append(line)
            if verbose and not quiet:
                print(f"; Line {line_num}: Pass-through", file=sys.stderr)
            continue
        
        # This is a high-level expression - compile it
        if verbose and not quiet:
            print(f"; Processing line {line_num}: {stripped}", file=sys.stderr)
        
        compiled = compile_line(stripped, lexer, parser, add_comments)
        if compiled:
            for code_line in compiled:
                result.append(code_line + "\n")
            if verbose and not quiet:
                print(f"; Generated {len(compiled)} lines", file=sys.stderr)
        else:
            error_count += 1
    
    # Add variable declarations
    result.extend([line + "\n" for line in codegen.generate_variable_declarations()])
    
    # Handle includes if requested
    if args and args.compile_includes:
        result, compiled_includes = process_includes(input_file, result, output_file, args)
        if compiled_includes and not quiet:
            print(f"Compiled {len(compiled_includes)} include file(s)")
    
    # Output results
    if args and args.dry_run:
        if not quiet:
            print("".join(result))
        return True
    
    if output_file:
        # Create backup if in-place mode
        if args and args.in_place and not args.no_backup:
            backup_file = input_file + '~'
            shutil.copy2(input_file, backup_file)
            if not quiet and not is_include:
                print(f"Backup saved: {backup_file}")
        
        with open(output_file, 'w') as f:
            f.writelines(result)
        
        if not quiet and not is_include:
            print(f"Successfully compiled {input_file} -> {output_file}")
            print(f"  Variables: {len(codegen.variables)}")
            print(f"  Temporaries: {len(codegen.temp_vars)}")
            
            # Output warnings about referenced but never assigned variables
            warnings = codegen.get_warnings()
            if warnings:
                for warning in warnings:
                    print(warning, file=sys.stderr)
    
    return error_count == 0

# ============================================================================
# MAIN
# ============================================================================

def main():
    parser = argparse.ArgumentParser(
        prog='expr2asm',
        description='Expression to Assembly Translator - Compile high-level expressions to 6502 assembly',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  %(prog)s game.s                    # Compile to game.asm
  %(prog)s game.s -o output.asm      # Specify output file
  %(prog)s game.s -i                 # In-place, backup as game.s~
  %(prog)s game.s -u                 # Undo: remove compiled code
  %(prog)s game.s -r                 # Redo: recompile existing blocks
  %(prog)s game.s -c                 # Compile included files too
  %(prog)s game.s -n                 # Dry-run: preview output
  %(prog)s game.s -t 100             # Start temp variables at tmp100

Author: Wil Elmenreich
Version: %(version)s
        """ % {'prog': 'expr2asm', 'version': __version__}
    )
    
    parser.add_argument('input', help='Input source file (.s)')
    parser.add_argument('-o', '--output', help='Output file (default: input.asm)')
    parser.add_argument('-i', '--in-place', action='store_true',
                        help='Replace source file, backup as .s~')
    parser.add_argument('-u', '--undo', action='store_true',
                        help='Reverse compilation: remove generated assembly')
    parser.add_argument('-r', '--redo', action='store_true',
                        help='Recompile existing blocks')
    parser.add_argument('-c', '--compile-includes', action='store_true',
                        help='Compile included .s files and update includes')
    parser.add_argument('-n', '--dry-run', action='store_true',
                        help='Preview output without writing files')
    parser.add_argument('-v', '--verbose', action='store_true',
                        help='Show detailed compilation info')
    parser.add_argument('-q', '--quiet', action='store_true',
                        help='Suppress non-error output')
    parser.add_argument('--no-backup', action='store_true',
                        help='Don\'t create backup when using -i')
    parser.add_argument('--no-comments', action='store_true',
                        help='Omit source comments in output')
    parser.add_argument('-t', '--temp-start', type=int, default=10,
                        help='Starting temp variable number (default: 10)')
    parser.add_argument('--version', action='version', 
                        version=f'%(prog)s {__version__}')
    
    args = parser.parse_args()
    
    # Validate input
    if not Path(args.input).exists():
        print(f"Error: Input file '{args.input}' not found", file=sys.stderr)
        sys.exit(1)
    
    # Check for conflicting options
    if args.undo and args.redo:
        print("Error: Cannot use -u/--undo and -r/--redo together", file=sys.stderr)
        sys.exit(1)
    
    if args.quiet and args.verbose:
        print("Error: Cannot use -q/--quiet and -v/--verbose together", file=sys.stderr)
        sys.exit(1)
    
    # Determine output file
    if args.in_place:
        output_file = args.input
    elif args.output:
        output_file = args.output
    elif args.undo or args.redo:
        # For undo/redo, default to modifying the input file itself
        output_file = args.input
    else:
        output_file = str(Path(args.input).with_suffix('.asm'))
    
    # Compile
    try:
        success = compile_file(args.input, output_file, args)
        sys.exit(0 if success else 1)
    except Exception as e:
        print(f"Error: {e}", file=sys.stderr)
        if args.verbose:
            import traceback
            traceback.print_exc()
        sys.exit(1)

if __name__ == '__main__':
    main()
