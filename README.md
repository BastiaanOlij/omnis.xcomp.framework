omnis.xcomp.framework
=====================

Framework classes for building Omnis Studio XCOMPs

This is a framework I'm working on that combines a number of classes I've (re)written in various shapes and sizes as I've written Omnis Studio XComps over the years. I though it would be nice to donate these to the public domain. If you end up using any of this code feel free to drop me a line and a notice of sorts would be appreciated.

I'm no longer actively maintaining the Studio 4 and Studio 6 support. Expect this to break on Studio 4, Studio 6 will most likely keep working for the foreseeable future. 
I maintain project files in the framework example repo but am only actively maintaining the Studio 8.1 builds.

More information about the SDK can be found here: https://omnis.net/documentation/extcompsdk/index.jsp

Directories are setup relative and assume the framework is installed within the Omnis SDK folders:
- Omnis SDK
 - complib
 - omnis.xcomp.framework
 - ...

Or you can submodule it:) 

Dependencies
============
There are support libraries you need to download separately to compile these sources and I've added placeholders for these libraries in the source tree. 
So far this is limited to STB

Hooks
=====
The subfolder hooks contains a few git hooks you can copy into the ```.git/hooks``` subfolder. This ensures clang-format is run on any changed file before committing the changes to the repository.
You do need to have clang-format installed. 

License
=======
Please note that I'm releasing these works, including my own derivatives under an MIT license. In a nutshell this means you can use the code presented here as you see fit including in commercial projects. All I ask is that you give some credit to this body of work. See the license.txt file for the full license text.

I'm sure I'm stating the obvious that this license does not cover the SDK from Omnis Software, only the code presented here.

----

Bastiaan Olij - 2018.
