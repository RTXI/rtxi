use ExtUtils::MakeMaker;

# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.

WriteMakefile(
    'NAME'	   => 'Comedi::Lib',
    'VERSION_FROM' => 'Lib.pm',
    'LIBS'	   => ['-lcomedi'],
    'DEFINE'	   => '',
    'INC'	   => '',
    'TYPEMAPS'     => ['../typemap'],
);
