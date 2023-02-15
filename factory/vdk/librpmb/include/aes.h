#ifndef _AES_H
#define _AES_H
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct aes_context_t
{
    uint32_t erk[64];     /* encryption round keys */
    uint32_t drk[64];     /* decryption round keys */
    int nr;             /* number of rounds */
} aes_context;

int  aes_set_key( aes_context *ctx, const uint8_t *key, int nbits );
void aes_encrypt( aes_context *ctx, uint8_t input[16], uint8_t output[16] );
void aes_decrypt( aes_context *ctx, uint8_t input[16], uint8_t output[16] );

#ifdef __cplusplus
}
#endif

#endif /* aes.h */
