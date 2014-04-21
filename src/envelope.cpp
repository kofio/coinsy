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

#include <coinsy/credentials_manager.hpp>
#include <coinsy/crypto.hpp>
#include <coinsy/envelope.hpp>
#include <coinsy/logger.hpp>
#include <coinsy/rsa.hpp>
#include <coinsy/stack_impl.hpp>

using namespace coinsy;

envelope::envelope(
    stack_impl & owner, const std::string & from, const std::string & json
    )
    : m_from(from)
    , m_json(json)
    , m_verified(false)
    , stack_impl_(owner)
{
    // ...
}

envelope::envelope(stack_impl & owner, const std::string & json)
    : m_signature_digest("sha256")
    , m_type("application/json")
    , m_json(json)
    , m_value(json)
    , m_verified(false)
    , stack_impl_(owner)
{
    // ...
}

void envelope::encode(const std::vector< std::shared_ptr<rsa> > & certificates)
{
    /**
     * Put the type into property tree.
     */
    ptree_.put("type", m_type);

    /**
     * If there are certificates the value MUST be encrypted.
     */
    bool encrypt = certificates.size() > 0;

    if (encrypt)
    {
        std::vector<char> encrypted(m_value.begin(), m_value.end());
        
        if (stack_impl_.credentials_manager_->seal(
            certificates, &encrypted[0], encrypted.size(), m_keys)
            )
        {
            m_value = std::string(&encrypted[0], encrypted.size());
        }
        else
        {
            log_error("Envelope failed to seal, leaving as plaintext.");
            
            encrypt = false;
        }
    }
    
    /**
     * Base64 encode the value.
     */
    m_value = crypto::base64_encode(m_value.data(), m_value.size());
    
    /**
     * Put the value into property tree.
     */
    ptree_.put("value", m_value);
    
    /**
     * Put the uri into property tree.
     */
    ptree_.put("signature.uri", "");
    
    /**
     * Put the digest into property tree.
     */
    ptree_.put("signature.digest", m_signature_digest);

    /**
     * Sign with NULL.
     */
    std::string signature = sign(m_value);
    
    /**
     * Calculate the signature value.
     */
    m_signature_value = crypto::base64_encode(
        signature.data(), signature.size()
    );
    
    /**
     * Put the signature into property tree.
     */
    ptree_.put("signature.value", m_signature_value);
    
    if (encrypt)
    {
        if (m_keys.size() > 0)
        {
            /**
             * Put the cipher algorithm into property tree.
             */
            ptree_.put("cipher.algorithm", "aes_256_ecb");
            
            /**
             * Put the cipher.keys.
             */
            ptree_.put_child("cipher.keys", boost::property_tree::ptree());
        
            /**
             * Get the cipher.keys.
             */
            boost::property_tree::ptree & cipher_keys = ptree_.get_child(
                "cipher.keys"
            );

            for (auto & i : m_keys)
            {
                boost::property_tree::ptree child;

                /**
                 * Put the value into the property tree.
                 */
                child.add("value", crypto::base64_encode(
                    reinterpret_cast<char *>(&i[0]), i.size())
                );
                
                /**
                 * Put the child into the property tree.
                 */
                cipher_keys.push_back(std::make_pair("", child));
            }
        }
        else
        {
            throw std::runtime_error("envelope encoding failed, no keys");
        }
    }
    
    /**
     * The std::stringstream.
     */
    std::stringstream ss;
    
    /**
     * Write property tree to json file.
     */
    write_json(ss, ptree_, false);
    
    /**
     * Assign the json.
     */
    m_json = ss.str();
}
        
bool envelope::decode(RSA * r)
{
    assert(r);
    
    /**
     * Allocate the json.
     */
    std::stringstream json;
    
    /**
     * Read the json into the stream.
     */
    json << m_json;
    
    /**
     * Load the json stream and put it's contents into the property tree. If
     * reading fails an exception is thrown.
     */
    read_json(json, ptree_);
    
    /**
     * Get the type.
     */
    m_type = ptree_.get<std::string> ("type", "");
    
    /**
     * Get the value.
     */
    m_value = ptree_.get<std::string> ("value", "");
    
    /**
     * Base64 decode the value.
     */
    m_json = crypto::base64_decode(m_value.data(), m_value.size());
    
    /**
     * Get the signature uri.
     */
    m_signature_uri = ptree_.get<std::string> ("signature.uri", "");
    
    /**
     * Get the signature digest.
     */
    m_signature_digest = ptree_.get<std::string> ("signature.digest", "");
    
    /**
     * Get the signature value.
     */
    std::string signature_value = ptree_.get<std::string> ("signature.value", "");
    
    /**
     * Get the signature value.
     */
    m_signature_value = crypto::base64_decode(
        signature_value.data(), signature_value.size()
    );
    
    /**
     * Verify the value.
     */
    m_verified = verify(r, m_value, m_signature_value);
    
    /**
     * Base64 decode the value.
     */
    m_value = crypto::base64_decode(m_value.data(), m_value.size());

    /**
     * Get the cipher algorithm.
     */
    auto cipher_algorithm  = ptree_.get<std::string> ("cipher.algorithm", "");

    /**
     * Check that the cipher algorithm is supported.
     */
    if (cipher_algorithm == "aes_256_ecb")
    {
        /**
         * Get the cipher keys.
         */
        boost::property_tree::ptree & cipher_keys =
            ptree_.get_child("cipher.keys")
        ;
        
        /**
         * Decode each of the keys.
         */
        for (auto it = cipher_keys.begin(); it != cipher_keys.end(); ++it)
        {
            auto key = crypto::base64_decode(
                it->second.get<std::string> ("value", "").data(),
                it->second.get<std::string> ("value", "").size()
            );

            if (key.size() > 0)
            {
                m_keys.push_back({key.begin(), key.end()});
            }
        }
        
        /**
         * If we have keys proceed with decryption.
         */
        if (m_keys.size() > 0)
        {
            if (
                stack_impl_.credentials_manager_->open(
                const_cast<char *> (m_value.data()), m_value.size(), m_keys)
                )
            {
                // ...
            }
            else
            {
                log_debug(
                    "Envelope decryption failed, no valid keys."
                );
            }
        }
        else
        {
            log_error("Envelope decrypting failed, got empty keys.");
        }
    }
    else if (cipher_algorithm.size() > 0)
    {
        log_error(
            "Envelope decrypting failed, got invalid "
            "cipher algorithm = " << cipher_algorithm << "."
        );
    }
    
    return m_verified;
}

const std::string & envelope::json() const
{
    return m_json;
}

const std::string & envelope::type() const
{
    return m_type;
}

const std::string & envelope::value() const
{
    return m_value;
}

const std::string & envelope::signature_uri() const
{
    return m_signature_uri;
}

const bool & envelope::verified() const
{
    return m_verified;
}

bool envelope::verify(RSA * r)
{
    if (!m_verified)
    {
        /**
         * Verify the value.
         */
        m_verified = verify(r, m_value, m_signature_value);
    }
    
    return m_verified;
}

bool envelope::verify(
    RSA * r, const std::string & value, const std::string & signature
    )
{
    return rsa::verify(
        r, value.data(), value.size(),
        (unsigned char *)signature.data(),
        signature.size()
    );
}

std::string envelope::sign(const std::string & value)
{
    return stack_impl_.credentials_manager_->sign(value);
}
