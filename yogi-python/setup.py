#!/usr/bin/env python3
import os
import re
import setuptools

core_header_file = os.path.realpath(os.path.dirname(
    __file__) + '/../yogi-core/include/yogi_core.h')
for _, line in enumerate(open(core_header_file)):
    if line.startswith('#define YOGI_HDR_VERSION '):
        version = re.search('"(.*)"', line).group(1)

config = {
    'description': 'Python bindings for the Yogi framework',
    'author': 'Johannes Bergmann',
    'author_email': 'j-bergmann@outlook.com',
    'url': 'https://github.com/jmbergmann/yogi',
    'download_url': 'https://github.com/jmbergmann/yogi',
    'version': version,
    'packages': setuptools.find_packages('.', exclude=['test', 'test.*']),
    'name': 'yogi-python',
    'license': 'GPLv3',
    'keywords': 'yogi decoupling networking',
    'test_suite': 'test',
    'python_requires': '>=3.5',
    'classifiers': [
        'Development Status :: 3 - Alpha',
        'License :: OSI Approved :: GPLv3',
        'Intended Audience :: Developers',
        'Topic :: Software Development :: Libraries'
    ]
}

setuptools.setup(**config)
