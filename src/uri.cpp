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

#include <cstdint>
#include <memory>

#include <coinsy/uri.hpp>

using namespace coinsy;

const char hex_2_dec[256] =
{
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9,-1,-1, -1,-1,-1,-1,
    -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1
};
    
std::string uri::decode(const std::string & src)
{
    const std::uint8_t * ptr_src = (const std::uint8_t *)src.c_str();
	const auto src_len = (int)src.length();
    const std::uint8_t * src_end = ptr_src + src_len;
    const std::uint8_t * src_last_dec = src_end - 2;

    std::unique_ptr<char> ptr_start(new char[src_len]);
    
    char * ptr_end = ptr_start.get();

    while (ptr_src < src_last_dec)
	{
		if (*ptr_src == '%')
        {
            char dec1, dec2;
            
            if (-1 != (dec1 = hex_2_dec[*(ptr_src + 1)])
                && -1 != (dec2 = hex_2_dec[*(ptr_src + 2)])
                )
            {
                *ptr_end++ = (dec1 << 4) + dec2;
                
                ptr_src += 3;
                
                continue;
            }
        }

        *ptr_end++ = *ptr_src++;
	}

    while (ptr_src < src_end)
    {
        *ptr_end++ = *ptr_src++;
    }

	return std::string(ptr_start.get(), ptr_end);
}

std::string uri::encode(const std::string & src)
{
    const char alpha_numeric[256] =
    {
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
        1,1,1,1, 1,1,1,1, 1,1,0,0, 0,0,0,0,
        0,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
        1,1,1,1, 1,1,1,1, 1,1,1,0, 0,0,0,0,
        0,1,1,1, 1,1,1,1, 1,1,1,1, 1,1,1,1,
        1,1,1,1, 1,1,1,1, 1,1,1,0, 0,0,0,0,
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,
        0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0
    };

    const char dec_2_hex[16 + 1] = "0123456789ABCDEF";
    
    const std::uint8_t * ptr_src = (const std::uint8_t *)src.c_str();
    
    const auto src_len = src.length();
    
    std::unique_ptr<char> ptr_start(new char[src_len * 3]);
    
    char * ptr_end = ptr_start.get();
    
    const std::uint8_t * const src_end = ptr_src + src_len;

    for (; ptr_src < src_end; ++ptr_src)
	{
		if (alpha_numeric[*ptr_src])
        {
            *ptr_end++ = *ptr_src;
        }
        else
        {
            *ptr_end++ = '%';
            *ptr_end++ = dec_2_hex[*ptr_src >> 4];
            *ptr_end++ = dec_2_hex[*ptr_src & 0x0F];
        }
	}

    return std::string((char *)ptr_start.get(), (char *)ptr_end);
}
