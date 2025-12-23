#!/usr/bin/env python3
"""
exprass - Expression to Assembly Translator
Compiles high-level expressions to 6502 assembly using LAMAlib macros

Author: Wil Elmenreich
December 2025
"""
__version__ = "0.62"

import ply.lex as lex
import ply.yacc as yacc
import argparse
import sys
import re
import shutil
import copy
import itertools
import glob
from pathlib import Path



# ============================================================================
# ASSEMBLY VARIABLE DETECTION
# ============================================================================

def detect_assembly_assignments(line):
    """
    Detect variables that are assigned in raw assembly code.
    Returns a set of variable names that are assigned.
    
    Detects patterns like:
    - for variable,start,to,end
    - for variable,start,downto,end
    - sta variable / sta variable+1
    - stx variable / stx variable+1
    - sty variable / sty variable+1
    - stax variable
    
    Filters out register names (A, X, Y, AX) which should never be variables.
    """
    assigned = set()
    stripped = line.strip()
    
    # Skip comments
    if stripped.startswith(';'):
        return assigned
    
    # Remove inline comments
    comment_pos = stripped.find(';')
    if comment_pos >= 0:
        stripped = stripped[:comment_pos].strip()
    
    if not stripped:
        return assigned
    
    # Reserved register names that should never be variables
    RESERVED_REGISTERS = {'a', 'x', 'y', 'ax'}
    
    # Detect FOR loops: for variable,start,to/downto,end
    for_match = re.match(r'for\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*,', stripped, re.IGNORECASE)
    if for_match:
        var = for_match.group(1)
        # Filter out register names
        if var.lower() not in RESERVED_REGISTERS:
            assigned.add(var)
        return assigned
    
    # Detect store instructions: sta/stx/sty variable (with optional +1)
    store_match = re.match(r'st[axy]\s+([a-zA-Z_$][a-zA-Z0-9_]*)(?:\+1)?', stripped, re.IGNORECASE)
    if store_match:
        var = store_match.group(1)
        # Filter out hex addresses (starting with $ or being pure hex digits)
        if var.startswith('$'):
            return assigned
        # Filter out absolute numeric addresses
        if re.match(r'^[0-9]', var):
            return assigned
        # Filter out register names
        if var.lower() not in RESERVED_REGISTERS:
            assigned.add(var)
        return assigned
    
    # Detect LAMAlib store: stax variable
    stax_match = re.match(r'stax\s+([a-zA-Z_][a-zA-Z0-9_]*)', stripped, re.IGNORECASE)
    if stax_match:
        var = stax_match.group(1)
        # Filter out register names
        if var.lower() not in RESERVED_REGISTERS:
            assigned.add(var)
        return assigned
    
    return assigned

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
        # Register usage flags (set during parsing, used in compile_line)
        self._uses_ax = False
        self._uses_a = False
        self._uses_x = False
        self._uses_y = False
    
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
    
    def generate_variable_declarations(self, wrap_temps=False):
        """Generate assembly declarations for all variables
        
        Args:
            wrap_temps: If True, wrap temp vars in .ifndef/.endif (for temp reuse mode)
        """
        lines = []
        lines.append("")
        lines.append("; +++ Variable declarations from exprass, all 16-bit")
        for var in sorted(self.variables):
            lines.append(f".ifndef {var}")
            lines.append(f"{var}:\t.res 2")
            lines.append(".endif")
        
        if self.temp_vars:
            lines.append("")
            lines.append("; Temporary variables")
            for tmp in sorted(self.temp_vars):
                if wrap_temps:
                    lines.append(f".ifndef {tmp}")
                    lines.append(f"{tmp}:\t.res 2")
                    lines.append(".endif")
                else:
                    lines.append(f"{tmp}:\t.res 2")
        
        lines.append("; --- End of variable declarations from exprass")
        
        return lines

# ============================================================================
# SCORING SYSTEM FOR CODE QUALITY
# ============================================================================

def score_instruction(instr):
    """
    Score a single instruction.
    Lower score is better.
    
    Scoring:
    - ldax, stax: 1 point
    - addax, subax, absax: 3 points
    - mul16: 50 points
    - div16, mod16: 95 points
    - Single 6502 commands (tya, txa, ldx #N, ldy #N, lda #N, tax, tay): 1 point
    - store/restore: 1 point
    - peek, peekw: 1 point
    - All other commands (lsrax, aslax, andax, orax, eorax): 2 points
    """
    instr = instr.strip()
    if not instr or instr.startswith(';'):
        return 0
    
    # Extract the opcode (first word)
    parts = instr.split()
    if not parts:
        return 0
    opcode = parts[0].lower()
    
    # 1-point ops: ldax, stax
    if opcode in ['ldax', 'stax']:
        return 1
    
    # 3-point ops: addax, subax
    if opcode in ['addax', 'subax', 'absax']:
        return 3
    
    # 50-point ops: mul16
    if opcode == 'mul16':
        return 50
    
    # 95-point ops: div16, mod16
    if opcode in ['div16', 'mod16']:
        return 95
    
    # 1-point single 6502 commands
    single_6502 = ['tya', 'txa', 'tax', 'tay', 'ldx', 'ldy', 'lda', 
                   'store', 'restore', 'peek', 'peekw', 'nop', 'rts', 'pha', 'pla']
    if opcode in single_6502:
        return 1
    
    # 2-point: other LAMAlib macros
    other_ops = ['lsrax', 'aslax', 'andax', 'orax', 'eorax', 'negax', 'incax', 'decax']
    if opcode in other_ops:
        return 2
    
    # Default: 1 point for unknown
    return 1

def score_code(code_lines):
    """Score a complete code sequence. Lower is better."""
    total = 0
    for line in code_lines:
        if isinstance(line, str):
            total += score_instruction(line)
    return total

def count_instructions(code_lines):
    """Count actual assembly instructions (exclude empty lines and comments)"""
    return len([line for line in code_lines if line and not line.strip().startswith(';')])

# ============================================================================
# EXPRESSION TREE FOR BRUTEFORCE OPTIMIZATION
# ============================================================================

