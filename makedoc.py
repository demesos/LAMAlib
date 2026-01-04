#!/usr/bin/env python3
"""
makedoc.py - LAMAlib Documentation Generator (All-in-One)

Generates comprehensive documentation from LAMAlib source files in a single markdown file.
Processes both traditional .inc files and modular .s files.

Usage:
    python makedoc.py [options]

Options:
    --output FILE       Output file (default: LAMAlibdoc.md)
    --include FILES     Space-separated list of .inc files to process
    --modules-dir DIR   Directory containing modules (default: modules/)
    -h, --help          Show this help message

Version: 2.0
Author: Generated for LAMAlib by Claude (Anthropic)
License: The Unlicense (public domain)
"""

import argparse
import re
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import List, Optional, Dict


# ============================================================================
# UTILITY FUNCTIONS
# ============================================================================

def strip_html_tags(text: str) -> str:
    """Remove HTML tags from text"""
    return re.sub(r'<[^>]+>', '', text)


# ============================================================================
# DATA STRUCTURES
# ============================================================================

@dataclass
class Parameter:
    """Module configuration parameter"""
    name: str
    default: Optional[str] = None
    comment: str = ""
    required: bool = False


@dataclass
class ModuleData:
    """Complete module documentation data"""
    name: str = ""
    version: str = ""
    author: str = ""
    description: str = ""
    purpose: str = ""
    features: List[str] = field(default_factory=list)
    usage_instructions: str = ""
    api_reference: str = ""
    parameters: List[Parameter] = field(default_factory=list)
    entry_points: List[str] = field(default_factory=list)
    filename: str = ""


@dataclass
class MacroDoc:
    """Documentation for a macro or function"""
    name: str = ""
    section: str = ""  # Which section this macro belongs to
    syntax: str = ""
    description: List[str] = field(default_factory=list)
    parameters: List[str] = field(default_factory=list)
    returns: str = ""
    registers_modified: str = ""
    notes: List[str] = field(default_factory=list)
    raw_lines: List[str] = field(default_factory=list)


@dataclass
class IncludeFileDoc:
    """Complete documentation from an .inc file"""
    filename: str = ""
    header: List[str] = field(default_factory=list)
    sections: List[tuple] = field(default_factory=list)  # (title, content)
    macros: List[MacroDoc] = field(default_factory=list)


# ============================================================================
# MODULE PARSER
# ============================================================================

