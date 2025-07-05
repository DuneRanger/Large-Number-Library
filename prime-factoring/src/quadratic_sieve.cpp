#pragma once
#include "headers.hpp"
#include "basic_factoriser.cpp"

namespace QS {
	class CustomBitset {
		private:
			std::vector<uint64_t> bits;
			
		public:
			std::size_t size;

			CustomBitset(int _bits) {
				size = _bits;
				// size/64 and size%64
				int words = 1 + (size >> 6) + bool(size & 0x3f);
				bits = std::vector<uint64_t>(words, 0);
			}
			// ~CustomBitset() {
			// 	delete[] bits;
			// }
			const bool operator[](std::size_t const ind) const {
				// ind/64 and ind%64
				assert(ind < size);
				assert(ind >= 0);
				return bits[ind >> 6] >> (ind & 0x3f) & 1;
			}

			void flip_bit(std::size_t const ind) {
				// ind/64 and ind%64
				assert(ind < size);
				assert(ind >= 0);
				int bit = ind & 0x3f;
				if (bit == 0) bits[ind >> 6] ^= 1;
				else bits[ind >> 6] ^= (1 << bit);
			}

			CustomBitset operator^=(CustomBitset const& rhs) {
				assert(size == rhs.size);
				int words = 1 + (size >> 6) + bool(size & 0x3f);
				for (int i = 0; i < words; i++) {
					this->bits[i] ^= rhs.bits[i];
				}
				return *this;
			}

			CustomBitset operator^(CustomBitset const& rhs) const {
				assert(size == rhs.size);
				CustomBitset result(size);
				int words = 1 + (size >> 6) + bool(size & 0x3f);
				for (int i = 0; i < words; i++) {
					result.bits[i] = this->bits[i] ^ rhs.bits[i];
				}
				return result;
			}
			
			void add(CustomBitset const& rhs) {
				*this ^= rhs;
			}

			void swap(CustomBitset& rhs) {
				CustomBitset temp = *this;
				*this = rhs;
				rhs = temp;
			}
	};
	
	// The following is a rudimentary implementation of the quadratic sieve algorithm, based on my limited understanding of how it works
	class QuadraticSieve {
		typedef uint64_t ui64;
		// polynomials used for QS
		// if this was Multi-Polynomial QS, then they would be in the form of Q(x) = Ax^2 + Bx + C
		// However since this is a simple single-polynomial version, then we have
		// Q(x) = (sqrt(N) + x)^2 - N = (A + x)^2 - B
		struct QS_poly {
			qs_int A;
			qs_int B;
			qs_int C;
	
			QS_poly(qs_int const& a, qs_int const& b, qs_int const& c) {
				A = a;
				B = b;
				C = c;
			}
			qs_int operator()(qs_int const& x) const { return (A+x)*(A+x) - C; }
			qs_int quad(qs_int const& x) const { return (A+x)*(A+x); }
		};

		// contains both the relation value and the solution to the quadratic residue
		// and the bitset of its exponents
		struct relation {
			// the value of poly(x) mod kN
			qs_int poly_value;
			// the value of R^2 = (A + x)^2 = kN mod kN
			qs_int residue_solution;

			CustomBitset exponents;

			relation(qs_int const& val, qs_int const& res, ui64 prime_count)
				: poly_value(val), residue_solution(res), exponents(prime_count) {}
		};

		private:
			std::vector<qs_int> factors;

			qs_int N; // what we're trying to factorise
			qs_int kN; // what we work with (modulo this)
			ui64 B; // smoothness bound

			std::vector<ui64> factor_base;
			std::vector<QS_poly> polynomials;
			
			int64_t sieve_start;
			ui64 sieve_interval;
			std::vector<relation> relations;
			
			#pragma region Helper
			// Finding out that this had to be modulo so as to not overflow took way too long
			ui64 pow_mod(ui64 n, ui64 exp, ui64 p) const {
				if (exp == 0) return 1;
				if (exp % 2) return (pow_mod(n, exp-1, p) * n) % p;
				n = pow_mod(n, exp/2, p);
				return (n * n) % p;
			}

