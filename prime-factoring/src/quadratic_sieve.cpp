#pragma once
#include "headers.hpp"
#include "basic_factoriser.cpp"

namespace QS {
	class CustomBitset {
		public:
			std::vector<uint64_t> bits;

			CustomBitset() {}
			CustomBitset(int _bits) {
				// _bits/64 and _bits%64
				int words = 1 + (_bits >> 6) + bool(_bits & 0x3f);
				bits = std::vector<uint64_t>(words , 0);
			}
			const bool operator[](std::size_t ind) const {
				// ind/64 and ind%64
				return bits[ind >> 6] >> (ind & 0x3f) & 1;
			}

			void flip_bit(std::size_t ind) {
				// ind/64 and ind%64
				int bit = ind & 0x3f;
				if (bit == 0) bits.at(ind >> 6) ^= 1;
				else bits.at(ind >> 6) ^= (1 << bit);
			}

			CustomBitset operator^=(const CustomBitset& rhs) {
				int words = this->bits.size();
				assert(words == rhs.bits.size());
				for (int i = 0; i < words; i++) {
					this->bits[i] ^= rhs.bits[i];
				}
				return *this;
			}

			CustomBitset operator^(const CustomBitset& rhs) {
				int words = this->bits.size();
				assert(words == rhs.bits.size());
				CustomBitset result(words*64);
				for (int i = 0; i < words; i++) {
					result.bits[i] = this->bits[i] ^ rhs.bits[i];
				}
				return result;
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
	
			QS_poly(const qs_int& a, const qs_int& b, const qs_int& c) {
				A = a;
				B = b;
				C = c;
			}
			qs_int operator()(const qs_int& x) { return (A+x)*(A+x) - C; }
			qs_int quad(const qs_int& x) { return (A+x)*(A+x); }
		};
		public:
			qs_int N; // what we're trying to factorise
			qs_int kN; // what we work with (modulo this)
			ui64 B; // smoothness bound
			std::vector<ui64> factor_base;
			std::vector<qs_int> factors;
			std::vector<QS_poly> polynomials;
	
			std::vector<qs_int> relations;
			std::vector<CustomBitset> matrix_mod2;
	
			#pragma region Helper
			// Finding you this had to be modulo to not overflow took way too long
			ui64 pow_mod(ui64 n, ui64 exp, ui64 p) {
				if (exp == 0) return 1;
				if (exp % 2) return (pow_mod(n, exp-1, p) * n) % p;
				n = pow_mod(n, exp/2, p);
				return (n * n) % p;
			}

