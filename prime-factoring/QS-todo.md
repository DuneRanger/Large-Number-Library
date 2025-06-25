# To implement:

- gcd(a, b) euclidean algorithm
- isQuadraticResidue mod p
- find quadratic residues (Euler's criterion)
- Choose smoothness bound = $exp(\frac{1}{2}\sqrt{\log N \cdot \log \log N})$
- Choose sieving interval
- isPerfectSquare modulo kN
- MatrixSolver modulo 2
- Tonelli-Shanks algorithm for sieving

# Implementation ordering

1. Calculate smoothness bound B
2. Find all prime numbers up to B
3. Euler's criterion to find quadratic residues in the found primes
4. Calculate the sieving interval (we need one more smooth relation than the primes in the factor base)
	- A generous estimate is six times the number of primes in the factor base
5. Calculate $(\sqrt{N} + x)^2 - N$ for the sieving interval
6. Solve for $x$ $(\sqrt{N} + x)^2 - N \eqq 0 (\mod p)$ for each prime $p$ in the factor base
7. Create and solve the matrix of exponents modulo 2
8. Combine the found relations from the matrix to produce $a^2 \eqq b^2 (\mod N)$
9. Calculate $gcd(a-b, N)$ and $gcd(a+b, N)$