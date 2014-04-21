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

#ifndef COINSY_CREDENTIALS_MANAGER_HPP
#define COINSY_CREDENTIALS_MANAGER_HPP

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <vector>

#include <boost/asio.hpp>

#include <coinsy/rsa.hpp>

namespace coinsy {

    class stack_impl;
    
    /**
     * Implements a credentials manager.
     */
    class credentials_manager
        : public std::enable_shared_from_this<credentials_manager>
    {
        public:
        
            /**
             * Constructor
             * @param ios The boost::asio::io_service.
             * @param owner The stack_impl.
             */
            explicit credentials_manager(
                boost::asio::io_service &, stack_impl &
            );
        
            /**
             * Starts
             */
            void start();
        
            /**
             * Stops
             */
            void stop();
        
            /**
             * Called when credentials are found.
             * @param The pairs
             */
            void on_find(std::map<std::string, std::string> & pairs);
        
            /**
             * Sets the on started handler.
             * @param f The function.
             */
            void set_on_started(const std::function<void ()> &);
    
            /**
             * Returns a base64 encoded public certificate.
             */
            std::string base64_public_cert();
        
            /**
             * Signs a value with the rsa private portion.
             * @param val The value.
             */
            std::string sign(const std::string &);
        
            /**
             * Verifies a query with the users rsa public portion.
             * @param username The username.
             * @param query The query.
             * @param signature The signature.
             */
            bool verify(
                const std::string &, const std::string &, const std::string &
            );
        
            /**
             * Performs a seal operation.
             */
            bool seal(
                const std::vector< std::shared_ptr<rsa> > & certificates,
                char * buf, const std::size_t & len,
                std::vector< std::vector<std::uint8_t> > & keys /* out */
            );

            /**
             * Performs an open operation.
             * @param buf The buffer.
             * @param len The length.
             * @param keys The keys.
             */
            bool open(
                char * buf, const std::size_t & len,
                const std::vector< std::vector<std::uint8_t> > & keys
            );
    
            /**
             * The ca public certificate.
             */
            static std::shared_ptr<rsa> & ca_rsa();
    
            /**
             * Sets the (our) credentials envelope.
             * @param val The value.
             */
            void set_credentials_envelope(const std::string & val);
        
            /**
             * The (our) credentials envelope.
             */
            const std::string & credentials_envelope() const;
        
            /**
             * Gets credentials for a username.
             * @param username The username.
             */
            std::vector< std::shared_ptr<rsa> > credentials(
                const std::string &
            );
        
            /**
             * Stores the (our) credentials.
             * @param delay The delay.
             */
            void store_credentials(const std::uint32_t & delay);
        
        private:
        
            /**
             * Called when rsa generation completes.
             */
            void rsa_on_generation();
        
            /**
             * The on started handler.
             */
            std::function<void ()> m_on_started;

            /**
             * The rsa.
             */
            std::shared_ptr<rsa> rsa_;
            
            /**
             * The ca public certificate.
             */
            static std::shared_ptr<rsa> m_ca_rsa;
    
            /**
             * The (our) credentials envelope.
             */
            std::string m_credentials_envelope;
        
            /**
             * The (theirs) credentials.
             */
            std::map<
                std::string, std::vector< std::shared_ptr<rsa> >
            > m_credentials;
        
        protected:
        
            /**
             * The boost::asio::io_service.
             */
            boost::asio::io_service & io_service_;
        
            /**
             * The boost::asio::strand.
             */
            boost::asio::strand strand_;
        
            /** 
             * The stack_impl.
             */
            stack_impl & stack_impl_;
        
            /**
             * The store credentials timer.
             */
            boost::asio::basic_waitable_timer<
                std::chrono::steady_clock
            > store_credentials_timer_;
    };
    
} // coinsy

#endif // COINSY_CREDENTIALS_MANAGER_HPP
