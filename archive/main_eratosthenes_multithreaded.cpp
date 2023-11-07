#include <iostream>
#include <vector>
#include <thread>

class PrimeCalculator {
public:
    static std::vector<int> getPrimes(int maxPrime) {
        std::vector<int> primeNumbers;

        if (maxPrime<2) {
            return primeNumbers;
        }

        // Run simple sieving for numbers up to sqrt(maxPrime)
        primeNumbers.reserve(maxPrime / 2);
        int sqrtMaxPrime = static_cast<int>(std::sqrt(maxPrime));
        std::vector<int> initialPrimeNumbers = simpleSieving(sqrtMaxPrime);

        // Process maxPrime=2 separately to not run threads for 1 segment element
        if (maxPrime==2) {
            return initialPrimeNumbers;
        }

        // Process maxPrime=3 separately to not run threads for 1 segment element
        if (maxPrime==3) {
            initialPrimeNumbers.push_back(3);
            return initialPrimeNumbers;
        }

        // Run segment sieving for numbers from sqrt(maxPrime) to maxPrime

        int num_threads = calculateThreadsNumber(maxPrime);
        std::vector<std::thread> threads;
        std::vector<std::vector<int> > primeNumbersSegments(num_threads);

        int segmentSieveStart = sqrtMaxPrime + 1;
        int segmentSize = (maxPrime - segmentSieveStart + 1) / num_threads;

        for (int i = 0; i < num_threads; ++i) {
            int segmentStart = segmentSieveStart + i * segmentSize;
            int segmentEnd = (i < num_threads - 1) ? (segmentSieveStart + (i + 1) * segmentSize - 1) : maxPrime;
            threads.emplace_back(segmentSieving,
                                 segmentStart, segmentEnd,
                                 std::cref(initialPrimeNumbers),
                                 std::ref(primeNumbersSegments[i]));
        }

        // Wait for threads to finish
        for (auto& thread : threads) {
            thread.join();
        }

        // Insert prime numbers up to sqrt(N)
        primeNumbers.insert(primeNumbers.end(), initialPrimeNumbers.begin(), initialPrimeNumbers.end());
        // Insert the segment-specific prime vectors into the final allPrimes vector
        for (int i = 0; i < num_threads; ++i) {
            primeNumbers.insert(primeNumbers.end(), primeNumbersSegments[i].begin(), primeNumbersSegments[i].end());
        }

        return primeNumbers;
    }
private:
    static std::vector<int> simpleSieving(int maxPrime) {
        // Sieving algorithm for numbers up to sqrt(N)
        std::vector<int> primeNumbers;
        std::vector<bool> isPrime(maxPrime + 1L, true);

        isPrime[0] = false;
        isPrime[1] = false;

        int sqrtMaxPrime = static_cast<int>(std::sqrt(maxPrime));

        // Starting from 3 and incrementing by 2 to check odd numbers only
        for (int candidate = 3; candidate <= sqrtMaxPrime; candidate += 2) {
            if (isPrime[candidate]) {
                for (long primeMultiple = candidate * candidate; primeMultiple <= maxPrime; primeMultiple += candidate) {
                    isPrime[primeMultiple] = false;
                }
            }
        }

        // Gather prime numbers
        primeNumbers.reserve(sqrtMaxPrime / 2);
        primeNumbers.push_back(2);
        for (long i = 3; i <= maxPrime; i += 2) {
            if (isPrime[i]) {
                primeNumbers.push_back(i);
            }
        }
        return primeNumbers;
    }
private:
    static void segmentSieving(int startSegment, int endSegment, const std::vector<int>& initialPrimeNumbers, std::vector<int>& primeNumbersSegment) {
        // Block sieving algorithm for numbers after sqrt(N)
        std::vector<bool> isPrime(endSegment - startSegment + 1, true);
        primeNumbersSegment.reserve((endSegment - startSegment + 1) / 2);
        // Sieve within the segment
        for (int p : initialPrimeNumbers) {
            int startNumber = std::max(p * p, (startSegment + p - 1) / p * p);
            for (long i = startNumber; i <= endSegment; i += p) {
                isPrime[i - startSegment] = false;
            }
        }

        // Gather primes in this segment
        for (long i = startSegment; i <= endSegment; ++i) {
            if (isPrime[i - startSegment]) {
                primeNumbersSegment.push_back(i);
            }
        }
    }
private:
    static int calculateThreadsNumber(int maxPrime) {
        const int maxThreads = static_cast<int>(std::thread::hardware_concurrency());
        // For small maxPrime Numbers
        int numThreads = maxPrime <= maxThreads ? 1 : std::min(static_cast<int>(maxPrime/maxThreads), maxThreads);
        //std::cout << "Threads number is " << numThreads << std::endl;
        return numThreads;
    }
};

int main(int argc, char **argv) {
    std::vector<int> primeNumbers = PrimeCalculator::getPrimes(std::stoi(argv[1]));
    if (!primeNumbers.empty()) {
        std::cout << primeNumbers.back() << std::endl;
    };
    return 0;
}