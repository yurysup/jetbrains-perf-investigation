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
        if (!isPrime(primeNumbers, candidate)) {
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
    static bool isPrime(const std::vector<int> &primeNumbers, int candidate) {
      int sqrtCandidate = static_cast<int>(std::sqrt(candidate));
      for (int prime : primeNumbers) {
          if (prime > sqrtCandidate) {
            break;
          }
          if (candidate % prime == 0) {
              return false;
          }
      }
      return true;
    }
};

int main(int argc, char **argv) {
    std::vector<int> primeNumbers = PrimeCalculator::getPrimes(std::stoi(argv[1]));
    if (!primeNumbers.empty()) {
        std::cout << primeNumbers.back() << std::endl;
    };
    return 0;
}
