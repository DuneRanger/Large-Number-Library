#include "../../../int_limited.hpp"
#include "../../factoriser_QS.hpp"
#include "../../factoriser_basic.hpp"
#include <chrono>
#include <iostream>
using largeNumberLibrary::int_limited;

std::vector<int> RSA_bits = {60, 70, 80, 100, 120, 140, 160, 180};

std::vector<std::vector<int_limited<256>>> test_values {
	{// 60 bit
	"633786492115673591",
	"215196598597425751",
	"384806215841869231",
	"439949102374895341",
	"145594534266203497"},
	{// 70 bit
	"291755493748243587619",
	"196789883117858327977",
	"642025467512630956151",
	"416659355868161387893",
	"492922449205586213207"},
	{// 80 bit
	"177759468966084069834499",
	"332765771161739973587659",
	"390311542772053014440773",
	"489164367749489197985861",
	"805001266314169056206141"},
	{// 100 bit
	"115702745353349312768573158633",
	"749551740654508967393414810519",
	"503425059313610434321985735989",
	"321256075258204917421630273489",
	"176842339732311848316594436463"},
	{// 120 bit
	"390098036545054063894582218647680237",
	"588618191756126046805597498319277397",
	"734479054409889317689156072167393527",
	"343349517373064096516854932930466241",
	"171688667276137031920680964082710961"},
	{// 140 bit
	"619694519146380221462141251832954405827753",
	"466361409027216134503565891704630069831529",
	"361741414448979637019939678577536904845857",
	"424658376862972230767306900502257461915859",
	"210091765118539776637258553791873756255841"},
	{// 160 bit
	"432917350727434323848110078785208181590163681509",
	"180797353448258645008630610007988566550747397563",
	"338819842737322034563934135675418741078729626639",
	"119055539352464310913872320714419068233583072941",
	"395736165982637031308027406392903192414767036549"},
	{// 180 bit
	"280084928794816349659746015016742688585234971175069443",
	"274440551306310144918682955918628005616122259860020733",
	"523420860930754706475607096964092940201329859064738703",
	"196873789160214104761110492397752708243734120989333769",
	"457476353909214471442470049095041949300318347903225717"},
};

int main() {
	bool QS_debug = false, sieve_debug = false;
	Factoriser::QuadraticSieve<256> QS(QS_debug, sieve_debug);
	Factoriser::Basic::prepare_primes();

	for (int i = 0; i < test_values.size(); i++) {
		double times = 0;
		std::cout << "Testing " << RSA_bits[i] << " bit values" << std::endl;
		for (int j = 0; j < test_values[i].size(); j++) {
			int_limited<256> value = test_values[i][j];
			auto start = std::chrono::steady_clock::now();
			std::vector<int_limited<256>> factors = QS.factorise(value);
			auto end = std::chrono::steady_clock::now();
	
			assert(factors.size() == 2);
			if (QS_debug) std::cout << "Factors: ";
			int_limited<256> test = 1;
			for (int_limited<256> factor : factors) {
				assert(Factoriser::Basic::is_prime(factor));
				if (QS_debug) std::cout << factor << " ";
				test *= factor;
			}
			assert (value == test);
			double elapsed = std::chrono::duration<double>(end - start).count();
			if (QS_debug) {
				std::cout << "\nFactors were asserted to be prime and their product was correct\n";
				std::cout << "Factorisation took " << elapsed << " seconds\n" << std::endl;
			} else {
				std::cout << elapsed << "s | ";
			}
			times += elapsed;
		}
		times /= test_values[i].size();
		std::cout << std::endl;
		std::cout << RSA_bits[i] << " bit RSA values took " << times << " second to factorise, on average\n" << std::endl;
	}
}