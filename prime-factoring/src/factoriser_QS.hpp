#pragma once
#include <vector>
#include <cassert>
#include <cmath>
#include <cstdint>
#include "factoriser_basic.hpp"
#include "factoriser_math.hpp"
#include "../../int_limited.hpp"

// Since I decided to have the int_limited bit_size be a template,
// all of this has to live in a header file
template<int bit_size>
class factoriser_QS {
	typedef largeNumberLibrary::int_limited<bit_size> qs_int;
	typedef largeNumberLibrary::int_limited<2*bit_size> qs_int_double;
	typedef uint64_t ui64;

	class CustomBitset {
		private:
			std::vector<uint64_t> bits;
			std::size_t words;
			
		public:
			std::size_t size;

			CustomBitset(int _bits) {
				size = _bits;
				// size/64 and size%64
				words = 1 + (size >> 6) + bool(size & 0x3f);
				bits = std::vector<uint64_t>(words, 0);
			}

			const bool operator[](std::size_t const ind) const {
				// ind/64 and ind%64
				assert(ind < size);
				assert(ind >= 0);
				return bits[ind >> 6] >> (ind & 0x3f) & 1;
			}

			// Serves as an access point to an index
			void flip_bit(std::size_t const ind) {
				// ind/64 and ind%64
				assert(ind < size);
				assert(ind >= 0);
				int bit = ind & 0x3f;
				if (bit == 0) bits[ind >> 6] ^= 1;
				else bits[ind >> 6] ^= (uint64_t(1) << bit);
			}

			CustomBitset operator^=(CustomBitset const& rhs) {
				assert(size == rhs.size);
				for (int i = 0; i < words; i++) {
					this->bits[i] ^= rhs.bits[i];
				}
				return *this;
			}

			CustomBitset operator^(CustomBitset const& rhs) const {
				assert(size == rhs.size);
				CustomBitset result(size);
				for (int i = 0; i < words; i++) {
					result.bits[i] = this->bits[i] ^ rhs.bits[i];
				}
				return result;
			}

			bool operator==(CustomBitset const& rhs) const {
				assert (size == rhs.size);
				for (int i = 0; i < words; i++) {
					if (rhs.bits[i] != this->bits[i]) return false;
				}
				return true;
			}

