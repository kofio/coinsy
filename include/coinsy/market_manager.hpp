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

#ifndef COINSY_MARKET_MANAGER_HPP
#define COINSY_MARKET_MANAGER_HPP

#include <cstdint>
#include <map>
#include <mutex>
#include <string>

#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>

#include <database/query.hpp>

namespace coinsy {
    
    class market;
    class stack_impl;
    
    /**
     * Implements a market manager.
     */
    class market_manager
    {
        public:
        
            /**
             * Constructor
             * @param ios The boost::asio::io_service.
             * @param owner The stack_impl.
             */
            market_manager(boost::asio::io_service &, stack_impl &);
        
            /**
             * Starts
             */
            void start();
        
            /**
             * Stops
             */
            void stop();
        
            /**
             * Adds a market.
             * @param key The key.
             * @note The key format must be of "LEFT_SYMBOL/RIGHT_SYMBOL".
             */
            bool add(const std::string &);
        
            /**
             * Removes a market.
             * @param key The key.
             * @note The key format must be of "LEFT_SYMBOL/RIGHT_SYMBOL".
             */
            bool remove(const std::string &);
        
            /**
             * Adds an ask order to the market.
             * @param key The key.
             * @param price The price.
             * @param quantity The quantity.
             * @note The key format must be of "LEFT_SYMBOL/RIGHT_SYMBOL".
             */
            std::uint32_t ask(
                const std::string & key, const float & price,
                const float & quantity
            );
        
            /**
             * Adds a bid order to the market.
             * @param key The key.
             * @param price The price.
             * @param quantity The quantity.
             * @note The key format must be of "LEFT_SYMBOL/RIGHT_SYMBOL".
             */
            std::uint32_t bid(
                const std::string & key, const float & price,
                const float & quantity
            );
        
            /**
             * Cancels an order.
             * @param key The key.
             * @param id The id.
             * @note The key format must be of "LEFT_SYMBOL/RIGHT_SYMBOL".
             */
            bool cancel(
                const std::string & key, const std::uint32_t & id
            );
        
            /**
             * Called by the stack_impl when a query is found.
             * @param q The database::query.
             */
            void on_find(database::query & q);
        
            /**
             * Called by the stack_impl when a route message is received.
             * @param q The database::query.
             */
            void on_route(database::query & q);
        
        private:
        
            // ...
        
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
             * The markets.
             */
            std::map<
                std::string, std::shared_ptr<market>
            > markets_;
        
            /**
             * The markets mutex.
             */
            std::mutex mutex_markets_;
    };
    
} // namespace coinsy

#endif // COINSY_MARKET_MANAGER_HPP