			int calc_Jacobi_symbol(ui64 x, ui64 p) {
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
							// in the other cases, the symbol stays the same
							case 3: case 5:
								symbol = -symbol;
								break;
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
			bool is_quadratic_residue(qs_int& N, ui64 p) {
				if (p == 2) return true;
				return calc_Jacobi_symbol(ui64(N%p), p) == 1;
			}
			// Assumes N != 0 (mod p)
			bool is_quadratic_residue(ui64 N, ui64 p) {
				if (p == 2) return true;
				return calc_Jacobi_symbol(N%p, p) == 1;
			}

			ui64 count_bits(ui64 val) {
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
			ui64 Tonelli_Shanks(QS_poly poly, ui64 prime) {
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
				std::cout << "N = " << N  << '\n';
				std::cout << "kN = " << kN << '\n';
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
				std::cout << "B = " << B << '\n';
			}
	
			// Requires B to be defined
			void prepare_factor_base() {
				std::vector<ui64> potential_primes = find_small_primes<ui64>(B);
				for (ui64 prime : potential_primes) {
					if (is_quadratic_residue(kN, prime)) factor_base.push_back(prime);
				}
				// for (int i = 0; i < factor_base.size(); i++) {
				// 	std::cout << factor_base[i] << ", ";
				// }
				// std::cout << std::endl;
				std::cout << "Factor base size is " << factor_base.size() << std::endl;
			}

			// requires kN to be defined
			void prepare_polynomials() {
				// Uses the basic polynomial Q(x) = (sqrt(N) + x)^2 - N
				polynomials.push_back(QS_poly(kN.isqrt()+1, 0, -kN));
				// !Warning! using multiple polynomials in the form of Q(x) = Ax^2 + Bx + C
				// requires changing the QS_poly struct and some details in Tonelli_Shanks and find_relation_candidates
				std::cout << "Polynomials used: ";
				for (int i = 0; i < polynomials.size(); i++) {
					std::cout << "(" << polynomials[i].A << " + x)^2 " << polynomials[i].C << " | ";
				}
				std::cout << std::endl;
			}

			std::vector<qs_int> find_relation_candidates(ui64 max, QS_poly poly) {
				std::vector<qs_int> values(max);
				std::vector<ui64> log_thresholds(max);
				for (int i = 0; i < max; i++) {
					values[i] = poly(i);
					log_thresholds[i] = values[i].ilog2() >> 10;
				}
				std::vector<ui64> log_primes(factor_base.size());
				for (int i = 0; i < factor_base.size(); i++) log_primes[i] = count_bits(factor_base[i]);
				std::vector<ui64> log_counts(max, 0);

				for (int i = 0; i < factor_base.size(); i++) {
					ui64 prime = factor_base[i];
					ui64 raw_root = Tonelli_Shanks(poly, prime);
					// since the raw_root is (A + x), then we need to get x1 and x2
					// x_1 = raw_root - A (mod p)	x_2 = (p - raw_root) - A (mod p)
					// We'll add extra "padding" of `prime + ` to not underflow
					ui64 A = ui64(poly.A%prime);
					ui64 x_1, x_2;
					x_1 = prime + raw_root - A;
					if (x_1 > prime) x_1 -= prime;
					x_2 = prime + prime - raw_root - A;
					if (x_2 > prime) x_2 -= prime;
					for (int j = x_1; j < max; j += prime) log_counts[j] += log_primes[i];
					if (x_1 != x_2) for (int j = x_2; j < max; j += prime) log_counts[j] += log_primes[i];
				}

				std::vector<qs_int> candidates;
				for (int i = 0; i < max; i++) {
					if (log_counts[i] >= log_thresholds[i]) candidates.push_back(values[i]%kN);
				}
				std::cout << "Found " << candidates.size() << " candidates out of " << max << " values" << std::endl;
				return candidates;
			}

			std::vector<qs_int> verify_candidates(std::vector<qs_int>& candidates) {
				std::vector<qs_int> verified;
				for (int i = 0; i < candidates.size(); i++) {
					qs_int value = candidates[i];
					for (ui64 prime : factor_base) {
						while (value % prime == 0) value /= prime;
					}
					if (value == 1) verified.push_back(candidates[i]);
				}
				std::cout << "Verified " << verified.size() << " candidates out of " << candidates.size() << " candidates" << std::endl;
				return verified;
			}

			// sieves from [min, max), returns the raw values found from sieving
			std::vector<qs_int> sieve(ui32 max, QS_poly poly) {
			std::vector<qs_int> sieve(ui64 max, QS_poly poly) {
				std::cout << "Sieving from 0 to " << max << '\n';
				std::vector<qs_int> candidates = find_relation_candidates(max, poly);
				return verify_candidates(candidates);
			}
	
			// Should only be called after enough relations have been gathered
			void create_matrix() {
				std::cout << "Creating matrix mod 2 of size " << relations.size() << " x " << factor_base.size() << std::endl;
				ui64 row_size = factor_base.size();
				for (int i = 0; i < relations.size(); i++) {
					matrix_mod2.push_back(CustomBitset(row_size));
					qs_int value = relations[i];
					for (int j = 0; j < factor_base.size(); j++) {
						ui64 prime = factor_base[j];
						while (value % prime == 0) {
							value /= prime;
							matrix_mod2[i].flip_bit(j);
						}
					}
				}
			}
	
			void solve_matrix() {}
	
			void find_factors() {}
			#pragma endregion Main
	
		public:
			bool debug = false;

			std::vector<qs_int> factorise(qs_int value) {
				N = value;
				prepare_kN();
				prepare_B();
				prepare_factor_base();
				prepare_polynomials();
				for (QS_poly poly : polynomials) {
					std::vector<qs_int> some_relations = sieve(6*B + 1000, poly);
					for (qs_int& val : some_relations) relations.push_back(val);
				}
 				polynomials.clear();
				create_matrix();
				factor_base.clear();
				solve_matrix();
				matrix_mod2.clear();
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

