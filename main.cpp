#include <iostream>
#include <vector>
#include <thread>
#include <shared_mutex>

class PrimeCalculator {
public:
    static std::vector<int> getPrimes(int maxPrime) {
        std::vector<int> primeNumbers;
        // index represents value of a number, bool value represents whether that number is prime or not
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

        // could be also maxPrime / log(maxPrime) as per the prime number theorem (PNT)
        primeNumbers.reserve(maxPrime / 2);
        primeNumbers.push_back(2);  // Add 2 to prime numbers list
        for (long i = 3; i <= maxPrime; i += 2) {
            if (isPrime[i]) {
                primeNumbers.push_back(i);
            }
        }

        return primeNumbers;
    }

public:
    static void getPrimesSegment(int startSegment, int endSegment, const std::vector<int>& smallPrimes, std::vector<int>& primes) {
        std::vector<bool> isPrime(endSegment - startSegment + 1, true);

        // Sieve within the segment [L, R]
        for (int p : smallPrimes) {
            int startNumber = std::max(p * p, (startSegment + p - 1) / p * p);
            for (long i = startNumber; i <= endSegment; i += p) {
                isPrime[i - startSegment] = false;
            }
        }

        // Add the primes in this segment to the shared list
        for (long i = std::max(2, startSegment); i <= endSegment; ++i) {
            if (isPrime[i - startSegment]) {
                // Lock the mutex to protect access to the shared vector
                //std::unique_lock<std::shared_mutex> lock(mtx);
                primes.push_back(i);
            }
        }
    }
};

int main(int argc, char **argv) {
    int maxPrime = std::stoi(argv[1]);
    //int maxPrime = 2147483647;
    std::vector<int> allPrimes;
    std::vector<std::thread> threads;
    std::shared_mutex mtx;

    // Pre-calculate primes up to sqrt(N)
    int sqrtPrime = static_cast<int>(std::sqrt(maxPrime));
    std::vector<int> smallPrimes = PrimeCalculator::getPrimes(sqrtPrime);

    int num_threads = 6;

    // Calculate segment size
    int startNumber = sqrtPrime;
    int segmentSize = (maxPrime - startNumber + 1) / num_threads;

    // Vector of segment-specific vectors
    std::vector<std::vector<int>> allPrimesSegments(num_threads);

    for (int i = 0; i < num_threads; ++i) {
        int segmentStart = startNumber + i * segmentSize;
        int segmentEnd = (i < num_threads - 1) ? (startNumber + (i + 1) * segmentSize - 1) : maxPrime;
        //std::cout << "Segment number " << i << " start " << segmentStart << " end " << segmentEnd << std::endl;

        threads.emplace_back(PrimeCalculator::getPrimesSegment,
                             segmentStart, segmentEnd,
                             std::cref(smallPrimes),
                             std::ref(allPrimesSegments[i]));
    }

    // Wait for threads to finish
    for (auto& thread : threads) {
        thread.join();
    }

    // insert prime numbers up to sqrt(N)
    allPrimes.insert(allPrimes.end(), smallPrimes.begin(), smallPrimes.end());
    // Concatenate the segment-specific vectors into the final allPrimes vector
    for (int i = 0; i < num_threads; ++i) {
        allPrimes.insert(allPrimes.end(), allPrimesSegments[i].begin(), allPrimesSegments[i].end());
    }

    std::cout << allPrimes.back() << std::endl;
    return 0;
}