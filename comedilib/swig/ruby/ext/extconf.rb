require 'mkmf'
dir_config('comedilib')
have_library('comedi')
create_makefile("comedi")
