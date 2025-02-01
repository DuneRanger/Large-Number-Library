# Near Future

- Rename toString, so that it is recognisably exclusive to library classes

### int_limited

- testing

## Documentation

- Re-format the current benchmark results into a nice markdown table
- Note that adding a larger bitSize to a smaller bitSize results results in truncated addition (truncated from the MSB of the larger)
  - for multiplication, I *believe* it should multiply with enough accuracy that it would be equivalent to extra accuracy and then truncation to fit in the left side
  - for division, it only matters if the right hand side is larger than the dividend, for which it will return zero

# Long term goals

- Unlimited size integer
- (unlimited size/precision) floats
- Benchmark for different sets of numbers (i.e. Only less than 32 bit, only larger than 96 bit - just the extremes on both sides)
- Implement input from stream
- Improve toString() for *int_limited* and *int_unlimited* so as to not be wasteful for small bases
  - Also possibly improve the design, so that it doesn't require a vector of maxWordCount at once (implement it as a buffer for adding into the string)
- Add conversion from a string to int_limited (and also from any base)

# Undecided options

- Export bits for *int_limited* currently returns all words, including those that have no effect on the value (i.e. words after the MSW)
  - At the time of writing this, *int_unlimited* is not yet implemented, however it will definitely only be returning all the words that contain a value
- Conversion from an class instance into a standard library integer currently simply returns the bits as is (for easier bit manipulation)
  - This could possibly be changed to return the integer types max/min value, if the instance has a higher/lower value
- Consider whether toString() should allow conversion to bases larger than an unsigned 64 bit integer for *int_limited* and *int_unlimited*
- Speed up multiplication of small negative numbers by testing out if their two's complement is relatively small (less than half the bits)