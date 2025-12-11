#!/usr/bin/env python3
"""
expr2asm - Expression to Assembly Translator
Compiles high-level expressions to 6502 assembly using LAMAlib macros

Author: Wil Elmenreich
"""
__version__ = "0.42"

import ply.lex as lex
import ply.yacc as yacc
import argparse
import sys
import re
import shutil
from pathlib import Path



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
                lines.append(f"{tmp}:\t.res 2")
        
        lines.append("; --- End of variable declarations from expr2asm")
        
        return lines

# ============================================================================
# COMMUTATIVE OPTIMIZATION HELPERS
# ============================================================================

def count_instructions(code_lines):
    """Count actual assembly instructions (exclude empty lines and comments)"""
    return len([line for line in code_lines if line and not line.strip().startswith(';')])

def generate_binary_op_code(left, right, op_name):
    """
    Generate assembly code for a binary operation: left op_name right
    Returns a list of assembly instructions
    """
    uses_ax = left.uses_ax or right.uses_ax
    uses_a = left.uses_a or right.uses_a
    uses_x = left.uses_x or right.uses_x
    uses_y = left.uses_y or right.uses_y
    
    if right.is_immediate:
        # right is a constant - use immediate addressing
        code = left.code + [f"{op_name} #{right.value}"]
    else:
        # Check if right is a simple variable
        is_simple_var = (len(right.code) == 1 and 
                        right.code[0].startswith('ldax ') and 
                        not right.code[0].startswith('ldax #'))
        
        if is_simple_var:
            # right is a simple variable
            var_name = right.code[0].split()[1]
            code = left.code + [f"{op_name} {var_name}"]
        else:
            # right is complex - need temp variable
            tmp = codegen.get_temp()
            code = left.code + [f"stax {tmp}"] + right.code + [f"{op_name} {tmp}"]
    
    return Expression(code, uses_ax=uses_ax, uses_a=uses_a, uses_x=uses_x, uses_y=uses_y)

def try_commutative(left, right, op_name):
    """
    Try both orderings of a commutative operation and return the shorter one.
    Implements commutative optimization: tries both (left op right) and (right op left)
    """
    # Generate code for original order: left op right
    expr1 = generate_binary_op_code(left, right, op_name)
    
    # Generate code for reversed order: right op left  
    expr2 = generate_binary_op_code(right, left, op_name)
    
    # Count instructions in each
    count1 = count_instructions(expr1.code)
    count2 = count_instructions(expr2.code)
    
    # Return the shorter one (prefer original on tie)
    return expr1 if count1 <= count2 else expr2

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
    'PEEK', 'PEEKW',
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
    r'(0[xX][0-9a-fA-F]+|\d+)'
    if t.value.startswith(('0x', '0X')):
        t.value = int(t.value, 16)
    else:
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
    """Represents an expression with generated assembly code"""
    
    def __init__(self, code, is_immediate=False, value=None, uses_ax=False, uses_a=False, uses_x=False, uses_y=False, tree=None):
        self.code = code if isinstance(code, list) else [code]
        self.is_immediate = is_immediate  # True if this is a literal number
        self.value = value                # The numeric value if immediate
        self.uses_ax = uses_ax            # Expression uses AX register as source
        self.uses_a = uses_a              # Expression uses A register as source
        self.uses_x = uses_x              # Expression uses X register as source
        self.uses_y = uses_y              # Expression uses Y register as source
        self.tree = tree                  # Optional ExprTree for commutative optimization

