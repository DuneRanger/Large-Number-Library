# Long term goals

- Unlimited size integer
- (unlimited size/precision) floats
- Implement input from stream
- Improve toString() so as to not be wasteful for small bases
  - Also possibly improve the design, so that it doesn't require a vector of maxWordCount at once (implement it as a buffer for adding into the string)
  - Maybe make it a function directly in the namespace for all classes?

# Undecided options

- Speed up multiplication of small negative numbers by testing out if their two's complement is relatively small (less than half the bits)
  - This can be done by simply checking their MSW and negating the result based on the sign
  - Will be implemented after benchmarking for `int_limited` is implemented