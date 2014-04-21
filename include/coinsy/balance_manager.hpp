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

#ifndef COINSY_BALANCE_MANAGER_HPP
#define COINSY_BALANCE_MANAGER_HPP

#include <functional>
#include <map>
#include <string>

#include <boost/asio.hpp>

namespace coinsy {

    class stack_impl;
    
    /**
     * The balance manager.
     */
    class balance_manager
        : public std::enable_shared_from_this<balance_manager>
    {
        public:
        
            /**
             * Constructor
             * @param ios The boost::asio::io_service.
             * @param owner The stack_impl.
             */
            explicit balance_manager(
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
             * Checks the balance for the specified symbol.
             * @param symbol The symbol.
             */
            float check(const std::string & symbol);
        
            /**
             * Sets the on balance handler.
             * @param f The std::function.
             */
            void set_on_balance(
                const std::function<void (const std::string &, const float &)> &
            );
        
        private:
        
            /**
             * The on balance handler.
             */
            std::function<
                void (const std::string &, const float &)
            > m_on_balance;
        
            /**
             * The balances.
             */
            std::map<std::string, float> m_balances;
        
        protected:
        
            /**
             * The maintanance interval in seconds.
             */
            enum { interval_maintanance = 60 };
        
            /**
             * Maintains the balances.
             * @param interval The time interval in seconds to delay.
             */
            void maintain(const std::uint32_t & interval);
        
            /**
             * Maintains the balances.
             * @param interval The time interval in seconds to delay.
             */
            void do_maintain(const std::uint32_t & interval);
        
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
             * The balance timer.
             */
            boost::asio::basic_waitable_timer<
                std::chrono::steady_clock
            > timer_balance_;
    };
}

#endif // COINSY_BALANCE_MANAGER_HPP
