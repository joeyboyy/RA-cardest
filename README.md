# RA-cardest

This is a CMake project. To compile the executables, create a build directory
and invoke CMake as usual. Note that the executables assume to be in a direct
child directory of this root dir.

Execute the executable RunAll from within the build directory to 
populate/overwrite the experiment results in the out/ directory.

This is all needed to reproduce the results. For further processing, the gnuplot
script plots.plt can be used (adjust it depending on use case) to 
populate/overwrite the plots/ directory.