#include <iostream>
#include <vector>
#include <string>
#include <future>


class PrimeCalculator {
public:
    static std::vector<int> getPrimes(int maxPrime) {
        std::vector<int> primeNumbers;

        for (int j = 2; j <= maxPrime; ++j) {
            primeNumbers.push_back(j);
        }

        std::vector<int> primeNumbersToRemove;
        std::mutex primeNumbersToRemoveMutex;

        std::vector<std::future<void>> futures;
        for (int candidate: primeNumbers) {
            auto future = std::async([candidate, &primeNumbers, &primeNumbersToRemove, &primeNumbersToRemoveMutex]() {
                try {
                    isPrime(primeNumbers, candidate);
                } catch (std::exception &e) {
                    std::unique_lock lock(primeNumbersToRemoveMutex);
                    primeNumbersToRemove.push_back(candidate);
                }
            });
            futures.push_back(std::move(future));
        }
        for (auto &future: futures) {
            future.wait();
        }

        for (int toRemove: primeNumbersToRemove) {
            primeNumbers.erase(std::find(primeNumbers.begin(), primeNumbers.end(), toRemove));
        }

        return primeNumbers;
    }
private:
    static void isPrime(const std::vector<int> &primeNumbers, int candidate) {
        for (int j = 0; j < candidate - 2; ++j) {
            if (candidate % primeNumbers[j] == 0) {
                throw std::exception();
            }
        }
    }
};

int main(int argc, char **argv) {
    for (int prime: PrimeCalculator::getPrimes(std::stoi(argv[1]))) {
        std::cout << prime << std::endl;
    }

    return 0;
}
