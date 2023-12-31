#include <iostream>
#include <vector>
#include <thread>

class BitArray {
public:
    BitArray(size_t size) {
        // Calculate the number of bytes needed to store the specified number of bits
        size_t numBytes = (size + 7) / 8;
        // Set all bits to 1
        bits_.resize(numBytes, 0xFF);
    }

    // Set a specific bit to 0
    void clearBit(size_t index) {
        size_t byteIndex = index >> 3;
        size_t bitIndex = index & 0x07;

        uint8_t mask = ~(1 << bitIndex);

        bits_[byteIndex] &= mask;
    }

    // Get the value of a specific bit (0 or 1)
    int getBit(size_t index) const {
        size_t byteIndex = index >> 3;
        size_t bitIndex = index & 0x07;

        return (bits_[byteIndex] >> bitIndex) & 1;
    }

    // Get the total number of bits in the BitArray
    size_t size() const {
        return bits_.size() * 8;
    }

private:
    std::vector<uint8_t> bits_;
};

class PrimeCalculator {
public:
    static std::vector<int> getPrimes(int maxPrime) {
        std::vector<int> primeNumbers;

        if (maxPrime < 2) {
            return primeNumbers;
        }

        BitArray isPrime(maxPrime + 1L);
        primeNumbers.reserve(maxPrime / 2);

        // Run simple sieving for numbers up to sqrt(maxPrime)
        int sqrtMaxPrime = static_cast<int>(std::sqrt(maxPrime));
        if (maxPrime < 100) {
            simpleSieving(maxPrime, std::ref(isPrime), std::ref(primeNumbers));
            return primeNumbers;
        }
        else {
            simpleSieving(sqrtMaxPrime, std::ref(isPrime), std::ref(primeNumbers));
        }

        //std::cout << "sqrtMaxPrime " << sqrtMaxPrime << std::endl;

        int lastInitialPrimeIndex = static_cast<int>(primeNumbers.size()) - 1;

        // Run segment sieving for numbers from sqrt(maxPrime) to maxPrime
        int num_threads = calculateThreadsNumber(maxPrime);
        //std::cout << "num_threads " << num_threads << std::endl;
        std::vector<std::thread> threads;

        int segmentSize = (maxPrime - sqrtMaxPrime + 1) / num_threads;
        //std::cout << "segmentSize " << segmentSize << std::endl;
        int segmentStart = 0;
        int segmentEnd = sqrtMaxPrime - 1;
        for (int i = 0; i < num_threads; ++i) {
            segmentStart = segmentEnd + 1;

            // Calculate the end of the segment
            // Align the end of the segment on a byte boundary, if it's not the last segment
            segmentEnd += segmentSize;
            if (i < num_threads - 1) {
                segmentEnd = ((segmentEnd + 7) / 8) * 8 - 1; // Ensure the segment end is a multiple of 8
            }

            // Make sure we do not go beyond maxPrime in the last segment
            if (segmentEnd > maxPrime || i == num_threads - 1) {
                segmentEnd = maxPrime;
            }

            //std::cout << "segmentStart " << segmentStart << " segmentEnd " << segmentEnd << std::endl;

            threads.emplace_back(segmentSieving,
                                 segmentStart,
                                 segmentEnd,
                                 lastInitialPrimeIndex,
                                 std::ref(isPrime),
                                 std::ref(primeNumbers));
        }

        // Wait for threads to finish
        for (auto& thread : threads) {
            thread.join();
        }

        for (long i = sqrtMaxPrime + 1; i<= maxPrime; i++){
            if (isPrime.getBit(i) == 1) {
                primeNumbers.push_back(i);
            }
        }

        return primeNumbers;
    }
private:
    static void simpleSieving(int maxPrime, BitArray& isPrime, std::vector<int>& primeNumbers) {
        // Sieving algorithm for numbers up to sqrt(N)
        isPrime.clearBit(0);
        isPrime.clearBit(1);

        int sqrtMaxPrime = static_cast<int>(std::sqrt(maxPrime));

        // Starting from 3 and incrementing by 2 to check odd numbers only
        for (int candidate = 3; candidate <= sqrtMaxPrime; candidate += 2) {
            if (isPrime.getBit(candidate) == 1) {
                for (long primeMultiple = candidate * candidate; primeMultiple <= maxPrime; primeMultiple += candidate) {
                    isPrime.clearBit(primeMultiple);
                }
            }
        }

        // Gather prime numbers
        primeNumbers.push_back(2);  // Add 2 to prime numbers list
        for (long i = 3; i <= maxPrime; i += 2) {
            if (isPrime.getBit(i) == 1) {
                primeNumbers.push_back(i);
            }
        }
    }
private:
    static void segmentSieving(int startSegment, int endSegment, int lastInitialPrimeIndex, BitArray& isPrime, std::vector<int>& primeNumbers) {
        // Block sieving algorithm for numbers after sqrt(N)
        // Sieve within the segment
        for (int i = 0; i<= lastInitialPrimeIndex; i++) {
            int prime = primeNumbers[i];
            int startNumber = std::max(prime * prime, (startSegment + prime - 1) / prime * prime);
            for (long multiple = startNumber; multiple <= endSegment; multiple += prime) {
                isPrime.clearBit(multiple);
            }
        }
    }
private:
    static int calculateThreadsNumber(int maxPrime) {
        const int maxThreads = static_cast<int>(std::thread::hardware_concurrency());
        // For small maxPrime Numbers
        int numThreads = maxPrime <= maxThreads ? 1 : std::min(static_cast<int>(maxPrime/maxThreads), maxThreads);
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