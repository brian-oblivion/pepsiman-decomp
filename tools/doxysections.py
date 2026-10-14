#!/usr/bin/env python3
"""Doxygen input filter for the sectioned .inc files.

    python3 tools/doxysections.py FILE

engine_code.inc and engine_data.inc emit one section per #include: an
`#if NAME_PART == 1 ... #elif NAME_PART == 2 ...` chain, where each section
redefines NAME_PART for the next include. Doxygen preprocesses a file once,
so it would see only section 1 and document nothing past it. This filter
blanks the chain's own lines (the #if/#elif/#endif on a `*_PART` macro and
its #ifndef/#define/#undef) and leaves every other line as it is, so Doxygen
reads all the sections and line numbers stay right. Conditionals nested in a
section are kept.
"""
import re
import sys

PART = re.compile(r"\b\w+_PART\b")
DIRECTIVE = re.compile(r"^\s*#\s*(\w+)")


def main(path):
    stack = []  # one entry per open #if: whether it is a section chain
    with open(path, encoding="utf-8", errors="replace") as f:
        for line in f:
            m = DIRECTIVE.match(line)
            word = m.group(1) if m else None
            blank = False
            if word in ("if", "ifdef", "ifndef"):
                stack.append(bool(PART.search(line)))
                blank = stack[-1]
            elif word in ("elif", "else"):
                blank = bool(stack) and stack[-1]
            elif word == "endif":
                blank = bool(stack) and stack.pop()
            elif word in ("define", "undef"):
                blank = bool(PART.search(line))
            sys.stdout.write("\n" if blank else line)


if __name__ == "__main__":
    main(sys.argv[1])