class ExprTree:
    """
    Represents an expression tree for commutative optimization.
    Allows exploring all 2^n orderings before generating code.
    """
    def __init__(self, node_type, left=None, right=None, op=None, value=None, is_commutative=False):
        self.node_type = node_type  # 'constant', 'variable', 'register', 'binop', 'unary'
        self.left = left            # Left operand (ExprTree or None)
        self.right = right          # Right operand (ExprTree or None)
        self.op = op                # Operation name (e.g., 'addax', 'mul16')
        self.value = value          # For constants or variable names
        self.is_commutative = is_commutative  # Whether this operation is commutative
        
        # Register usage tracking
        self.uses_ax = False
        self.uses_a = False
        self.uses_x = False
        self.uses_y = False
    
    def count_commutative_ops(self):
        """Count number of commutative operations in this tree"""
        count = 1 if self.is_commutative else 0
        if self.left:
            count += self.left.count_commutative_ops()
        if self.right:
            count += self.right.count_commutative_ops()
        return count
    
    def enumerate_variants(self):
        """
        Generate all 2^n variants where n = number of commutative operations.
        Returns a list of ExprTree objects, each representing a different ordering.
        """
        import copy
        
        n_commutative = self.count_commutative_ops()
        if n_commutative == 0:
            return [self]
        
        variants = []
        # Generate 2^n combinations (0 = original order, 1 = swapped)
        for i in range(2 ** n_commutative):
            variant = copy.deepcopy(self)
            variant._apply_swap_pattern(i, [0])  # [0] is a mutable counter
            variants.append(variant)
        
        return variants
    
    def _apply_swap_pattern(self, pattern, counter):
        """
        Apply a swap pattern to commutative operations.
        pattern: integer representing which ops to swap (bit pattern)
        counter: mutable list with current bit position
        """
        # Process children first (depth-first)
        if self.left:
            self.left._apply_swap_pattern(pattern, counter)
        if self.right:
            self.right._apply_swap_pattern(pattern, counter)
        
        # If this node is commutative, check if we should swap
        if self.is_commutative and self.left and self.right:
            bit_position = counter[0]
            counter[0] += 1
            
            # Check if this bit is set in the pattern
            if (pattern >> bit_position) & 1:
                # Swap operands
                self.left, self.right = self.right, self.left
    
    def to_expression(self):
        """Convert this ExprTree to an Expression with generated code"""
        if self.node_type == 'constant':
            return Expression([f"ldax #{self.value}"], is_immediate=True, value=self.value)
        
        elif self.node_type == 'variable':
            return Expression([f"ldax {self.value}"])
        
        elif self.node_type == 'register':
            # Register reference - generate restore code
            if self.value == 'ax':
                return Expression(["restore AX"], uses_ax=True)
            elif self.value == 'a':
                return Expression(["restore A", "ldx #0"], uses_a=True)
            elif self.value == 'x':
                return Expression(["restore X", "txa", "ldx #0"], uses_x=True)
            elif self.value == 'y':
                return Expression(["restore Y", "tya", "ldx #0"], uses_y=True)
        
        elif self.node_type == 'binop':
            # Binary operation - recursively convert children
            left_expr = self.left.to_expression()
            right_expr = self.right.to_expression()
            
            # Constant folding: if both operands are constants, evaluate at compile time
            if left_expr.is_immediate and right_expr.is_immediate:
                result = None
                if self.op == 'addax':
                    result = (left_expr.value + right_expr.value) & 0xFFFF
                elif self.op == 'subax':
                    result = (left_expr.value - right_expr.value) & 0xFFFF
                elif self.op == 'mul16':
                    result = (left_expr.value * right_expr.value) & 0xFFFF
                elif self.op == 'div16':
                    result = left_expr.value // right_expr.value if right_expr.value != 0 else 0
                elif self.op == 'mod16':
                    result = left_expr.value % right_expr.value if right_expr.value != 0 else 0
                elif self.op == 'andax':
                    result = left_expr.value & right_expr.value
                elif self.op == 'orax':
                    result = left_expr.value | right_expr.value
                elif self.op == 'eorax':
                    result = left_expr.value ^ right_expr.value
                
                if result is not None:
                    return Expression([f"ldax #{result}"], is_immediate=True, value=result)
            
            # Merge register usage
            uses_ax = left_expr.uses_ax or right_expr.uses_ax or self.uses_ax
            uses_a = left_expr.uses_a or right_expr.uses_a or self.uses_a
            uses_x = left_expr.uses_x or right_expr.uses_x or self.uses_x
            uses_y = left_expr.uses_y or right_expr.uses_y or self.uses_y
            
            # Generate code based on operand types
            if right_expr.is_immediate:
                code = left_expr.code + [f"{self.op} #{right_expr.value}"]
            else:
                is_simple_var = (len(right_expr.code) == 1 and 
                                right_expr.code[0].startswith('ldax ') and 
                                not right_expr.code[0].startswith('ldax #'))
                
                if is_simple_var:
                    var_name = right_expr.code[0].split()[1]
                    code = left_expr.code + [f"{self.op} {var_name}"]
                else:
                    tmp = codegen.get_temp()
                    code = left_expr.code + [f"stax {tmp}"] + right_expr.code + [f"{self.op} {tmp}"]
            
            return Expression(code, uses_ax=uses_ax, uses_a=uses_a, uses_x=uses_x, uses_y=uses_y)
        
        return Expression(["nop"])  # Fallback

