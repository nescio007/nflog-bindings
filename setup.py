from setuptools import setup, Extension

nflog = Extension('_nflog', sources=['nflog.c', 'nflog_common.c', 'nflog_utils.c', 'exception.c', 'libnetfilter_log.i'], include_dirs=['.'], libraries=['netfilter_log'])

setup(
    name='nflog-bindings',
    version='0.0.1',
    ext_modules=[nflog],
    py_modules=['nflog'],
    url='',
    license='',
    author='',
    author_email='',
    description=''
)
