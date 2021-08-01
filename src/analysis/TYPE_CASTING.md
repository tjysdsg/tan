# Implicit Type Cast

There's little implicit type cast (type coercion) supported by tan. Implicit type cast not only introduces unnecessary
complexity to the language and the compiler, but it also provides no significant advantages to the developers. Moreover,
many times this feature prevents programmers from spotting bugs (such as signed to unsigned cast) which can be easily
noticed if explicit type cast is written.

## Supported Implicit Type Cast

TODO:

Any implicit cast can be done the same way using explicit cast. Therefore, the exact behaviors will be listed in the
sections below.

# Explicit Type Cast Rules

The philosophy of explicit type cast rules is to provide extensive and clear definition of the behaviors of type cast.

TODO: the majority of the following is copied from rust, should add tests

## Cast between Numerical Types

Unlike implicit csat, arbitrary cast between numerical types can be performed, and the behaviors are well-defined.

- Casting between two integers of the same size is a no-op
- Casting from a larger integer to a smaller integer will truncate
- Casting from a smaller integer to a larger integer will
    - zero-extend if the source is unsigned
    - sign-extend if the source is signed
- Casting from a float to an integer will round the float towards zero
    - NaN will return 0
    - Values larger than the maximum integer value will saturate to the maximum value of the integer type.
    - Values smaller than the minimum integer value will saturate to the minimum value of the integer type.
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
