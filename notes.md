$$
a = A + B = A_{bin}\cdot 2^{x} + B
\\
c = C + D = C_{bin}\cdot 2^{x} + D
$$

# Division

$$
\\ \ \\
\frac{a}{c} = \frac{A+B}{C + D}
= \frac{A}{C + D} + \frac{B}{C + D}
$$

## if $C = 0$

$$
\frac{A}{C + D} + \frac{B}{C + D}
=\frac{A}{D} + \frac{B}{D}
= \frac{A_{bin} \cdot 2^{x}}{D} + \frac{B}{D}
$$v

### Version 1
$$
= \frac{A_{bin}}{D}\cdot 2^{x} + \frac{(A_{bin} \mod D)\cdot 2^{x}}{D} + \frac{B}{D}
\\ \ \\
= \frac{A_{bin}}{D}\cdot 2^{x} + \frac{(A_{bin} \mod D)\cdot 2^{x}}{D} + \frac{B}{D}
$$
But $\frac{(A_{bin} \mod D)\cdot 2^{x}}{D}$ requires underflow control (manual division) 
<!-- ### Version 2
$$
= \frac{2^{x}}{D}\cdot A_{bin}  + \frac{(2^{x} \mod D)\cdot A_{bin}}{D} + \frac{B}{D}
\\ \ \\
= \frac{2^{x}-1+1}{D}\cdot A_{bin}  + \frac{(2^{x} \mod D)\cdot A_{bin}}{D} + \frac{B}{D}
\\ \ \\
= \bigg(\frac{2^{x}-1}{D} + \frac{1+ (2^{x}-1)\mod D}{D}\bigg)\cdot A_{bin}  + \frac{(2^{x} \mod D)\cdot A_{bin}}{D} + \frac{B}{D}
\\ \ \\
= \bigg(\frac{2^{x}-1}{D} + \frac{1+ (2^{x}-1)\mod D}{D}\bigg)\cdot A_{bin}  + \frac{((1+(2^{x}-1) \mod D)\mod D)\cdot A_{bin}}{D} + \frac{B}{D}
$$ -->

## if $C \neq 0$

$$
\frac{A}{C + D} + \frac{B}{C + D}
=\frac{A}{C+D}
$$


# Multiplication

$$
a \cdot c = (A+B)(C + D)
$$

## if $C = 0$

$$
(A+B)(C + D) = (A+B)D = AD + BD
$$
But $BD$ requires overflow control (manual multiplication)

## if $C \neq 0$

$$
(A+B)(C+D) = A(C+D) + B(C+D) = AD + B(C+D)
\\
= AD + BC + BD
$$
Only $BD$ requires manual multiplication

I have now (later) found out that this is the basics required for the Karatsuba algorithm. \
The main difference is that I can afford to simply ignore $AC$, since it completely overflows, meaning that calculating $AD + BC$ with the help of $AC$ and $BD$ isn't actually viable \
The main overhead is calculating over the barrier between the two unsigned 64 bit integers, which I believe can't really be solved with the algorithm

Update: I read the wiki article and now and I can see how the recursive application can be useful. I'm just a bit afraid that the overhead of constructing the large integer class might slow it down quite a bit \
In any case, I'll attempt to implement it and then I'll check out the speed differenceIn any case, I'll attempt to implement it and then I'll check out the speed difference

Notes about compiling with optimization:
- O1 doesn't compile out the for loop, but it may possibly compile out the computation (an empty for loop took longer than one with addition)
- O2 compiles out for loops, but with an if statement it seemed to leave it alone