def expr_to_tree(expr):
    """Convert an Expression to an ExprTree node"""
    if expr.is_immediate:
        return ExprTree('constant', value=expr.value)
    elif len(expr.code) == 1 and expr.code[0].startswith('ldax '):
        # Simple variable load
        var_name = expr.code[0].split()[1]
        if var_name.startswith('#'):
            # It's a constant
            value = int(var_name[1:])
            return ExprTree('constant', value=value)
        else:
            # It's a variable
            return ExprTree('variable', value=var_name)
    elif len(expr.code) >= 1 and expr.code[0] in ['restore AX', 'restore A', 'restore X', 'restore Y']:
        # Register reference
        reg_name = expr.code[0].split()[1].lower()
        return ExprTree('register', value=reg_name)
    else:
        # Complex expression - can't easily convert back to tree
        # Return None to indicate no tree available
        return None

def build_binop_with_tree(left, right, op_name, is_commutative=False):
    """
    Build an Expression with ExprTree for a binary operation.
    Returns Expression with both code and tree.
    """
    # Get or create trees for operands
    left_tree = left.tree if hasattr(left, 'tree') and left.tree else expr_to_tree(left)
    right_tree = right.tree if hasattr(right, 'tree') and right.tree else expr_to_tree(right)
    
    # Build tree if both operands have trees
    if left_tree and right_tree:
        tree = ExprTree('binop', left=left_tree, right=right_tree, op=op_name, is_commutative=is_commutative)
        # Copy register usage from children
        tree.uses_ax = left.uses_ax or right.uses_ax
        tree.uses_a = left.uses_a or right.uses_a
        tree.uses_x = left.uses_x or right.uses_x
        tree.uses_y = left.uses_y or right.uses_y
    else:
        tree = None
    
    return tree

def optimize_commutative(expr):
    """
    Try all 2^n orderings of commutative operations and return the shortest.
    For each variant: generates code, applies constant folding (already done in to_expression),
    runs post-optimizer, then counts instructions.
    Returns the original expression if no tree is available or no optimization found.
    """
    if not hasattr(expr, 'tree') or expr.tree is None:
        return expr
    
    tree = expr.tree
    n_commutative = tree.count_commutative_ops()
    
    if n_commutative == 0:
        return expr  # No commutative ops to optimize
    
    # Save current temp counter
    saved_temp_counter = codegen.temp_counter
    
    best_expr = expr
    best_code = expr.code
    best_count = count_instructions(expr.code)
    best_temp_count = codegen.temp_counter - codegen.temp_start
    
    # Try all 2^n variants
    variants = tree.enumerate_variants()
    
    for variant in variants:
        # Reset temp counter for fair comparison
        codegen.temp_counter = saved_temp_counter
        
        # Generate code for this variant (constant folding happens in to_expression)
        try:
            variant_expr = variant.to_expression()
            
            # Apply post-optimizer to this variant
            optimized_code, _ = optimize_code(variant_expr.code)
            variant_count = count_instructions(optimized_code)
            variant_temp_count = codegen.temp_counter - codegen.temp_start
            
            # Keep if shorter, or same length but uses fewer temps
            if (variant_count < best_count or 
                (variant_count == best_count and variant_temp_count < best_temp_count)):
                best_expr = variant_expr
                best_code = optimized_code
                best_count = variant_count
                best_temp_count = variant_temp_count
        except Exception as e:
            # If variant generation fails, skip it
            pass
    
    # Update best expression with optimized code
    best_expr.code = best_code
    
    # Restore temp counter to match best variant
    codegen.temp_counter = saved_temp_counter + best_temp_count
    
    return best_expr

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
    
    # Apply commutative optimization - try all 2^n orderings
    expr = optimize_commutative(expr)
    
    codegen.add_variable(var_name)
    
    # Set register usage flags for compile_line to use
    codegen._uses_ax = expr.uses_ax
    codegen._uses_a = expr.uses_a
    codegen._uses_x = expr.uses_x
    codegen._uses_y = expr.uses_y
    
    p[0] = expr.code + [f"stax {var_name}"]
    codegen.reset_temps()

