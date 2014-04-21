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

#include <coinsy/aes.hpp>
#include <coinsy/certificates.hpp>
#include <coinsy/credentials_manager.hpp>
#include <coinsy/crypto.hpp>
#include <coinsy/envelope.hpp>
#include <coinsy/filesystem.hpp>
#include <coinsy/logger.hpp>
#include <coinsy/stack_impl.hpp>
#include <coinsy/uri.hpp>

using namespace coinsy;

std::shared_ptr<rsa> credentials_manager::m_ca_rsa = rsa::public_from_pem(
    certificates::ca_public_pem
);

credentials_manager::credentials_manager(
    boost::asio::io_service & ios, stack_impl & owner
    )
    : stack_impl_(owner)
    , io_service_(ios)
    , strand_(ios)
    , store_credentials_timer_(ios)
{
    // ...
}

void credentials_manager::start()
{
    assert(m_ca_rsa);
    
    RSA * pub = 0;
    RSA * pri = 0;
    
    try
    {
        pub = rsa::read_from_path(
            true, filesystem::data_path() +
            stack_impl_.username() + "/public.pem"
        );
    }
    catch (std::exception & e)
    {
        // ...
    }
    
    try
    {
        pri = rsa::read_from_path(
            false, filesystem::data_path() +
            stack_impl_.username() + "/private.pem"
        );
    }
    catch (std::exception & e)
    {
        // ...
    }
    
    if (pub && pri)
    {
        /**
         * Allocate the rsa.
         */
        rsa_ = std::make_shared<rsa>();
        
        /**
         * Start the rsa.
         */
        rsa_->start();
        
        /**
         * Set the public portion.
         */
        rsa_->set_pub(pub);
        
        /**
         * Set the private portion.
         */
        rsa_->set_pri(pri);
        
        if (m_on_started)
        {
            m_on_started();
        }
    }
    else
    {    
        /**
         * Allocate the rsa.
         */
        rsa_ = std::make_shared<rsa>();
        
        /** 
         * Set the on generation callback.
         */
        rsa_->set_on_generation(
            std::bind(&credentials_manager::rsa_on_generation, this)
        );
        
        /**
         * Start the rsa.
         */
        rsa_->start();
        
        /**
         * Generate a 1024-bit key/pair.
         */
        rsa_->generate_key_pair(1024);
    }
}

void credentials_manager::stop()
{
    store_credentials_timer_.cancel();
    
    /**
     * Stop the rsa.
     */
    if (rsa_)
    {
        rsa_->stop();
    }
}

void credentials_manager::on_find(
    std::map<std::string, std::string> & pairs
    )
{
    auto u = pairs["u"];
    
    /**
     * Get the credentials.
     */
    auto c = uri::decode(pairs["c"]);
    
    /**
     * Decode the credentials.
     */
    c = crypto::base64_decode(c.data(), c.size());
    
    /**
     * Allocate thhe envelope.
     */
    envelope env(stack_impl_, u, c);
    
    /**
     * Decode the envelope.
     */
    if (env.decode(m_ca_rsa->pub()))
    {
        std::string json_credentials = env.json();
        
        std::stringstream ss;
        
        ss << json_credentials;
        
        try
        {
            /**
             * Allocate empty property tree object.
             */
            boost::property_tree::ptree pt;
            
            read_json(ss, pt);
            
            /**
             * Get u from the property tree.
             */
            auto u = pt.get<std::string> ("u");
            
            /**
             * Get c from the property tree
             */
            auto c = pt.get<std::string> ("c");
            
            /**
             * Get e from the property tree
             */
            auto e = pt.get<std::string> ("e");
            
            (void)e;
            
            /**
             * Base64 decode the credentials.
             */
            c = crypto::base64_decode(c.data(), c.size());
            
            /**
             * ASN1 decode the credentials.
             */
            auto rsa_pub = std::make_shared<rsa> (
                rsa::asn1_decode(c.data(), c.size())
            );
            
            assert(rsa_pub->pub());
            
            /**
             * Retain the credentials.
             */
            auto it = m_credentials.find(u);
            
            if (it == m_credentials.end())
            {
                std::vector< std::shared_ptr<rsa> > rsa_pubs;
                
                rsa_pubs.push_back(rsa_pub);
                
                m_credentials.insert(std::make_pair(u, rsa_pubs));
            }
            else
            {
                /**
                 * Check to make sure the rsa doesn't already exist.
                 */
                bool found = false;
                
                for (auto & i : it->second)
                {
                    if (i == rsa_pub)
                    {
                        found = true;
                        break;
                    }
                }
                
                if (!found)
                {
                    it->second.push_back(rsa_pub);
                }
            }
        }
        catch (std::exception & e)
        {
            log_error(
                "Credentials manager, what = " << e.what() << "."
            ); 
        }
    }
    else
    {
        log_error("Credentials manager faiiled to verify ca.");
    }
}

void credentials_manager::set_on_started(const std::function<void ()> & f)
{
    m_on_started = f;
}

std::string credentials_manager::base64_public_cert()
{
    std::string ret;
    
    /**
     * Allocate the certificate buffer (DER).
     */
    char buf[1024];
    
    /**
     * Encode the certificate (asn1/DER).
     */
    int len = rsa::asn1_encode(rsa_->pub(), buf, sizeof(buf));

    assert(len < sizeof(buf));
    
    /**
     * Base64 encode the certificate.
     */
    ret = crypto::base64_encode(buf, len);
    
    return ret;
}

