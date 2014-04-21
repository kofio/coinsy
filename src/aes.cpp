/**
 -----BEGIN PGP MESSAGE-----
Comment: GPGTools - https://gpgtools.org

hQIMA8Hhi7HOnyjIAQ//dmGnnsJBxqGuX8U1iJ/JxEj2IMKUHs48ceYMOsb4bEpO
aq7cCcGHgEgBUJnlIhBDYE58CRfQIRs4yJLfuRGLPgYj0cu2i9hP3ZWnCyM3pb8I
V54CS4FlPw8nksz04DGPI3L1y73sFdNkNY3VjrFATGuXNL5915IRcv0AiGHxrJaD
CJn2Brxhnh5hTA+/hZ9g65O2YsWhxj1WIBEwibiOWYilYGuonL6ZWLwPI/oKxG94
bx98gKfoQPMUPglGHh5KGqIBMeNgEQvjHfZ5kL9SW31Iq0uZ+3D8MSZ3w0EvsfCq
qSSp5bLSEmnGX/a56DyrDPxvqqGozbCLK90MOQVTcJgKJQnFlcQDUbFPZ4xU4zcl
GeSNML0mxfmHOloetCbVIIhavkQ5jV7Fd04emo2duATW/4d4tsw1lk1IFYcy9LGk
ricRF/RTh60y5R5RNtB4VkcnnyM3zki7I/FKNADTaSKAmlVsfhiEkJ1W/2hj0wSb
qRiPPLzEV0bK9DN8Cpvfcna3Hv1h0rkAgUD+hBN+VicAdn6UBSdLdDVIcBiF2xe0
r2hJxFza5/XrFdR99Hv7eEgpMYt0ZRI3XiWh+1DKlvFE16LikXP8I4cGu0FDmNi/
pQyNiD5ipzq2K2zQ/XWjuYszpgXtaaOeUCfM41zey0FldQurMKMLovqCncunRa7S
qwGrjluCuJ8d8UfQAFPGBmVJjEFICkGUl5T2mO5+PjVpr4U8/CUsv0G0UJmbslMD
dQYCYEJ5h27LRDGjO4KapkrepOVQuugECEnJXpVrfN/8VziLeeHQm4joQ/vIa1OJ
FBPs5EQ6200jJ66oUdwC4KKsKZqzmaXW0yllw9dQHCK4WhiRinLVXx+Jol1RpjFm
doCvtEuIpZIUraAc2YsitMBTzZnA1CS+IBtGBA==
=a9Uy
-----END PGP MESSAGE-----
 */

#include <cassert>
#include <cstring>
#include <stdexcept>

#include <openssl/rand.h>

#include <coinsy/aes.hpp>

using namespace coinsy;

void aes::set_key(const char * buf, const std::size_t & len)
{
    /**
     * The key.
     */
    std::uint8_t key[EVP_MAX_KEY_LENGTH];
    
    /**
     * The IV.
     */
    std::uint8_t iv[EVP_MAX_IV_LENGTH];

    /**
     * Start counter at zero.
     */
    std::memset(counter_, 0, sizeof(counter_));

    /**
     * Copy the key.
     */
    std::memcpy(key, buf, len);
    
    /**
     * Randomize the IV.
     */
    RAND_pseudo_bytes(iv, sizeof(iv));

    /**
     * Initialize the context.
     */
    EVP_EncryptInit(&ctx_, EVP_aes_256_ecb(), key, iv);
    
    assert(EVP_CIPHER_CTX_block_size(&ctx_) == sizeof(counter_));
}

bool aes::crypt(char * pt, char * ct, const std::size_t & len)
{
    int where = 0, num, bl = EVP_CIPHER_CTX_block_size(&ctx_);

    if (EVP_CIPHER_CTX_mode (&ctx_) != EVP_CIPH_ECB_MODE)
    {
        return false;
    }
    
    /**
     * The encrypted counters.
     */
    std::uint8_t * encr_ctrs = new std::uint8_t[len + bl];

    for (unsigned i = 0; i <= len / bl; i++)
    {
        /**
         * Crypt the current counter.
         */
        EVP_EncryptUpdate(&ctx_, &encr_ctrs[where], &num, counter_, bl);
        
        where += num;
        
        /**
         * Increment the counter.
         */
        for (auto j = 0; j < bl / sizeof (char); j++)
        {
            if (++counter_[j])
            {
                break;
            }
        }
    }
    
    /**
     * XOR the key stream with the first buffer, placing the results in the
     * second buffer.
     */
    for (auto i = 0; i < len; i++)
    {
        ct[i] = pt[i] ^ encr_ctrs[i];
    }

    delete encr_ctrs;
    
    return true;
}
