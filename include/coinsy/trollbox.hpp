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

#ifndef COINSY_TROLLBOX_HPP
#define COINSY_TROLLBOX_HPP

#include <cstdint>
#include <mutex>
#include <string>
#include <vector>

#include <database/query.hpp>

namespace coinsy {

    /**
     * Implements a trollbox.
     */
    class trollbox : public std::enable_shared_from_this<trollbox>
    {
        public:
        
            /**
             * Constructor
             * @param ios The boost::asio::io_service.
             * @param owner The stack_impl.
             * @param key The key.
             */
            explicit trollbox(
                boost::asio::io_service &, stack_impl &, const std::string &
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
             * Performs a troll operation.
             * @param usename The username.
             * @param message The message.
             */
            void troll(
                const std::string & username, const std::string & message
            );
        
            /**
             * Called when a matching troll is found.
             * @param q The query.
             */
            void on_find_troll(database::query & q);
        
        private:
        
            /**
             * Performs a troll lookup operation.
             * @param interval The interval.
             */
            void find_trolls(const std::uint32_t & interval);
        
            /**
             * The market.
             */
            std::string m_market;
        
        protected:
        
            /**
             * Performs a maintenance operation.
             * @param interval The interval.
             */
            void do_maintenance(const std::uint32_t & interval);
        
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
             * The lookup timer.
             */
            boost::asio::basic_waitable_timer<
                std::chrono::steady_clock
            > timer_lookup_;
        
            /**
             * Implements a troll.
             */
            typedef struct troll_s
            {
                std::string username;
                std::string message;
                std::time_t expires;
        
                /**
                 * operator==
                 */
                bool operator==(const troll_s & rhs) const
                {
                    return username == rhs.username && message == rhs.message;
                }
        
            } troll_t;
        
            /**
             * The trolls mutex.
             */
            std::mutex mutex_trolls_;
        
            /**
             * The trolls.
             */
            std::vector<troll_t> trolls_;
        
            /**
             * The maintenance timer.
             */
            boost::asio::basic_waitable_timer<
                std::chrono::steady_clock
            > timer_maintenance_;
    };
}

#endif // COINSY_TROLLBOX_HPP
