/*
 * Copyright (c) 2024 FDL(Future cryptography Design Lab.) Kookmin University
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "encrypt.h"

void encrypt(OUT Word* s_hat, IN const PublicKey* pk, IN const Word* e_hat)
{
    Word* e_hat_tmp;
    get_new_data(Word, e_hat_tmp, PARAM_N_WORDS);

    Word* tmp = NULL;
    get_new_data(Word, tmp, PARAM_N_WORDS);
    
    /* Retrieve the parity-check matrix H_hat = [ I_{n-k} | H_hat_{[n-k:n]} ] */
    memcpy(s_hat, e_hat, (sizeof(Word)) * SYND_WORDS);
    memcpy(e_hat_tmp, e_hat + SYND_WORDS, sizeof(Word) * PARAM_K_WORDS);

    /* [H] X e_j */
    matXvec(tmp, (Word*)(pk->H_hat), e_hat_tmp, SYND_BITS, PARAM_K);

    /* [I|H] X e_j */
    for (int i = 0; i < SYND_WORDS; i++)
    {
        s_hat[i] ^= tmp[i];
    }

    free(e_hat_tmp);
    free(tmp);
}