class ExprNode:
    """
    A node in the expression tree.
    Used to generate all commutative permutations of an expression.
    """
    def __init__(self, node_type, value=None, op=None, children=None, is_commutative=False):
        self.node_type = node_type  # 'const', 'var', 'reg', 'binop', 'unary', 'peek', 'peekw', 'abs'
        self.value = value          # For const: numeric value, for var/reg: name
        self.op = op                # For binop: operation name ('addax', 'mul16', etc.)
        self.children = children or []  # Child nodes
        self.is_commutative = is_commutative  # True for +, *, &, |, ^
        
        # Register usage tracking
        self.uses_ax = False
        self.uses_a = False
        self.uses_x = False
        self.uses_y = False
    
    def clone(self):
        """Deep clone this node"""
        new_node = ExprNode(
            self.node_type,
            value=self.value,
            op=self.op,
            children=[c.clone() for c in self.children],
            is_commutative=self.is_commutative
        )
        new_node.uses_ax = self.uses_ax
        new_node.uses_a = self.uses_a
        new_node.uses_x = self.uses_x
        new_node.uses_y = self.uses_y
        return new_node
    
    def get_commutative_nodes(self):
        """Get all nodes in this tree that are commutative binary ops"""
        result = []
        if self.is_commutative and len(self.children) == 2:
            result.append(self)
        for child in self.children:
            result.extend(child.get_commutative_nodes())
        return result
    
    def enumerate_all_orderings(self):
        """
        Generate all 2^n orderings where n = number of commutative operations.
        Returns list of ExprNode trees representing each ordering.
        """
        comm_nodes = self.get_commutative_nodes()
        n = len(comm_nodes)
        
        if n == 0:
            return [self]
        
        # Generate all 2^n combinations
        variants = []
        for swap_pattern in range(2 ** n):
            variant = self.clone()
            comm_nodes_in_variant = variant.get_commutative_nodes()
            
            for i, node in enumerate(comm_nodes_in_variant):
                if (swap_pattern >> i) & 1:
                    # Swap children
                    node.children[0], node.children[1] = node.children[1], node.children[0]
            
            variants.append(variant)
        
        return variants
    
    def generate_code(self, codegen_instance, reg_temps=None):
        """
        Generate assembly code for this expression tree.
        Returns (code_list, uses_ax, uses_a, uses_x, uses_y)
        
        reg_temps: Optional dict mapping register names to temp variables
                   for registers that are used multiple times
        """
        if reg_temps is None:
            reg_temps = {}
        
        if self.node_type == 'const':
            return ([f"ldax #{self.value}"], False, False, False, False)
        
        elif self.node_type == 'var':
            return ([f"ldax {self.value}"], False, False, False, False)
        
        elif self.node_type == 'reg':
            reg = self.value.lower()
            # Check if this register has a temp variable assigned (multi-use case)
            if reg in reg_temps:
                # Use temp variable instead of restore
                return ([f"ldax {reg_temps[reg]}"], 
                        reg == 'ax', reg == 'a', reg == 'x', reg == 'y')
            
            # Single-use case: use restore
            if reg == 'ax':
                return (["restore AX"], True, False, False, False)
            elif reg == 'a':
                return (["restore A", "ldx #0"], False, True, False, False)
            elif reg == 'x':
                # Use new restore X,to,A pattern
                return (["restore X,to,A", "ldx #0"], False, False, True, False)
            elif reg == 'y':
                # Use new restore Y,to,A pattern
                return (["restore Y,to,A", "ldx #0"], False, False, False, True)
        
        elif self.node_type == 'binop':
            left_code, l_ax, l_a, l_x, l_y = self.children[0].generate_code(codegen_instance, reg_temps)
            right_code, r_ax, r_a, r_x, r_y = self.children[1].generate_code(codegen_instance, reg_temps)
            
            uses_ax = l_ax or r_ax
            uses_a = l_a or r_a
            uses_x = l_x or r_x
            uses_y = l_y or r_y
            
            # Check if right is immediate (single ldax #N)
            right_is_immediate = (len(right_code) == 1 and 
                                  right_code[0].startswith('ldax #'))
            
            # Check if right is simple variable (single ldax var)
            right_is_simple_var = (len(right_code) == 1 and 
                                   right_code[0].startswith('ldax ') and
                                   not right_code[0].startswith('ldax #'))
            
            # Check if left is simple (for non-commutative ops with complex right)
            left_is_immediate = (len(left_code) == 1 and 
                                 left_code[0].startswith('ldax #'))
            left_is_simple_var = (len(left_code) == 1 and 
                                  left_code[0].startswith('ldax ') and
                                  not left_code[0].startswith('ldax #'))
            
            # Non-commutative operations need special handling for operand order
            # subax tmp computes AX - tmp, div16 tmp computes AX / tmp, mod16 tmp computes AX % tmp
            # So we need left in AX and right in tmp
            is_non_commutative = self.op in ('subax', 'div16', 'mod16')
            
            if right_is_immediate:
                # Extract value - left in AX, right as immediate
                val = right_code[0].split('#')[1]
                code = left_code + [f"{self.op} #{val}"]
            elif right_is_simple_var:
                # Extract variable name - left in AX, right from variable
                var = right_code[0].split()[1]
                code = left_code + [f"{self.op} {var}"]
            elif is_non_commutative:
                # Non-commutative with complex right side
                # Need: left in AX, right in tmp
                # So compute right first, store to tmp, then compute left
                tmp = codegen_instance.get_temp()
                code = right_code + [f"stax {tmp}"] + left_code + [f"{self.op} {tmp}"]
            else:
                # Commutative with complex right side
                # Order doesn't matter, compute left first, store, compute right
                tmp = codegen_instance.get_temp()
                code = left_code + [f"stax {tmp}"] + right_code + [f"{self.op} {tmp}"]
            
            return (code, uses_ax, uses_a, uses_x, uses_y)
        
        elif self.node_type == 'peek':
            addr_code, uses_ax, uses_a, uses_x, uses_y = self.children[0].generate_code(codegen_instance, reg_temps)
            
            # Check if address is simple (constant or variable)
            addr_is_simple = (len(addr_code) == 1 and addr_code[0].startswith('ldax '))
            
            # peek only sets A (8-bit), so we need ldx #0 for proper 16-bit result
            if addr_is_simple:
                if addr_code[0].startswith('ldax #'):
                    addr = '#' + addr_code[0].split('#')[1]
                else:
                    addr = addr_code[0].split()[1]
                code = [f"peek {addr}", "ldx #0"]
            else:
                code = addr_code + ["peek ax", "ldx #0"]
            
            return (code, uses_ax, uses_a, uses_x, uses_y)
        
        elif self.node_type == 'peekw':
            addr_code, uses_ax, uses_a, uses_x, uses_y = self.children[0].generate_code(codegen_instance, reg_temps)
            
            addr_is_simple = (len(addr_code) == 1 and addr_code[0].startswith('ldax '))
            
            if addr_is_simple:
                if addr_code[0].startswith('ldax #'):
                    addr = '#' + addr_code[0].split('#')[1]
                else:
                    addr = addr_code[0].split()[1]
                code = [f"peekw {addr}"]
            else:
                code = addr_code + ["peekw ax"]
            
            return (code, uses_ax, uses_a, uses_x, uses_y)
        
        elif self.node_type == 'abs':
            child_code, uses_ax, uses_a, uses_x, uses_y = self.children[0].generate_code(codegen_instance, reg_temps)
            # absax operates on AX register, result in AX
            code = child_code + ["absax"]
            return (code, uses_ax, uses_a, uses_x, uses_y)
        
        elif self.node_type == 'unary':
            child_code, uses_ax, uses_a, uses_x, uses_y = self.children[0].generate_code(codegen_instance, reg_temps)
            code = child_code + [self.op]
            return (code, uses_ax, uses_a, uses_x, uses_y)
        
        return (["nop"], False, False, False, False)
    
    def count_register_refs(self):
        """
        Count how many times each register is referenced in this tree.
        Returns dict: {'ax': count, 'a': count, 'x': count, 'y': count}
        """
        counts = {'ax': 0, 'a': 0, 'x': 0, 'y': 0}
        
        if self.node_type == 'reg':
            reg = self.value.lower()
            if reg in counts:
                counts[reg] = 1
        
        for child in self.children:
            child_counts = child.count_register_refs()
            for reg in counts:
                counts[reg] += child_counts[reg]
        
        return counts
    
    def __repr__(self):
        if self.node_type == 'const':
            return f"Const({self.value})"
        elif self.node_type == 'var':
            return f"Var({self.value})"
        elif self.node_type == 'reg':
            return f"Reg({self.value})"
        elif self.node_type == 'binop':
            return f"BinOp({self.op}, {self.children[0]}, {self.children[1]})"
        elif self.node_type in ['peek', 'peekw', 'abs']:
            return f"{self.node_type.upper()}({self.children[0]})"
        return f"Node({self.node_type})"

# ============================================================================
# BRUTEFORCE OPTIMIZER
# ============================================================================

