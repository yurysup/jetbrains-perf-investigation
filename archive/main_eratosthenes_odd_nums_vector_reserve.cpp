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
            isPrime[2] = true;

            int sqrtMaxPrime = static_cast<int>(std::sqrt(maxPrime));
            // Starting from 3 and incrementing by 2 to check odd numbers only
            for (int candidate = 3; candidate <= sqrtMaxPrime; candidate += 2) {
                if (isPrime[candidate]) {
                    for (int primeMultiple = candidate * candidate; primeMultiple <= maxPrime; primeMultiple += candidate) {
                        isPrime[primeMultiple] = false;
                    }
                }
            }

            std::vector<int> primeNumbers;
            primeNumbers.reserve(maxPrime / 2);
            primeNumbers.push_back(2);  // Add 2 to prime numbers list
            for (int i = 3; i <= maxPrime; i += 2) {
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