def p_statement_let_register_ax(p):
    """statement : LET REG_AX EQUAL expression"""
    expr = p[4]
    
    # Apply commutative optimization - try all 2^n orderings
    expr = optimize_commutative(expr)
    
    # Set register usage flags
    codegen._uses_ax = expr.uses_ax
    codegen._uses_a = expr.uses_a
    codegen._uses_x = expr.uses_x
    codegen._uses_y = expr.uses_y
    
    # Result already in AX, no stax needed
    p[0] = expr.code
    codegen.reset_temps()

def p_statement_let_register_a(p):
    """statement : LET REG_A EQUAL expression"""
    expr = p[4]
    
    # Apply commutative optimization - try all 2^n orderings
    expr = optimize_commutative(expr)
    
    # Set register usage flags
    codegen._uses_ax = expr.uses_ax
    codegen._uses_a = expr.uses_a
    codegen._uses_x = expr.uses_x
    codegen._uses_y = expr.uses_y
    
    # Low byte already in A, no additional code needed
    p[0] = expr.code
    codegen.reset_temps()

def p_statement_let_register_x(p):
    """statement : LET REG_X EQUAL expression"""
    expr = p[4]
    
    # Apply commutative optimization - try all 2^n orderings
    expr = optimize_commutative(expr)
    
    # Set register usage flags
    codegen._uses_ax = expr.uses_ax
    codegen._uses_a = expr.uses_a
    codegen._uses_x = expr.uses_x
    codegen._uses_y = expr.uses_y
    
    # Move low byte from A to X
    p[0] = expr.code + ["tax"]
    codegen.reset_temps()

def p_statement_let_register_y(p):
    """statement : LET REG_Y EQUAL expression"""
    expr = p[4]
    
    # Apply commutative optimization - try all 2^n orderings
    expr = optimize_commutative(expr)
    
    # Set register usage flags
    codegen._uses_ax = expr.uses_ax
    codegen._uses_a = expr.uses_a
    codegen._uses_x = expr.uses_x
    codegen._uses_y = expr.uses_y
    
    # Apply commutative optimization - try all 2^n orderings
    expr = optimize_commutative(expr)
    
    # Set register usage flags
    codegen._uses_ax = expr.uses_ax
    codegen._uses_a = expr.uses_a
    codegen._uses_x = expr.uses_x
    codegen._uses_y = expr.uses_y
    
    # Move low byte from A to Y
    p[0] = expr.code + ["tay"]
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
    
    # Apply commutative optimization - try all 2^n orderings
    expr = optimize_commutative(expr)
    
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
        p[0] = Expression([f"ldax #{result}"], is_immediate=True, value=result)
        return
    
    # Merge register usage flags
    uses_ax = left.uses_ax or right.uses_ax
    uses_a = left.uses_a or right.uses_a
    uses_x = left.uses_x or right.uses_x
    uses_y = left.uses_y or right.uses_y
    
    # Build ExprTree for commutative operations (addition only, not subtraction)
    tree = build_binop_with_tree(left, right, 'addax', is_commutative=(p[2] == '+'))
    
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
        p[0] = Expression([f"ldax #{result}"], is_immediate=True, value=result)
        return
    
    shift_op = "aslax" if p[2] == '<<' else "lsrax"
    code = left.code[:]
    for _ in range(shift_amount):
        code.append(shift_op)
    
    p[0] = Expression(code, uses_ax=uses_ax, uses_a=uses_a, uses_x=uses_x, uses_y=uses_y)

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
        p[0] = Expression([f"ldax #{result}"], is_immediate=True, value=result)
        return
    
    # Merge register usage flags
    uses_ax = left.uses_ax or right.uses_ax
    uses_a = left.uses_a or right.uses_a
    uses_x = left.uses_x or right.uses_x
    uses_y = left.uses_y or right.uses_y
    
    # Build ExprTree for commutative AND operation
    tree = build_binop_with_tree(left, right, 'andax', is_commutative=True)
    
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
        p[0] = Expression([f"ldax #{result}"], is_immediate=True, value=result)
        return
    
    # Merge register usage flags
    uses_ax = left.uses_ax or right.uses_ax
    uses_a = left.uses_a or right.uses_a
    uses_x = left.uses_x or right.uses_x
    uses_y = left.uses_y or right.uses_y
    
    # Build ExprTree for commutative XOR operation
    tree = build_binop_with_tree(left, right, 'eorax', is_commutative=True)
    
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
        p[0] = Expression([f"ldax #{result}"], is_immediate=True, value=result)
        return
    
    # Merge register usage flags
    uses_ax = left.uses_ax or right.uses_ax
    uses_a = left.uses_a or right.uses_a
    uses_x = left.uses_x or right.uses_x
    uses_y = left.uses_y or right.uses_y
    
    # Build ExprTree for commutative OR operation
    tree = build_binop_with_tree(left, right, 'orax', is_commutative=True)
    
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
        p[0] = Expression([f"ldax #{result}"], is_immediate=True, value=result)
        return
    
    # Merge register usage flags
    uses_ax = left.uses_ax or right.uses_ax
    uses_a = left.uses_a or right.uses_a
    uses_x = left.uses_x or right.uses_x
    uses_y = left.uses_y or right.uses_y
    
    op_map = {'*': 'mul16', '/': 'div16', '%': 'mod16'}
    op = op_map[p[2]]
    
    # Build ExprTree for commutative operations (multiplication only, not division/modulo)
    tree = build_binop_with_tree(left, right, op, is_commutative=(p[2] == '*'))
    
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
    p[0] = Expression([f"ldax #{p[1]}"], is_immediate=True, value=p[1])

