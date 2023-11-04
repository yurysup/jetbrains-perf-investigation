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

    for (int candidate: primeNumbers) {
        if (!isPrime(primeNumbers, candidate)) {
            primeNumbersToRemove.push_back(candidate);
        }
    }

    for (int toRemove: primeNumbersToRemove) {
      primeNumbers.erase(std::find(primeNumbers.begin(), primeNumbers.end(), toRemove));
    }

    return primeNumbers;
  }
 private:
    static bool isPrime(const std::vector<int> &primeNumbers, int candidate) {
      //only need to check for divisibility up to the square root of the candidate number
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