def bruteforce_optimize(expr_tree, codegen_instance):
    """
    Try all 2^n orderings of commutative operations and return the best code.
    Uses score_code() to evaluate each variant.
    Returns (best_code, uses_ax, uses_a, uses_x, uses_y, reg_temps)
    
    reg_temps is a dict mapping register names to temp variables for
    registers that are used multiple times in the expression.
    """
    variants = expr_tree.enumerate_all_orderings()
    
    best_code = None
    best_score = float('inf')
    best_uses = (False, False, False, False)
    best_reg_temps = {}
    
    # Save temp counter
    saved_temp_counter = codegen_instance.temp_counter
    
    # Count register references to determine which need temp variables
    reg_counts = expr_tree.count_register_refs()
    
    for variant in variants:
        # Reset temp counter for fair comparison
        codegen_instance.temp_counter = saved_temp_counter
        
        # Allocate temp variables for multi-use registers
        reg_temps = {}
        for reg in ['ax', 'a', 'x', 'y']:
            if reg_counts.get(reg, 0) > 1:
                reg_temps[reg] = codegen_instance.get_temp()
        
        try:
            code, uses_ax, uses_a, uses_x, uses_y = variant.generate_code(codegen_instance, reg_temps)
            
            # Apply optimizer to the generated code
            optimized_code, _ = optimize_code(code)
            
            score = score_code(optimized_code)
            
            if score < best_score:
                best_score = score
                best_code = optimized_code
                best_uses = (uses_ax, uses_a, uses_x, uses_y)
                best_reg_temps = reg_temps.copy()
                best_temp_count = codegen_instance.temp_counter - codegen_instance.temp_start
        except Exception as e:
            # Skip variants that fail to generate
            pass
    
    if best_code is None:
        # Fallback to original
        codegen_instance.temp_counter = saved_temp_counter
        
        # Allocate temp variables for multi-use registers
        reg_temps = {}
        for reg in ['ax', 'a', 'x', 'y']:
            if reg_counts.get(reg, 0) > 1:
                reg_temps[reg] = codegen_instance.get_temp()
        
        code, uses_ax, uses_a, uses_x, uses_y = expr_tree.generate_code(codegen_instance, reg_temps)
        return (code, uses_ax, uses_a, uses_x, uses_y, reg_temps)
    
    # Set temp counter to match best variant
    codegen_instance.temp_counter = saved_temp_counter + best_temp_count
    
    return (best_code, best_uses[0], best_uses[1], best_uses[2], best_uses[3], best_reg_temps)

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
    'PEEK', 'PEEKW', 'ABS',
    'REG_A', 'REG_X', 'REG_Y', 'REG_AX'
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
    r'(0[xX][0-9a-fA-F]+|\$[0-9a-fA-F]+|%[01]+|\d+)'
    if t.value.startswith(('0x', '0X')):
        # Hexadecimal with 0x prefix
        t.value = int(t.value, 16)
    elif t.value.startswith('$'):
        # Hexadecimal with $ prefix (6502 style)
        t.value = int(t.value[1:], 16)
    elif t.value.startswith('%'):
        # Binary with % prefix
        t.value = int(t.value[1:], 2)
    else:
        # Decimal
        t.value = int(t.value)
    return t

def t_VARIABLE(t):
    r'[a-zA-Z_][a-zA-Z0-9_]*'
    # Check if it's a reserved word (case-insensitive)
    lower = t.value.lower()
    if lower == 'let':
        t.type = 'LET'
    elif lower == 'peekw':
        t.type = 'PEEKW'
    elif lower == 'peek':
        t.type = 'PEEK'
    elif lower == 'abs':
        t.type = 'ABS'
    elif lower == 'ax':
        t.type = 'REG_AX'
    elif lower == 'a':
        t.type = 'REG_A'
    elif lower == 'x':
        t.type = 'REG_X'
    elif lower == 'y':
        t.type = 'REG_Y'
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
    """Represents an expression with generated assembly code and expression tree"""
    
    def __init__(self, code, is_immediate=False, value=None, uses_ax=False, uses_a=False, uses_x=False, uses_y=False, tree=None):
        self.code = code if isinstance(code, list) else [code]
        self.is_immediate = is_immediate  # True if this is a literal number
        self.value = value                # The numeric value if immediate
        self.uses_ax = uses_ax            # Expression uses AX register as source
        self.uses_a = uses_a              # Expression uses A register as source
        self.uses_x = uses_x              # Expression uses X register as source
        self.uses_y = uses_y              # Expression uses Y register as source
        self.tree = tree                  # ExprNode for bruteforce optimization

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
    
    # Use bruteforce optimization if tree is available
    if expr.tree is not None:
        code, uses_ax, uses_a, uses_x, uses_y, reg_temps = bruteforce_optimize(expr.tree, codegen)
        codegen._uses_ax = uses_ax
        codegen._uses_a = uses_a
        codegen._uses_x = uses_x
        codegen._uses_y = uses_y
        codegen._reg_temps = reg_temps
        p[0] = code + [f"stax {var_name}"]
    else:
        codegen._uses_ax = expr.uses_ax
        codegen._uses_a = expr.uses_a
        codegen._uses_x = expr.uses_x
        codegen._uses_y = expr.uses_y
        codegen._reg_temps = {}
        p[0] = expr.code + [f"stax {var_name}"]
    
    codegen.reset_temps()

def p_statement_let_register_ax(p):
    """statement : LET REG_AX EQUAL expression"""
    expr = p[4]
    
    if expr.tree is not None:
        code, uses_ax, uses_a, uses_x, uses_y, reg_temps = bruteforce_optimize(expr.tree, codegen)
        codegen._uses_ax = uses_ax
        codegen._uses_a = uses_a
        codegen._uses_x = uses_x
        codegen._uses_y = uses_y
        codegen._reg_temps = reg_temps
        p[0] = code
    else:
        codegen._uses_ax = expr.uses_ax
        codegen._uses_a = expr.uses_a
        codegen._uses_x = expr.uses_x
        codegen._uses_y = expr.uses_y
        codegen._reg_temps = {}
        p[0] = expr.code
    
    codegen.reset_temps()

def p_statement_let_register_a(p):
    """statement : LET REG_A EQUAL expression"""
    expr = p[4]
    
    if expr.tree is not None:
        code, uses_ax, uses_a, uses_x, uses_y, reg_temps = bruteforce_optimize(expr.tree, codegen)
        codegen._uses_ax = uses_ax
        codegen._uses_a = uses_a
        codegen._uses_x = uses_x
        codegen._uses_y = uses_y
        codegen._reg_temps = reg_temps
        # Strip trailing ldx #0 - not needed when only A matters
        while code and code[-1].strip() == "ldx #0":
            code = code[:-1]
        p[0] = code
    else:
        codegen._uses_ax = expr.uses_ax
        codegen._uses_a = expr.uses_a
        codegen._uses_x = expr.uses_x
        codegen._uses_y = expr.uses_y
        codegen._reg_temps = {}
        code = expr.code[:]
        # Strip trailing ldx #0
        while code and code[-1].strip() == "ldx #0":
            code = code[:-1]
        p[0] = code
    
    codegen.reset_temps()

