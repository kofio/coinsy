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

#ifndef COINSY_RSA_HPP
#define COINSY_RSA_HPP

#include <cstdint>
#include <mutex>
#include <string>
#include <thread>

#include <openssl/rsa.h>

namespace coinsy {

    class rsa
    {
        public:
        
            /**
             * Constrcutor
             */
            rsa();
        
            /**
             * Constrcutor
             * @param r The RSA.
             */
            rsa(RSA *);
        
            /**
             * Destructor
             */
            ~rsa();
        
            /**
             * Starts
             */
            void start();
        
            /**
             * Stops
             */
            void stop();
        
            void set_pub(RSA * r);
        
            RSA * pub();
        
            void set_pri(RSA * r);
        
            RSA * pri();
        
            bool operator==(rsa & rhs)const
            {
                return BN_cmp(m_pub->n, rhs.pub()->n) == 0;
            }
        
            /**
             * Generates an RSA key/pair.
             * @param bits
             */
            void generate_key_pair(const std::uint32_t &);
			
			/**
			 * Set the generation handler.
			 * @param f The function.
			 */
			void set_on_generation(const std::function<void ()> &);
			
            /**
             * Computes a signature over the given message.
             * @param r
             * @param message_buf
             * @param message_len
             * @param signature_buf
             * @param signature_len
             */
            static bool sign(
                RSA *, const char *, const std::size_t &, unsigned char *,
                std::size_t &
            );
            
            /**
             * Performs signature verification over the given message.
             * @param r
             * @param message_buf
             * @param message_len
             * @param signature_buf
             * @param signature_len
             */
            static bool verify(
                RSA *, const char *, const std::size_t &, unsigned char *,
                const std::size_t &
            );
        
			/**
			 * Performs rsa public key encryption.
			 * @param key The key.
			 * @param to The to.
			 * @param from The from
			 * @param fromlen The fromlen.
			 */
            static int public_encrypt(
				RSA *, char *, const char *,
				const std::size_t &
			);


			/**
			 * Performs rsa public key decryption.
			 * @param key The key.
			 * @param to The to.
			 * @param from The from
			 * @param fromlen The fromlen.
			 */
            static int public_decrypt(
				RSA *, char *, const char * from,
				const std::size_t & fromlen
			);
        
			/**
			 * Performs rsa private key decryption.
			 * @param key The key.
			 * @param to The to.
			 * @param from The from
			 * @param fromlen The fromlen.
			 */
            static int private_decrypt(
				RSA *, char *, const char * from,
				const std::size_t & fromlen
			);
        
            /**
             *
             *
             */
            static int asn1_encode(
                RSA * key, char * dest, const std::size_t & dest_len
            );
        
            /**
             *
             * @note Caller is responsible for freeing return value.
             */
            static RSA * asn1_decode(const char * buf, const std::size_t & len);

            static std::shared_ptr<rsa> public_from_pem(char * buf);

            static std::shared_ptr<rsa> private_from_pem(char * buf);

            static void write_to_path(
                RSA * key, const bool & is_public, const std::string & path
            );
        
            static RSA * read_from_path(
                const bool & is_public, const std::string & path
            );
        
            static int seal(
                RSA * key, unsigned char ** ek, int * ekl,
                const char * in, int inl, char * out, int * outl
            );
    
            /**
             * Runs the test case.
             */
            static int run_test();
			
        private:
        
            /**
             * Generates an RSA key/pair.
             * @param bits The number of bits.
             */
            void do_generate_key_pair(const std::uint32_t &);
			
			/**
			 * The generation handler.
			 */
			std::function<void ()> m_on_generation;
        
            /**
             * The rsa.
             */
            RSA * m_rsa;
        
            /**
             * The rsa public portion.
             */
            RSA * m_pub;
        
            /**
             * The rsa private portion.
             */
            RSA * m_pri;
        
        protected:
			
			/**
			 * The generation thread.
			 */
			std::shared_ptr<std::thread> thread_;
    };
    
} // namespace coinsy

#endif // COINSY_RSA_HPP
