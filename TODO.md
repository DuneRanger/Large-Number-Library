## Documentation

- Re-format the current benchmark results into a nice markdown table

# Long term goals

- Unlimited size integer
- (unlimited size/precision) floats
- Benchmark for different sets of numbers (i.e. Only less than 32 bit, only larger than 96 bit - just the extremes on both sides)
- Implement input from stream
- Improve toString() so as to not be wasteful for small bases
  - Also possibly improve the design, so that it doesn't require a vector of maxWordCount at once (implement it as a buffer for adding into the string)
  - Maybe make it a function directly in the namespace for all classes?
- Add conversion from a string to int_limited (and also from any base)

# Undecided options

- Consider whether toString() should allow conversion to bases larger than an unsigned 64 bit integer for `int_limited`
- Speed up multiplication of small negative numbers by testing out if their two's complement is relatively small (less than half the bits)
  - This can be done by simply checking their MSW and negating the result based on the sign
  - Will be implemented after benchmarking for `int_limited` is implemented