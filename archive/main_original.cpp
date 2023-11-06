#include <iostream>
#include <vector>
#include <string>
#include <future>

class BigIntegerIterator {
 public:
  explicit BigIntegerIterator(int i) {
    contain.reserve(500);
    reference.reserve(500);

    contain.push_back("" + std::to_string(i) + "");
    reference.push_back(i);
  }

  int getContain() {
    return std::max(std::stoi(contain.at(0)), reference.at(0));
  }
// why using vectors and reserve for single value?
 private:
  std::vector<std::string> contain;
  std::vector<int> reference;
};

class PrimeCalculator {
 public:
  static std::vector<int> getPrimes(int maxPrime) {
    std::vector<int> primeNumbers;
    std::vector<BigIntegerIterator> myFiller;
    // why using two vars for iterating / filling vector?
    // maxPrime + 1 is added and it's a bug
    // even number can be skipped - not primes by default
    int i = 2;
    // BigIntegerIterator is not needed, we can push_back() right into primeNumbers
    for (int j = 0; j < maxPrime; ++j) {
      myFiller.push_back(BigIntegerIterator(i++));
    }
    // primeNumbers memory should be preallocated using .reserve() as we know N preemptively
    for (auto integer: myFiller) {
      primeNumbers.push_back(integer.getContain());
    }

    std::vector<int> primeNumbersToRemove;
    std::mutex primeNumbersToRemoveMutex;
      
    // inefficient work division for threads - spawning too many threads to check divisilibity of a single number
    // lock on .push_back() is inevitable but can cause performance problems
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
      
    // erasing by finding an element to erase is inefficient
    for (int toRemove: primeNumbersToRemove) {
      primeNumbers.erase(std::find(primeNumbers.begin(), primeNumbers.end(), toRemove));
    }

    return primeNumbers;
  }
 private:
  static void isPrime(const std::vector<int> &primeNumbers, int candidate) {
      // throwing an exception is completely unnecessary, boolean should be used instead
      // only need to check for divisibility up to the square root of the candidate number
      // only need to check for divisibility using known primes
    for (int j = 0; j < candidate - 2; ++j) {
      if (candidate % primeNumbers[j] == 0) {
        throw std::exception();
      }
    }
  }
};

int main(int argc, char **argv) {
    // excessive stdout can cause performance problems
  for (int prime: PrimeCalculator::getPrimes(std::stoi(argv[1]))) {
    std::cout << prime << std::endl;
  }

  return 0;
}