class ModuleParser:
    """Parser for LAMAlib module files"""
    
    # Regex patterns
    HEADER_START = re.compile(r';\*\*\*+')  # Lines with many asterisks (;***)
    MODULE_LINE = re.compile(r';\*\s*Module:\s*(.+)')
    VERSION_LINE = re.compile(r';\*\s*Version\s+(.+)')
    AUTHOR_LINE = re.compile(r';\*\s*by\s+(.+)')
    PURPOSE_START = re.compile(r';\*\s*Purpose:')
    FEATURES_START = re.compile(r';\*\s*Features:')
    CONFIG_START = re.compile(r';\*\s*Configuration and Inclusion:')
    USAGE_START = re.compile(r';\*\s*Main Program Usage:')
    COMMENT_LINE = re.compile(r';\*\s*(.*)') # Allow empty lines too
    FEATURE_LINE = re.compile(r';\*\s*-\s*(.+)')
    DEF_CONST = re.compile(
        r'^\s*def_const\s+([A-Z_0-9]+)(?:\s*,\s*(.+?))?\s*(?:;(.+))?$'
    )
    INIT_LABEL = re.compile(r'^init:\s*$')
    RUN_LABEL = re.compile(r'^run:\s*$')
    JUMP_TABLE = re.compile(r'^\s*jmp\s+(\w+)')
    
    def parse(self, filepath: Path) -> ModuleData:
        """Parse a module file and extract documentation"""
        module = ModuleData(filename=filepath.name)
        
        with open(filepath, 'r', encoding='utf-8') as f:
            lines = [line.rstrip('\r\n') for line in f.readlines()]
        
        # Parse in sections
        self._parse_header(lines, module)
        self._parse_parameters(lines, module)
        self._parse_entry_points(lines, module)
        
        return module
    
    def _parse_header(self, lines: List[str], module: ModuleData):
        """Parse the header documentation block"""
        in_header = False
        in_body = False  # After we've passed name/version/author
        current_section = None
        section_content = []
        
        for i, line in enumerate(lines):
            # Detect header start
            if self.HEADER_START.match(line) and not in_header:
                in_header = True
                continue
            
            # Detect header end (second asterisk line, or .include statement)
            if in_header and (self.HEADER_START.match(line) or line.strip().startswith('.include')):
                # Save any pending section
                if current_section:
                    self._save_section(current_section, section_content, module)
                break
            
            if not in_header:
                continue
            
            # Parse module name (only if not already set)
            if not module.name:
                match = self.MODULE_LINE.match(line)
                if match:
                    module.name = match.group(1).strip()
                    continue
            
            # Parse version (only if not already set)
            if not module.version:
                match = self.VERSION_LINE.match(line)
                if match:
                    module.version = match.group(1).strip()
                    continue
            
            # Parse author (only if not already set and not in body)
            if not module.author and not in_body:
                match = self.AUTHOR_LINE.match(line)
                if match:
                    module.author = match.group(1).strip()
                    continue
            
            # Detect section starts (these mark the start of body content)
            if self.PURPOSE_START.match(line):
                in_body = True
                if current_section:
                    self._save_section(current_section, section_content, module)
                current_section = 'purpose'
                section_content = []
                continue
            
            if self.FEATURES_START.match(line):
                in_body = True
                if current_section:
                    self._save_section(current_section, section_content, module)
                current_section = 'features'
                section_content = []
                continue
            
            if self.CONFIG_START.match(line):
                in_body = True
                if current_section:
                    self._save_section(current_section, section_content, module)
                current_section = 'config'
                section_content = []
                continue
            
            if self.USAGE_START.match(line):
                in_body = True
                if current_section:
                    self._save_section(current_section, section_content, module)
                current_section = 'usage'
                section_content = []
                continue
            
            # Extract content based on current section
            if current_section == 'features':
                match = self.FEATURE_LINE.match(line)
                if match:
                    section_content.append(match.group(1).strip())
                # Also capture continuation lines (indented feature descriptions)
                elif line.strip().startswith(';*  ') and section_content:
                    # This is a continuation of the previous feature
                    match = self.COMMENT_LINE.match(line)
                    if match:
                        content = match.group(1).strip()
                        if content:
                            section_content[-1] += ' ' + content
            elif current_section:
                match = self.COMMENT_LINE.match(line)
                if match:
                    content = match.group(1).strip()
                    if content:  # Only add non-empty lines
                        section_content.append(content)
    
    def _save_section(self, section_type: str, content: List[str], module: ModuleData):
        """Save parsed section content to module"""
        text = '\n'.join(content)
        
        if section_type == 'purpose':
            module.purpose = text
        elif section_type == 'features':
            module.features = content
        elif section_type == 'config':
            module.usage_instructions = text
        elif section_type == 'usage':
            module.api_reference = text
    
    def _parse_parameters(self, lines: List[str], module: ModuleData):
        """Parse def_const parameter definitions"""
        in_params = False
        
        for line in lines:
            # Detect parameter section
            if ';* parameters' in line.lower():
                in_params = True
                continue
            
            # Exit parameter section
            if in_params and line.strip().startswith(';***'):
                break
            
            if not in_params:
                continue
            
            # Parse def_const line
            match = self.DEF_CONST.match(line)
            if match:
                name = match.group(1)
                default = match.group(2)
                comment = match.group(3) if match.group(3) else ""
                
                # Clean up default value
                if default:
                    default = default.strip()
                    # Check if it's actually a default or just whitespace
                    if not default:
                        default = None
                
                param = Parameter(
                    name=name,
                    default=default,
                    comment=comment.strip() if comment else "",
                    required=(default is None)
                )
                module.parameters.append(param)
    
    def _parse_entry_points(self, lines: List[str], module: ModuleData):
        """Parse module entry points (init, run, etc.)"""
        for line in lines:
            # Check for init
            if self.INIT_LABEL.match(line):
                if 'init' not in module.entry_points:
                    module.entry_points.append('init')
            
            # Check for run
            if self.RUN_LABEL.match(line):
                if 'run' not in module.entry_points:
                    module.entry_points.append('run')
            
            # Check jump table
            match = self.JUMP_TABLE.match(line)
            if match:
                entry = match.group(1)
                if entry not in module.entry_points and entry not in ['init', 'run']:
                    module.entry_points.append(entry)


