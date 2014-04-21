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
 
#include <cassert>
#include <iostream>
#include <memory>
#include <sstream>

#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>

#include <coinsy/rsa.hpp>

using namespace coinsy;

static std::mutex * g_mutex = 0;

static int rsa_callback(int p, int n, BN_GENCB * arg)
{
    char c = 'B';

    if (p == 0)
	{
		c = '.';
	}
	
    if (p == 1)
	{
		c = '+';
	}
	
    if (p == 2)
	{
		c = '*';
	}
	
    if (p == 3)
	{
		c = '\n';
	}
    
    fputc(c, stderr);
    
    return 1;
}

static void locking_function(int mode, int n, const char * file, int line)
{
	if (mode & CRYPTO_LOCK)
    {
		g_mutex[n].lock();
    }
	else
    {
		g_mutex[n].unlock();
    }
}

static unsigned long id_function(void)
{
	std::ostringstream oss;
	oss << std::this_thread::get_id();
	std::hash<std::string> h;
	return ((unsigned long) h(oss.str()));
}

struct CRYPTO_dynlock_value
{
	std::mutex mutex;
};

static CRYPTO_dynlock_value * dyn_create_function(const char * file, int line)
{
	struct CRYPTO_dynlock_value * value = new CRYPTO_dynlock_value;
    
	if (!value)
    {
		return 0;
    }
    
	return value;
}

static void dyn_lock_function(
    int mode, CRYPTO_dynlock_value * l, const char * file, int line
    )
{
	if (mode & CRYPTO_LOCK)
    {
		l->mutex.lock();
    }
	else
    {
		l->mutex.unlock();
    }
}

static void dyn_destroy_function(
    CRYPTO_dynlock_value * l, const char * file, int line
    )
{
	delete l;
}

static int thread_setup()
{
	g_mutex = new std::mutex[CRYPTO_num_locks()];
    
	if (!g_mutex)
    {
		return 0;
    }
    
	CRYPTO_set_id_callback(id_function);
	CRYPTO_set_locking_callback(locking_function);
	CRYPTO_set_dynlock_create_callback(dyn_create_function);
	CRYPTO_set_dynlock_lock_callback(dyn_lock_function);
	CRYPTO_set_dynlock_destroy_callback(dyn_destroy_function);
	return 1;
}

static int thread_cleanup()
{
	if (!g_mutex)
    {
		return 0;
    }
    
	CRYPTO_set_id_callback(0);
	CRYPTO_set_locking_callback(0);
	CRYPTO_set_dynlock_create_callback(0);
	CRYPTO_set_dynlock_lock_callback(0);
	CRYPTO_set_dynlock_destroy_callback(0);
	delete [] g_mutex;
	g_mutex = 0;
	return 1;
}

rsa::rsa()
    : m_rsa(0)
    , m_pub(0)
    , m_pri(0)
{
    // ...
}

rsa::rsa(RSA * r)
    : m_rsa(r)
    , m_pub(0)
    , m_pri(0)
{
    // ...
}

rsa::~rsa()
{
    if (m_rsa)
    {
        RSA_free(m_rsa);
    }
    
    if (m_pub)
    {
        RSA_free(m_pub);
    }
    
    if (m_pri)
    {
        RSA_free(m_pri);
    }
}

void rsa::start()
{
	static bool g_init = false;

	if (!g_init)
	{
		g_init = true;
		SSL_library_init();
        OpenSSL_add_all_algorithms();
		SSL_load_error_strings();
		thread_setup();
	}
}

void rsa::stop()
{
    // ...
}

RSA * rsa::pub()
{
    return m_rsa ? m_rsa : m_pub;
}

void rsa::set_pub(RSA * r)
{
    m_pub = r;
}

void rsa::set_pri(RSA * r)
{
    m_pri = r;
}

RSA * rsa::pri()
{
    return m_rsa ? m_rsa : m_pri;
}

void rsa::generate_key_pair(const std::uint32_t & bits)
{
	thread_.reset(
		new std::thread(std::bind(&rsa::do_generate_key_pair, this, bits))
	);
}

void rsa::do_generate_key_pair(const std::uint32_t & bits)
{
    BN_GENCB cb;

    BIO * bio_err = 0;
    
    BN_GENCB_set(&cb, rsa_callback, bio_err);

    BIGNUM * e = BN_new();

    if (e)
    {
        if (BN_set_word(e, 65537))
        {
            m_rsa = RSA_new();

            if (m_rsa)
            {
                if (RSA_generate_key_ex(m_rsa, bits, e, &cb) == -1)
                {
                    RSA_free(m_rsa), m_rsa = 0;
                }
                else
                {
					if (m_on_generation && RSA_check_key(m_rsa))
					{
						m_on_generation();
					}
#if 0
					RSA_print_fp(stdout, m_rsa, 0);
#endif
                }
            }
        }

        BN_free(e);
    }

    if (!m_rsa)
    {
#if 0
        std::cerr <<
            __FUNCTION__ << ": error = " <<
            ERR_error_string(ERR_get_error(), 0) <<
        std::endl;
#endif
    }

    thread_->detach();
	thread_.reset();
}