def p_statement_let_register_x(p):
    """statement : LET REG_X EQUAL expression"""
    expr = p[4]
    
    if expr.tree is not None:
        code, uses_ax, uses_a, uses_x, uses_y, reg_temps = bruteforce_optimize(expr.tree, codegen)
        codegen._uses_ax = uses_ax
        codegen._uses_a = uses_a
        codegen._uses_x = uses_x
        codegen._uses_y = uses_y
        codegen._reg_temps = reg_temps
        # Strip trailing ldx #0 before tax - ldx #0 is superfluous
        while code and code[-1].strip() == "ldx #0":
            code = code[:-1]
        p[0] = code + ["tax"]
    else:
        codegen._uses_ax = expr.uses_ax
        codegen._uses_a = expr.uses_a
        codegen._uses_x = expr.uses_x
        codegen._uses_y = expr.uses_y
        codegen._reg_temps = {}
        code = expr.code[:]
        while code and code[-1].strip() == "ldx #0":
            code = code[:-1]
        p[0] = code + ["tax"]
    
    codegen.reset_temps()

def p_statement_let_register_y(p):
    """statement : LET REG_Y EQUAL expression"""
    expr = p[4]
    
    if expr.tree is not None:
        code, uses_ax, uses_a, uses_x, uses_y, reg_temps = bruteforce_optimize(expr.tree, codegen)
        codegen._uses_ax = uses_ax
        codegen._uses_a = uses_a
        codegen._uses_x = uses_x
        codegen._uses_y = uses_y
        codegen._reg_temps = reg_temps
        # Strip trailing ldx #0 before tay - ldx #0 is superfluous
        while code and code[-1].strip() == "ldx #0":
            code = code[:-1]
        p[0] = code + ["tay"]
    else:
        codegen._uses_ax = expr.uses_ax
        codegen._uses_a = expr.uses_a
        codegen._uses_x = expr.uses_x
        codegen._uses_y = expr.uses_y
        codegen._reg_temps = {}
        code = expr.code[:]
        while code and code[-1].strip() == "ldx #0":
            code = code[:-1]
        p[0] = code + ["tay"]
    
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
    
    # Constant folding: if both operands are constants, evaluate at compile time
    if left.is_immediate and right.is_immediate:
        if p[2] == '+':
            result = (left.value + right.value) & 0xFFFF
        else:  # '-'
            result = (left.value - right.value) & 0xFFFF
        tree = ExprNode('const', value=result)
        p[0] = Expression([f"ldax #{result}"], is_immediate=True, value=result, tree=tree)
        return
    
    # Merge register usage flags
    uses_ax = left.uses_ax or right.uses_ax
    uses_a = left.uses_a or right.uses_a
    uses_x = left.uses_x or right.uses_x
    uses_y = left.uses_y or right.uses_y
    
    # Build expression tree
    op_name = 'addax' if p[2] == '+' else 'subax'
    is_comm = (p[2] == '+')  # Only addition is commutative
    
    if left.tree and right.tree:
        tree = ExprNode('binop', op=op_name, children=[left.tree, right.tree], is_commutative=is_comm)
        tree.uses_ax = uses_ax
        tree.uses_a = uses_a
        tree.uses_x = uses_x
        tree.uses_y = uses_y
    else:
        tree = None
    
    # Generate code (will be optimized by bruteforce optimizer later)
    if right.is_immediate:
        op = "addax" if p[2] == '+' else "subax"
        p[0] = Expression(left.code + [f"{op} #{right.value}"], 
                         uses_ax=uses_ax, uses_a=uses_a, uses_x=uses_x, uses_y=uses_y, tree=tree)
    else:
        is_simple_var = (len(right.code) == 1 and 
                        right.code[0].startswith('ldax ') and 
                        not right.code[0].startswith('ldax #'))
        
        if is_simple_var:
            var_name = right.code[0].split()[1]
            op = "addax" if p[2] == '+' else "subax"
            p[0] = Expression(left.code + [f"{op} {var_name}"], 
                             uses_ax=uses_ax, uses_a=uses_a, uses_x=uses_x, uses_y=uses_y, tree=tree)
        elif p[2] == '-':
            tmp = codegen.get_temp()
            p[0] = Expression(right.code + [f"stax {tmp}"] + 
                             left.code + [f"subax {tmp}"], 
                             uses_ax=uses_ax, uses_a=uses_a, uses_x=uses_x, uses_y=uses_y, tree=tree)
        else:
            tmp = codegen.get_temp()
            p[0] = Expression(left.code + [f"stax {tmp}"] + 
                             right.code + [f"addax {tmp}"], 
                             uses_ax=uses_ax, uses_a=uses_a, uses_x=uses_x, uses_y=uses_y, tree=tree)

def p_expression_shift(p):
    """expression : shift_expr"""
    p[0] = p[1]

def p_shift_binop(p):
    """shift_expr : shift_expr LSHIFT and_expr
                  | shift_expr RSHIFT and_expr"""
    left = p[1]
    right = p[3]
    
    # Merge register usage flags
    uses_ax = left.uses_ax or right.uses_ax
    uses_a = left.uses_a or right.uses_a
    uses_x = left.uses_x or right.uses_x
    uses_y = left.uses_y or right.uses_y
    
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
    
    # Constant folding: if left operand is constant, evaluate at compile time
    if left.is_immediate:
        if p[2] == '<<':
            result = (left.value << shift_amount) & 0xFFFF
        else:  # '>>'
            result = left.value >> shift_amount
        tree = ExprNode('const', value=result)
        p[0] = Expression([f"ldax #{result}"], is_immediate=True, value=result, tree=tree)
        return
    
    shift_op = "aslax" if p[2] == '<<' else "lsrax"
    code = left.code[:]
    for _ in range(shift_amount):
        code.append(shift_op)
    
    # Build tree for shift
    if left.tree:
        tree = ExprNode('unary', op=shift_op, children=[left.tree])
        for _ in range(shift_amount - 1):
            tree = ExprNode('unary', op=shift_op, children=[tree])
        tree.uses_ax = uses_ax
        tree.uses_a = uses_a
        tree.uses_x = uses_x
        tree.uses_y = uses_y
    else:
        tree = None
    
    p[0] = Expression(code, uses_ax=uses_ax, uses_a=uses_a, uses_x=uses_x, uses_y=uses_y, tree=tree)

def p_shift_and(p):
    """shift_expr : and_expr"""
    p[0] = p[1]

def p_and_binop(p):
    """and_expr : and_expr AND xor_expr"""
    left = p[1]
    right = p[3]
    
    # Constant folding: if both operands are constants, evaluate at compile time
    if left.is_immediate and right.is_immediate:
        result = left.value & right.value
        tree = ExprNode('const', value=result)
        p[0] = Expression([f"ldax #{result}"], is_immediate=True, value=result, tree=tree)
        return
    
    # Merge register usage flags
    uses_ax = left.uses_ax or right.uses_ax
    uses_a = left.uses_a or right.uses_a
    uses_x = left.uses_x or right.uses_x
    uses_y = left.uses_y or right.uses_y
    
    # Build expression tree (AND is commutative)
    if left.tree and right.tree:
        tree = ExprNode('binop', op='andax', children=[left.tree, right.tree], is_commutative=True)
        tree.uses_ax = uses_ax
        tree.uses_a = uses_a
        tree.uses_x = uses_x
        tree.uses_y = uses_y
    else:
        tree = None
    
    if right.is_immediate:
        p[0] = Expression(left.code + [f"andax #{right.value}"],
                         uses_ax=uses_ax, uses_a=uses_a, uses_x=uses_x, uses_y=uses_y, tree=tree)
    else:
        is_simple_var = (len(right.code) == 1 and 
                        right.code[0].startswith('ldax ') and 
                        not right.code[0].startswith('ldax #'))
        if is_simple_var:
            var_name = right.code[0].split()[1]
            p[0] = Expression(left.code + [f"andax {var_name}"],
                             uses_ax=uses_ax, uses_a=uses_a, uses_x=uses_x, uses_y=uses_y, tree=tree)
        else:
            tmp = codegen.get_temp()
            p[0] = Expression(left.code + [f"stax {tmp}"] + 
                             right.code + [f"andax {tmp}"],
                             uses_ax=uses_ax, uses_a=uses_a, uses_x=uses_x, uses_y=uses_y, tree=tree)

