use ExtUtils::MakeMaker;
# See lib/ExtUtils/MakeMaker.pm for details of how to influence
# the contents of the Makefile that is written.
WriteMakefile(
    'NAME'	   => 'Comedi',
    'VERSION_FROM' => 'Comedi.pm',
    'LIBS'	   => [],
    'DEFINE'	   => '',
    'INC'	   => '',

    'dist'      => { COMPRESS=>"gzip", SUFFIX=>"gz" },
);
