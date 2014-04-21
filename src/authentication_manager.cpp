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

#include <boost/asio.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <coinsy/authentication_manager.hpp>
#include <coinsy/constants.hpp>
#include <coinsy/credentials_manager.hpp>
#include <coinsy/crypto.hpp>
#include <coinsy/http_transport.hpp>
#include <coinsy/envelope.hpp>
#include <coinsy/logger.hpp>
#include <coinsy/rsa.hpp>
#include <coinsy/stack_impl.hpp>

using namespace coinsy;

authentication_manager::authentication_manager(
    boost::asio::io_service & ios, stack_impl & owner
    )
    : m_proxy_tid(0)
    , io_service_(ios)
    , strand_(ios)
    , stack_impl_(owner)
{
    // ...
}

void authentication_manager::sign_in(
    const std::string & username, const std::string & password,
    const std::string & base64_cert,
    const std::function<void (std::map<std::string, std::string>)> & f
    )
{
    m_on_complete = f;
    
    /**
     * Compute the secret.
     */
    std::string secret = crypto::hmac_sha512(username, password);
    
    /**
     * Formulate the url from one of the hostnames.
     */
    std::string url =
        "https://www." + std::string(constants::auth_hostnames[0]) +
        "/a?u=" + username + "&s=" + secret
    ;
    std::shared_ptr<http_transport> t =
        std::make_shared<http_transport>(io_service_, url)
    ;
    
    t->set_request_body(base64_cert);
    
    t->headers()["Content-Type"] = "text/plain";
    
    t->start(
        [this, username, secret, base64_cert](
        boost::system::error_code ec, std::shared_ptr<http_transport> t)
        {
            if (ec)
            {
                log_info(
                    "Authentication manager failed to connect, trying proxy."
                );
                
                /**
                 * Sign in through a random proxy.
                 */
                proxy_sign_in(username, secret, base64_cert);
            }
            else
            {
                if (t->status_code() == 200)
                {
                    /**
                     * Handle the json.
                     */
                    handle_json(t->response_body());
                }
                else
                {
                    log_error(
                        "Authentication manager request failed, status "
                        "code = " << t->status_code() << "."
                    );
                    
                    /**
                     * Sign in through a random proxy.
                     */
                    proxy_sign_in(username, secret, base64_cert);
                }
            }
        }
    );
}

void authentication_manager::on_proxy(
    const std::uint16_t & tid, const char * addr,
    const std::uint16_t & port, const std::string & value
    )
{
    if (m_proxy_tid == tid)
    {
        m_proxy_tid = 0;
        
        /**
         * Handle the json.
         */
        handle_json(value);
    }
}

void authentication_manager::proxy_sign_in(
    const std::string & username, const std::string & secret,
    const std::string  & base64_cert
    )
{
    io_service_.post(strand_.wrap(
        [this, username, secret, base64_cert]()
    {
        std::stringstream body;
        
        try
        {
            /**
             * Allocate empty property tree object.
             */
            boost::property_tree::ptree pt;
            
            /**
             * Put username into property tree.
             */
            pt.put("u", username);
            
            /**
             * Put secret into property tree.
             */
            pt.put("s", secret);
            
            /**
             * Put base64_cert into property tree.
             */
            pt.put("c", base64_cert);
            
            /**
             * Write property tree to json file.
             */
            write_json(body, pt);
        }
        catch (std::exception & e)
        {
            log_error(
                "Authentication manager, what = " << e.what() << "."
            ); 
        }

        std::string encrypted(body.str().size(), 0);
        
        unsigned char * ek[1];
        
        ek[0] = (unsigned char *)malloc(
            RSA_size(credentials_manager::ca_rsa()->pub())
        );
        
        int ekl;
        int outl;
        
        /**
         * Encrypt the credentials with an authentication node's
         * public certificate.
         */
        if (
            rsa::seal(credentials_manager::ca_rsa()->pub(), ek, &ekl,
            body.str().data(), body.str().size(),
            (char *)encrypted.data(), &outl)
            )
        {
            std::string key(reinterpret_cast<char *>(ek[0]), ekl);
            
            std::stringstream sealed;
            
            try
            {
                /**
                 * Allocate empty property tree object.
                 */
                boost::property_tree::ptree pt;
                
                /**
                 * Put key into property tree.
                 */
                pt.put("k", crypto::base64_encode(
                    key.data(), key.size())
                );
                
                /**
                 * Put encrypted into property tree.
                 */
                pt.put("e", crypto::base64_encode(
                    encrypted.data(), encrypted.size())
                );
                
                /**
                 * Write property tree to json file.
                 */
                write_json(sealed, pt);
            }
            catch (std::exception & e)
            {
                log_error(e.what());
            }
            
            /**
             * Create the authentication request.
             */
            std::stringstream ss;
            ss << "POST" << " "  << "/a2/" << " HTTP/1.0\r\n";
            ss << "Host: " << constants::auth_hostnames[0] << "\r\n";
            ss << "Accept: */*\r\n";
            ss << "Connection: close\r\n";
            ss << "Content-Length: " << sealed.str().size() << "\r\n";
            ss << "\r\n";
            ss << sealed.str();

            /**
             * Proxy the http request.
             */
            m_proxy_tid = stack_impl_.proxy(
                constants::auth_addresses[0], 80,
                ss.str().data(), ss.str().size()
            );
        }
        else
        {
            log_error("Authentication manager seal failed.");
        }
        
        free(ek[0]);
    }));
}

