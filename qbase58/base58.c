#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Base58 Alphabet */
static const char ALPHABET[] = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";

/* Decode Table */
static int8_t DECODE_TABLE[256];
static int DECODE_TABLE_INIT = 0;

static void init_decode_table(void) {
    if (DECODE_TABLE_INIT) return;  // already done
    for (int i = 0; i < 256; i++) {
        DECODE_TABLE[i] = -1;
    }
    DECODE_TABLE[(unsigned char)'1'] = 0;  
    DECODE_TABLE[(unsigned char)'2'] = 1;  
    DECODE_TABLE[(unsigned char)'3'] = 2;  
    DECODE_TABLE[(unsigned char)'4'] = 3;  
    DECODE_TABLE[(unsigned char)'5'] = 4;  
    DECODE_TABLE[(unsigned char)'6'] = 5;  
    DECODE_TABLE[(unsigned char)'7'] = 6;  
    DECODE_TABLE[(unsigned char)'8'] = 7;  
    DECODE_TABLE[(unsigned char)'9'] = 8;
    DECODE_TABLE[(unsigned char)'A'] = 9;  
    DECODE_TABLE[(unsigned char)'B'] = 10; 
    DECODE_TABLE[(unsigned char)'C'] = 11; 
    DECODE_TABLE[(unsigned char)'D'] = 12; 
    DECODE_TABLE[(unsigned char)'E'] = 13; 
    DECODE_TABLE[(unsigned char)'F'] = 14; 
    DECODE_TABLE[(unsigned char)'G'] = 15; 
    DECODE_TABLE[(unsigned char)'H'] = 16;
    DECODE_TABLE[(unsigned char)'J'] = 17; 
    DECODE_TABLE[(unsigned char)'K'] = 18; 
    DECODE_TABLE[(unsigned char)'L'] = 19; 
    DECODE_TABLE[(unsigned char)'M'] = 20; 
    DECODE_TABLE[(unsigned char)'N'] = 21; 
    DECODE_TABLE[(unsigned char)'P'] = 22; 
    DECODE_TABLE[(unsigned char)'Q'] = 23; 
    DECODE_TABLE[(unsigned char)'R'] = 24;
    DECODE_TABLE[(unsigned char)'S'] = 25; 
    DECODE_TABLE[(unsigned char)'T'] = 26; 
    DECODE_TABLE[(unsigned char)'U'] = 27; 
    DECODE_TABLE[(unsigned char)'V'] = 28; 
    DECODE_TABLE[(unsigned char)'W'] = 29; 
    DECODE_TABLE[(unsigned char)'X'] = 30; 
    DECODE_TABLE[(unsigned char)'Y'] = 31; 
    DECODE_TABLE[(unsigned char)'Z'] = 32;
    DECODE_TABLE[(unsigned char)'a'] = 33; 
    DECODE_TABLE[(unsigned char)'b'] = 34; 
    DECODE_TABLE[(unsigned char)'c'] = 35; 
    DECODE_TABLE[(unsigned char)'d'] = 36; 
    DECODE_TABLE[(unsigned char)'e'] = 37; 
    DECODE_TABLE[(unsigned char)'f'] = 38; 
    DECODE_TABLE[(unsigned char)'g'] = 39; 
    DECODE_TABLE[(unsigned char)'h'] = 40;
    DECODE_TABLE[(unsigned char)'i'] = 41; 
    DECODE_TABLE[(unsigned char)'j'] = 42; 
    DECODE_TABLE[(unsigned char)'k'] = 43; 
    DECODE_TABLE[(unsigned char)'m'] = 44; 
    DECODE_TABLE[(unsigned char)'n'] = 45; 
    DECODE_TABLE[(unsigned char)'o'] = 46; 
    DECODE_TABLE[(unsigned char)'p'] = 47; 
    DECODE_TABLE[(unsigned char)'q'] = 48;
    DECODE_TABLE[(unsigned char)'r'] = 49; 
    DECODE_TABLE[(unsigned char)'s'] = 50; 
    DECODE_TABLE[(unsigned char)'t'] = 51; 
    DECODE_TABLE[(unsigned char)'u'] = 52; 
    DECODE_TABLE[(unsigned char)'v'] = 53; 
    DECODE_TABLE[(unsigned char)'w'] = 54; 
    DECODE_TABLE[(unsigned char)'x'] = 55; 
    DECODE_TABLE[(unsigned char)'y'] = 56;
    DECODE_TABLE[(unsigned char)'z'] = 57;

    DECODE_TABLE_INIT = 1;
}

