#ifndef BASE58_H
#define BASE58_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int base58_encode(const unsigned char* input, size_t input_len,
                  char** output, size_t* output_len);

int base58_decode(const char* input, unsigned char** output,
                  size_t* output_len);

#ifdef __cplusplus
}
#endif

#endif