def p_and_xor(p):
    """and_expr : xor_expr"""
    p[0] = p[1]

def p_xor_binop(p):
    """xor_expr : xor_expr XOR or_expr"""
    left = p[1]
    right = p[3]
    
    # Constant folding: if both operands are constants, evaluate at compile time
    if left.is_immediate and right.is_immediate:
        result = left.value ^ right.value
        tree = ExprNode('const', value=result)
        p[0] = Expression([f"ldax #{result}"], is_immediate=True, value=result, tree=tree)
        return
    
    # Merge register usage flags
    uses_ax = left.uses_ax or right.uses_ax
    uses_a = left.uses_a or right.uses_a
    uses_x = left.uses_x or right.uses_x
    uses_y = left.uses_y or right.uses_y
    
    # Build expression tree (XOR is commutative)
    if left.tree and right.tree:
        tree = ExprNode('binop', op='eorax', children=[left.tree, right.tree], is_commutative=True)
        tree.uses_ax = uses_ax
        tree.uses_a = uses_a
        tree.uses_x = uses_x
        tree.uses_y = uses_y
    else:
        tree = None
    
    if right.is_immediate:
        p[0] = Expression(left.code + [f"eorax #{right.value}"],
                         uses_ax=uses_ax, uses_a=uses_a, uses_x=uses_x, uses_y=uses_y, tree=tree)
    else:
        is_simple_var = (len(right.code) == 1 and 
                        right.code[0].startswith('ldax ') and 
                        not right.code[0].startswith('ldax #'))
        if is_simple_var:
            var_name = right.code[0].split()[1]
            p[0] = Expression(left.code + [f"eorax {var_name}"],
                             uses_ax=uses_ax, uses_a=uses_a, uses_x=uses_x, uses_y=uses_y, tree=tree)
        else:
            tmp = codegen.get_temp()
            p[0] = Expression(left.code + [f"stax {tmp}"] + 
                             right.code + [f"eorax {tmp}"],
                             uses_ax=uses_ax, uses_a=uses_a, uses_x=uses_x, uses_y=uses_y, tree=tree)

def p_xor_or(p):
    """xor_expr : or_expr"""
    p[0] = p[1]

def p_or_binop(p):
    """or_expr : or_expr OR term"""
    left = p[1]
    right = p[3]
    
    # Constant folding: if both operands are constants, evaluate at compile time
    if left.is_immediate and right.is_immediate:
        result = left.value | right.value
        tree = ExprNode('const', value=result)
        p[0] = Expression([f"ldax #{result}"], is_immediate=True, value=result, tree=tree)
        return
    
    # Merge register usage flags
    uses_ax = left.uses_ax or right.uses_ax
    uses_a = left.uses_a or right.uses_a
    uses_x = left.uses_x or right.uses_x
    uses_y = left.uses_y or right.uses_y
    
    # Build expression tree (OR is commutative)
    if left.tree and right.tree:
        tree = ExprNode('binop', op='orax', children=[left.tree, right.tree], is_commutative=True)
        tree.uses_ax = uses_ax
        tree.uses_a = uses_a
        tree.uses_x = uses_x
        tree.uses_y = uses_y
    else:
        tree = None
    
    if right.is_immediate:
        p[0] = Expression(left.code + [f"orax #{right.value}"],
                         uses_ax=uses_ax, uses_a=uses_a, uses_x=uses_x, uses_y=uses_y, tree=tree)
    else:
        is_simple_var = (len(right.code) == 1 and 
                        right.code[0].startswith('ldax ') and 
                        not right.code[0].startswith('ldax #'))
        if is_simple_var:
            var_name = right.code[0].split()[1]
            p[0] = Expression(left.code + [f"orax {var_name}"],
                             uses_ax=uses_ax, uses_a=uses_a, uses_x=uses_x, uses_y=uses_y, tree=tree)
        else:
            tmp = codegen.get_temp()
            p[0] = Expression(left.code + [f"stax {tmp}"] + 
                             right.code + [f"orax {tmp}"],
                             uses_ax=uses_ax, uses_a=uses_a, uses_x=uses_x, uses_y=uses_y, tree=tree)

def p_or_term(p):
    """or_expr : term"""
    p[0] = p[1]

def p_term_binop(p):
    """term : term TIMES factor
            | term DIVIDE factor
            | term MODULO factor"""
    left = p[1]
    right = p[3]
    
    # Constant folding: if both operands are constants, evaluate at compile time
    if left.is_immediate and right.is_immediate:
        if p[2] == '*':
            result = (left.value * right.value) & 0xFFFF
        elif p[2] == '/':
            if right.value == 0:
                print(f"Error: Division by zero", file=sys.stderr)
                result = 0
            else:
                result = left.value // right.value
        else:  # '%'
            if right.value == 0:
                print(f"Error: Modulo by zero", file=sys.stderr)
                result = 0
            else:
                result = left.value % right.value
        tree = ExprNode('const', value=result)
        p[0] = Expression([f"ldax #{result}"], is_immediate=True, value=result, tree=tree)
        return
    
    # Merge register usage flags
    uses_ax = left.uses_ax or right.uses_ax
    uses_a = left.uses_a or right.uses_a
    uses_x = left.uses_x or right.uses_x
    uses_y = left.uses_y or right.uses_y
    
    op_map = {'*': 'mul16', '/': 'div16', '%': 'mod16'}
    op = op_map[p[2]]
    
    # Build expression tree (only * is commutative)
    is_comm = (p[2] == '*')
    if left.tree and right.tree:
        tree = ExprNode('binop', op=op, children=[left.tree, right.tree], is_commutative=is_comm)
        tree.uses_ax = uses_ax
        tree.uses_a = uses_a
        tree.uses_x = uses_x
        tree.uses_y = uses_y
    else:
        tree = None
    
    if right.is_immediate:
        p[0] = Expression(left.code + [f"{op} #{right.value}"],
                         uses_ax=uses_ax, uses_a=uses_a, uses_x=uses_x, uses_y=uses_y, tree=tree)
    else:
        is_simple_var = (len(right.code) == 1 and 
                        right.code[0].startswith('ldax ') and 
                        not right.code[0].startswith('ldax #'))
        
        if is_simple_var:
            var_name = right.code[0].split()[1]
            p[0] = Expression(left.code + [f"{op} {var_name}"],
                             uses_ax=uses_ax, uses_a=uses_a, uses_x=uses_x, uses_y=uses_y, tree=tree)
        elif p[2] in ['/', '%']:
            tmp = codegen.get_temp()
            p[0] = Expression(right.code + [f"stax {tmp}"] + 
                             left.code + [f"{op} {tmp}"],
                             uses_ax=uses_ax, uses_a=uses_a, uses_x=uses_x, uses_y=uses_y, tree=tree)
        else:
            tmp = codegen.get_temp()
            p[0] = Expression(left.code + [f"stax {tmp}"] + 
                             right.code + [f"{op} {tmp}"],
                             uses_ax=uses_ax, uses_a=uses_a, uses_x=uses_x, uses_y=uses_y, tree=tree)

def p_term_factor(p):
    """term : factor"""
    p[0] = p[1]

def p_factor_number(p):
    """factor : NUMBER"""
    tree = ExprNode('const', value=p[1])
    p[0] = Expression([f"ldax #{p[1]}"], is_immediate=True, value=p[1], tree=tree)