void rsa::set_on_generation(const std::function<void ()> & func)
{
	m_on_generation = func;
}

bool rsa::sign(
    RSA * r, const char * message_buf, const std::size_t & message_len,
    unsigned char * signature_buf, std::size_t & signature_len
    )
{
    const EVP_MD * m = EVP_get_digestbyname("ripemd160");
    EVP_MD_CTX ctx;
    unsigned char * digest = (unsigned char *)malloc(EVP_MAX_MD_SIZE);
    unsigned int digest_len;
    
    EVP_DigestInit(&ctx, m);
    EVP_DigestUpdate(&ctx, message_buf, message_len);
    EVP_DigestFinal(&ctx, digest, &digest_len);
    
    int ret = RSA_sign(
        NID_ripemd160, digest, digest_len, signature_buf,
        (unsigned int *)&signature_len, r
    );
    
    free(digest);
    
    if (!ret)
    {
#if 0
		char reason[120];
		
		ERR_error_string(ERR_get_error(), reason);
#endif
        return false;
    }
    
    ERR_clear_error();
    
    return ret == 1;
}

bool rsa::verify(
    RSA * r, const char * message_buf, const std::size_t & message_len,
    unsigned char * signature_buf, const std::size_t & signature_len
    )
{
    const EVP_MD * m = EVP_get_digestbyname("ripemd160");
    EVP_MD_CTX ctx;
    unsigned char * digest = (unsigned char *)malloc(EVP_MAX_MD_SIZE);
    unsigned int digest_len;
    
    EVP_DigestInit(&ctx, m);
    EVP_DigestUpdate(&ctx, message_buf, message_len);
    EVP_DigestFinal(&ctx, digest, &digest_len);

    int ret = RSA_verify(
        NID_ripemd160, digest, digest_len, signature_buf, signature_len, r
    );
    
    free(digest);
    
    if (!ret)
    {
#if 0
		char reason[120];
		
		ERR_error_string(ERR_get_error(), reason);
#endif
        return false;
    }
    
    ERR_clear_error();
    
    return ret == 1;
}

int rsa::public_encrypt(
	RSA * key, char * to, const char * from,
	const std::size_t & fromlen
	)
{
	int r = RSA_public_encrypt(
		(int)fromlen, (unsigned char *)from, (unsigned char *)to, key,
		RSA_PKCS1_OAEP_PADDING
	);

 	if (r < 0)
 	{
#if 0
		char reason[120];
		
		ERR_error_string(ERR_get_error(), reason);
		
 		log_error(
			"Error performing RSA encryption, reason = " << reason << "."
		);
#endif
  	}
    
    ERR_clear_error();
  	
   	return r;
}

int rsa::public_decrypt(
	RSA * key, char * to, const char * from,
	const std::size_t & fromlen
	)
{
	if (!key->n)
	{
		throw std::runtime_error(
            "RSA public decrypt failed invalid public key."
        );
	}

	int r = RSA_public_decrypt(
		(int)fromlen, (unsigned char *)from, (unsigned char *)to, key,
		RSA_PKCS1_OAEP_PADDING
	);
 
   	if (r < 0)
   	{
#if 0
		char reason[120];
		
		ERR_error_string(ERR_get_error(), reason);
		
		log_error(
			"Error performing RSA decryption, reason = " << reason << "."
		);
#endif
   	}
   	
    ERR_clear_error();
    
   	return r;
}

int rsa::private_decrypt(
	RSA * key, char * to, const char * from,
	const std::size_t & fromlen
	)
{    
	if (!key->p)
	{
		throw std::runtime_error("Invalid private key.");
	}

	int r = RSA_private_decrypt(
		(int)fromlen, (unsigned char *)from, (unsigned char *)to, key,
		RSA_PKCS1_OAEP_PADDING
	);

   	if (r < 0)
   	{
#if 0
		char reason[120];
		
		ERR_error_string(ERR_get_error(), reason);
		
		log_error(
			"Error performing RSA decryption, reason = " << reason << "."
		);
#endif
   	}
   	
    ERR_clear_error();
    
   	return r;
}

int rsa::asn1_encode(RSA * key, char * dest, const std::size_t & dest_len)
{
	unsigned char * buf, * cp;
   
	int len = i2d_RSAPublicKey(key, 0);
   
   	if (len < 0 || len > (int)dest_len)
   	{
		return -1;
   	}
    
    cp = buf = new unsigned char[len + 1];
   
   	len = i2d_RSAPublicKey(key, &cp);
   
	if (len < 0)
	{
   		delete buf, buf = 0;
   		
     	return -1;
   	}

	std::memcpy(dest, buf, len);
	
	delete buf, buf = 0;
	
	return len;
}
 
