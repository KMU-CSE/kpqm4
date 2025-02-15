// SPDX-License-Identifier: Apache-2.0 or CC0-1.0
#include "randombytes.h"
#include "hal.h"
#include "api.h"

#include <string.h>
#include <stdio.h>

#define NTESTS 10

// https://stackoverflow.com/a/1489985/1711232
#define PASTER(x, y) x##y
#define EVALUATOR(x, y) PASTER(x, y)
#define NAMESPACE(fun) EVALUATOR(MUPQ_NAMESPACE, fun)

// use different names so we can have empty namespaces
#define MUPQ_CRYPTO_BYTES           NAMESPACE(CRYPTO_BYTES)
#define MUPQ_CRYPTO_PUBLICKEYBYTES  NAMESPACE(CRYPTO_PUBLICKEYBYTES)
#define MUPQ_CRYPTO_SECRETKEYBYTES  NAMESPACE(CRYPTO_SECRETKEYBYTES)
#define MUPQ_CRYPTO_CIPHERTEXTBYTES NAMESPACE(CRYPTO_CIPHERTEXTBYTES)
#define MUPQ_CRYPTO_ALGNAME NAMESPACE(CRYPTO_ALGNAME)

#define MUPQ_crypto_kem_keypair NAMESPACE(crypto_kem_keypair)
#define MUPQ_crypto_kem_enc NAMESPACE(crypto_kem_enc)
#define MUPQ_crypto_kem_dec NAMESPACE(crypto_kem_dec)

const uint8_t canary[8] = {
  0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF
};

/* allocate a bit more for all keys and messages and
 * make sure it is not touched by the implementations.
 */
static void write_canary(uint8_t *d) {
  for (size_t i = 0; i < 8; i++) {
    d[i] = canary[i];
  }
}

static int check_canary(const uint8_t *d) {
  for (size_t i = 0; i < 8; i++) {
    if (d[i] != canary[i]) {
      return -1;
    }
  }
  return 0;
}

static void printbytes(const unsigned char *x, unsigned long long xlen)
{
  char outs[2*xlen+1];
  unsigned long long i;
  for(i=0;i<xlen;i++)
    sprintf(outs+2*i, "%02x", x[i]);
  outs[2*xlen] = 0;
  hal_send_str(outs);
}

static int test_keys(void)
{
  unsigned char key_a[MUPQ_CRYPTO_BYTES+16], key_b[MUPQ_CRYPTO_BYTES+16];
  unsigned char pk[MUPQ_CRYPTO_PUBLICKEYBYTES+16];
  unsigned char sendb[MUPQ_CRYPTO_CIPHERTEXTBYTES+16];
  unsigned char sk[MUPQ_CRYPTO_SECRETKEYBYTES+16];

  write_canary(key_a); write_canary(key_a+sizeof(key_a)-8);
  write_canary(key_b); write_canary(key_b+sizeof(key_b)-8);
  write_canary(pk); write_canary(pk+sizeof(pk)-8);
  write_canary(sendb); write_canary(sendb+sizeof(sendb)-8);
  write_canary(sk); write_canary(sk+sizeof(sk)-8);


  int i;

  for(i=0; i<NTESTS; i++)
  {
    //Alice generates a public key
#ifdef KPQM4_PALOMA
#include "key.h"
#else
    MUPQ_crypto_kem_keypair(pk+8, sk+8);
#endif
    hal_send_str("DONE key pair generation!");

    //Bob derives a secret key and creates a response
    MUPQ_crypto_kem_enc(sendb+8, key_b+8, pk+8);

    MUPQ_crypto_kem_enc(sendb+8, key_b+8, pk+8);
    hal_send_str("DONE encapsulation!");

    //Alice uses Bobs response to get her secret key
#if defined (KPQM4_PALOMA)
//todo fix this
    MUPQ_crypto_kem_dec(key_a, sendb, sk);
#elif defined (KPQM4)
    MUPQ_crypto_kem_dec(key_a+8, sk+8, pk+8, sendb+8);
#else
    MUPQ_crypto_kem_dec(key_a+8, sendb+8, sk+8);
#endif
    hal_send_str("DONE decapsulation!");

    if(memcmp(key_a+8, key_b+8, MUPQ_CRYPTO_BYTES))
    { 
      hal_send_str("ERROR KEYS\n");
    }
    else if(check_canary(key_a) || check_canary(key_a+sizeof(key_a)-8) ||
            check_canary(key_b) || check_canary(key_b+sizeof(key_b)-8) ||
            check_canary(pk) || check_canary(pk+sizeof(pk)-8) ||
            check_canary(sendb) || check_canary(sendb+sizeof(sendb)-8) ||
            check_canary(sk) || check_canary(sk+sizeof(sk)-8))
    {
      hal_send_str("ERROR canary overwritten\n");
    }
    else
    {
      hal_send_str("OK KEYS\n");
    }
    hal_send_str("+");
  }
  return 0;
}


