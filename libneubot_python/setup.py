# Adapted from: http://docs.python.org/3/extending/building.html

from distutils.core import setup, Extension

module1 = Extension('_libneubot_python',
                    define_macros = [('MAJOR_VERSION', '1'),
                                     ('MINOR_VERSION', '0')],
                    include_dirs = ['/usr/local/include'],
                    libraries = ['neubot'],
                    library_dirs = ['/usr/local/lib'],
                    sources = ['neubot_wrap.cxx'])

setup (name = 'LibNeubotPython',
       version = '1.0',
       description = 'Neubot Python Library',
       author = 'Simone Basso',
       author_email = 'bassosimone@gmail.com',
       url = 'http://www.neubot.org',
       long_description = '''
Neubot Python Library
''',
       ext_modules = [module1])
