#!/usr/bin/env python3

import os

def ensure_header_comment(filepath, base_dir='source'):
    abs_path = os.path.abspath(filepath)
    base_path = os.path.abspath(base_dir)
    rel_path = os.path.relpath(abs_path, base_path)
    expected_comment = f'// {base_dir}/{rel_path.replace(os.sep, "/")}'

    with open(filepath, 'r') as file:
        lines = file.readlines()

    if lines and lines[0].strip() == expected_comment:
        print(f"[OK] {filepath}")
        return

    lines.insert(0, expected_comment + '\n\n')
    with open(filepath, 'w') as file:
        file.writelines(lines)

    print(f"[FIXED] {filepath}")

def process_directory(base_dir='source'):
    for root, _, files in os.walk(base_dir):
        for name in files:
            if name.endswith(('.hpp', '.cpp')):
                full_path = os.path.join(root, name)
                ensure_header_comment(full_path, base_dir)

if __name__ == "__main__":
    process_directory('source')