RSA * rsa::asn1_decode(const char * buf, const std::size_t & len)
{
	RSA * ret = 0;
	unsigned char * tmp;
	const unsigned char * cp;
	
	cp = tmp = new unsigned char[len];
 	
 	std::memcpy(tmp, buf, len);
 	
   	ret = d2i_RSAPublicKey(0, &cp, len);
   	
   	delete tmp, tmp = 0;
   	
  	if (!ret)
  	{
    	return 0;
   	}
   	
	return ret;
}

std::shared_ptr<rsa> rsa::public_from_pem(char * buf)
{
    RSA * ret = 0;
    
    BIO * bio = BIO_new_mem_buf(buf, -1);
    
    if (!bio)
    {
#if 0
        ERR_print_errors_fp(stdout);
#endif
        return std::shared_ptr<rsa> ();
    }

    X509 * x = PEM_read_bio_X509(bio, 0, 0, 0);
    
    BIO_free(bio);
    
    if (!x)
    {
#if 0
        ERR_print_errors_fp(stdout);
#endif
        return std::shared_ptr<rsa> ();
    }

    EVP_PKEY * pkey = X509_get_pubkey(x);
    
    X509_free(x);
    
    if (!pkey)
    {
#if 0
        ERR_print_errors_fp(stdout);
#endif
        return std::shared_ptr<rsa> ();
    }
    
    ret = EVP_PKEY_get1_RSA(pkey);
    
    EVP_PKEY_free(pkey);
    
    if (!ret)
    {
#if 0
        ERR_print_errors_fp(stdout);
#endif
        return std::shared_ptr<rsa> ();
    }
    
    return std::make_shared<rsa> (ret);
}

std::shared_ptr<rsa> rsa::private_from_pem(char * buf)
{
    RSA * ret = 0;
    
    BIO * bio = BIO_new_mem_buf(buf, -1);
    
    if (!bio)
    {
        ERR_print_errors_fp(stdout);
        
        return std::shared_ptr<rsa> ();
    }

    EVP_PKEY * pkey = PEM_read_bio_PrivateKey(bio, 0, 0, 0);
    
    BIO_free(bio);
    
    if (!pkey)
    {
        ERR_print_errors_fp(stdout);
        
        return std::shared_ptr<rsa> ();
    }

    ret = EVP_PKEY_get1_RSA(pkey);
    
    EVP_PKEY_free(pkey);
    
    if (!ret)
    {
        ERR_print_errors_fp(stdout);
        
        return std::shared_ptr<rsa> ();
    }
    
    return std::make_shared<rsa> (ret);
}

void rsa::write_to_path(
	RSA * key, const bool & is_public, const std::string & path
	)
{
	assert(key);
	
	FILE * fp;
	EVP_CIPHER * enc = 0;
	
	fp = fopen(path.c_str() , "w");

	if (fp)
	{
		if (is_public)
		{
			PEM_write_RSAPublicKey(fp, key);
		}
		else
		{
			PEM_write_RSAPrivateKey(fp, key, enc, 0, 0, 0, 0);
		}
		
        fflush(fp);
		fclose(fp);
	}
	else
	{
		throw std::runtime_error("failed to open file for writing");
	}
}

RSA * rsa::read_from_path(
    const bool & is_public, const std::string & path
    )
{
	RSA * rsa = 0;
	FILE * fp;
	
	fp = fopen(path.c_str() , "r");

	if (fp)
	{
		if (is_public)
		{
			PEM_read_RSAPublicKey(fp, &rsa, 0, 0);
#if 0
            if (rsa)
            {
                RSA_print_fp(stdout, rsa, 0);
            }
#endif
		}
		else
		{
			PEM_read_RSAPrivateKey(fp, &rsa, 0, 0);
#if 0
            if (rsa)
            {
                RSA_print_fp(stdout, rsa, 0);
            }
#endif
		}
		
		fclose(fp);
	}
	else
	{
		throw std::runtime_error("failed to open file for reading");
	}
	
	return rsa;
}

int rsa::seal(
    RSA * key, unsigned char ** ek, int * ekl,
    const char * in, int inl, char * out, int * outl
    )
{
    int ret = -1;
    
    EVP_CIPHER_CTX ctx;
    
    EVP_PKEY ** keys = (EVP_PKEY **)malloc(sizeof(EVP_PKEY) * 1);

    keys[0] = EVP_PKEY_new();
    
    EVP_PKEY_set1_RSA(keys[0], key);
    
    EVP_SealInit(&ctx, EVP_rc4(), ek, ekl, 0 /* iv */, keys, 1);

    EVP_SealUpdate(
        &ctx, (unsigned char *)out, outl, (const unsigned char *)in, inl
    );
    
    ret = EVP_SealFinal(&ctx, (unsigned char *)out, outl);
    
    EVP_PKEY_free(keys[0]);
    
    return ret;
}

int rsa::run_test()
{
    return 0;
}