# ============================================================================
# INCLUDE PARSER
# ============================================================================

class IncludeParser:
    """Parser for LAMAlib .inc files"""
    
    def parse(self, filepath: Path) -> IncludeFileDoc:
        """Parse an .inc file and extract documentation"""
        doc = IncludeFileDoc(filename=filepath.name)
        
        # Read and process file (handle .include directives)
        lines = self._process_file(filepath)
        
        # Parse header (initial comment block)
        header_end = self._parse_header(lines, doc)
        
        # Parse rest of content (sections and macros)
        self._parse_content(lines[header_end:], doc)
        
        return doc
    
    def _process_file(self, filepath: Path) -> List[str]:
        """Process file and handle .include directives"""
        lines = []
        base_dir = filepath.parent
        
        with open(filepath, 'r', encoding='utf-8') as f:
            for line in f:
                line = line.rstrip('\r\n')
                
                # Handle .include directives
                parts = line.split()
                if parts and parts[0] == '.include':
                    # Extract included filename
                    include_file = parts[1].strip().strip('"')
                    include_path = base_dir / include_file
                    
                    if include_path.exists():
                        with open(include_path, 'r', encoding='utf-8') as inc_f:
                            for inc_line in inc_f:
                                lines.append(inc_line.rstrip('\r\n'))
                    else:
                        # Include file not found, just keep the line
                        lines.append(line)
                else:
                    lines.append(line)
        
        return lines
    
    def _parse_header(self, lines: List[str], doc: IncludeFileDoc) -> int:
        """Parse the header comment block at the top of the file"""
        i = 0
        
        # Skip to first comment line
        while i < len(lines) and not lines[i].startswith(';'):
            i += 1
        
        # Read header comments
        while i < len(lines):
            line = lines[i]
            
            # Stop at first non-comment or empty line
            if not line.startswith(';'):
                if len(line.strip()) == 0:
                    i += 1
                break
            
            # Extract comment content
            content = line.lstrip(';').lstrip(' ')
            doc.header.append(content)
            i += 1
        
        return i
    
    def _parse_content(self, lines: List[str], doc: IncludeFileDoc):
        """Parse the main content (sections and macro documentation)"""
        current_macro = None
        last_line_empty = True
        current_section_title = None
        
        for line in lines:
            # Empty line
            if len(line.strip()) == 0:
                last_line_empty = True
                continue
            
            # Extract comment content
            if line.startswith(';;'):
                content = line.lstrip(';').lstrip(' ')
                
                # Check for section header (HTML tags)
                if content.startswith('<h'):
                    # Extract section title
                    title = re.sub(r'<[^>]+>', '', content).strip()
                    doc.sections.append((title, []))
                    current_section_title = title
                    last_line_empty = False
                    continue
                
                # New macro definition (after blank line)
                if last_line_empty and content:
                    # Save previous macro
                    if current_macro:
                        doc.macros.append(current_macro)
                    
                    # Start new macro
                    current_macro = MacroDoc()
                    current_macro.name = self._extract_macro_name(content)
                    current_macro.section = current_section_title or ""  # Assign to current section
                    current_macro.syntax = content
                    current_macro.raw_lines.append(content)
                else:
                    # Continuation of current macro or section
                    if current_macro:
                        current_macro.raw_lines.append(content)
                        self._parse_macro_line(content, current_macro)
                    elif current_section_title and doc.sections:
                        doc.sections[-1][1].append(content)
                
                last_line_empty = False
            else:
                last_line_empty = False
        
        # Save last macro
        if current_macro:
            doc.macros.append(current_macro)
    
    def _extract_macro_name(self, syntax: str) -> str:
        """Extract the macro name from syntax line"""
        # Strip HTML tags first
        clean_syntax = strip_html_tags(syntax)
        
        # Handle multiple commands separated by semicolon
        cmds = clean_syntax.split(';')
        
        # Take first command and extract the macro name (first word)
        first_cmd = cmds[0].strip()
        words = first_cmd.split()
        
        if words:
            return words[0]
        
        return clean_syntax[:20]  # Fallback
    
    def _parse_macro_line(self, line: str, macro: MacroDoc):
        """Parse individual lines of macro documentation"""
        line_lower = line.lower()
        
        # Accumulate description lines
        if not any(key in line_lower for key in ['result', 'return', 'register', 'modified', 'note']):
            if line and not line.startswith('<'):  # Skip HTML tags
                macro.description.append(line)
        
        # Extract returns information
        if 'result' in line_lower or 'return' in line_lower:
            macro.returns = line
        
        # Extract registers modified
        if 'register' in line_lower and 'modified' in line_lower:
            macro.registers_modified = line
        
        # Extract notes
        if 'note' in line_lower:
            macro.notes.append(line)


