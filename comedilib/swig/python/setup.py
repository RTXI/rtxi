from distutils.core import setup, Extension

module1 = Extension('_comedi',
			define_macros = [('MAJOR_VERSION','0'),
					('MINOR_VERSION','1')],
			include_dirs = ['../include'],
			library_dirs = ['../lib'],
			libraries = ['comedi'],
			sources = ['comedi_wrap.c'])
			
setup (name='_comedi',
	version = '0.1',
	description = 'Python wrapper module for the Comedi data-acquisition drivers',
	author = 'Bryan E. Cole',
	author_email = 'bryan.cole@teraview.co.uk',
	long_description = ''' Wrapper for the Comedi data-acquisition library ''',
	ext_modules = [module1])
