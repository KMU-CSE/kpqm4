#ifndef KEM_SMAUG_H
#define KEM_SMAUG_H

#include <stdint.h>
#include <stdio.h>

#define CRYPTO_SECRETKEYBYTES 352 + 1440
#define CRYPTO_PUBLICKEYBYTES 1440
#define CRYPTO_CIPHERTEXTBYTES 1376
#define CRYPTO_BYTES 32

int crypto_kem_keypair(uint8_t *pk, uint8_t *sk);
int crypto_kem_enc(uint8_t *ct, uint8_t *ss,
                                    const uint8_t *pk);
int crypto_kem_dec(uint8_t *ss, const uint8_t *ctxt,
                                    const uint8_t *sk);

#endif // KEM_SMAUG_H
