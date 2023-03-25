# Implicit Type Cast

## Supported Implicit Type Cast

The table specifies all legal implicit type conversion in this language.
`char` is considered as an unsigned integer whose size is 1 byte.
The specific behaviors of these casts are listed in the next section.

| index | From                                 | To                                              |
|-------|--------------------------------------|-------------------------------------------------|
| 1     | int                                  | int with the same signedness with a bigger size |
| 2     | unsigned int                         | signed int with a bigger size                   |
| 3     | signed int                           | unsigned int that with a bigger size            |
| 4     | float                                | float with a bigger size                        |
| 5     | int                                  | float                                           |
| 6     | bool                                 | any type of int or float                        |
| 7     | int/float/pointer                    | bool                                            |
| 8     | pointer of a derived class           | pointer to the base class                       |
| 9     | int/float that is compile-time known | int/float that can fit the value                |

The most common use cases is that when operands of a binary operation have different types, one of them will have its
type converted to the other's.

NOTE: if multiple implicit cast rules can apply, the compiler use the rule with a lower index.
For example,

```
// rule #6 or #7?
var b = true;
var i = 100;
print(b + i); // 101

// rule #2 or #3?
var u: u32 = 100;
var s = 100;
var res = s + u; // type is unsigned
```

## Implicit Type Casting in Function Calls

In function calls, arguments can be implicitly converted into the parameter types if allowed.

```
fn square(a: float): float {
    return a * 2;
}

square(-100); // works
```

# Behaviors of Type Casts

## Primitive Types

- Casting between two integers of the same size is a no-op
- Casting from a larger integer to a smaller integer will truncate
- Casting from a smaller integer to a larger integer will
    - zero-extend if the source is unsigned
    - sign-extend if the source is signed
- Casting from a float to an integer will round the float towards zero
    - `NaN` will return 0
    - Values larger than the maximum integer value will saturate to the maximum value of the integer type
    - Values smaller than the minimum integer value will saturate to the minimum value of the integer type
- Casting from an integer to float will produce the closest possible float
    - if necessary, rounding is according to roundTiesToEven mode
    - on overflow, infinity (of the same sign as the input) is produced
- Casting from an `f32` to an `f64` is perfect and lossless
- Casting from an `f64` to an `f32` will produce the closest possible `f32`
    - if necessary, rounding is according to roundTiesToEven mode
    - on overflow, infinity (of the same sign as the input) is produced
- Casting from a numerical type `n` to `bool` is equivalent to `n != 0`
    - for float, `NaN` is `false`
- `false` casts to `0`, `true` casts to `1`
- `char` casts to the value of the code point, then uses a numeric cast if needed.

## Pointers

TODO

## Arrays and Pointers

TODO