def p_factor_variable(p):
    """factor : VARIABLE"""
    codegen.reference_variable(p[1])
    p[0] = Expression([f"ldax {p[1]}"])

def p_factor_register_ax(p):
    """factor : REG_AX"""
    # AX register - restore saved value
    p[0] = Expression(["restore AX"], uses_ax=True)

def p_factor_register_a(p):
    """factor : REG_A"""
    # A register - restore and extend to 16-bit
    p[0] = Expression(["restore A", "ldx #0"], uses_a=True)

def p_factor_register_x(p):
    """factor : REG_X"""
    # X register - restore to X, move to A, extend to 16-bit
    p[0] = Expression(["restore X", "txa", "ldx #0"], uses_x=True)

def p_factor_register_y(p):
    """factor : REG_Y"""
    # Y register - restore to Y, move to A, extend to 16-bit
    p[0] = Expression(["restore Y", "tya", "ldx #0"], uses_y=True)

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
                     uses_y=addr_expr.uses_y)

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
    
    # Propagate register usage flags from address expression
    p[0] = Expression(code,
                     uses_ax=addr_expr.uses_ax,
                     uses_a=addr_expr.uses_a,
                     uses_x=addr_expr.uses_x,
                     uses_y=addr_expr.uses_y)

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
    
    # First pass: Remove unnecessary store/restore pairs
    while i < len(lines):
        line = lines[i].strip()
        
        # Optimize 8-bit constant loading for single registers
        # Pattern: ldax #N / tax → ldx #N (for 8-bit constants)
        if i + 1 < len(lines):
            next_line = lines[i + 1].strip()
            
            if line.startswith("ldax #") and next_line == "tax":
                try:
                    value = int(line.split('#')[1])
                    if 0 <= value <= 255:
                        # 8-bit constant - use ldx directly
                        result.append(f"ldx #{value}")
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
                        result.append(f"ldy #{value}")
                        i += 2
                        removed_count += 1
                        continue
                except:
                    pass
        
        # Optimize: ldax #N (when 8-bit) for A register
        # When assigning to A register only, we can use lda #N
        if line.startswith("ldax #"):
            try:
                value = int(line.split('#')[1])
                # Check if this is the last instruction (assignment to A)
                # Look ahead to see if there's no further processing
                is_final = (i + 1 >= len(lines) or 
                           lines[i + 1].strip().startswith(';') or
                           lines[i + 1].strip() == '')
                
                if 0 <= value <= 255 and is_final:
                    # Check previous few lines to see if this is for A register
                    # This is tricky - we'd need context. Skip for now.
                    pass
            except:
                pass
        
        # Check for store followed by restore with intervening instructions
        if line in ["store AX", "store A", "store X", "store Y"]:
            reg = line.split()[1]
            
            # Look for matching restore
            restore_idx = -1
            for j in range(i + 1, len(lines)):
                check_line = lines[j].strip()
                
                # Found matching restore
                if check_line == f"restore {reg}":
                    restore_idx = j
                    break
                
                # Stop if register is used (restored earlier or modified)
                if check_line.startswith(f"restore {reg}"):
                    break
                if reg == "AX" and check_line.startswith("restore AX"):
                    break
                if reg == "A" and check_line.startswith("restore A"):
                    break
                if reg == "X" and check_line.startswith("restore X"):
                    break
                if reg == "Y" and check_line.startswith("restore Y"):
                    break
                
                # Check if intervening instruction modifies or uses the register
                # If only other stores happen, we can eliminate store/restore
                if not check_line.startswith("store ") and check_line:
                    # Something else happens - can't eliminate
                    break
            
            # Check if we can eliminate this store/restore pair
            if restore_idx != -1:
                # Check if only stores happen between
                only_stores = True
                for j in range(i + 1, restore_idx):
                    check_line = lines[j].strip()
                    if check_line and not check_line.startswith("store "):
                        only_stores = False
                        break
                
                if only_stores:
                    # Skip the store, keep everything in between, skip the restore
                    i += 1  # Skip store
                    while i < restore_idx:
                        result.append(lines[i])
                        i += 1
                    i += 1  # Skip restore
                    removed_count += 1
                    continue
        
        # Check for store Y / restore Y with no mul16/div16/mod16 in between
        if line == "store Y":
            restore_idx = -1
            has_mul_div_mod = False
            
            for j in range(i + 1, len(lines)):
                check_line = lines[j].strip()
                
                if check_line == "restore Y":
                    restore_idx = j
                    break
                
                # Check for operations that clobber Y
                if any(op in check_line for op in ["mul16", "div16", "mod16"]):
                    has_mul_div_mod = True
                    break
            
            # If no mul/div/mod between store Y and restore Y, eliminate both
            if restore_idx != -1 and not has_mul_div_mod:
                i += 1  # Skip store Y
                while i < restore_idx:
                    result.append(lines[i])
                    i += 1
                i += 1  # Skip restore Y
                removed_count += 1
                continue
        
        # Check for immediate store/restore patterns (no intervening code)
        if i + 1 < len(lines):
            next_line = lines[i + 1].strip()
            
            # Pattern: store REG / restore REG (adjacent)
            for reg in ["AX", "A", "X", "Y"]:
                if line == f"store {reg}" and next_line == f"restore {reg}":
                    i += 2
                    removed_count += 1
                    continue
            
            # Pattern: stax tempN / ldax tempN (check if temp not used elsewhere)
            if line.startswith("stax tmp") and next_line.startswith("ldax tmp"):
                stax_tmp = line.split()[1]
                ldax_tmp = next_line.split()[1]
                if stax_tmp == ldax_tmp:
                    # Check if this temp is used anywhere else in remaining code
                    temp_used_elsewhere = False
                    for j in range(i + 2, len(lines)):
                        if stax_tmp in lines[j]:
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
    
    return result, removed_count

