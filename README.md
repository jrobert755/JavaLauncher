# JavaLauncher
A C++ program designed to launch java applications

#Compiling
Compilation for all environments requires three items:
CMake >= 3.0
A static copy of zlib.
JDK 1.8 JNI Header files

Before copying, put the header files for zlib in the zlib/include
folder and the static zlib library file in the zlib/lib folder. This
is required for libarchive to compile support for gzip.

Also copy all header files and folders in the JDK's include folder to
the top leve include folder. This is because the working of the Oracle
Java license is vague, and doesn't cover these files, so to be on the
safe side they are not included.

After compiling, a single executable will be generated. All libraries
required are statically linked against the executable.

#OS Support
Since this is in its infancy, it will most likely only work on Windows
with Visual Studio, as that is the environment I am currently working
on. As the features get closer to being finished, testing on all of
the enivroments will begin, getting compatability for Windows, Linux,
and Mac OSX.

#Configuration file
JavaLauncher uses a configuration file for setting up the java code to
run. It has the following parameters:
jarFile=something.jar
mainClass=some.package.SomeClass
classpath=SomeClasspath
If a jarfile is specified, and it includes a manifest file with a
main class property, then the above mainClass doesn't need to be set.