static int test_invalid_sk_a(void)
{
  unsigned char key_a[MUPQ_CRYPTO_BYTES], key_b[MUPQ_CRYPTO_BYTES];
  unsigned char sendb[MUPQ_CRYPTO_CIPHERTEXTBYTES];
  int i;

  for(i=0; i<1; i++)
  {
    //Alice generates a public key
#ifdef KPQM4_PALOMA
#include "key.h"
#else
    unsigned char pk[MUPQ_CRYPTO_PUBLICKEYBYTES];
    unsigned char sk[MUPQ_CRYPTO_SECRETKEYBYTES];
    MUPQ_crypto_kem_keypair(pk, sk);
#endif

    //Bob derives a secret key and creates a response
    MUPQ_crypto_kem_enc(sendb, key_b, pk);

    //Replace secret key with random values
    randombytes(sk, MUPQ_CRYPTO_SECRETKEYBYTES);

    //Alice uses Bobs response to get her secre key
#if defined (KPQM4_PALOMA)
    MUPQ_crypto_kem_dec(key_a, sendb, sk);
#elif defined (KPQM4)
    MUPQ_crypto_kem_dec(key_a, sk, pk, sendb);
#else
    MUPQ_crypto_kem_dec(key_a, sendb, sk);
#endif

    if(!memcmp(key_a, key_b, MUPQ_CRYPTO_BYTES))
    {
      hal_send_str("ERROR invalid sk_a\n");
    }
    else
    {
      hal_send_str("OK invalid sk_a\n");
    }
    hal_send_str("+");
  }

  return 0;
}


static int test_invalid_ciphertext(void)
{
  unsigned char key_a[MUPQ_CRYPTO_BYTES], key_b[MUPQ_CRYPTO_BYTES];
  unsigned char sendb[MUPQ_CRYPTO_CIPHERTEXTBYTES];
  int i;
  size_t pos;

  for(i=0; i<NTESTS; i++)
  {
    randombytes((unsigned char *)&pos, sizeof(size_t));

    //Alice generates a public key
#ifdef KPQM4_PALOMA
#include "key.h"
#else
    unsigned char sk_a[MUPQ_CRYPTO_SECRETKEYBYTES];
    unsigned char pk[MUPQ_CRYPTO_PUBLICKEYBYTES];
    MUPQ_crypto_kem_keypair(pk, sk_a);
#endif

    //Bob derives a secret key and creates a response
    MUPQ_crypto_kem_enc(sendb, key_b, pk);

    // Change ciphertext to random value
    randombytes(sendb, sizeof(sendb));

    //Alice uses Bobs response to get her secret key
#if defined (KPQM4_PALOMA)
    MUPQ_crypto_kem_dec(key_a, sendb, sk);
#elif defined (KPQM4)
    MUPQ_crypto_kem_dec(key_a, sk_a, pk, sendb);
#else
    MUPQ_crypto_kem_dec(key_a, sendb, sk_a);
#endif

    if(!memcmp(key_a, key_b, MUPQ_CRYPTO_BYTES))
    {
      hal_send_str("ERROR invalid ciphertext\n");
    }
    else
    {
      hal_send_str("OK invalid ciphertext\n");
    }
    hal_send_str("+");
  }

  return 0;
}

int main(void)
{
  hal_setup(CLOCK_FAST);

  // marker for automated testing
  hal_send_str("==========================");
#ifndef KPQM4_PALOMA
  test_keys();
#endif
  test_invalid_sk_a();
  test_invalid_ciphertext();
  hal_send_str("#");

  return 0;
}
