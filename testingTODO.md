# int_limited

## Arithmetic

- Figure out if I need to add the bitSize template to all functions, or if I can just let it be without the template and have the operations defined regardless of the bitSize
- Double check addition, if it works correctly when adding a shorter, negative number (am I doing extension correctly)
- Note that updating LSW and MSW in any arithmetic that make use of bit operations, where they are also updated, is redundant

## Printing

- Test toString()

## Assignment

- Test value assignment between int_limited for different sizes
- Test truncation of extra bits for bitSizes not equal to a multiple of 64