			std::string to_string() const {
				std::string output = "";
				for (int i = 0; i < size; i++) {
					output += std::to_string((*this)[i]);
				}
				return output;
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
	
	// polynomials used for quadratic sieve (QS)
	struct QS_poly {
		qs_int A;
		qs_int B;
		qs_int C;

		// a vector of solutions of Q(x) = 0 (mod p)
		// where p is a prime from the factor base (with the same index)
		std::vector<ui64> solutions_mod_p;
	
		// Since this is a simple single-polynomial version of the quadratic sieve,
		// there is a special constructor for Q(x) = (A + x)^2 - C = (sqrt(kN) + x)^2 - kN
		QS_poly(qs_int const& kN) {
			// (sqrt(kN) + x)^2 - kN = x^2 + 2*sqrt(kN) + sqrt(kN)^2 - kN;
			A = 1;
			qs_int sqrt_kN = kN.isqrt()+1;
			B = sqrt_kN << 1;
			C = sqrt_kN*sqrt_kN - kN;
		}
		
		// If this was Multi-Polynomial QS, then they would be in the form of Q(x) = Ax^2 + Bx + C
		// And this constructor would be used
		QS_poly(qs_int const& a, qs_int const& b, qs_int const& c) {
			A = a;
			B = b;
			C = c;
		}

		// Returns the value of the polynomial for a given x
		qs_int operator()(qs_int const& x) const {
			return (A*x*x) + (B*x) + C;
		}
	};

	// The structure for a relation used in the quadratic sieve
	// Contains the relation/poly value, the solution to the quadratic residue
	// and the bitset of relation value's exponents
	struct relation {
		// the value of poly(x) mod kN
		qs_int poly_value;
		// the value of R^2 = (A + x)^2 = 0 (mod kN)
		qs_int residue_solution;

		// A list of all of the pairs prime exponents that make up poly_value
		// This is used to drastically improve find_factors_from_relations(), instead of multiplying 10000+ bit values
		// Big thanks to https://github.com/michel-leonard/C-Quadratic-Sieve/blob/main/quadratic-sieve.c#L1013
		// for making me realise that this method allows work completely modulo kN 
		std::vector<ui64> exponents;
		// A bitset of the parity of the exponents of poly_value for all prime in the factor base
		// It differs from the pairs in that its size is equal to the factor base 
		CustomBitset exponents_mod_2;

		relation(qs_int const& val, qs_int const& res, ui64 prime_count)
			: poly_value(val), residue_solution(res), exponents(prime_count), exponents_mod_2(prime_count) {}
	};

	// A struct for the global variables commonly used in quadratic sieve sub-functions
	// AKA - the variables that get passed a lot
	struct QS_global {
		qs_int N;
		// what we work with (modulo kN)
		qs_int kN;
		// the prime number smoothness bound
		ui64 B;

		// a vector of B-smooth primes for which kN is a quadratic residue
		std::vector<ui64> factor_base;
		int64_t sieve_start;
		ui64 sieve_interval;
	};


	// The following is a rudimentary implementation of the quadratic sieve algorithm
	// based on my understanding of how it works

	qs_int calc_kN(qs_int const& N) const {
		// Since I don't understand why most implementations increase kN to at least 115 bits, I'll just leave it be for now
		qs_int kN = N;
		if (debug) std::cout << "N = " << N << " (" << N.ilog2() << " bits) | kN = " << kN << " (" << kN.ilog2() << " bits)" << std::endl;
		return kN;
	}

	ui64 calc_B(qs_int const& kN) const {
		// A rough heuristic based on the algorithm's complexity
		// exp((0.5 + o(1))*(ln(N)ln(ln(N))^(0.5)))
		
		ui64 B = std::ceil(std::exp(0.51*std::sqrt(
							(kN.ilog2()) * std::log(kN.ilog2())
						)
					));
		B /= 6;
		if (debug) std::cout << "B = " << B << " | ";
		return B;
	}

	// Directly fills globals.factor_base with the found B-smooth primes
	void prepare_factor_base(QS_global& globals) const {
		std::vector<ui64> potential_primes;
		factoriser_basic::find_small_primes(globals.B, potential_primes);
		if (debug) std::cout << potential_primes.size() << " potential primes | ";
		for (ui64 prime : potential_primes) {
			if (factoriser_math::is_quadratic_residue(globals.kN, prime)) globals.factor_base.push_back(prime);
		}
		if (debug) std::cout << globals.factor_base.size() << " factor base size" << std::endl;;
	}
	
	void prepare_polynomials(QS_global const& globals, std::vector<QS_poly>& polynomials) const {
		// Uses the basic polynomial Q(x) = (sqrt(kN) + x)^2 - kN
		polynomials.push_back(QS_poly(globals.kN));

		for (QS_poly& poly : polynomials) {
			for (uint32_t i = 0; i < globals.factor_base.size(); i++) {
				ui64 prime = globals.factor_base[i];
				// A simplified case for the single polynomial QS
				qs_int rhs = globals.kN;
				ui64 root1 = factoriser_math::Tonelli_Shanks(rhs, prime);
				if (root1 == 0) continue;
				poly.solutions_mod_p.push_back(root1);
			}
		}

		if (debug) {
			std::cout << "Polynomials used: ";
			for (uint32_t i = 0; i < polynomials.size(); i++) {
				// special log for single polynomial QS
				std::cout << "(" << (polynomials[i].C + globals.kN).isqrt() << " + x)^2 - " << globals.kN << " | ";
				// std::cout << polynomials[i].A << "x^2 + " << polynomials[i].B << "x + " << polynomials[i].C << " | ";
			}
			std::cout << std::endl;
		}
	}

	void set_sieve_bounds(QS_global& globals) const {
		globals.sieve_start = 0;
		globals.sieve_interval = 2*globals.B;
	}

	void find_relation_candidates(QS_global globals, QS_poly const& poly, std::vector<relation>& candidates) const {
		ui64 fb_size = globals.factor_base.size();
		ui64 interval = globals.sieve_interval;
		// NOTE:
		// Only calculating the log_threshold for a few values is *significantly* faster
		// compared to calculating it for each poly(x) individually (though it is also less accurate)
		ui64 base = poly(globals.sieve_start + interval/10).ilog2();
		// We want a very low threshold, because verifying bad candidates is much slower than another sieve
		// Experimentally, + (base >> 3) means having 1-2x less correct verifications, but up to 4-10x less candidates per sieve
		ui64 threshold = (base >> 1) + (base >> 2) + (base >> 3);// - factoriser_math::count_bits(globals.factor_base.back());
		std::vector<ui64> log_thresholds(interval, threshold);

		std::vector<ui64> log_primes(fb_size);
		for (int i = 0; i < fb_size; i++) log_primes[i] = factoriser_math::count_bits(globals.factor_base[i]);
		std::vector<ui64> log_counts(interval, 0);

		if (sieve_debug) std::cout << "Prepared... ";

		// Solve it specially for the values (K + x)^2 = (sqrt(kN) + x)^x
		// Since we know this is the single polynomial version
		// And we won't have to convert Ax^2 + Bx + C into (K + L*x)^2 + M
		qs_int K = globals.kN.isqrt()+1;
		for (int i = 0; i < fb_size; i++) {
			ui64 prime = globals.factor_base[i];
			ui64 root1 = poly.solutions_mod_p[i];
			if (root1 == 0) continue;
			
			ui64 A = ui64(K%prime);
			ui64 offset = globals.sieve_start%prime;
			// since root1 is (A + x), then we need to extract x
			// x_1 = root1 - A (mod p
			// x_2 = (p - root1) - A (mod p)
			// We add an extra padding of `prime + ` to not underflow
			ui64 x_1 = prime + root1 - A - offset;
			if (x_1 >= prime) x_1 -= prime;

			for (int j = x_1; j < interval; j += prime) log_counts[j] += log_primes[i];

			if (prime != 2) {
				ui64 x_2 = prime + prime - root1 - A - offset;
				if (x_2 >= prime) x_2 -= prime;
	
				for (int j = x_2; j < interval; j += prime) log_counts[j] += log_primes[i];
			}
		}

		for (int i = 0; i < interval; i++) {
			if (log_counts[i] >= log_thresholds[i]){
				candidates.push_back(
					relation(poly(globals.sieve_start + i)%globals.kN, \
						(K + globals.sieve_start + i)%globals.kN, \
						fb_size)
				);
			}
		}
		if (sieve_debug) std::cout << candidates.size() << " candidates | ";
	}

	void verify_candidates(QS_global globals, std::vector<relation>& candidates, std::vector<relation>& verified) const {
		// currently the slowest part of the sieving process
		if (sieve_debug) std::cout << "Verifying... ";
		for (int i = 0; i < candidates.size(); i++) {
			qs_int value = candidates[i].poly_value;
			for (int j = 0; j < globals.factor_base.size(); j++) {
				ui64 prime = globals.factor_base[j];
				if (value % prime == 0) {
					while (value % prime == 0) {
						value /= prime;
						candidates[i].exponents[j]++;
						candidates[i].exponents_mod_2.flip_bit(j);
					}
				}
				if (value == 1) {
					verified.push_back(candidates[i]);
					break;
				}
			}
		}
		if (sieve_debug) std::cout << verified.size() << " verified" << std::endl;
	}

	// Gathers relations from all polynomials and sets a new intervals
	void sieve(QS_global& globals, std::vector<QS_poly> const& polynomials, std::vector<relation>& relations) const {
		if (sieve_debug) {
			std::cout << "Sieving from " << globals.sieve_start << " to " << (globals.sieve_start + globals.sieve_interval);
			std::cout << " (" << globals.sieve_interval << " values) | ";
		}
		for (QS_poly const& poly : polynomials) {
			std::vector<relation> candidates;
			find_relation_candidates(globals, poly, candidates);
			std::vector<relation> verified;
			verify_candidates(globals, candidates, verified);
			for (relation& v : verified) {
				relations.push_back(v);
			}
		}
		globals.sieve_start += globals.sieve_interval;
		if (10*relations.size() < globals.factor_base.size()) {
			globals.sieve_interval += (globals.sieve_interval >> 2);
		}
	}

	// Should only be called after enough relations have been gathered
	void prepare_matrix(std::vector<relation> const& relations, std::vector<CustomBitset>& matrix) const {
		for (int i = 0; i < relations.size(); i++) {
			matrix.push_back(relations[i].exponents_mod_2);
		}
		if (debug) std::cout << "Matrix (mod 2) of size " << relations.size() << " x " << relations[0].exponents_mod_2.size << " created" << std::endl;
	}

	// utilises gauss elimination to solve the matrix of exponents
	void solve_matrix(std::vector<CustomBitset>& matrix, std::vector<CustomBitset>& solutions) const {
		if (debug) std::cout << "Solving matrix with Gauss elimination... ";
		std::vector<CustomBitset> solution_matrix(matrix.size(), CustomBitset(matrix.size()));
		for (int i = 0; i < matrix.size(); i++) solution_matrix[i].flip_bit(i);

		int row_start = 0;
		for (int col = 0; col < matrix[0].size; col++) {
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
		if (debug) std::cout << solutions.size() << " solutions found" << std::endl;
	}

	void find_factors_from_relations(QS_global& globals, std::vector<relation> const& relations, std::vector<qs_int>& prime_factors) {
		using factoriser_math::element_in_vector;
		using factoriser_math::gcd;
		std::vector<CustomBitset> matrix_mod2;
		prepare_matrix(relations, matrix_mod2);

		std::vector<CustomBitset> solutions;
		solve_matrix(matrix_mod2, solutions);

		std::vector<qs_int> divisors;

		// NOTE:
		// Now that the Smoothness bound and factor base have changed sizes, this is no longer the slowest part of the algorithm
		// Because we only now usually find 5-50 solutions
		// However, the solution cap is still required, because sometimes we still find 1000+ solutions
		int solution_count = 0;
		const int solution_cap = 100;
		if (debug) std::cout << "Finding factors from " << std::min(solution_cap, solutions.size) << " solutions...";
		for (CustomBitset& bitset : solutions) {
			if (solution_count++ > solution_cap) break;
			qs_int_double res_sols = 1;
			qs_int_double poly_vals = 1;
			// list of exponents of the factor base that make up poly_vals
			std::vector<ui64> poly_vals_exps(globals.factor_base.size());
			// If we do not modulo, then we can end up multiplying 10000+ bit values
			// Thus we continually work with the already square values, which we can modulo throughout
			for (int i = 0; i < bitset.size; i++) {
				if (!bitset[i]) continue;
				res_sols *= relations[i].residue_solution;
				res_sols %= globals.N;
				for (int j = 0; j < globals.factor_base.size(); j++) {
					poly_vals_exps[j] += relations[i].exponents[j];
				}
			}
			for (int i = 0; i < globals.factor_base.size(); i++) {
				assert(poly_vals_exps[i]%2 == 0);
				// divides exponent by two to already square-root the value
				poly_vals *= factoriser_math::pow_mod<2*bit_size>(globals.factor_base[i], poly_vals_exps[i]>>1, globals.N);
				poly_vals %= globals.N;
			}
			
			qs_int factor_1, factor_2;
			if (res_sols > poly_vals) factor_1 = gcd(qs_int(res_sols - poly_vals), globals.N);
			else factor_1 = gcd(qs_int(poly_vals - res_sols), globals.N);

			factor_2 = gcd(qs_int(res_sols + poly_vals), globals.N);

			// The list of divisors should be relatively sparse when pruned liked this
			if (factor_1 != 1 && factor_1 != globals.N && !element_in_vector(factor_1, divisors)) {
				divisors.push_back(factor_1);
			}
			if (factor_2 != 1 && factor_2 != globals.N&& !element_in_vector(factor_2, divisors)) {
				divisors.push_back(factor_2);
			}
		}
		if (debug) std::cout << divisors.size() << " Unique divisors found | ";
		// Factorise composite divisors into primes
		std::vector<qs_int> possible_primes;
		{
			std::vector<qs_int> big_divisors;
			// First we sort out prime divisors and small primes in composites
			for (qs_int& divisor : divisors) {
				// skip prime divisors
				if (factoriser_basic::is_prime(divisor) && !element_in_vector(divisor, possible_primes)) {
					possible_primes.push_back(divisor);
					continue;
				}

				// get small primes and add them to possible factors
				for (ui64 prime : factoriser_basic::trial_division(divisor)) {
					divisor /= prime;
					if (!element_in_vector(divisor, possible_primes)) possible_primes.push_back(prime);
				}
				if (divisor == 1) continue;
				if (factoriser_basic::is_prime(divisor)) possible_primes.push_back(divisor);
				else if (!element_in_vector(divisor, big_divisors)) big_divisors.push_back(divisor);
			}

			// now we sort out big divisors individually
			factoriser_QS QS;
			for (qs_int& divisor : big_divisors) {
				if (debug) std::cout << std::endl << "Divisor " << divisor << " is being recursively factored by a new quadratic sieve instance!";
				for (qs_int prime : QS.quadratic_sieve(divisor)) {
					divisor /= prime;
					if (!element_in_vector(divisor, possible_primes)) possible_primes.push_back(prime);
				}
			}
			if (big_divisors.size() && debug) std::cout << std::endl;
		}
		for (qs_int& factor : possible_primes) {
			while (globals.N%factor == 0) {
				globals.N /= factor;
				prime_factors.push_back(factor);
			}
		}
		if (debug) std::cout << prime_factors.size() << " prime factors found" << std::endl;
	}

	public:
		bool debug = false;
		bool sieve_debug = false;
		factoriser_QS() {}
		factoriser_QS(bool _debug) : debug(_debug) {}
		factoriser_QS(bool _debug, bool _sieve_debug) : debug(_debug), sieve_debug(_sieve_debug) {}


		std::vector<qs_int> quadratic_sieve(qs_int const& value) {
			if (debug) std::cout << "QUADRATIC SIEVE DEBUG LOG:" << std::endl;
			QS_global globals;
			globals.N = value;
			globals.kN = calc_kN(globals.N);
			globals.B = calc_B(globals.kN);

			prepare_factor_base(globals);

			std::vector<QS_poly> polynomials;
			prepare_polynomials(globals, polynomials);
			set_sieve_bounds(globals);

			std::vector<relation> relations;
			// 100% is enough to guarantee a solution
			// 90% seems to work as well for most inputs, but I can't guarantee it's always enough
			ui64 percentage = 100;
			ui64 sieve_count = 0;
			ui64 sieve_start = globals.sieve_start;
			while (relations.size() < percentage*globals.factor_base.size()/100) {
				sieve(globals, polynomials, relations);
				sieve_count++;
			}
			if (debug) {
				std::cout << "Sieved from " << sieve_start << " to " << (globals.sieve_start + globals.sieve_interval);
				std::cout << " through " << sieve_count << " sieve iterations | ";
				std::cout << "Found " << relations.size() << " relations" << std::endl;
			}

			std::vector<qs_int> prime_factors;
			find_factors_from_relations(globals, relations, prime_factors);

			if (debug) std::cout << "QUADRATIC SIEVE COMPLETED" << std::endl;
			return prime_factors;
		}

		std::vector<qs_int> quadratic_sieve(int64_t value) {
			return quadratic_sieve(qs_int(value));
		}
};