			int calc_Jacobi_symbol(ui64 x, ui64 p) const {
				if (p%2 == 0) throw std::domain_error("Error: Even denominator used when calculating Jacobi symbol");
				ui64 a = x%p;
				int symbol = 1;
				// until we reach the base case (p will be 1 if they are coprime)
				while (a != 0) {
					// First change a to be an odd number
					// (2a | n) = (2 | n) * (a | n)
					while (a%2 == 0) {
						a /= 2;
						// (2 | n) = (-1)^((n^2 - 1) / 8)
						switch(p%8) {
							case 3: case 5:
							symbol = -symbol;
							break;
							// in the other cases, the symbol stays the same
						}
					}
					// this section only applies if a,p are coprime
					// but if they aren't, then we can guarantee p will be non-zero at the end
					// (a | p)*(p | a) = (-1)^((a-1)/2 * (p-1)/2)
					// so (a | p) = (-1)^((a-1)/2*(p-1)/2) / (p | a)
					// and (-1)^((a-1)/2*(p-1)/2) is -1 when the following applies
					if (a%4 == 3 && p%4 == 3) symbol = -symbol;
					ui64 t = a;
					a = p % a;
					p = t;
				}
	
				if (p != 1) return 0;
				else return symbol;
			}
	
			// Assumes N != 0 (mod p)
			bool is_quadratic_residue(qs_int const& N, ui64 p) const {
				if (p == 2) return true;
				return calc_Jacobi_symbol(ui64(N%p), p) == 1;
			}
			// Assumes N != 0 (mod p)
			bool is_quadratic_residue(ui64 N, ui64 p) const {
				if (p == 2) return true;
				return calc_Jacobi_symbol(N%p, p) == 1;
			}

			ui64 count_bits(ui64 val) const {
				ui64 bits = 0;
				for (ui64 i = 16; i > 0; i >>= 1) {
					ui64 t = val >> i;
					if (t) { val = t; bits += i; }
				}
				if (val) bits += 1;
				return bits;
			}

			// Mostly just implements the pseudo code from:
			// https://en.wikipedia.org/wiki/Tonelli%E2%80%93Shanks_algorithm#The_algorithm
			ui64 Tonelli_Shanks(QS_poly const& poly, ui64 prime) const {
				// We're solving (A + x)^2 = -C (mod p)
				// Tonelli_Shanks algorithm finds R such, that R^2 = N (mod p)
				ui64 N = ui64((-poly.C)%prime);
				if (prime == 2) return poly.A%2 != N%2;
				// because n^((p-1)/2) = 1 (mod p), we find Q,S such that Q*2^S = p-1
				ui64 Q = prime-1, S = 0;
				while (Q%2 == 0) { Q /= 2; S++; }
				// Next we find a quadratic non-residue (where z^((p-1)/2) = -1 (mod p))
				ui64 z = 2;
				while (is_quadratic_residue(z, prime)) z++;
				ui64 M = S;
				ui64 c = pow_mod(z, Q, prime);
				ui64 t = pow_mod(N, Q, prime);
				ui64 R = pow_mod(N, (Q+1)/2, prime);
				while (t > 1) {
					ui64 t_exp = (t*t) % prime;
					ui64 i = 1;
					while (t_exp != 1 && i < M) { t_exp = (t_exp*t_exp)%prime; i++; }
					if (i == M) throw std::domain_error("Error: " + kN.toString() + " (kN) isn't a quadratic residue modulo " + std::to_string(prime));
					ui64 b = pow_mod(c, 1 << (M-i-1), prime);
					M = i;
					c = (b*b)%prime;
					t = (t*c)%prime;
					R = (R*b)%prime;
				}
				if (t == 0) return 0;
				return R;
			}
			#pragma endregion Helper
	
			#pragma region Main
			// Requires N to be defined
			void prepare_kN() {
				// Since I don't understand why most implementations increase kN to at least 115 bits, I'll just leave it be for now
				kN = N;
				// if (kN.ilog2() < 80) kN <<= (80 - kN.ilog2());
				if (debug) std::cout << "N = " << N << " (" << N.ilog2() << " bits) | kN = " << kN << " (" << kN.ilog2() << " bits)" << std::endl;
			}
	
			// Requires kN to be defined
			void prepare_B() {
				// A rough heuristic (the complexity of this algorithm)
				// exp((0.5 + o(1))*(ln(N)ln(ln(N))^(0.5) ))
				const double log2e = 1.44;
				B = std::ceil(std::exp(0.51*std::sqrt(
							(kN.ilog2()) * std::log(kN.ilog2())
						)
					));
				if (debug) std::cout << "B = " << B << " | ";
			}
	
