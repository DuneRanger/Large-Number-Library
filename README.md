# Large Number Library
 
This is a C++ library for operations with large numbers. \
It is a student project for my first university semester (MatFyz).

For instructions on how to use the library, please see the [User Documentation](documentation-user.md). \
If you wish to contribute to the library, please see the [Developer Documentation](documentation-dev.md) for information about design decisions.

## Currently supported

- int128 - A 128 bit precision signed integer
- int_limited - An arbitrary, fixed size signed integer

# Large Number Factoring

As part of my second university semester, I implemented the quadratic sieve algorithm and a (unfinished) generic factoriser.
It lives in the directory `./factoring` under the namespace `Factoriser::`

For more information about the contents and usage, please see the [User Documentation](./factoring/documentation-user.md) or the [Developer Documentation](./factoring/documentation-dev.md).
