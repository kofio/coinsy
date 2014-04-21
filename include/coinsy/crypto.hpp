/**
 -----BEGIN PGP MESSAGE-----
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

#ifndef COINSY_CRYPTO_HPP
#define COINSY_CRYPTO_HPP

#include <cassert>
#include <cstdint>
#include <locale>
#include <random>
#include <sstream>
#include <string>

#include <openssl/evp.h>
#include <openssl/hmac.h>

#include <boost/uuid/sha1.hpp>

namespace coinsy {

    class crypto
    {
        public:

            /**
             * Generates a random string.
             * @param len The length of the string to be generated.
             */
            static std::string random_string(const std::size_t & len)
            {
                std::string ret;
                
                static const std::string chars =
                    "abcdefghijklmnopqrstuvwxyz"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789"
                ;
            
                static std::random_device rd;
                static std::mt19937_64 mt(rd());
                std::uniform_int_distribution<std::size_t> dist(
                    0, chars.size() - 1
                );

                for (auto i = 0 ; i < len; ++i )
                {
                    ret += chars[dist(mt)] ;
                }
                
                return ret;
            }
        
            /**
             * Performs base64 encoding.
             * @param buf
             * @param len
             */
            static std::string base64_encode(
                const char * buf, const std::size_t & len
                )
            {
                static const char b64_forward_table[65] =
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz"
                    "0123456789+/"
                ;
                std::string bindata(buf, len);

                if (
                    bindata.size() > (
                    std::numeric_limits<std::string::size_type>::max() / 4u) * 3u
                    )
                {
                    throw std::runtime_error(
                        "String is too large for base64 encoding."
                        );
                }

                const std::size_t binlen = bindata.size();
                
                /**
                 * Pad the end with '='.
                 */
                std::string ret((((binlen + 2) / 3) * 4), '=');
                
                std::size_t outpos = 0;
                int bits_collected = 0;
                unsigned int accumulator = 0;
                
                const std::string::const_iterator binend = bindata.end();

                for (
                    std::string::const_iterator i = bindata.begin();
                    i != binend; ++i
                    )
                {
                    accumulator = (accumulator << 8) | (*i & 0xffu);
                    bits_collected += 8;
                    
                    while (bits_collected >= 6)
                    {
                        bits_collected -= 6;
                        ret[outpos++] = b64_forward_table[
                            (accumulator >> bits_collected) & 0x3fu
                        ];
                    }
                }
               
                if (bits_collected > 0)
                {
                    assert(bits_collected < 6);
                    accumulator <<= 6 - bits_collected;
                    ret[outpos++] = b64_forward_table[accumulator & 0x3fu];
                }
               
                assert(outpos >= (ret.size() - 2));
                assert(outpos <= ret.size());
                
                return ret;
            }

            /**
             * Performs base64 decoding.
             * @param buf
             * @param len
             */
            static std::string base64_decode(
                const char * buf, const std::size_t & len
                )
            {
                static const char b64_reverse_table[128] =
                {
                   64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
                   64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
                   64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
                   52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
                   64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
                   15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
                   64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
                   41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64
                };
            
                std::string ascdata(buf, len);
                
                std::string ret;
                int bits_collected = 0;
                unsigned int accumulator = 0;

                for (
                    std::string::const_iterator i = ascdata.begin();
                    i != ascdata.end(); ++i
                    )
                {
                    const int c = *i;
                    
                    /**
                     * Skip whitespace and padding.
                     */
                    if (isspace(c) || c == '=')
                    {
                        continue;
                    }
                    
                    if ((c > 127) || (c < 0) || (b64_reverse_table[c] > 63))
                    {
                        throw std::runtime_error("Illegal characters");
                    }
                    
                    accumulator = (accumulator << 6) | b64_reverse_table[c];
                    bits_collected += 6;
                    
                    if (bits_collected >= 8)
                    {
                        bits_collected -= 8;
                        ret += (char)((accumulator >> bits_collected) & 0xffu);
                    }
                }
               
                return ret;
            }
        
            static std::string hmac_sha512(
                const std::string & value, const std::string & key
                )
            {
                std::uint8_t * digest = HMAC(
                    EVP_sha512(), key.data(), key.size(),
                    (std::uint8_t *)value.data(), value.size(), NULL, NULL
                );

                char hex[(64 * 2) + 1];
                
                for(auto i = 0; i < 64; i++)
                {
                     sprintf(&hex[i * 2], "%02x", (std::uint32_t)digest[i]);
                }
                
                return std::string(hex, 64 * 2);
            }
        
        private:
        
            // ...
        
        protected:
        
            // ...
    };
    
} // namespace coinsy

#endif // COINSY_CRYPTO_HPP
