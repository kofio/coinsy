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

#ifndef COINSY_HTTP_TRANSPORT_HPP
#define COINSY_HTTP_TRANSPORT_HPP

#include <chrono>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <sstream>

#if (defined __IPHONE_OS_VERSION_MAX_ALLOWED)
#import <CFNetwork/CFSocketStream.h>
#endif // __IPHONE_OS_VERSION_MAX_ALLOWED

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

namespace coinsy {

    class http_transport
        : public std::enable_shared_from_this<http_transport>
    {
        public:
        
            /**
             * Constructor
             * @param ios The boost::asio::io_service.
             * @param url The url.
             */
            explicit http_transport(
                boost::asio::io_service &, const std::string &
            );

            /**
             * Destructor
             */
            ~http_transport();
        
            /**
             * Starts the transport.
             * f The completion handler.
             */
            void start(
                const std::function<void (boost::system::error_code,
                std::shared_ptr<http_transport>)> &
            );
        
            /**
             * Stops the transport.
             */
            void stop();
        
            /**
             * If true the connection is secure.
             */
            const bool & secure() const;
        
            /**
             * The url.
             */
            const std::string & url() const;
        
            /**
             * The hostname.
             */
            const std::string & hostname() const;
        
            /**
             * The path.
             */
            const std::string & path() const;
        
            /**
             * The status code.
             */
            const std::int32_t & status_code() const;
        
            /**
             * The headers.
             */
            std::map<std::string, std::string> & headers();
        
            /**
             * Set the request body.
             * @param val The value.
             */
            void set_request_body(const std::string &);
        
            /**
             * The request body.
             */
            const std::string & request_body() const;
        
            /**
             * The response body.
             */
            const std::string response_body() const;

            /**
             * Runs the test case.
             */
            static int run_test();
        
        private:
        
            void do_connect(boost::asio::ip::tcp::resolver::iterator endpoint_iterator);
        
            void do_write(boost::asio::streambuf & buf);
        
            /**
             * If true the connection is secure.
             */
            bool m_secure;
        
            /**
             * The method.
             */
            std::string m_method;
        
            /**
             * The url.
             */
            std::string m_url;
        
            /**
             * The hostname.
             */
            std::string m_hostname;
        
            /**
             * The path.
             */
            std::string m_path;
        
            /**
             * The status code.
             */
            std::int32_t m_status_code;
        
            /**
             * The headers.
             */
            std::map<std::string, std::string> m_headers;
        
            /**
             * The request body.
             */
            std::string m_request_body;
        
            /**
             * The response body.
             */
            std::stringstream m_response_body;
        
            /**
             * The completion handler.
             */
            std::function<
                void (boost::system::error_code,
                std::shared_ptr<http_transport>)
            > m_on_complete;
        
        protected:
        
            /**
             * The maximum redirects.
             */
            enum { max_redirects = 10 };
            
            /**
             * Follows a redirect.
             */
            void follow_redirect(const std::string & url);
        
            /**
             * Parses the url into hostname, path and url parameters.
             */
            void parse_url();
        
            /**
             * Generates the request.
             */
            void generate_request();
        
            /**
             * Set's up the socket for VoIP operation on iOS.
             */
            void set_voip();
        
            /**
             * char2hex
             */
            std::string char2hex(char);
        
            /**
             * urlencode
             */
            std::string urlencode(const std::string &);
        
            /**
             * Handles the status line.
             */
            void handle_status_line();
        
            /**
             * Handles the headers.
             */
            void handle_headers();
        
            /**
             * Handles the body.
             */
            void handle_body();
        
            /**
             * The boost::asio::io_service.
             */
            boost::asio::io_service & io_service_;
        
            /**
             * The boost::asio::strand.
             */
            boost::asio::strand strand_;
        
            /**
             * The timeout timer.
             */
            boost::asio::basic_waitable_timer<
                std::chrono::steady_clock
            > timeout_timer_;
        
            /**
             * The ssl socket.
             */
            std::shared_ptr<
                boost::asio::ssl::stream<boost::asio::ip::tcp::socket>
            > ssl_socket_;
        
            /**
             * The request.
             */
            std::unique_ptr<boost::asio::streambuf> request_;
        
            /**
             * The response.
             */
            std::unique_ptr<boost::asio::streambuf> response_;
        
            /**
             * The number of redirects so far.
             */
            std::uint8_t redirects_;
#if (defined __IPHONE_OS_VERSION_MAX_ALLOWED)
            /**
             * Read and Write streams for background voip usage on iOS.
             */
            CFReadStreamRef readStreamRef_;
            CFWriteStreamRef writeStreamRef_;
#endif // __IPHONE_OS_VERSION_MAX_ALLOWED
    };
    
} // namespace coinsy

#endif // COINSY_HTTP_TRANSPORT_HPP
