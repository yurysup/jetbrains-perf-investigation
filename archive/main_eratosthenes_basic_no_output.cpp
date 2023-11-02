#include <iostream>
#include <vector>
#include <string>
#include <future>


class PrimeCalculator {
    public:
        static std::vector<int> getPrimes(int maxPrime) {
            std::vector<bool> isPrime(maxPrime + 1, true);
            isPrime[0] = false;
            isPrime[1] = false;

            int sqrtMaxPrime = static_cast<int>(std::sqrt(maxPrime));

            for (int candidate = 2; candidate <= sqrtMaxPrime; ++candidate) {
                if (isPrime[candidate]) {
                    for (int primeMultiple = candidate * candidate; primeMultiple <= maxPrime; primeMultiple += candidate) {
                        isPrime[primeMultiple] = false;
                    }
                }
            }

            std::vector<int> primeNumbers;
            for (int i = 2; i <= maxPrime; ++i) {
                if (isPrime[i]) {
                    primeNumbers.push_back(i);
                }
            }

            return primeNumbers;
        }
};

int main(int argc, char **argv) {
    PrimeCalculator::getPrimes(std::stoi(argv[1]));
    return 0;
}
