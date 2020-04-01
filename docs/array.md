# Arrays & Pointers

## Basic Rules

**All arrays converted to pointers at the very early stage of compiling in tan**

That means:

- `[i32, 2]` = `i32*`
- `[[i32, 2], 3]` = `i32**`
- `str` = `u8*`

### Why?

This makes the language more consistent. Also, the usage of raw pointers in tan is
discouraged, and users should use a custom `struct` to represent an array. Use
what we often do in C as an example:

```
struct MyI32Array {
    i32* ptr;
    u32 len;
}
```

// TODO: builtin array type

## Nested Arrays

tan does support nested arrays. However, the offsets of the outer dimensions should
be calculated by hand, For example, the following evaluate to 1 INSTEAD OF 2:

```
var nested: [[i32, 3], 4] = [[1, 1, 1], [2, 2, 2], [3, 3, 3], [3, 3, 3]];
var e: i32 = nested[1][0];
```

because the size of elements in nested (the size of `[i32, 3]`) is not preserved.
Therefore, `nested[1][0]` is equivalent to adding `1` to a pointer of type `i32**`,
and then add `0` to it. Also, because the data in nested arrays are consecutive in
memory (the memory of `nested` is like `1, 1, 1, 2, 2, 2, ...`), `e` is the value of
the second `i32` in memory, which is `1`.

## Memory of Arrays

Nothing special, just remember the following when you are dealing with arrays:

- All arrays (nested or not) have consecutive memory.
- The size of allocated memory is NOT automatically extended. // TODO: implement `malloc`

## Conclusion

**The recommended way is to avoid raw pointers for any arrays**

As you have experienced in C, raw pointers don't have any bound checking capabilities,
so programmers do it themselves. So why not just avoid using raw pointers for arrays?

Instead, wrap the pointer, create some interfaces to manipulate it, and do the checking
in the interface implementations.