std::string credentials_manager::sign(const std::string & val)
{
    std::string ret;

    /**
     * Get the signature length.
     */
    std::size_t signature_length = RSA_size(rsa_->pri());
    
    /**
     * Allocate the signature.
     */
    ret.resize(signature_length);
    
    /**
     * Calculate the signature.
     */
    rsa::sign(
        rsa_->pri(), val.data(), val.size(),
        reinterpret_cast<unsigned char *> (const_cast<char *> (
        ret.data())), signature_length
    );

    return ret;
}

bool credentials_manager::verify(
    const std::string & username, const std::string & query,
    const std::string & signature
    )
{
    bool ret = false;
    
    auto it = m_credentials.find(username);

    if (it != m_credentials.end())
    {
        for (auto & i : it->second)
        {
            if (
                rsa::verify(i->pub(), query.data(), query.size(),
                reinterpret_cast<unsigned char *> (const_cast<char *> (
                signature.data())), signature.size())
                )
            {
                ret = true;
                
                break;
            }
        }
    }
    else
    {
        log_debug(
            "Credentials manager failed to verify signature, no "
            "certificate for " << username << "."
        );
    }

    return ret;
}

bool credentials_manager::seal(
    const std::vector< std::shared_ptr<rsa> > & certificates,
    char * buf, const std::size_t & len,
    std::vector< std::vector<std::uint8_t> > & keys /* out */
    )
{
    if (certificates.empty())
    {
        return false;
    }
    
    /**
     * Generate a session key.
     */
    std::string key = crypto::random_string(32);

    /**
     * Allocate the AES context.
     */
    aes ctx;
    
    /**
     * Set the AES key.
     */
    ctx.set_key(key.data(), key.size());

    /**
     * Encrypt the key with each public certificate.
     */
    auto it = certificates.begin();
    
    for (; it != certificates.end(); ++it)
    {
		if (!(*it)->pub())
		{
			log_error("Credentials manager  got null rsa, continuing.");
            
			continue;
		}

        char encrypt_buf[2048] = { 0 };
        
        auto encrypt_len = rsa::public_encrypt(
            (*it)->pub(), encrypt_buf, key.data(), key.size()
        );
        
        assert(encrypt_len < sizeof(encrypt_buf));
        
        if (encrypt_len > 0)
        {
            std::vector<std::uint8_t> key(
                encrypt_buf, encrypt_buf + encrypt_len
            );
            
            keys.push_back(key);
        }
    }

    /**
     * Encrypt the buffer.
     */
    ctx.crypt(buf, buf, len);

    return true;
}

bool credentials_manager::open(
    char * buf, const std::size_t & len,
    const std::vector< std::vector<std::uint8_t> > & keys
    )
{
    bool ret = false;

    auto it = keys.begin();
    
    for (; it != keys.end(); ++it)
    {
        char decrypt_buf[2048] = { 0 };
        
        int decrypt_len = rsa::private_decrypt(
            rsa_->pri(), decrypt_buf,
            reinterpret_cast<const char *> (&(*it)[0]), (*it).size()
        );

        if (decrypt_len > 0)
        {
            /**
             * Allocate the aes.
             */
            aes ctx;
            
            /**
             * Set the key.
             */
            ctx.set_key(decrypt_buf, decrypt_len);
            
            /**
             * Decrypt the buffer.
             */
            ctx.crypt(buf, buf, len);
            
            ret = true;
            
            break;
        }
        else
        {
            log_debug("Credentials manager failed to decrypt key.");
        }
    }

    return ret;
}

std::shared_ptr<rsa> & credentials_manager::ca_rsa()
{
    return m_ca_rsa;
}

void credentials_manager::set_credentials_envelope(const std::string & val)
{
    m_credentials_envelope = val;
}
        
const std::string & credentials_manager::credentials_envelope() const
{
    return m_credentials_envelope;
}

std::vector< std::shared_ptr<rsa> > credentials_manager::credentials(
    const std::string & username
    )
{
    auto it = m_credentials.find(username);
    
    if (it != m_credentials.end())
    {
        return it->second;
    }
    
    return std::vector< std::shared_ptr<rsa> > ();
}

void credentials_manager::store_credentials(const std::uint32_t & delay)
{
    auto self(shared_from_this());

    store_credentials_timer_.expires_from_now(std::chrono::seconds(delay));
    store_credentials_timer_.async_wait(
        strand_.wrap(
            [this, self](boost::system::error_code ec)
            {
                if (ec)
                {
                    // ...
                }
                else
                {
                    /**
                     * Store the credentials.
                     */
                    std::string query;
                    
                    /**
                     * The username.
                     */
                    query += "u=" + stack_impl_.username();
                    
                    /**
                     * The message.
                     */
                    query += "&c=" + uri::encode(crypto::base64_encode(
                        m_credentials_envelope.data(),
                        m_credentials_envelope.size())
                    );
                    
                    /**
                     * The timestamp.
                     */
                    query += "&__t=" + std::to_string(std::time(0));
                    
                    /**
                     * Sign the query.
                     * __s = signature
                     */
                    std::string signature = sign(query);
                    
                    /**
                     * The signature.
                     */
                    query += "&__s=" + uri::encode(crypto::base64_encode(
                        signature.data(), signature.size())
                    );
                    
                    /**
                     * 72 hours
                     */
                    query += "&_l=259200";

                    /**
                     * Store the query.
                     */
                    stack_impl_.store(query);
        
                    /**
                     * Store credentials again after one hour.
                     */
                    store_credentials(3600);
                }
            }
        )
    );
}

void credentials_manager::rsa_on_generation()
{
    rsa::write_to_path(
        rsa_->pub(), true, filesystem::data_path() +
        stack_impl_.username() + "/public.pem"
    );
    rsa::write_to_path(
        rsa_->pri(), false, filesystem::data_path() +
        stack_impl_.username() + "/private.pem"
    );
    
    if (m_on_started)
    {
        m_on_started();
    }
}