def p_factor_variable(p):
    """factor : VARIABLE"""
    codegen.reference_variable(p[1])
    tree = ExprNode('var', value=p[1])
    p[0] = Expression([f"ldax {p[1]}"], tree=tree)

def p_factor_register_ax(p):
    """factor : REG_AX"""
    tree = ExprNode('reg', value='ax')
    tree.uses_ax = True
    p[0] = Expression(["restore AX"], uses_ax=True, tree=tree)

def p_factor_register_a(p):
    """factor : REG_A"""
    tree = ExprNode('reg', value='a')
    tree.uses_a = True
    p[0] = Expression(["restore A", "ldx #0"], uses_a=True, tree=tree)

def p_factor_register_x(p):
    """factor : REG_X"""
    tree = ExprNode('reg', value='x')
    tree.uses_x = True
    # Use new restore X,to,A pattern
    p[0] = Expression(["restore X,to,A", "ldx #0"], uses_x=True, tree=tree)

def p_factor_register_y(p):
    """factor : REG_Y"""
    tree = ExprNode('reg', value='y')
    tree.uses_y = True
    # Use new restore Y,to,A pattern
    p[0] = Expression(["restore Y,to,A", "ldx #0"], uses_y=True, tree=tree)

def p_factor_peek(p):
    """factor : PEEK LPAREN expression RPAREN
              | PEEK LPAREN expression COMMA VARIABLE RPAREN"""
    addr_expr = p[3]
    
    # Build expression tree for peek
    if addr_expr.tree:
        tree = ExprNode('peek', children=[addr_expr.tree])
        tree.uses_ax = addr_expr.uses_ax
        tree.uses_a = addr_expr.uses_a
        tree.uses_x = addr_expr.uses_x
        tree.uses_y = addr_expr.uses_y
    else:
        tree = None
    
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
            code = [f"peek {addr}"]
        else:
            # peek(addr, reg) - specified register
            reg = p[5]
            code = [f"peek {addr},{reg}"]
    else:
        # Complex expression - address in AX
        code = addr_expr.code[:]
        if len(p) == 5:
            # peek(ax) - result in A
            code.append("peek ax")
        else:
            # peek(ax, reg)
            reg = p[5]
            code.append(f"peek ax,{reg}")
    
    # Propagate register usage flags from address expression
    p[0] = Expression(code, 
                     uses_ax=addr_expr.uses_ax,
                     uses_a=addr_expr.uses_a,
                     uses_x=addr_expr.uses_x,
                     uses_y=addr_expr.uses_y,
                     tree=tree)

def p_factor_peekw(p):
    """factor : PEEKW LPAREN expression RPAREN"""
    addr_expr = p[3]
    
    # Build expression tree for peekw
    if addr_expr.tree:
        tree = ExprNode('peekw', children=[addr_expr.tree])
        tree.uses_ax = addr_expr.uses_ax
        tree.uses_a = addr_expr.uses_a
        tree.uses_x = addr_expr.uses_x
        tree.uses_y = addr_expr.uses_y
    else:
        tree = None
    
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
    
    # Propagate register usage flags from address expression
    p[0] = Expression(code,
                     uses_ax=addr_expr.uses_ax,
                     uses_a=addr_expr.uses_a,
                     uses_x=addr_expr.uses_x,
                     uses_y=addr_expr.uses_y,
                     tree=tree)

def p_factor_abs(p):
    """factor : ABS LPAREN expression RPAREN"""
    expr = p[3]
    
    # Build expression tree for abs
    if expr.tree:
        tree = ExprNode('abs', children=[expr.tree])
        tree.uses_ax = expr.uses_ax
        tree.uses_a = expr.uses_a
        tree.uses_x = expr.uses_x
        tree.uses_y = expr.uses_y
    else:
        tree = None
    
    # ABS always operates on AX and returns result in AX
    code = expr.code[:]
    code.append("absax")
    
    # Propagate register usage flags from input expression
    p[0] = Expression(code,
                     uses_ax=expr.uses_ax,
                     uses_a=expr.uses_a,
                     uses_x=expr.uses_x,
                     uses_y=expr.uses_y,
                     tree=tree)

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