# ============================================================================
# COMPREHENSIVE GENERATOR
# ============================================================================

class ComprehensiveGenerator:
    """Generate single comprehensive documentation file"""
    
    def __init__(self):
        self.toc = []  # Table of contents entries
    
    def generate(self, output_path: Path, include_docs: List[IncludeFileDoc], 
                 module_docs: List[ModuleData]):
        """Generate comprehensive documentation in a single file"""
        with open(output_path, 'w', encoding='utf-8') as f:
            # Write title
            f.write("# LAMAlib Documentation\n\n")
            
            # Reserve space for TOC (we'll build it as we go)
            toc_placeholder_pos = f.tell()
            f.write("<!-- TOC_PLACEHOLDER -->\n\n")
            
            # Write header/introduction from first include file
            if include_docs and include_docs[0].header:
                self._write_introduction(f, include_docs[0])
            
            # Write API Reference
            if include_docs:
                self._write_api_reference(f, include_docs)
            
            # Write Modules
            if module_docs:
                self._write_modules(f, module_docs)
            
            # Go back and write the actual TOC
            content = open(output_path, 'r', encoding='utf-8').read()
            toc_md = self._generate_toc_markdown()
            content = content.replace('<!-- TOC_PLACEHOLDER -->', toc_md)
            
            # Write final content
            with open(output_path, 'w', encoding='utf-8') as f2:
                f2.write(content)
    
    def _add_toc_entry(self, title: str, level: int = 1, anchor: str = None):
        """Add entry to table of contents"""
        if anchor is None:
            anchor = self._make_anchor(title)
        
        indent = "  " * (level - 1)
        self.toc.append((level, title, anchor, indent))
    
    def _make_anchor(self, title: str) -> str:
        """Convert title to GitHub-style markdown anchor"""
        # Strip HTML tags first
        title = strip_html_tags(title)
        
        # Convert to lowercase
        anchor = title.lower()
        
        # Replace spaces with hyphens
        anchor = anchor.replace(' ', '-')
        
        # Remove characters that aren't alphanumeric or hyphens
        anchor = ''.join(c for c in anchor if c.isalnum() or c == '-')
        
        # Remove multiple consecutive hyphens
        while '--' in anchor:
            anchor = anchor.replace('--', '-')
        
        # Remove leading/trailing hyphens
        anchor = anchor.strip('-')
        
        return anchor
    
    def _generate_toc_markdown(self) -> str:
        """Generate the table of contents markdown"""
        lines = ["## Table of Contents\n"]
        
        for level, title, anchor, indent in self.toc:
            lines.append(f"{indent}- [{title}](#{anchor})\n")
        
        return ''.join(lines)
    
    def _write_introduction(self, f, first_doc: IncludeFileDoc):
        """Write complete introduction from header comments"""
        for line in first_doc.header:
            # Strip HTML tags but convert entities first
            clean_line = strip_html_tags(line)
            clean_line = self._convert_html_entities(clean_line)
            
            if clean_line.strip():
                # Skip the main title (we already have it)
                if 'LAMAlib' in clean_line and 'Lightweight' in clean_line:
                    continue
                    
                # Detect headers and convert to markdown
                if line.startswith('<h1>'):
                    f.write(f"\n# {clean_line}\n\n")
                elif line.startswith('<h2>'):
                    f.write(f"\n## {clean_line}\n\n")
                elif line.startswith('<h3>'):
                    f.write(f"\n### {clean_line}\n\n")
                else:
                    # Add two spaces before newline for hard line break
                    f.write(f"{clean_line}  \n")
        
        f.write("\n---\n\n")
    
    def _convert_html_entities(self, text: str) -> str:
        """Convert HTML entities to their text equivalents"""
        # Common HTML entities
        text = text.replace('&nbsp;', ' ')
        text = text.replace('&lt;', '<')
        text = text.replace('&gt;', '>')
        text = text.replace('&amp;', '&')
        text = text.replace('&quot;', '"')
        text = text.replace('&#39;', "'")
        return text
    
    def _write_api_reference(self, f, include_docs: List[IncludeFileDoc]):
        """Write complete API reference section"""
        f.write("# API Reference\n\n")
        self._add_toc_entry("API Reference", level=1)
        
        # Collect all macros from all docs
        all_macros = []
        for doc in include_docs:
            all_macros.extend(doc.macros)
        
        # Collect all sections (in order they appear)
        all_sections = []
        sections_seen = set()
        
        for doc in include_docs:
            for section_title, section_content in doc.sections:
                # Clean HTML tags from section title
                clean_title = strip_html_tags(section_title)
                if not clean_title.strip():
                    continue
                
                # Only add each section once (first occurrence)
                if clean_title not in sections_seen:
                    all_sections.append((clean_title, section_content))
                    sections_seen.add(clean_title)
        
        # Process each section
        for section_title, section_content in all_sections:
            # Write section header
            f.write(f"## {section_title}\n\n")
            self._add_toc_entry(section_title, level=2)
            
            # Write section content if any
            if section_content:
                for line in section_content:
                    clean_line = strip_html_tags(line)
                    clean_line = self._convert_html_entities(clean_line)
                    if clean_line.strip():
                        # Add two spaces before newline for hard line break
                        f.write(f"{clean_line}  \n")
                f.write("\n")
            
            # Write macros belonging to this section
            section_macros = [m for m in all_macros if m.section == section_title]
            
            if section_macros:
                # Sort macros alphabetically within the section
                for macro in sorted(section_macros, key=lambda m: m.name.lower()):
                    self._write_macro(f, macro)
        
        # Handle macros that don't belong to any section (orphans)
        orphan_macros = [m for m in all_macros if not m.section or m.section not in sections_seen]
        
        if orphan_macros:
            # Create a catch-all section for orphans
            f.write(f"## Other Macros\n\n")
            self._add_toc_entry("Other Macros", level=2)
            
            for macro in sorted(orphan_macros, key=lambda m: m.name.lower()):
                self._write_macro(f, macro)
        
        f.write("\n---\n\n")
    
    def _get_section_name(self, filename: str) -> str:
        """Convert filename to section name"""
        name = Path(filename).stem
        
        # Handle special cases
        name_map = {
            'LAMAlib': 'Core Macros',
            'LAMAlib-macros16': '16-bit Macros',
            'LAMAlib-sprites': 'Sprite System',
            'LAMAlib-structured': 'Structured Programming',
            'LAMAlib-routines': 'Utility Routines',
            'LAMAlib-strings': 'String Operations',
            'LAMAlib-gfx': 'Graphics Functions',
        }
        
        return name_map.get(name, name)
    
    def _write_macro(self, f, macro):
        """Write documentation for a single macro"""
        # Macro name as subheading (strip HTML tags)
        clean_name = strip_html_tags(macro.name)
        f.write(f"### `{clean_name}`\n\n")
        
        # Syntax (strip HTML tags and convert entities)
        if macro.syntax:
            clean_syntax = strip_html_tags(macro.syntax)
            clean_syntax = self._convert_html_entities(clean_syntax)
            f.write(f"**Syntax:** `{clean_syntax}`\n\n")
        
        # Description (strip HTML tags from each line and convert entities)
        if macro.description:
            for line in macro.description:
                clean_line = strip_html_tags(line)
                clean_line = self._convert_html_entities(clean_line)
                if clean_line and clean_line.strip():
                    stripped = clean_line.strip()
                    
                    # Check if this looks like an alternate syntax
                    is_alternate = False
                    
                    # Check 1: Starts with the macro name (common for alternate syntax)
                    if stripped.lower().startswith(macro.name.lower() + ' '):
                        is_alternate = True
                    # Check 2: Short line with special syntax characters
                    elif (len(stripped) < 40 and 
                          any(c in stripped for c in ['#', '(', '[']) and
                          not any(word in stripped.lower() for word in ['if', 'when', 'this', 'that', 'the', 'for', 'example', 'address', 'value'])):
                        is_alternate = True
                    
                    if is_alternate:
                        # Format as alternate syntax (no line break needed)
                        f.write(f"**Alternate:** `{stripped}`\n\n")
                    else:
                        # Regular description - add hard line break
                        f.write(f"{clean_line}  \n")
            f.write("\n")
        
        # Returns (strip HTML tags and convert entities)
        if macro.returns:
            clean_returns = strip_html_tags(macro.returns)
            clean_returns = self._convert_html_entities(clean_returns)
            f.write(f"**Returns:** {clean_returns}\n\n")
        
        # Registers modified (strip HTML tags and convert entities)
        if macro.registers_modified:
            clean_regs = strip_html_tags(macro.registers_modified)
            clean_regs = self._convert_html_entities(clean_regs)
            f.write(f"**{clean_regs}**\n\n")
        
        # Notes (strip HTML tags and convert entities)
        if macro.notes:
            f.write("**Notes:**\n")
            for note in macro.notes:
                clean_note = strip_html_tags(note)
                clean_note = self._convert_html_entities(clean_note)
                f.write(f"- {clean_note}\n")
            f.write("\n")
    
    def _write_modules(self, f, module_docs: List[ModuleData]):
        """Write complete modules section"""
        f.write("# Modules\n\n")
        self._add_toc_entry("Modules", level=1)
        
        f.write("LAMAlib modules are reusable, configurable components that can be included in your programs. ")
        f.write("Each module is configured using `def_const` parameters and included within a scope.\n\n")
        
        # Write each module
        for module in sorted(module_docs, key=lambda m: m.name.lower() if m.name else 'zzz'):
            if not module.name:
                continue
            
            self._write_module(f, module)
    
    def _write_module(self, f, module: ModuleData):
        """Write documentation for a single module"""
        # Module name
        f.write(f"## {module.name}\n\n")
        self._add_toc_entry(module.name, level=2)
        
        # Metadata
        if module.version or module.author:
            if module.version:
                f.write(f"**Version:** {module.version}  \n")
            if module.author:
                f.write(f"**Author:** {module.author}  \n")
            f.write("\n")
        
        # Purpose
        if module.purpose:
            f.write(f"{module.purpose}\n\n")
        
        # Features
        if module.features:
            f.write("**Features:**\n")
            for feature in module.features:
                f.write(f"- {feature}\n")
            f.write("\n")
        
        # Configuration Parameters
        if module.parameters:
            f.write("**Configuration Parameters:**\n\n")
            
            f.write("| Parameter | Default | Required | Description |\n")
            f.write("|-----------|---------|----------|-------------|\n")
            
            for param in module.parameters:
                default_str = f"`{param.default}`" if param.default else "—"
                required_str = "✓" if param.required else ""
                comment_str = param.comment.replace('|', '\\|') if param.comment else ""
                
                f.write(f"| `{param.name}` | {default_str} | {required_str} | {comment_str} |\n")
            
            f.write("\n")
        
        # Usage
        f.write("**Usage:**\n\n")
        f.write("```assembly\n")
        f.write(f".scope {module.name}\n")
        
        # Required parameters
        required = [p for p in module.parameters if p.required]
        if required:
            f.write("  ; Set required parameters\n")
            for p in required:
                f.write(f"  {p.name}=value\n")
        
        f.write(f"  .include \"modules/{module.filename}\"\n")
        f.write(".endscope\n\n")
        
        # Entry points
        if 'init' in module.entry_points:
            f.write(f"m_init {module.name}\n")
        if 'run' in module.entry_points:
            f.write(f"m_run {module.name}\n")
        
        f.write("```\n\n")
        
        # API Reference
        if module.api_reference:
            f.write("**API:**\n\n")
            f.write("```\n")
            f.write(f"{module.api_reference}\n")
            f.write("```\n\n")
        
        f.write("---\n\n")


