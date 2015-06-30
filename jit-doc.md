<!-- -*- Markdown -*- -->
# the JIT inside MELT monitor

This is the `jit-doc.md` file (in [markdown][markdown] syntax) of the MELT monitor.

The source file `codjit.c` contains a just-in-time compilation
framework, implemented thru the applicable function
`generate_jit_module` which is given a module item of kind
`code_module` having a `functions` attribute associated with some
value *funsval*. If that *funsval* is a node, it is applied (to the
module and the code generator items) to give a sequence. The items
from the *funsval* sequence (i.e. set or tuple) are functions.

The result of the `generate_jit_module` application is the (modified)
module itself on JIT achievement, or some error string on JIT failure.


## Functions

Functions are "compiled" into some code which is applicable.

A JITable function **should have** 
[markdown]: http://daringfireball.net/projects/markdown/syntax
"markdown syntax"