/* 
 * Convert raw bytes -> array of 64-bit words (big-endian).
 */
static inline uint64_t* bytes_to_u64(const unsigned char* in, size_t len, size_t* out_words) {
    size_t w = (len + 7) / 8;
    *out_words = w;
    uint64_t* arr = (uint64_t*)calloc(w, sizeof(uint64_t));
    if (!arr) return NULL;

    // Copy bytes big-endian
    size_t r = len % 8; 
    size_t idx = 0;
    if (r) {
        uint64_t tmp = 0UL;
        for (size_t i = 0; i < r; i++) {
            tmp = (tmp << 8) | in[i];
        }
        arr[0] = tmp;
        idx = r;
    }
    size_t wpos = r ? 1 : 0;
    for (; idx < len; idx += 8, wpos++) {
        uint64_t block = 0UL;
        for (int b = 0; b < 8; b++) {
            block = (block << 8) | in[idx + b];
        }
        arr[wpos] = block;
    }
    return arr;
}

/* 
 * Convert 64-bit words (big-endian) back to raw bytes.
 */
static inline unsigned char* u64_to_bytes(const uint64_t* arr, size_t words, size_t total_len) {
    unsigned char* out = (unsigned char*)malloc(total_len);
    if (!out) return NULL;
    memset(out, 0, total_len);

    size_t r = total_len % 8;
    size_t idx = 0;
    if (r) {
        uint64_t val = arr[0];
        for (int i = (int)r - 1; i >= 0; i--) {
            out[i] = (unsigned char)(val & 0xFF);
            val >>= 8;
        }
        idx = r;
    }
    size_t wpos = r ? 1 : 0;
    for (; idx < total_len; idx += 8, wpos++) {
        uint64_t block = arr[wpos];
        for (int b = 7; b >= 0; b--) {
            out[idx + b] = (unsigned char)(block & 0xFF);
            block >>= 8;
        }
    }
    return out;
}

/*
 * Divide big-int by 58 in-place, returning remainder.
 */
static inline uint32_t divmod58(uint64_t* arr, size_t words) {
    __uint128_t remainder = 0;
    for (size_t i = 0; i < words; i++) {
        remainder = (remainder << 64) | arr[i];
        uint64_t q = (uint64_t)(remainder / 58);
        arr[i] = q;
        remainder -= (__uint128_t)q * 58;
    }
    return (uint32_t)remainder;
}

/*
 * Multiply big-int by 58 and add an integer.
 */
static inline void muladd58(uint64_t* arr, size_t words, uint64_t add_val) {
    __uint128_t carry = add_val;
    for (int i = (int)words - 1; i >= 0; i--) {
        __uint128_t z = (__uint128_t)arr[i] * 58 + carry;
        arr[i] = (uint64_t)z;
        carry = z >> 64;
    }
}

/* ======================================================================= */
/*                        Public API: base58_encode                        */
/* ======================================================================= */
int base58_encode(const unsigned char* in, size_t in_len, char** out_str, size_t* out_len) {
    if (!in || !in_len) {
        *out_str = (char*)malloc(1);
        if (!*out_str) return 1;
        (*out_str)[0] = '\0';
        if (out_len) *out_len = 0;
        return 0;
    }
    /* 1) Count leading zeros => '1' in base58 */
    size_t zeroCount = 0;
    while (zeroCount < in_len && !in[zeroCount]) {
        zeroCount++;
    }

    /* 2) Convert bytes => 64-bit array */
    size_t words;
    uint64_t* arr = bytes_to_u64(in + zeroCount, in_len - zeroCount, &words);
    if (!arr) return 1;

    /* 3) Repeatedly divide by 58 to extract digits */
    size_t cap = (in_len * 138UL) / 100UL + 4UL;
    char* buf = (char*)malloc(cap);
    if (!buf) {
        free(arr);
        return 1;
    }
    size_t idx = 0;

    if (words > 0) {
        // Trim leading zero words
        size_t top = 0;
        while (top < words && !arr[top]) {
            top++;
        }
        if (top == words) {
            // Entire array is zero
            buf[idx++] = '1';
        } else {
            // Shift to front
            if (top > 0) {
                size_t new_len = words - top;
                memmove(arr, arr + top, new_len * sizeof(uint64_t));
                words = new_len;
            }
            // Repeated divmod
            while (words > 0) {
                uint32_t rem = divmod58(arr, words);
                buf[idx++] = ALPHABET[rem];
                // trim leading zeros
                size_t new_top = 0;
                while (new_top < words && arr[new_top] == 0) {
                    new_top++;
                }
                if (new_top == words) {
                    break;
                }
                if (new_top > 0) {
                    size_t new_len = words - new_top;
                    memmove(arr, arr + new_top, new_len * sizeof(uint64_t));
                    words = new_len;
                }
            }
        }
    } else {
        // Input was all zeros
        buf[idx++] = '1';
    }

    /* 4) Add '1' for each leading zero byte in input */
    for (size_t i = 0; i < zeroCount; i++) {
        buf[idx++] = '1';
    }

    /* 5) Reverse the result */
    for (size_t i = 0; i < idx / 2; i++) {
        char tmp = buf[i];
        buf[i] = buf[idx - 1 - i];
        buf[idx - 1 - i] = tmp;
    }

    buf[idx] = '\0';
    free(arr);

    char* rr = (char*)realloc(buf, idx + 1);
    if (rr) buf = rr;
    *out_str = buf;
    if (out_len) {
        *out_len = idx;
    }
    return 0;
}

