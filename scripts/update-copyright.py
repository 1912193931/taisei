#!/usr/bin/env python3

import pathlib, sys, re

header = r"""\1/*
 * This software is licensed under the terms of the MIT-License
 * See COPYING for further information.
 * ---
 * Copyright (c) 2011-2017, Lukas Weber <laochailan@web.de>.
 * Copyright (c) 2012-2017, Andrei Alexeyev <akari@alienslab.net>.
 */

"""

header_regex = re.compile(r'^(#if 0.*?\s#endif\s*)?(\s*/\*.*?\*/)?\s*', re.MULTILINE | re.DOTALL)

if __name__ == '__main__':
    for path in (pathlib.Path(sys.argv[0]).parent.parent / 'src').glob('**/*.[ch]'):
        path.write_text(header_regex.sub(header, path.read_text(), 1))
