# Packages

Packages are a group of tan source files that share their public and private symbols.
Each package is compiled into a single object file and then linked against libraries or other packages.

## Creating a Package

We can tell the compiler that a source file belongs to a package by adding a global statement:

```
package "<package_name>";
```

If this line doesn't exist, the file is treated as a separate package with the filename as the package name.

## File Structure for Packages

```
- my_package
   - src1.tan
   - src2.tan
```

And `src1.tan` and `src2.tan` should contain a `package "my_package";` statement.
However, the compilation won't fail if this is not true, but the source file won't be treated as a part of that package.

## Import Resolution

Assuming we have a library called `dope` and a source file `sus.tan` wrote by someone else:

```
- dope
   - xx.tan
   - xx.tan
   - libdope.so
- sus.tan
```

We can import `dope` by using the `import "dope"` and import `sus.tan` using `import "sus"`.
When the compiler sees them, it will try to find the path to these source files, and import public symbols from them so
that they can be used in our own code.
During compilation, these files are not compiled. Instead, we link the object files generated from our code
to `libdope.so` to produce whatever program we want.

The current implementation locate source files of an external package by:

1. using the absolute path to a file or folder if specified, otherwise:
2. searching for a directory or file with the same name as the package, with the following priorities:
    1. Search in the same directory that contains the current source file
    2. Search in user-defined include dirs
    3. Search in system directories (for example, the compiler always know the location of `runtime` library)
3. skipping candidates that don't contain has a `package` statement with the same package name

Future plan includes configurable mapping between packages and their search paths.

## Conventions

Some soft rules (might change):

- As mentioned above, it's really convenient to make the package name and the folder name identical
- Follow the same rules for variable names when deciding the package name