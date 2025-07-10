#!/usr/bin/env python
import subprocess
import sys
import re
import os

def parse_symbols(elf_path):
    """Extract function symbols and their addresses from the ELF file."""
    cmd = ["readelf", "-Ws", elf_path]
    output = subprocess.check_output(cmd, text=True)

    symbols = []
    for line in output.splitlines():
        fields = line.split()
        if len(fields) < 8:
            continue
        if fields[3] == "FUNC":
            try:
                addr = int(fields[1], 16)
                name = fields[7]
                symbols.append((addr, name))
            except ValueError:
                continue
    return symbols

def find_nearest_symbol(rip, symbols):
    """Find the symbol whose address is the closest but <= RIP."""
    symbols = sorted(symbols)  # sort by address
    nearest = None
    for addr, name in symbols:
        if addr <= rip:
            nearest = (addr, name)
        else:
            break
    return nearest

def parse_lineinfo(elf_path):
    """Parse line info from readelf --debug-dump=decodedline output."""
    cmd = ["readelf", "--debug-dump=decodedline", elf_path]
    try:
        output = subprocess.check_output(cmd, text=True)
    except subprocess.CalledProcessError:
        return []
    lineinfo = []  # List of (start_addr, file, line)
    for line in output.splitlines():
        fields = line.split()
        if len(fields) >= 3:
            # Try to match address as last field
            try:
                addr = int(fields[-1], 16)
                # File name may have spaces, so join all but last two fields
                file = " ".join(fields[:-2])
                line_num = int(fields[-2])
                lineinfo.append((addr, file, line_num))
            except ValueError:
                continue
    # Sort by address
    lineinfo.sort()
    return lineinfo

def find_lineinfo(rip, lineinfo):
    """Find the file and line whose address is the closest but <= RIP."""
    result = None
    for addr, file, line in lineinfo:
        if addr <= rip:
            result = (file, line)
        else:
            break
    return result

def enhance_stacktrace(debug_block, symbols, lineinfo):
    """Enhance stacktrace XML in a debug block, showing function (file:line) if possible, else function+offset, else file:line, else raw address."""
    stacktrace_match = re.search(r'<stacktrace>(.*?)</stacktrace>', debug_block, re.DOTALL)
    if not stacktrace_match:
        return None
    stacktrace_content = stacktrace_match.group(1)
    rip_matches = re.findall(r'<rip>(0x[0-9A-Fa-f]+)</rip>', stacktrace_content)
    if not rip_matches:
        return None
    enhanced_lines = []
    enhanced_lines.append("stacktrace:")
    for rip_str in rip_matches:
        try:
            rip = int(rip_str, 16)
            nearest = find_nearest_symbol(rip, symbols)
            lineinfo_entry = find_lineinfo(rip, lineinfo)
            if nearest and lineinfo_entry:
                # function (file:line)
                name = nearest[1]
                file, line = lineinfo_entry
                enhanced_lines.append(f"  {name} ({file}:{line})")
            elif nearest:
                # function+offset
                addr, name = nearest
                offset = rip - addr
                enhanced_lines.append(f"  {name}+0x{offset:x}")
            elif lineinfo_entry:
                # file:line only
                file, line = lineinfo_entry
                enhanced_lines.append(f"  {file}:{line}")
            else:
                # raw address
                enhanced_lines.append(f"  {rip_str}")
        except ValueError:
            continue
    return "\n".join(enhanced_lines)

def process_debug_block(debug_block, symbols, lineinfo):
    """Process a <debug> block. Extendable for more debug types."""
    enhanced = enhance_stacktrace(debug_block, symbols, lineinfo)
    if enhanced:
        return enhanced
    return None

def main():
    if len(sys.argv) != 2:
        print(f"Usage: {sys.argv[0]} <kernel-elf>", file=sys.stderr)
        sys.exit(1)
    elf_path = sys.argv[1]
    if not os.path.exists(elf_path):
        print(f"Kernel ELF file not found: {elf_path}", file=sys.stderr)
        sys.exit(1)
    try:
        symbols = parse_symbols(elf_path)
        lineinfo = parse_lineinfo(elf_path)
    except Exception as e:
        print(f"Failed to parse ELF file {elf_path}: {e}", file=sys.stderr)
        sys.exit(1)
    in_debug = False
    debug_block_lines = []
    try:
        for line in sys.stdin:
            if not in_debug:
                if "<debug>" in line:
                    in_debug = True
                    debug_block_lines = [line]
                else:
                    print(line, end='')
                    sys.stdout.flush()
            else:
                debug_block_lines.append(line)
                if "</debug>" in line:
                    in_debug = False
                    debug_block = ''.join(debug_block_lines)
                    enhanced = process_debug_block(debug_block, symbols, lineinfo)
                    if enhanced:
                        print(enhanced)
                        sys.stdout.flush()
                    debug_block_lines = []
    except KeyboardInterrupt:
        pass
    except BrokenPipeError:
        pass

if __name__ == "__main__":
    main()