void authentication_manager::handle_json(const std::string & json)
{
    std::stringstream ss;
    
    /**
     * Example:
     * {"message":"Success","status":0,"envelope":"eyJ0..."}
     */
    ss << json;

    /**
     * Allocate empty property tree object.
     */
    boost::property_tree::ptree pt;
    
    std::map<std::string, std::string> result;
    
    try
    {
        /**
         * Read the json.
         */
        read_json(ss, pt);

        result["status"] = pt.get<std::string> (
            "status", ""
        );

        result["message"] = pt.get<std::string> (
            "message", ""
        );
        
        result["envelope"] = pt.get<std::string> (
            "envelope", ""
        );
        
        if (result["status"] == "0")
        {
            if (result["envelope"].size() > 0)
            {
                /**
                 * Base64 decode the envelope.
                 * Example:
                 *  {
                 *      "type":"application\/json",
                 *      "value":"eyJ1IjoiZ3Jh...I6IjAifQ==",
                 *      "signature":
                 *      {
                 *          "uri":"",
                 *          "digest":"sha512",
                 *          "value":"3SOGROqjk...YqaXefY="
                 *      }
                 *  }
                 */
                std::string envelope = crypto::base64_decode(
                    result["envelope"].data(), result["envelope"].size()
                );
    
                /**
                 * Get the envelope value.
                 */
                std::string value = handle_envelope(envelope);
                
                /**
                 * Set the credentials.
                 */
                std::string credentials = value;
                
                if (credentials.size() > 0)
                {
                    std::cout <<
                        "Authentication manager sign in success, message = " <<
                        result["message"] << "." <<
                    std::endl;
                    
                    result["envelope"] = envelope;
                }
                else
                {
                    result["status"] = "-1";
                    result["message"] = "invalid envelope";
                }
            }
            else
            {
                std::cout <<
                    "Authentication manager sign in success, message = " <<
                    result["message"] << "." <<
                std::endl;
            }
        }
        else
        {
            std::cerr <<
                "Authentication manager sign in failure, message = " <<
                result["message"] << ", status = " << result["status"] << "." <<
            std::endl;
        }
    }
    catch (std::exception & e)
    {
        std::cerr <<
            "Authentication manager sign in failure, what = " <<
            e.what() << "." <<
        std::endl;
        
        result["status"] = "-1";
        result["message"] = e.what();
    }
    
    if (m_on_complete)
    {
        m_on_complete(result);
    }
}

std::string authentication_manager::handle_envelope(
    const std::string & json
    )
{
    /**
     * Allocate the envelope.
     */
    envelope env(stack_impl_, std::string(), json);
    
    /**
     * Decode the envelope.
     */
    if (env.decode(credentials_manager::ca_rsa()->pub()))
    {
        return env.value();
    }

    return std::string();
}
