# Implicit Type Cast

There's little implicit type cast (type coercion) supported by tan. Implicit type cast not only introduces unnecessary
complexity to the language and the compiler, but it also provides no significant advantages to the developers. Moreover,
many times this feature prevents programmers from spotting bugs (such as signed to unsigned cast) which can be easily
noticed if explicit type cast is written.

## Supported Implicit Type Cast

The specific behaviors of these casts is the same as explicit cast listed in the next section.

The table specifies all legal implicit type conversion in this language.
`char` is considered as an unsigned int whose size is 1 byte.

| index | From                                 | To                                               |
|-------|--------------------------------------|--------------------------------------------------|
| 1     | int                                  | int with the same signedness with a bigger size  |
| 2     | unsigned int                         | signed int with a bigger size                    |
| 3     | signed int                           | unsigned int that with a bigger size             |
| 4     | float                                | float with a bigger size                         |
| 5     | int                                  | float                                            |
| 6     | int/float/pointer                    | bool (xx != 0, for float, NaN is false)          |
| 7     | bool                                 | any type of int or float (true => 1, false => 0) |
| 8     | pointer of a derived class           | pointer to the base class                        |
| 9     | int/float that is compile-time known | int/float that can fit the value                 |

In many binary operations where two operands have different types, one of the operands will have its type promoted
according to these rules.

The resulting promoted type is guaranteed to be one of the two types.

NOTE: if multiple implicit cast rules can apply, the compiler use the one with lower
index.
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

## Implicit Type Casting in Function Overloading

In function call, arguments can be implicitly converted into the type(s) specified in the function signature.

```
fn square(a: float): float {
    return a * 2;
}

square(-100); // works
```

Implicit type conversion can lead to multiple legal function overloads.
In these cases, the compiler will select the candidate that exactly matches the given argument types.

```
fn work(a: float, b: float): void {
    print("float");
}

fn work(a: int, b: int) : void {
    print("int");
}

work(1, 2); // prints "int"
work(1.0, 2.0); // prints "float"
// work(1.0, 2); // error!
```

# Explicit Type Cast Rules

## Cast between Numerical Types

Unlike implicit csat, arbitrary cast between numerical types can be performed, and the behaviors are well-defined.

- Casting between two integers of the same size is a no-op
- Casting from a larger integer to a smaller integer will truncate
- Casting from a smaller integer to a larger integer will
    - zero-extend if the source is unsigned
    - sign-extend if the source is signed
- Casting from a float to an integer will round the float towards zero
    - NaN will return 0
    - Values larger than the maximum integer value will saturate to the maximum value of the integer type
    - Values smaller than the minimum integer value will saturate to the minimum value of the integer type
- Casting from an integer to float will produce the closest possible float
    - if necessary, rounding is according to roundTiesToEven mode
    - on overflow, infinity (of the same sign as the input) is produced
- Casting from an f32 to an f64 is perfect and lossless
- Casting from an f64 to an f32 will produce the closest possible f32
    - if necessary, rounding is according to roundTiesToEven mode
    - on overflow, infinity (of the same sign as the input) is produced
- false casts to 0, true casts to 1
- char casts to the value of the code point, then uses a numeric cast if needed.

## Cast between Pointers

TODO

## Cast between Arrays and Pointers

TODO
