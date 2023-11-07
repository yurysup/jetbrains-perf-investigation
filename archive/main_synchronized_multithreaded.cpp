#include <iostream>
#include <vector>
#include <thread>

class PrimeCalculator {
public:
    static std::vector<int> getPrimes(int maxPrime) {
        std::vector<int> primeNumbers;
        std::mutex primeNumbersLock;

        if (maxPrime < 2) {
            return primeNumbers;
        }

        // Run simple sieving for numbers up to sqrt(maxPrime)
        primeNumbers.reserve(maxPrime / 2);
        int sqrtMaxPrime = static_cast<int>(std::sqrt(maxPrime));
        simpleSieving(sqrtMaxPrime, std::ref(primeNumbers));

        // Process maxPrime=3 separately to not run threads for 1 segment element
        if (maxPrime<=3) {
            if (maxPrime == 3) {
                primeNumbers.push_back(3);
            }
            return primeNumbers;
        }

        int lastInitialPrimeIndex = static_cast<int>(primeNumbers.size()) - 1;
        //std::cout << "lastInitialPrimeIndex " << lastInitialPrimeIndex << std::endl;

        // Run segment sieving for numbers from sqrt(maxPrime) to maxPrime
        int num_threads = calculateThreadsNumber(maxPrime);
        std::vector<std::thread> threads;

        int segmentSieveStart = sqrtMaxPrime + 1;
        int segmentSize = (maxPrime - segmentSieveStart + 1) / num_threads;

        for (int i = 0; i < num_threads; ++i) {
            int segmentStart = segmentSieveStart + i * segmentSize;
            //std::cout << "segmentStart " << segmentStart << std::endl;
            int segmentEnd = (i < num_threads - 1) ? (segmentSieveStart + (i + 1) * segmentSize - 1) : maxPrime;
            //std::cout << "segmentEnd " << segmentEnd << std::endl;

            threads.emplace_back(segmentSieving,
                                 segmentStart,
                                 segmentEnd,
                                 lastInitialPrimeIndex,
                                 std::ref(primeNumbers),
                                 std::ref(primeNumbersLock));
        }

        // Wait for threads to finish
        for (auto& thread : threads) {
            thread.join();
        }

        std::sort(primeNumbers.begin(), primeNumbers.end());

        return primeNumbers;
    }
private:
    static void simpleSieving(int maxPrime, std::vector<int>& primeNumbers) {
        // Sieving algorithm for numbers up to sqrt(N)
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
        primeNumbers.push_back(2);  // Add 2 to prime numbers list
        for (long i = 3; i <= maxPrime; i += 2) {
            if (isPrime[i]) {
                primeNumbers.push_back(i);
            }
        }
    }
private:
    static void segmentSieving(int startSegment, int endSegment, int lastInitialPrimeIndex, std::vector<int>& primeNumbers, std::mutex& primeNumbersLock) {
        // Block sieving algorithm for numbers after sqrt(N)
        std::vector<bool> isPrime(endSegment - startSegment + 1, true);
        // Sieve within the segment
        for (int i = 0; i<= lastInitialPrimeIndex; i++) {
            int prime = primeNumbers[i];
            int startNumber = std::max(prime * prime, (startSegment + prime - 1) / prime * prime);
            for (long j = startNumber; j <= endSegment; j += prime) {
                isPrime[j - startSegment] = false;
            }
        }

        // Add the primes in this segment to the shared vector
        for (long i = startSegment; i <= endSegment; ++i) {
            if (isPrime[i - startSegment]) {
                std::lock_guard<std::mutex> lock(primeNumbersLock);
                primeNumbers.push_back(i);
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