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

#ifndef COINSY_MARKET_HPP
#define COINSY_MARKET_HPP

#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <boost/asio.hpp>

#include <database/query.hpp>

namespace coinsy {

    class ask;
    class bid;
    class stack_impl;
    
    /**
     * Implements a market.
     */
    class market : public std::enable_shared_from_this<market>
    {
        public:
        
            /**
             * Constructor
             * @param ios The boost::asio::io_service.
             * @param owner The stack_impl.
             * @param symbols The symbols.
             */
            market(boost::asio::io_service &, stack_impl &, const std::string &);
        
            /**
             * Starts
             */
            void start();
        
            /**
             * Stops
             */
            void stop();
        
            /**
             * Places an ask order in the market.
             * @param price The price.
             * @param quantity The quantity.
             */
            std::uint32_t place_ask(const float & price, const float & quantity);
        
            /**
             * Places a bid order in the market.
             * @param price The price.
             * @param quantity The quantity.
             */
            std::uint32_t place_bid(const float & price, const float & quantity);
        
            /**
             * Cancels an order.
             * @param id The id.
             */
            bool cancel_order(const std::uint32_t & id);
        
            /**
             * Caled when an ask is found.
             * @param q The query.
             */
            void on_find_ask(database::query & q);
        
            /**
             * Caled when an bid is found.
             * @param q The query.
             */
            void on_find_bid(database::query & q);

            /**
             * Caled when a buy is received.
             * @param q The query.
             */
            void on_route_buy(database::query & q);
        
            /**
             * Caled when a sell is received.
             * @param q The query.
             */
            void on_route_sell(database::query & q);
        
            /**
             * Caled when a trade is found.
             * @param q The query.
             */
            void on_find_trade(database::query & q);
        
            /**
             * The name.
             */
            const std::string name() const;
        
            /**
             * The left symbol.
             */
            const std::string & left_symbol() const;
        
            /**
             * The right symbol.
             */
            const std::string & right_symbol() const;

            /**
             * operator==
             */
            bool operator==(const market &) const;
  
        private:
        
            /**
             * The left symbol.
             */
            std::string m_left_symbol;
        
            /**
             * The right symbol.
             */
            std::string m_right_symbol;
        
            /**
             * The lowest ask.
             */
            float m_lowest_ask;
        
            /**
             * The highest bid.
             */
            float m_highest_bid;

            /**
             * The open asks.
             */
            std::vector< std::shared_ptr<ask> > m_open_asks;
        
            /**
             * The open bids.
             */
            std::vector< std::shared_ptr<bid> > m_open_bids;

            /**
             * The pending asks.
             */
            std::vector< std::shared_ptr<ask> > m_pending_asks;
        
            /**
             * The pending bids.
             */
            std::vector< std::shared_ptr<bid> > m_pending_bids;
        
            /**
             * The public asks.
             */
            std::vector< std::shared_ptr<ask> > m_public_asks;
        
            /**
             * The public bids.
             */
            std::vector< std::shared_ptr<bid> > m_public_bids;
        
        protected:
        
            /**
             * Performs a ask lookup on the symbol pair.
             */
            void lookup_ask();
        
            /**
             * Performs a bid lookup on the symbol pair.
             */
            void lookup_bid();
        
            /**
             * Performs a public asks maintenance operation.
             */
            void maintain_public_asks();
        
            /**
             * Performs a public bids maintenance operation.
             */
            void maintain_public_bids();
        
            /**
             * The lookup interval in seconds.
             */
            enum { interval_lookup = 4 };
        
            /**
             * The maximum number of lookup results.
             * @note The database supports a maximum of 200 results.
             */
            enum { max_lookup_results = 200 };
        
            /**
             * The maintanance interval in seconds.
             */
            enum { interval_maintanance = 4 };
        
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
             * The open asks mutex.
             */
            std::mutex mutex_open_asks_;
        
            /**
             * The open bids mutex.
             */
            std::mutex mutex_open_bids_;
        
            /**
             * The pending asks mutex.
             */
            std::mutex mutex_pending_asks_;
        
            /**
             * The pending bids mutex.
             */
            std::mutex mutex_pending_bids_;
        
            /**
             * The public asks mutex.
             */
            std::mutex mutex_public_asks_;
        
            /**
             * The public bids mutex.
             */
            std::mutex mutex_public_bids_;
        
            /**
             * The ask lookup timer.
             */
            boost::asio::basic_waitable_timer<
                std::chrono::steady_clock
            > timer_ask_lookup_;
        
            /**
             * The bid lookup timer.
             */
            boost::asio::basic_waitable_timer<
                std::chrono::steady_clock
            > timer_bid_lookup_;
        
            /**
             * The public asks timer.
             */
            boost::asio::basic_waitable_timer<
                std::chrono::steady_clock
            > timer_public_asks_;
        
            /**
             * The public bids timer.
             */
            boost::asio::basic_waitable_timer<
                std::chrono::steady_clock
            > timer_public_bids_;
    };

} // namespace coinsy

#endif // COINSY_MARKET_HPP