			// Requires B to be defined
			void prepare_factor_base() {
				std::vector<ui64> potential_primes = find_small_primes<ui64>(B);
				if (debug) std::cout << potential_primes.size() << " potential primes | ";
				for (ui64 prime : potential_primes) {
					if (is_quadratic_residue(kN, prime)) factor_base.push_back(prime);
				}
				if (debug) std::cout << factor_base.size() << " factor base size" << std::endl;;
			}

			// requires kN to be defined
			void prepare_polynomials() {
				// Uses the basic polynomial Q(x) = (sqrt(N) + x)^2 - N
				polynomials.push_back(QS_poly(kN.isqrt()+1, 0, -kN));
				// !Warning! using multiple polynomials in the form of Q(x) = Ax^2 + Bx + C
				// requires changing the QS_poly struct and some details in Tonelli_Shanks and find_relation_candidates

				if (debug) {
					std::cout << "Polynomials used: ";
					for (int i = 0; i < polynomials.size(); i++) {
						std::cout << "(" << polynomials[i].A << " + x)^2 " << polynomials[i].C << " | ";
					}
					std::cout << std::endl;
				}
			}

			// requires B to be defined
			void prepare_sieve_bounds() {
				sieve_start = 0;
				sieve_interval = 10*B;
			}
			
			void find_relation_candidates(int64_t start, ui64 interval, QS_poly const& poly, std::vector<relation>& candidates) const {
				// NOTE:
				// Only calculating the log_threshold for a few values is *significantly* faster
				// compared to calculating it for each poly(x) individually (though it is also less accurate)
				ui64 base = poly(start).ilog2();
				ui64 threshold = (base >> 1) + (base >> 2);
				std::vector<ui64> log_thresholds(interval, threshold);

				std::vector<ui64> log_primes(factor_base.size());
				for (int i = 0; i < factor_base.size(); i++) log_primes[i] = count_bits(factor_base[i]);
				std::vector<ui64> log_counts(interval, 0);

				if (debug) std::cout << "Prepared... ";

				for (int i = 0; i < factor_base.size(); i++) {
					ui64 prime = factor_base[i];
					ui64 raw_root = Tonelli_Shanks(poly, prime);
					// since the raw_root is (A + x), then we need to get x1 and x2
					// x_1 = raw_root - A (mod p)	x_2 = (p - raw_root) - A (mod p)
					// We add an extra padding of `prime + ` to not underflow
					ui64 A = ui64(poly.A%prime);
					ui64 offset = start%prime;
					ui64 x_1 = 0, x_2 = 0;
					if (raw_root != 0){
						x_1 = prime + raw_root - A - offset;
						if (x_1 >= prime) x_1 -= prime;
						x_2 = prime + prime - raw_root - A - offset;
						if (x_2 >= prime) x_2 -= prime;
					}
					for (int j = x_1; j < interval; j += prime) log_counts[j] += log_primes[i];
					if (x_1 != x_2) for (int j = x_2; j < interval; j += prime) log_counts[j] += log_primes[i];
				}

				for (int i = 0; i < interval; i++) {
					if (log_counts[i] >= log_thresholds[i]) candidates.push_back(relation(poly(start + i)%kN, (poly.A + start + i)%kN, factor_base.size()));
				}
				if (debug) std::cout << candidates.size() << " candidates | ";
			}

			void verify_candidates(std::vector<relation> const& candidates, std::vector<relation>& verified) const {
				if (debug) std::cout << "Verifying... ";
				for (int i = 0; i < candidates.size(); i++) {
					qs_int value = candidates[i].poly_value;
					CustomBitset exponents(factor_base.size());
					for (int j = 0; j < factor_base.size(); j++) {
						ui64 prime = factor_base[j];
						while (value % prime == 0) {
							value /= prime;
							exponents.flip_bit(j);
						}
						if (value == 1) {
							verified.push_back(candidates[i]);
							verified.back().exponents = exponents;
							break;
						}
					}
				}
				if (debug) std::cout << verified.size() << " verified" << std::endl;
			}

