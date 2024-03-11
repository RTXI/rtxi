# RTXI DSP Libraries

The dsp directory in the library folder has a number of utility functions and classes specifically for
digital signal processing. The design and implementation files were originally copied over from the text
"Simulating Wireless Communication Systems: Practical Models in C/C++" by C. Britton Rorabaugh. The
original code was derived from a codebase that, as far as i can tell, does not exist anymore. We are 
bound by the decisions of the original coder, as well as the developer who decided to rely on this
code for a number of rtxi plugins. For the future maintainer and user of this utility library it may 
be neccessary to obtain a copy of this book as this will better explain the logic behind the algorithms
here.

We have gone through great lengths to update this code to conform to modern C++ standards, as well as 
documenting it as much as possible so it may not be neccessary to obtain the book. Given this information, 
know that if something is not explained very well, or of you need additional details, then the book 
could shine some light to an otherwise undocumented software. The following is an attempt at
distilling the design of the dsp libraries for you to get started with.

## Generic Window

files: gen_win.h gen_window.cpp

Many of the classes that rely on fast fourier transforms and frequency transformations make use of the base
class GenericWindow. This base class is used for the windowing method to reduce leakage when dealing with 
Discrete Fourier Transforms (DFTs). The derived Windows such as the triangular window derive from this class.