def optimize_code(lines):
    """Post-optimizer to remove redundant store/restore pairs and temp variable pairs"""
    result = []
    i = 0
    removed_count = 0
    
    # Convert strings to list if needed
    if isinstance(lines, str):
        lines = lines.split('\n')
    
    # First pass: Remove unnecessary store/restore pairs
    while i < len(lines):
        line = lines[i].strip() if isinstance(lines[i], str) else lines[i]
        
        # Optimize 8-bit constant loading for single registers
        # Pattern: ldax #N / tax → ldx #N (for 8-bit constants)
        if i + 1 < len(lines):
            next_line = lines[i + 1].strip() if isinstance(lines[i + 1], str) else lines[i + 1]
            
            if line.startswith("ldax #") and next_line == "tax":
                try:
                    value = int(line.split('#')[1])
                    if 0 <= value <= 255:
                        # 8-bit constant - use ldx directly
                        result.append(f"ldx #{value}\n")
                        i += 2
                        removed_count += 1
                        continue
                except:
                    pass
            
            # Pattern: ldax #N / tay → ldy #N (for 8-bit constants)
            if line.startswith("ldax #") and next_line == "tay":
                try:
                    value = int(line.split('#')[1])
                    if 0 <= value <= 255:
                        # 8-bit constant - use ldy directly
                        result.append(f"ldy #{value}\n")
                        i += 2
                        removed_count += 1
                        continue
                except:
                    pass
            
            # Pattern: ldx #0 / tax → tax (ldx #0 is superfluous before tax)
            if line == "ldx #0" and next_line == "tax":
                result.append("tax\n")
                i += 2
                removed_count += 1
                continue
            
            # Pattern: ldx #0 / tay → tay (ldx #0 is superfluous before tay)
            if line == "ldx #0" and next_line == "tay":
                result.append("tay\n")
                i += 2
                removed_count += 1
                continue
        
        # Check for store followed by restore with intervening instructions
        if line in ["store AX", "store A", "store X", "store Y"]:
            reg = line.split()[1]
            
            # Look for matching restore
            restore_idx = -1
            restore_line = None
            for j in range(i + 1, len(lines)):
                check_line = lines[j].strip() if isinstance(lines[j], str) else lines[j]
                
                # Found matching restore (including restore X,to,A patterns)
                if check_line == f"restore {reg}" or check_line.startswith(f"restore {reg},"):
                    restore_idx = j
                    restore_line = check_line
                    break
                
                # Stop if register is used (restored earlier or modified)
                if check_line.startswith(f"restore {reg}"):
                    break
                
                # Check if intervening instruction modifies or uses the register
                # If only other stores happen, we can potentially optimize
                if not check_line.startswith("store ") and check_line:
                    # Something else happens - can't eliminate
                    break
            
            # Check if we can optimize this store/restore pair
            if restore_idx != -1:
                # Check if only stores happen between
                only_stores = True
                for j in range(i + 1, restore_idx):
                    check_line = lines[j].strip() if isinstance(lines[j], str) else lines[j]
                    if check_line and not check_line.startswith("store "):
                        only_stores = False
                        break
                
                if only_stores:
                    # Check what kind of restore it is
                    is_restore_to_a = restore_line.startswith(f"restore {reg},to,")
                    
                    # Skip the store
                    i += 1
                    
                    # Keep everything in between (other stores)
                    while i < restore_idx:
                        result.append(lines[i])
                        i += 1
                    
                    # Handle the restore
                    if is_restore_to_a:
                        # restore X,to,A -> txa; restore Y,to,A -> tya
                        if reg == "X":
                            result.append("txa\n")
                        elif reg == "Y":
                            result.append("tya\n")
                        elif reg == "A":
                            # restore A,to,A is just nothing needed (value already in A)
                            pass
                        elif reg == "AX":
                            # restore AX,to,A doesn't make sense, keep as-is
                            result.append(restore_line)
                    # For plain restore (not to,A), we just skip it entirely
                    
                    i += 1  # Skip restore
                    removed_count += 1
                    continue
        
        # Check for store Y / restore Y with no mul16/div16/mod16 in between
        if line == "store Y":
            restore_idx = -1
            restore_line = None
            has_mul_div_mod = False
            
            for j in range(i + 1, len(lines)):
                check_line = lines[j].strip() if isinstance(lines[j], str) else lines[j]
                
                if check_line == "restore Y" or check_line.startswith("restore Y,"):
                    restore_idx = j
                    restore_line = check_line
                    break
                
                # Check for operations that clobber Y
                if any(op in check_line for op in ["mul16", "div16", "mod16"]):
                    has_mul_div_mod = True
                    break
            
            # If no mul/div/mod between store Y and restore Y, eliminate store and optimize restore
            if restore_idx != -1 and not has_mul_div_mod:
                is_restore_to_a = restore_line.startswith("restore Y,to,")
                
                i += 1  # Skip store Y
                while i < restore_idx:
                    result.append(lines[i])
                    i += 1
                
                # Handle the restore
                if is_restore_to_a:
                    result.append("tya\n")
                # For plain restore Y, just skip it
                
                i += 1  # Skip restore Y
                removed_count += 1
                continue
        
        # Check for immediate store/restore patterns (no intervening code)
        if i + 1 < len(lines):
            next_line = lines[i + 1].strip() if isinstance(lines[i + 1], str) else lines[i + 1]
            
            # Pattern: store REG / restore REG or restore REG,to,A (adjacent)
            matched = False
            for reg in ["AX", "A", "X", "Y"]:
                if line == f"store {reg}":
                    if next_line == f"restore {reg}":
                        # Plain restore - just skip both
                        i += 2
                        removed_count += 1
                        matched = True
                        break
                    elif next_line.startswith(f"restore {reg},to,"):
                        # restore to A - replace with transfer
                        if reg == "X":
                            result.append("txa\n")
                        elif reg == "Y":
                            result.append("tya\n")
                        # For A and AX, the value is already where we need it
                        i += 2
                        removed_count += 1
                        matched = True
                        break
            
            if matched:
                continue
            
            # Pattern: stax tempN / ldax tempN (check if temp not used elsewhere)
            if line.startswith("stax tmp") and next_line.startswith("ldax tmp"):
                stax_tmp = line.split()[1]
                ldax_tmp = next_line.split()[1]
                if stax_tmp == ldax_tmp:
                    # Check if this temp is used anywhere else in remaining code
                    temp_used_elsewhere = False
                    for j in range(i + 2, len(lines)):
                        line_to_check = lines[j] if isinstance(lines[j], str) else lines[j]
                        if stax_tmp in line_to_check:
                            temp_used_elsewhere = True
                            break
                    
                    if not temp_used_elsewhere:
                        # Skip both lines - the value is already in AX
                        i += 2
                        removed_count += 1
                        continue
        
        # Keep this line
        result.append(lines[i])
        i += 1
    
    # If we made changes, run again to catch any newly exposed optimizations
    if removed_count > 0:
        result, additional_removed = optimize_code(result)
        removed_count += additional_removed
    
    return result, removed_count

def renumber_temp_variables(lines, temp_start=10, reuse_temps=True):
    """
    Renumber temp variables in generated code.
    
    Args:
        lines: List of code lines
        temp_start: Starting number for temp variables (default: 10)
        reuse_temps: If True, reset numbering at each expression (default behavior)
                     If False, keep incrementing across all expressions (--no-temp-reuse)
    
    Returns:
        (renumbered_lines, used_temp_set): Tuple of renumbered code and set of temp vars used
    """
    import re
    
    result = []
    temp_mapping = {}  # Maps old temp names to new temp names
    next_temp = temp_start  # Next temp number to assign
    used_temps = set()
    
    # Pattern to match temp variable references like tmp10, tmp11, etc.
    temp_pattern = re.compile(r'\btmp(\d+)\b')
    
    for line in lines:
        line_str = line if isinstance(line, str) else str(line)
        
        # Check for expression boundary (start of new let statement)
        if '; +++ let' in line_str:
            # Reset temp mapping for new expression
            temp_mapping = {}
            if reuse_temps:
                # Reset counter for reuse mode
                next_temp = temp_start
        
        # Find all temp references in this line
        def replace_temp(match):
            nonlocal next_temp
            old_temp = f"tmp{match.group(1)}"
            
            if old_temp not in temp_mapping:
                # Assign a new temp number
                new_temp = f"tmp{next_temp}"
                next_temp += 1
                temp_mapping[old_temp] = new_temp
            
            used_temps.add(temp_mapping[old_temp])
            return temp_mapping[old_temp]
        
        # Replace all temp references
        new_line = temp_pattern.sub(replace_temp, line_str)
        result.append(new_line)
    
    return result, used_temps