			// Gathers relations from all polynomials, always from new intervals
			// Also directly adds verified relations as rows to the matrix to save time
			void sieve() {
				if (debug) {
					std::cout << "Sieving from " << sieve_start << " to " << (sieve_start + sieve_interval);
					std::cout << " (" << sieve_interval << " values) | ";
				}
				for (QS_poly const& poly : polynomials) {
					std::vector<relation> candidates;
					find_relation_candidates(sieve_start, sieve_interval, poly, candidates);
					std::vector<relation> verified;
					verify_candidates(candidates, verified);
					for (relation& v : verified) {
						relations.push_back(v);
					}
				
				}
				sieve_start += sieve_interval;
				if (10*relations.size() < factor_base.size()) sieve_interval += (sieve_interval >> 2);
			}
	
			// Should only be called after enough relations have been gathered
			void prepare_matrix(std::vector<CustomBitset>& matrix) const {
				for (int i = 0; i < relations.size(); i++) {
					matrix.push_back(relations[i].exponents);
				}
				if (debug) std::cout << "Matrix (mod 2) of size " << relations.size() << " x " << factor_base.size() << " created" << std::endl;
			}
	
			// utilises gauss elimination to solve the matrix of exponents
			void solve_matrix(std::vector<CustomBitset>& matrix, std::vector<CustomBitset>& solutions) const {
				if (debug) std::cout << "Solving matrix with Gauss elimination... ";
				std::vector<CustomBitset> solution_matrix(matrix.size(), CustomBitset(matrix.size()));
				for (int i = 0; i < matrix.size(); i++) solution_matrix[i].flip_bit(i);

				int row_start = 0;
				for (int col = 0; col < factor_base.size(); col++) {
					int row = row_start;
					for (; row < matrix.size(); row++) if (matrix[row][col]) break;
					if (row == matrix.size()) continue;

					matrix[row_start].swap(matrix[row]);
					solution_matrix[row_start].swap(solution_matrix[row]);
					
					int elim_ind = row_start;
					row_start++;
					for (row = row_start; row < matrix.size(); row++) {
						if (matrix[row][col]) {
							matrix[row].add(matrix[elim_ind]);
							solution_matrix[row].add(solution_matrix[elim_ind]);
						}
					}
				}
				for (int i = row_start; i < matrix.size(); i++) {
					solutions.push_back(solution_matrix[i]);
				}
				if (debug) std::cout << "Solved" << std::endl;
			}
	
			void find_factors_from_relations() {
				std::vector<CustomBitset> matrix_mod2;
				prepare_matrix(matrix_mod2);
				std::vector<CustomBitset> solutions;
				solve_matrix(matrix_mod2, solutions);
			}

			void find_factors() {}
			#pragma endregion Main
	
		public:
			bool debug = false;

			QuadraticSieve() {};
			QuadraticSieve(bool _debug) : debug(_debug) {};

			std::vector<qs_int> factorise(qs_int value) {
				if (debug) std::cout << "Input Value: " << value << std::endl;
				std::vector<ui64> small_factors = trial_division(value);
				for (ui64 prime : small_factors) {
					value /= prime;
					factors.push_back(prime);
				}
				if (value == 1) return factors;
				N = value;
				prepare_kN();
				prepare_B();
				prepare_factor_base();
				prepare_polynomials();
				prepare_sieve_bounds();
				while (relations.size() <= factor_base.size()) sieve();
				
 				polynomials.clear();

				find_factors_from_relations();

				factor_base.clear();
				relations.clear();
				return factors;
			}
			// std::vector<qs_int> factorise(std::string value) {
			// 	for (int i = 0; i < value.size(); i++) {
			// 		if (value[i] < '0' || value[i] > '9') throw std::logic_error("Error: invalid numerical string for Factoriser::factorise");
			// 	}
			// 	std::string max_val = qs_int::MAX_VALUE().toString();
			// 	if (value.size() > max_val.size()) throw std::logic_error("Error: string value cannot be represented by qs_int for Factoriser::factorise\n(The max value is: " + qs_int::MAX_VALUE().toString() + ")");
			// 	for (int i = 0; i < value.size(); i++) {
			// 		if (value[i] < max_val[i]) break;
			// 		if (value[i] == max_val[i]) continue;
			// 		throw std::logic_error("Error: string value cannot be represented by qs_int for Factoriser::factorise\n(The max value is: " + qs_int::MAX_VALUE().toString() + ")");
			// 	}
			// 	return factorise(qs_int(value));
			// }
	};
}

