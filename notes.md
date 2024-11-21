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
$$

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