def compile_line(line, lexer, parser, add_comments=True):
    """Compile a single let statement"""
    try:
        # Parse to get the expression
        parsed_result = parser.parse(line, lexer=lexer)
        
        if parsed_result:
            # Save registers AT THE START if they're used in the expression
            saves = []
            
            # Only save registers that will be referenced in the expression
            # NOT registers that are just being assigned to
            if hasattr(codegen, '_uses_ax') and codegen._uses_ax:
                saves.append("store AX")
            if hasattr(codegen, '_uses_a') and codegen._uses_a:
                saves.append("store A")
            if hasattr(codegen, '_uses_x') and codegen._uses_x:
                saves.append("store X")
            if hasattr(codegen, '_uses_y') and codegen._uses_y:
                saves.append("store Y")
            
            # Clear flags for next compilation
            codegen._uses_ax = False
            codegen._uses_a = False
            codegen._uses_x = False
            codegen._uses_y = False
            
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
    
    # Add variable declarations
    result.extend([line + "\n" for line in codegen.generate_variable_declarations()])
    
    # Optimize: remove redundant store/restore pairs and temp variable pairs
    result, removed = optimize_code(result)
    if removed > 0 and verbose and not quiet:
        print(f"Optimizer removed {removed} redundant operation(s)", file=sys.stderr)
    
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
