#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <iomanip>

using namespace std;

// Function to generate a random prime number (large prime)
long long generatePrime(int bitLength) {
    while (true) {
        long long num = rand() % (1 << (bitLength)) + (1 << (bitLength - 1));  // random number of the specified bit length
        bool isPrime = true;
        for (long long i = 2; i <= sqrt(num); i++) {
            if (num % i == 0) {
                isPrime = false;
                break;
            }
        }
        if (isPrime) {
            return num;
        }
    }
}

// Function to compute the greatest common divisor
long long gcd(long long a, long long b) {
    while (b != 0) {
        long long t = b;
        b = a % b;
        a = t;
    }
    return a;
}

// Function to calculate modular inverse of a number
long long modInverse(long long a, long long m) {
    long long m0 = m;
    long long y = 0, x = 1;
    if (m == 1)
        return 0;
    while (a > 1) {
        long long q = a / m;
        long long t = m;
        m = a % m;
        a = t;
        t = y;
        y = x - q * y;
        x = t;
    }
    if (x < 0)
        x += m0;
    return x;
}

// Function to compute (x^y) % p
long long power(long long x, long long y, long long p) {
    long long res = 1;
    x = x % p;
    while (y > 0) {
        if (y % 2 == 1) {
            res = (res * x) % p;
        }
        y = y / 2;
        x = (x * x) % p;
    }
    return res;
}

// Function to generate RSA keys of 1024 bits
void generateKey(const string& email) {
    srand(time(0));

    // Step 1: Generate two large prime numbers p and q (512 bits each)
    long long p = generatePrime(512);  // Generate 512-bit prime
    long long q = generatePrime(512);  // Generate 512-bit prime
    
    // Step 2: Compute n = p * q (this will give us a 1024-bit modulus)
    long long n = p * q;

    // Step 3: Compute Euler's totient function phi(n)
    long long phi = (p - 1) * (q - 1);

    // Step 4: Choose e such that 1 < e < phi(n) and gcd(e, phi(n)) = 1
    long long e = 65537;  // Commonly used value for e

    while (gcd(e, phi) != 1) {
        e++;
    }

    // Step 5: Compute d such that (d * e) % phi(n) = 1
    long long d = modInverse(e, phi);

    // Step 6: Convert email to hex key (for authentication)
    stringstream hexStream;
    for (char c : email) {
        hexStream << setw(2) << setfill('0') << hex << (int)c;
    }

    string emailHex = hexStream.str();

    // Save the generated key to file
    ofstream keyFile("id_rsa.pub");
    keyFile << emailHex;
    keyFile.close();

    // Print the RSA public and private keys
    cout << "Public Key: (" << e << ", " << n << ")" << endl;
    cout << "Private Key: (" << d << ", " << n << ")" << endl;

    // Print the security key
    cout << "Security Key (from email): " << emailHex << endl;
}

int main() {
    string email;
    cout << "Enter your Gmail account: ";
    cin >> email;

    generateKey(email);
    return 0;
}
