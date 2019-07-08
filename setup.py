import os
import re
from setuptools import setup, Extension

here = os.path.abspath(os.path.dirname(__file__))

def read(*parts):
    with open(os.path.join(here, *parts), 'r') as fp:
        return fp.read()

def find_version(*file_paths):
    version_file = read(*file_paths)
    version_match = re.search(r"^#define NFLOG_BINDINGS_VERSION ['\"]([^'\"]*)['\"]", version_file, re.M)
    if version_match:
        return version_match.group(1)
    raise RuntimeError("Unable to find version string.")

nflog = Extension('_nflog', sources=['nflog.c', 'nflog_common.c', 'nflog_utils.c', 'exception.c', 'libnetfilter_log.i'], include_dirs=['.'], libraries=['netfilter_log'])

setup(
    name='nflog-bindings',
    version=find_version("nflog_version.h"),
    ext_modules=[nflog],
    py_modules=['nflog'],
    url='',
    license='',
    author='',
    author_email='',
    description=''
)
