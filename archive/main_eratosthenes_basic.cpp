#include <iostream>
#include <vector>
#include <string>
#include <future>


class PrimeCalculator {
public:
    static std::vector<int> getPrimes(int maxPrime) {
        std::vector<int> primeNumbers;
        if (maxPrime < 2) {
            return primeNumbers;
        }
        std::vector<bool> isPrime(maxPrime + 1L, true);
        isPrime[0] = false;
        isPrime[1] = false;

        int sqrtMaxPrime = static_cast<int>(std::sqrt(maxPrime));

        for (int candidate = 2; candidate <= sqrtMaxPrime; ++candidate) {
            if (isPrime[candidate]) {
                for (long primeMultiple = candidate * candidate; primeMultiple <= maxPrime; primeMultiple += candidate) {
                    isPrime[primeMultiple] = false;
                }
            }
        }

        for (long i = 2; i <= maxPrime; ++i) {
            if (isPrime[i]) {
                primeNumbers.push_back(i);
            }
        }

        return primeNumbers;
    }
};

int main(int argc, char **argv) {
    std::vector<int> primeNumbers = PrimeCalculator::getPrimes(std::stoi(argv[1]));
    if (!primeNumbers.empty()) {
        std::cout << primeNumbers.back() << std::endl;
    };
    return 0;
}