/* ======================================================================= */
/*                        Public API: base58_decode                        */
/* ======================================================================= */
int base58_decode(const char* in, unsigned char** out_data, size_t* out_size) {
    init_decode_table();  // Ensure DECODE_TABLE is ready

    if (!in || !*in) {
        *out_data = malloc(1);
        if (*out_data) {
            (*out_data)[0] = 0; // empty
        }
        if (out_size) *out_size = 0;
        return 0;
    }
    /* 1) Trim whitespace */
    while (*in == ' ' || *in == '\n' || *in == '\r' || *in == '\t') {
        in++;
    }
    size_t len = strlen(in);
    while (len > 0) {
        char c = in[len - 1];
        if (c == ' ' || c == '\n' || c == '\r' || c == '\t') {
            len--;
        } else {
            break;
        }
    }
    if (!len) {
        *out_data = NULL;
        if (out_size) *out_size = 0;
        return 0;
    }
    /* 2) Count leading '1' => # of zero bytes */
    size_t leadingOnes = 0;
    while (leadingOnes < len && in[leadingOnes] == '1') {
        leadingOnes++;
    }

    /* 3) Convert base58 -> big-int in 64-bit array */
    size_t words = (len * 733UL) / 1000UL / 8UL + 2UL;
    uint64_t* arr = (uint64_t*)calloc(words, sizeof(uint64_t));
    if (!arr) return 1;

    for (size_t i = leadingOnes; i < len; i++) {
        int8_t val = DECODE_TABLE[(unsigned char)in[i]];
        if (val < 0) {
            free(arr);
            return 2; // invalid char
        }
        muladd58(arr, words, (uint64_t)val);
    }

    /* 4) Convert 64-bit array -> bytes, then strip leading zero words */
    size_t top = 0;
    while (top < words && !arr[top]) {
        top++;
    }
    if (top == words) {
        // All zero
        *out_data = (unsigned char*)calloc(leadingOnes, 1);
        if (!*out_data) {
            free(arr);
            return 1;
        }
        if (out_size) *out_size = leadingOnes;
        free(arr);
        return 0;
    }
    size_t new_len = words - top;
    memmove(arr, arr + top, new_len * sizeof(uint64_t));

    // Determine total byte length
    size_t highBytes = (new_len - 1) * 8;
    uint64_t first = arr[0];
    int leadingBits = 64;
    while (leadingBits > 0 && !(first & ((uint64_t)1 << (leadingBits - 1)))) {
        leadingBits--;
    }
    size_t firstBytes = (leadingBits + 7) / 8;
    size_t totalBytes = highBytes + firstBytes;
    size_t finalBytes = leadingOnes + totalBytes;

    unsigned char* out = (unsigned char*)malloc(finalBytes);
    if (!out) {
        free(arr);
        return 1;
    }
    memset(out, 0, leadingOnes);

    // Write arr[0]
    {
        uint64_t tmp = arr[0];
        for (size_t i = 0; i < firstBytes; i++) {
            out[leadingOnes + firstBytes - 1 - i] = (unsigned char)(tmp & 0xFF);
            tmp >>= 8;
        }
    }
    // Write subsequent words
    for (size_t w = 1; w < new_len; w++) {
        uint64_t val = arr[w];
        size_t offset = leadingOnes + firstBytes + (w - 1) * 8;
        for (int b = 7; b >= 0; b--) {
            out[offset + b] = (unsigned char)(val & 0xFF);
            val >>= 8;
        }
    }
    free(arr);

    *out_data = out;
    if (out_size) {
        *out_size = finalBytes;
    }
    return 0;
}
