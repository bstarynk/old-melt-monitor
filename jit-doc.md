<!-- -*- Markdown -*- -->
# the JIT inside MELT monitor

This is the `jit-doc.md` file (in [markdown][markdown] syntax) of the MELT monitor. The Just-In-Time translation is using [GCC-JIT][GCCJIT] and needs a recent version (from GCC 5.2 or GCC 6 at least) of GCC.

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

Some statements are considered (during JIT compilation) as
**leading statement**s or leaders. They will become the first statement of their
*basic* blocks. See wikipage about [basic blocks][]

The following control flow related stmt-ops are understood:

* `if` *test* *then-block* [ *else-block* ]; the next statement is a leader.

* `item_switch` *expr* *case* ... , where each *case* is a node like
  `^case(` *caseitem* *block* `)`; the next statement is a leader.

* `int_switch` *expr* *case* ... ; *case*-s can be like `^case(`
  *case-int* *block* ) or `^case_range`( *low-case-int*
  *high-case-int* *block* ).


* `jump` *block-item* ; is an unconditional jump (so could play the
  role of a `continue` to restart the block); there should not be any
  next statement, or else it should be a leader...

* `code` *sub-statement* ..., with the first sub-statement being a
leader (like blocks in C). An implicit block is then made and stored in
the `block` attribute of that statement.

* `block` *blockitem* *sub-statement* ... likewise, but the block is
  explicitly given - if it is nil, it becomes created and stored as
  the component #1

* `loop` *blockitem* *sub-statement* ... likewise, for an infinite
loop of given block item, but the block is explicitly given - if it is
nil, it becomes created and stored as the component #1.

* `break` *blockitem* to jump to the next instruction after the given block.

* `apply` *signature* *result-vars* ... *function-expr*
  *argument-expr* ... ; where *signature* is a (compile-time) item of
  kind `signature`; if the application fails, the current function
  also fails

* `apply_else` *signature* *result-vars* ... *function-expr*
  *argument-expr* ... *else-block* ; if the application fails, the
  *final* else block is executed so it is a leader.



## expressions

Each expression has a type, reified as some item of `type` kind
(e.g. `integer`, `double`, `item`, `value` ....)

### predefined unary operators

* `jit_abs` for the absolute value of the numerical `integer` or `double`
  argument. Uses [`GCC_JIT_UNARY_OP_ABS`][GCCJITUNARYOPABS] from GCCJIT.

* `jit_bitnot` for the bitwise not (like unary `~` in C) of the argument.
Uses [`GCC_JIT_UNARY_OP_BITWISE_NEGATE`][GCCJITUNARYOPBITWISENEGATE] from GCCJIT.

* `jit_minus` for the opposite numerical value (like unary `-` in C) of the numerical
  argument. Uses [`GCC_JIT_UNARY_OP_MINUS`][GCCJITUNARYOPMINUS] from GCCJIT.

* `jit_negate` for the logical negation (like unary `!` in C) of the
`integer`, `double`, or `item` argument.  Uses
[`GCC_JIT_UNARY_OP_LOGICAL_NEGATE`][GCCJITUNARYOPLOGICALNEGATE] from
GCCJIT.


### predefined binary operators

[markdown]: http://daringfireball.net/projects/markdown/syntax
"markdown syntax"

[basic blocks]: http://en.wikipedia.org/wiki/Basic_block

[GCCJIT]: https://gcc.gnu.org/onlinedocs/jit/

[GCCJITUNARYOPABS]: https://gcc.gnu.org/onlinedocs/jit/topics/expressions.html#GCC_JIT_UNARY_OP_ABS

[GCCJITUNARYOPMINUS]: https://gcc.gnu.org/onlinedocs/jit/topics/expressions.html#GCC_JIT_UNARY_OP_MINUS


[GCCJITUNARYOPLOGICALNEGATE]: https://gcc.gnu.org/onlinedocs/jit/topics/expressions.html#GCC_JIT_UNARY_OP_LOGICAL_NEGATE

[GCCJITUNARYOPBITWISENEGATE]: https://gcc.gnu.org/onlinedocs/jit/topics/expressions.html#GCC_JIT_UNARY_OP_LOGICAL_NEGATE

