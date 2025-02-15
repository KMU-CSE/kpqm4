#ifndef HAETAE_SIGN_H
#define HAETAE_SIGN_H

#include "params.h"
#include "poly.h"
#include "polymat.h"
#include "polyvec.h"
#include <stddef.h>
#include <stdint.h>

int crypto_sign_keypair(uint8_t *pk, uint8_t *sk);

int crypto_sign_signature(uint8_t *sig, size_t *siglen, const uint8_t *m,
                          size_t mlen, const uint8_t *sk);

int crypto_sign(uint8_t *sm, size_t *smlen, const uint8_t *m, size_t mlen,
                     const uint8_t *sk);

int crypto_sign_verify(const uint8_t *sig, size_t siglen, const uint8_t *m,
                       size_t mlen, const uint8_t *pk);

int crypto_sign_open(uint8_t *m, size_t *mlen, const uint8_t *sm, size_t smlen,
                     const uint8_t *pk);

#endif // HAETAE_SIGN_H