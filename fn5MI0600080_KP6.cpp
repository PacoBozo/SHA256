/**
*  
* Solution to course project # 6
* Introduction to programming course
* Faculty of Mathematics and Informatics of Sofia University
* Winter semester 2022/2023
*
* @author Ali Nader
* @idnumber 5MI0600080* @compiler Visual Studio
*
* <file with main function>
*
*/

#include <iostream>
#include <fstream>
using namespace std;


const int blocksize = 512; //bits
const int blockbytes = 64; //bytes

//Initialize array of constansts (first 32 bits of the fractional parts of the square roots of the first 8 primes)
unsigned int h0 = 0x6a09e667, h1 = 0xbb67ae85, h2 = 0x3c6ef372, h3 = 0xa54ff53a, h4 = 0x510e527f, h5 = 0x9b05688c, h6 = 0x1f83d9ab, h7 = 0x5be0cd19;

const int wsize = 64; // 64 32-bit words
unsigned int w[wsize]; 

//Bitwise operations
// Right Rotate
unsigned int ROTR(unsigned int x, int n) {
    return (x >> n) | (x << (32-n));
}

// Right Shift
unsigned int SHR(unsigned int x, int n) {
    return x >> n;
}

// Choose
unsigned int Ch(unsigned int x, unsigned int y, unsigned int z) {
    return (x & y) ^ (~x & z);
}

// Majority
unsigned int Maj(unsigned int x, unsigned int y, unsigned int z) {
    return (x & y) ^ (x & z) ^ (y & z);
}

// Combinations of Bitwise operations

unsigned int Sigma0(unsigned int x) {
    return ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22);
}

unsigned int Sigma1(unsigned int x) {
    return ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25);
}

unsigned int sigma0(unsigned int x) {
    return ROTR(x, 7) ^ ROTR(x, 18) ^ SHR(x, 3);
}

unsigned int sigma1(unsigned int x) {
    return ROTR(x, 17) ^ ROTR(x, 19) ^ SHR(x, 10);
}

//Initialize array of constansts (first 32 bits of the fractional parts of the cube roots of the first 64 primes)
unsigned int k[64] = {
   0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
   0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
   0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
   0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
   0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
   0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
   0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
   0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

void read_input(string line, string& input_message) {
    ifstream filein("input.txt");
    if (filein.is_open()){
        while(getline(filein, line)) {
            input_message = line;
        }
        filein.close();
    }
    else {
        cout << "Unable to open filein" << endl;
    }
}

// Prepare message to be processed by turning it into multiple of 512
void Padding(string& input_message, int total_len, int msg_len) {

    input_message.resize(total_len);
    int pad_start = msg_len;

    //Add a single 1 bit and then 0s until 512-64 bits
    input_message[pad_start++] = 0x80;
    while (pad_start < total_len) {
        input_message[pad_start++] = 0x00;
    }

    //Add 64 bit big-endian integer
    uint64_t bits = msg_len * 8;

    input_message[total_len - 8] = bits >> 56;
    input_message[total_len - 7] = bits >> 48;
    input_message[total_len - 6] = bits >> 40;
    input_message[total_len - 5] = bits >> 32;
    input_message[total_len - 4] = bits >> 24;
    input_message[total_len - 3] = bits >> 16;
    input_message[total_len - 2] = bits >> 8;
    input_message[total_len - 1] = bits;

}


void process_block(string block, int count) {
    // Initialize first 16 32-bit words of the message schedule array by dividing the block
    for (int i = 0; i < 16; i++) {
        w[i] = 0;
        for (int j = 0; j < 4; j++) {
            w[i] |= (block[i * 4 + j + count*blockbytes] & 0xff) << (24 - j * 8);
        }
        
    }

    // Compute the remaining 48 words
    for (int i = 16; i < wsize; i++) {
        w[i] = sigma1(w[i-2]) + w[i-7] + sigma0(w[i-15]) + w[i-16];
    }

    // Initialize working variables to current hash value
    unsigned int a = h0, b = h1, c = h2, d = h3, e = h4, f = h5, g = h6, h = h7;

    // Perform the main hash computation (Compression)
    for (int i = 0; i < wsize; i++) {
        unsigned int t1 = h + Sigma1(e) + Ch(e, f, g) + k[i] + w[i];
        unsigned int t2 = Sigma0(a) + Maj(a, b, c);
        h = g;
        g = f;
        f = e;
        e = d + t1;
        d = c;
        c = b;
        b = a;
        a = t1 + t2;
    }

    // Update the hash values by adding compressed chunk to current hash value
    h0 += a;
    h1 += b;
    h2 += c;
    h3 += d;
    h4 += e;
    h5 += f;
    h6 += g;
    h7 += h;
}


void print(unsigned int h) {
    ofstream fileout("output.txt", ios::app);
    if (fileout.is_open()) {
        //Check if hex value starts with 0
        int i=0;
        while ((h >> 28 -i) == 0) {
            fileout << "0";
            i += 4;
        }

        fileout << hex << h;

        fileout.close();
    } 
    else {
        cout << "Unable to open fileout" << endl;
    }
}

void read_output(string hash){
    ifstream result("output.txt");
    if (result.is_open()) {
        while(getline(result, hash)) {
        
         cout << hash <<endl;
        }
        while (!result.fail()); {
            result.close();
        }
    }
    else {
        cout << "Unable to open result" << endl;
    }
}

int main() {
    //Read message from input.txt file
    string line;
    string input_message;
    read_input(line, input_message);
    
    //cout << input_message << endl;

    // Validate the input message
    if (input_message.empty()) {
        cout << "Error: Input message is empty." << endl;
        }


    // Prepare the message for processing (Padding)
    int msg_len = input_message.length();
    int pad_len = blockbytes - (msg_len % blockbytes);
    if (pad_len < 9) pad_len += blockbytes;
    int total_len = msg_len + pad_len;

    Padding(input_message, total_len, msg_len);
    
    // Process the message in 512 bit blocks
    int block = 0;
    int block_count = total_len / blockbytes;
    for (int i = 0; i < block_count; i++) {
        process_block(input_message, block);
        block++;
    }

    // Output the final hash in output.txt file
    ofstream fileout("output.txt", ios::out);
    print(h0) ; print(h1) ; print(h2) ; print(h3) ; print(h4) ; print(h5) ; print(h6) ; print(h7) ; 
    
    // Read hash from file and output to the console
    string hash;
    read_output(hash);

    return 0;
    }
