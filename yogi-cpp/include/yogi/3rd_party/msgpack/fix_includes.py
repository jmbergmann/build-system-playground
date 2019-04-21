#!/usr/bin/env python3
import os
import sys
import re

SOURCE_DIRECTORY = os.path.dirname(sys.argv[0])
HEADER_FILE_EXTENSIONS = ['.h', '.hpp']


def get_header_files() -> [(str, str)]:
    files = []
    for dir, _, filenames in os.walk(SOURCE_DIRECTORY):
        for filename in filenames:
            if os.path.splitext(filename)[1] in HEADER_FILE_EXTENSIONS:
                files.append((dir, filename))
    return files


def fix_includes(dir, filename) -> None:
    path = os.path.join(dir, filename)
    with open(path, 'r') as file:
        content = file.read()

    include_statements = re.findall(r'(# *include *["<](.*)[">])', content)
    for inc_stat in include_statements:
        orig_stat = inc_stat[0]
        orig_path = inc_stat[1]
        new_path = os.path.relpath(os.path.join(
            SOURCE_DIRECTORY, orig_path), dir).replace('\\', '/')
        new_stat = orig_stat.replace(orig_path, new_path)
        content = content.replace(orig_stat, new_stat)


if __name__ == "__main__":
    files = get_header_files()
    files = [files[2]]
    for file in files:
        fix_includes(file[0], file[1])
