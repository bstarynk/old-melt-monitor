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

* `code` : *code*,  the starting block of the function.

* `closed` : *clostup*, a tuple of closed "variables", i.e. items.

* `constants` : *constseq*, a tuple or sequence of constant items. If
  a constant item contains some `value` attribute, it is the
  constant's value; otherwise the constant item stays for itself.

* `variables` : *varseq*, a tuple or sequence of variable items. 


Constants, formals, results, and variables should have a `type`
attribute (a predefined item of kind `type`, like `item` or
`locked_item` or `integer` or `value` etc..).


## Blocks
Blocks are items of kind `block` ; their components are statement items.

## Statements

Statements are items of kind `code_statement`; every statement of a
function should be unique: two different blocks of a function cannot
share a statement item. A statement should have a non-empty component
sequence. The first component (of rank 0) is called the operation of
that statement. It should be an item, called a statement operation (or
*stmt-op*)

Some statements are considered (during JIT compilation) as **leading
statement**s or leaders. They will become the first statement of their
*basic* blocks. See wikipage about [basic blocks][]

The following control flow related stmt-ops are understood:

* `if` *test* *then* [ *else* ]; both *then* and *else* are
  leaders, and so is the next statement.

* `int_switch` *expr* *case* ... ; sons except the first are
  *cases*, i.e. nodes like `^case(` *casevalue* *statement* `)`. Each
  case statement, and the next statement, is a leader.

* `item_switch` *expr* *case* ... , similar to `int_switch` except
that the discriminating *expr* should give an item.

* `jump` *block-item* ; is an unconditional jump (so could play the
  role of a `continue` to restart the block)

* `code` *sub-statement* ..., with the first sub-statement being a
leader (like blocks in C). An implicit block is then made and stored in
the `block` attribute of that statement.

* `block` *blockitem* *sub-statement* ... likewise, but the block is
  explicitly given.

* `loop` *blockitem* *sub-statement* ... likewise, for an infinite
loop of given block item, but the block is explicitly given.

* `break` *blockitem* to jump to the next instruction after the given block.

* `apply` *signature* *result-vars* ... *function-expr*
  *argument-expr* ... ; where *signature* is a (compile-time) item of
  kind `signature`; if the application fails, the current function
  also fails

* `apply_else` *signature* *result-vars* ... *function-expr*
  *argument-expr* ... *else-statement* ; if the application fails, the
  *final* else substatement is executed so it is a leader.

[markdown]: http://daringfireball.net/projects/markdown/syntax
"markdown syntax"

[basic blocks]: http://en.wikipedia.org/wiki/Basic_block


