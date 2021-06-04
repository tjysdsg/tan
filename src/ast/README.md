# Immutability of pointers to AST nodes

Once an AST node pointer is created, the value of it shouldn't be changed for the purpose of changing the content of the
referred object.

If the type of the node that the pointer points to could be changed, use a proxy node.

For example, when parsing unary and binary operators, there are some ambiguous cases where we do not know for sure if
the operator is unary or binary (e.g. unary plus or binary sum). Naturally, we can obtain a reference to the pointer,
and modify the value of the pointer after we know the operator's type.

However, the problem with this approach is similar to the problem of dangling pointer: after we modify the pointer,
other copies of the same pointer (might be scattered across the entire AST) points to an object that contains invalid
information. This is very likely to cause bugs.

Instead, we create a `BinaryOrUnary` class that acts like a proxy to the actual class. After we know the operator's
type, we can tell `BinaryOrUnary` whether it is a unary or binary operator, and pass in the pointer that points to the
object with valid information.

Another thing, don't forget to add relevant code for `BinaryOrUnary`'s parsing, analyzing, and codegen, which will be
trivial (proxy)
