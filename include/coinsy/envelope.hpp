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

#ifndef COINSY_ENVELOPE_HPP
#define COINSY_ENVELOPE_HPP

#include <cstdint>
#include <string>
#include <vector>

#include <openssl/rsa.h>

#include <boost/property_tree/json_parser.hpp>

namespace coinsy {

    class rsa;
    class stack_impl;
    
    /**
     * Implements a json envelope.
     */
    class envelope
    {
        public:
        
            /**
             * Constructor
             */
            envelope();
        
            /**
             * Constructor
             * @param owner The stack_impl.
             * @param from The from.
             * @param json The JSON.
             */
            envelope(stack_impl &, const std::string &, const std::string &);
        
            /**
             * Constructor
             * @param owner The stack.
             * @param json The JSON.
             */
            envelope(stack_impl &, const std::string &);
        
            /**
             * Encodes
             * @param certificates The certificates.
             */
            void encode(
                const std::vector< std::shared_ptr<rsa> > & certificates
            );
        
            /**
             * Decodes
             * @param r The rsa.
             */
            bool decode(RSA * r);
        
            /**
             * The json.
             */
            const std::string & json() const;
        
            /**
             * The type.
             */
            const std::string & type() const;
        
            /**
             * The value.
             */
            const std::string & value() const;
        
            /**
             * The signature uri.
             */
            const std::string & signature_uri() const;
        
            /**
             * If true the contents have been verified.
             */
            const bool & verified() const;
        
            /**
             * Verifies the signature.
             * @param r The rsa.
             */
            bool verify(RSA * r);
        
        private:

            /**
             * Verifies the signature.
             * @param r The rsa.
             * @param value The value to verify.
             * @param signature The signature.
             */
            bool verify(RSA * r, const std::string &, const std::string &);
        
            /**
             * Signs the envelope.
             * @param value The value.
             */
            std::string sign(const std::string &);
        
            /**
             * The from.
             */
            std::string m_from;
        
            /**
             * The JSON.
             */
            std::string m_json;
        
            /**
             * The type.
             */
            std::string m_type;
        
            /**
             * The value.
             */
            std::string m_value;
        
            /**
             * The signature uri.
             */
            std::string m_signature_uri;
        
            /**
             * The signature digest.
             */
            std::string m_signature_digest;
        
            /**
             * The signature value.
             */
            std::string m_signature_value;
        
            /**
             * If true the contents have been verified.
             */
            bool m_verified;
        
            /**
             * The keys.
             */
            std::vector< std::vector<std::uint8_t > > m_keys;
        
        protected:
        
            /** 
             * The stack_impl.
             */
            stack_impl & stack_impl_;
        
            /**
             * The boost::property_tree::ptree.
             */
            boost::property_tree::ptree ptree_;
    };
    
} // namespace coinsy

#endif // COINSY_ENVELOPE_HPP