def compile_line(line, lexer, parser, add_comments=True):
    """Compile a single let statement"""
    try:
        # Parse to get the expression
        parsed_result = parser.parse(line, lexer=lexer)
        
        if parsed_result:
            # Get reg_temps for multi-use registers (initialized by parser)
            reg_temps = getattr(codegen, '_reg_temps', {})
            
            # Save registers AT THE START if they're used in the expression
            saves = []
            
            # For multi-use registers, save to temp variable instead of store
            # Order matters: save AX/A first, then X, then Y
            if 'ax' in reg_temps:
                saves.append(f"stax {reg_temps['ax']}")
            elif hasattr(codegen, '_uses_ax') and codegen._uses_ax:
                saves.append("store AX")
            
            if 'a' in reg_temps:
                # Save A to 16-bit temp variable
                saves.append(f"sta {reg_temps['a']}")
                saves.append("lda #0")
                saves.append(f"sta {reg_temps['a']}+1")
            elif hasattr(codegen, '_uses_a') and codegen._uses_a:
                saves.append("store A")
            
            if 'x' in reg_temps:
                # Save X to 16-bit temp variable
                saves.append("txa")
                saves.append(f"sta {reg_temps['x']}")
                saves.append("lda #0")
                saves.append(f"sta {reg_temps['x']}+1")
            elif hasattr(codegen, '_uses_x') and codegen._uses_x:
                saves.append("store X")
            
            if 'y' in reg_temps:
                # Save Y to 16-bit temp variable
                saves.append("tya")
                saves.append(f"sta {reg_temps['y']}")
                saves.append("lda #0")
                saves.append(f"sta {reg_temps['y']}+1")
            elif hasattr(codegen, '_uses_y') and codegen._uses_y:
                saves.append("store Y")
            
            # Clear flags for next compilation
            codegen._uses_ax = False
            codegen._uses_a = False
            codegen._uses_x = False
            codegen._uses_y = False
            codegen._reg_temps = {}
            
            result = []
            if add_comments:
                result.append(f"; +++ {line}")
            result.extend(saves)
            result.extend(parsed_result)
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
        if line.startswith('; +++ Variable declarations from exprass'):
            start_line = i
            i += 1
            
            # Collect until end marker
            while i < len(lines):
                line = lines[i].rstrip()
                if line.startswith('; --- End of variable declarations from exprass'):
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
    result.append("; Generated by exprass - Expression to Assembly Translator\n")
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
        
        # Strip trailing comments from the line
        comment_pos = stripped.find(';')
        if comment_pos >= 0:
            stripped_no_comment = stripped[:comment_pos].strip()
        else:
            stripped_no_comment = stripped
        
        # Check if this line is a high-level expression (starts with let)
        is_expression = stripped_no_comment.startswith('let ')
        
        if not is_expression:
            # Pass through raw assembly code unchanged
            result.append(line)
            
            # Detect variables assigned in assembly code
            assigned_vars = detect_assembly_assignments(stripped_no_comment)
            for var in assigned_vars:
                codegen.add_variable(var)
            
            if verbose and not quiet:
                print(f"; Line {line_num}: Pass-through", file=sys.stderr)
            continue
        
        # This is a high-level expression - compile it
        if verbose and not quiet:
            print(f"; Processing line {line_num}: {stripped_no_comment}", file=sys.stderr)
        
        compiled = compile_line(stripped_no_comment, lexer, parser, add_comments)
        if compiled:
            for code_line in compiled:
                result.append(code_line + "\n")
            if verbose and not quiet:
                print(f"; Generated {len(compiled)} lines", file=sys.stderr)
        else:
            error_count += 1
    
    # Add variable declarations (without temps - we'll add them after renumbering)
    # First add just the regular variables
    var_decl_lines = []
    var_decl_lines.append("")
    var_decl_lines.append("; +++ Variable declarations from exprass, all 16-bit")
    for var in sorted(codegen.variables):
        var_decl_lines.append(f".ifndef {var}")
        var_decl_lines.append(f"{var}:\t.res 2")
        var_decl_lines.append(".endif")
    # Placeholder for temp vars - will be filled after renumbering
    var_decl_lines.append("")
    var_decl_lines.append("; Temporary variables")
    var_decl_lines.append("; TEMP_PLACEHOLDER")
    var_decl_lines.append("; --- End of variable declarations from exprass")
    result.extend([line + "\n" for line in var_decl_lines])
    
    # Optimize: remove redundant store/restore pairs and temp variable pairs
    result, removed = optimize_code(result)
    if removed > 0 and verbose and not quiet:
        print(f"Optimizer removed {removed} redundant operation(s)", file=sys.stderr)
    
    # Renumber temp variables
    no_temp_reuse = getattr(args, 'no_temp_reuse', False) if args else False
    result, used_temps = renumber_temp_variables(result, temp_start, reuse_temps=not no_temp_reuse)
    
    # Replace the temp placeholder with actual temp declarations
    wrap_temps = not no_temp_reuse  # Wrap in .ifndef/.endif when reusing temps
    temp_decl_lines = []
    for tmp in sorted(used_temps):
        if wrap_temps:
            temp_decl_lines.append(f".ifndef {tmp}\n")
            temp_decl_lines.append(f"{tmp}:\t.res 2\n")
            temp_decl_lines.append(".endif\n")
        else:
            temp_decl_lines.append(f"{tmp}:\t.res 2\n")
    
    # Find and replace the placeholder
    new_result = []
    for line in result:
        if '; TEMP_PLACEHOLDER' in line:
            new_result.extend(temp_decl_lines)
        else:
            new_result.append(line)
    result = new_result
    
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
            print(f"  Temporaries: {len(used_temps)}")
            
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
        prog='exprass',
        description='Expression to Assembly Translator - Compile high-level expressions to 6502 assembly',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  %(prog)s game.s                    # Compile single file to game.asm
  %(prog)s file1.s file2.s file3.s   # Compile multiple files
  %(prog)s *.s                       # Compile all .s files in directory
  %(prog)s game.s -o output.asm      # Specify output file (single input only)
  %(prog)s game.s -i                 # In-place, backup as game.s~
  %(prog)s game.s -u                 # Undo: remove compiled code
  %(prog)s game.s -r                 # Redo: recompile existing blocks
  %(prog)s game.s -c                 # Compile included .s files too
  %(prog)s file1.s file2.s -c        # Compile multiple files + their includes
  %(prog)s game.s -n                 # Dry-run: preview output
  %(prog)s game.s -t 100             # Start temp variables at tmp100
  %(prog)s game.s --no-temp-reuse    # Unique temp names across expressions

Author: Wil Elmenreich
Version: %(version)s
        """ % {'prog': 'exprass', 'version': __version__}
    )
    
    parser.add_argument('input', nargs='+', help='Input source file(s) (.s)')
    parser.add_argument('-o', '--output', help='Output file (only valid with single input)')
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
    parser.add_argument('--no-temp-reuse', action='store_true',
                        help='Don\'t reuse temp variable names across expressions')
    parser.add_argument('--version', action='version', 
                        version=f'%(prog)s {__version__}')
    
    args = parser.parse_args()
    
    # Expand wildcards (needed for Windows where shell doesn't expand)
    expanded_inputs = []
    for pattern in args.input:
        # Check if pattern contains wildcards
        if '*' in pattern or '?' in pattern or '[' in pattern:
            matches = glob.glob(pattern)
            if matches:
                expanded_inputs.extend(matches)
            else:
                # No matches found - keep original (will error below)
                expanded_inputs.append(pattern)
        else:
            expanded_inputs.append(pattern)
    
    args.input = expanded_inputs
    
    # Validate inputs
    for input_file in args.input:
        if not Path(input_file).exists():
            print(f"Error: Input file '{input_file}' not found", file=sys.stderr)
            sys.exit(1)
    
    # Check for conflicting options
    if args.undo and args.redo:
        print("Error: Cannot use -u/--undo and -r/--redo together", file=sys.stderr)
        sys.exit(1)
    
    if args.quiet and args.verbose:
        print("Error: Cannot use -q/--quiet and -v/--verbose together", file=sys.stderr)
        sys.exit(1)
    
    # Check if -o is used with multiple inputs
    if args.output and len(args.input) > 1:
        print("Error: Cannot use -o/--output with multiple input files", file=sys.stderr)
        sys.exit(1)
    
    # Process each input file
    total_files = len(args.input)
    failed_files = []
    
    for idx, input_file in enumerate(args.input):
        # Show progress for multiple files (unless quiet)
        if total_files > 1 and not args.quiet:
            print(f"\n[{idx+1}/{total_files}] Processing {input_file}...", file=sys.stderr)
        
        # Determine output file
        if args.in_place:
            output_file = input_file
        elif args.output:
            output_file = args.output
        elif args.undo or args.redo:
            # For undo/redo, default to modifying the input file itself
            output_file = input_file
        else:
            output_file = str(Path(input_file).with_suffix('.asm'))
        
        # Compile
        try:
            success = compile_file(input_file, output_file, args)
            if not success:
                failed_files.append(input_file)
        except Exception as e:
            print(f"Error processing {input_file}: {e}", file=sys.stderr)
            if args.verbose:
                import traceback
                traceback.print_exc()
            failed_files.append(input_file)
    
    # Summary for multiple files
    if total_files > 1 and not args.quiet:
        print(f"\n{'='*60}", file=sys.stderr)
        print(f"Summary: Processed {total_files} file(s)", file=sys.stderr)
        if failed_files:
            print(f"Failed: {len(failed_files)} file(s)", file=sys.stderr)
            for f in failed_files:
                print(f"  - {f}", file=sys.stderr)
        else:
            print(f"All files compiled successfully!", file=sys.stderr)
        print(f"{'='*60}", file=sys.stderr)
    
    # Exit with appropriate code
    sys.exit(1 if failed_files else 0)

if __name__ == '__main__':
    main()