# ============================================================================
# MAIN DOCUMENTATION GENERATOR
# ============================================================================

class LAMAlibDocGenerator:
    """Main documentation generator for LAMAlib"""
    
    def __init__(self):
        """Initialize the documentation generator"""
        self.module_parser = ModuleParser()
        self.include_parser = IncludeParser()
        self.comprehensive_gen = ComprehensiveGenerator()
    
    def generate_all(self, output_file: Path, include_files: List[Path], 
                     modules_dir: Path):
        """Generate complete documentation in a single file"""
        print("=" * 70)
        print("LAMAlib Documentation Generator")
        print("=" * 70)
        print()
        
        # Parse include files
        include_docs = []
        if include_files:
            print(f"Processing {len(include_files)} .inc files...")
            for inc_file in include_files:
                if inc_file.exists():
                    print(f"  • {inc_file.name}")
                    try:
                        doc = self.include_parser.parse(inc_file)
                        include_docs.append(doc)
                    except Exception as e:
                        print(f"    Error: {e}")
                else:
                    print(f"  ✗ {inc_file} not found")
            print()
        
        # Parse modules
        module_docs = []
        if modules_dir.exists():
            module_files = sorted(modules_dir.glob('m_*.s'))
            if module_files:
                print(f"Processing {len(module_files)} modules...")
                for module_file in module_files:
                    print(f"  • {module_file.name}")
                    try:
                        module = self.module_parser.parse(module_file)
                        module_docs.append(module)
                    except Exception as e:
                        print(f"    Error: {e}")
                print()
        else:
            print(f"Note: modules directory '{modules_dir}' not found, skipping modules\n")
        
        # Generate comprehensive documentation
        print(f"Generating documentation → {output_file}")
        try:
            self.comprehensive_gen.generate(output_file, include_docs, module_docs)
            
            # Print summary
            print()
            print("=" * 70)
            print("✓ Documentation generated successfully!")
            print("=" * 70)
            print()
            print(f"Output file: {output_file.absolute()}")
            print(f"  • API sections: {len(include_docs)}")
            print(f"  • Modules: {len(module_docs)}")
            print(f"  • TOC entries: {len(self.comprehensive_gen.toc)}")
            print()
            
        except Exception as e:
            print(f"\n✗ Error generating documentation: {e}")
            import traceback
            traceback.print_exc()
            return False
        
        return True


