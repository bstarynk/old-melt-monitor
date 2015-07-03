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

A JITable function item **should have the following attributes**

* `function_signature` : *sigitm*, an item giving the signature (so of
  kind `function_signature`, like `signature_1itm1val_to_item` for
  example)

* `formals` : *formaltup*, a tuple of formals (compatible with the
  `input_types` of *sigitm*)

* `results` : *resultup*, a tuple of results (compatible with the
`output_types` of *sigitm*)

* `code` : *code*, an instruction or a block, usually a node of connective
`code` giving a sequence of instruction nodes.

* `closed` : *clostup*, a tuple of closed "variables", i.e. items.

* `constants` : *constseq*, a tuple or sequence of constant items. If
  a constant item contains some `value` attribute, it is the
  constant's value; otherwise the constant item stays for itself.

* `variables` : *varseq*, a tuple or sequence of variable items. 


Constants, formals, results, and variables should have a `type`
attribute (a predefined item of kind `type`, like `item` or
`locked_item` or `integer` or `value` etc..).


## Blocks
Blocks are items of kind `block`

## Statements

Statements are generally nodes. A statement which is a block item is
understood as an unconditional jump to that item block. The connective
of a statement node is called the statement operation or stmt-op.

[markdown]: http://daringfireball.net/projects/markdown/syntax
"markdown syntax"