# ============================================================================
# MAIN ENTRY POINT
# ============================================================================

def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(
        description='Generate comprehensive LAMAlib documentation in a single file',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  # Generate with default settings (LAMAlib.inc + LAMAlib-sprites.inc)
  python makedoc.py
  
  # Specify custom include files
  python makedoc.py --include LAMAlib.inc LAMAlib-macros16.inc
  
  # Custom output file
  python makedoc.py --output MyDoc.md
        """
    )
    
    parser.add_argument(
        '--output',
        type=str,
        default='LAMAlibdoc.md',
        help='Output file (default: LAMAlibdoc.md)'
    )
    
    parser.add_argument(
        '--include',
        type=str,
        nargs='+',
        default=['LAMAlib.inc', 'LAMAlib-sprites.inc'],
        help='Include files to process (default: LAMAlib.inc LAMAlib-sprites.inc)'
    )
    
    parser.add_argument(
        '--modules-dir',
        type=str,
        default='modules',
        help='Directory containing modules (default: modules/)'
    )
    
    args = parser.parse_args()
    
    # Convert paths
    output_file = Path(args.output)
    include_files = [Path(f) for f in args.include]
    modules_dir = Path(args.modules_dir)
    
    # Create generator and run
    generator = LAMAlibDocGenerator()
    
    try:
        success = generator.generate_all(output_file, include_files, modules_dir)
        return 0 if success else 1
        
    except KeyboardInterrupt:
        print("\n\nInterrupted by user")
        return 1
    except Exception as e:
        print(f"\nFatal error: {e}", file=sys.stderr)
        import traceback
        traceback.print_exc()
        return 1


if __name__ == '__main__':
    sys.exit(main())
