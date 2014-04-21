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

#ifndef COINSY_BID_HPP
#define COINSY_BID_HPP

#include <cassert>
#include <cstdint>
#include <deque>
#include <map>
#include <mutex>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#include <boost/asio.hpp>
#include <boost/lexical_cast.hpp>

#include <database/query.hpp>

#include <coinsy/credentials_manager.hpp>
#include <coinsy/crypto.hpp>
#include <coinsy/logger.hpp>
#include <coinsy/stack_impl.hpp>
#include <coinsy/uri.hpp>

namespace coinsy {

    /**
     * Implements an bid.
     */
    class bid : public std::enable_shared_from_this<bid>
    {
        public:
        
            /**
             * The states.
             */
            typedef enum
            {
                state_none,
                state_pending,
                state_processing,
                state_confirming,
                state_confirmed,
                state_closed,
            } state_t;
        
            /**
             * Constructor
             */
            bid(
                boost::asio::io_service & ios, stack_impl & owner,
                const std::string & market, const float & price,
                const float & quantity, const std::string & buyer,
                const std::uint32_t & id = 0,
                const std::time_t & timestamp = std::time(0),
                const std::time_t & expires = 0
                )
                : m_state(state_none)
                , m_id(id)
                , m_id_transaction(0)
                , m_id_buy(0)
                , m_market(market)
                , m_price(price)
                , m_quantity(quantity)
                , m_buyer(buyer)
                , m_timestamp(timestamp)
                , io_service_(ios)
                , strand_(ios)
                , stack_impl_(owner)
                , timer_asks_(ios)
                , timer_store_(ios)
                , timer_pending_(ios)
                , timer_store_trade_(ios)
                , timer_lookup_trade_(ios)
            {
                static std::random_device rd;
                static std::mt19937_64 mt(rd());
                std::uniform_int_distribution<std::uint32_t> dist;

                if (id == 0)
                {
                    /**
                     * Set the pseudo-random id.
                     */
                    m_id = dist(mt);
                }
            }
        
            /**
             * Starts the bid.
             */
            void start()
            {
                if (m_buyer == stack_impl_.username())
                {
                    /**
                     * Perform a store operation.
                     */
                    store();
                    
                    /**
                     * Start processing asks.
                     */
                    do_process_asks();
                }
                else
                {
                    throw std::runtime_error("Buyer must be myself");
                }
            }
        
            /**
             * Stops the bid.
             */
            void stop()
            {
                m_state = state_none;
                
                timer_asks_.cancel();
                timer_store_.cancel();
                timer_store_trade_.cancel();
                timer_lookup_trade_.cancel();
            }
        
            /**
             * Called when a matching ask is found.
             * @param q The query.
             */
            void on_find_ask(database::query & q)
            {
                // :TODO: state mutex
            
                std::lock_guard<std::recursive_mutex> l1(mutex_asks_cache_);

                bool found = false;
                
                for (auto & i : asks_cache_)
                {
                    if (i.str() == q.str())
                    {
                        found = true;
                        
                        break;
                    }
                }
                
                if (found == false)
                {
                    /**
                     * Cache the ask.
                     */
                    asks_cache_.push_back(q);
                }
                
                std::lock_guard<std::recursive_mutex> l2(mutex_asks_);
                
                /**
                 * Queue the ask.
                 */
                if (asks_.size() == 0)
                {
                    asks_.push_back(q);
                }
                else
                {
                    bool found = false;
                    
                    for (auto & i : asks_)
                    {
                        if (i.str() == q.str())
                        {
                            found = true;
                            
                            break;
                        }
                    }
                    
                    if (found == false)
                    {
                        asks_.push_back(q);
                    }
                }
                
                /**
                 * Randomize the asks so that each bid won't be trying the
                 * same ask at the same time.
                 */
                std::random_shuffle(asks_.begin(), asks_.end());
            }
        
            /**
             * Called when a matching trade is found.
             * @param q The query.
             */
            void on_find_trade(database::query & q)
            {
                if (m_state >= state_confirmed)
                {
                    // ...
                }
                else
                {
                    if (q.pairs().find("seller") != q.pairs().end())
                    {
                        auto __p = q.pairs()["__p"];
                        auto __q = q.pairs()["__q"];
                        auto seller = q.pairs()["seller"];
                        auto tid = boost::lexical_cast<
                            std::uint32_t>(q.pairs()["tid"])
                        ;
                        
                        /**
                         * Confirm the trade.
                         */
                        if (tid == m_id_transaction)
                        {
                            timer_lookup_trade_.cancel();
                            
                            m_state = state_confirmed;
                            
                            /**
                             * Allocate the info.
                             */
                            std::map<std::string, std::string> info;

                            info["state"] = "confirmed";
                            info["market"] = m_market;
                            info["price"] = __p;
                            info["quantity"] = __q;
                            info["seller"] = seller;
                            info["timestamp"] = std::to_string(
                                m_timestamp
                            );
                    
                            /**
                             * Callback that the trade is confirmed.
                             */
                            stack_impl_.on_trade(
                                m_market, m_id_transaction, info
                            );
                        }
                    }
                }
            }
        
            /**
             * Called when a sell is received.
             * @param q The database::query.
             */
            bool on_route_sell(database::query & q)
            {
                bool ret = false;
                
                auto __p = q.pairs()["__p"];
                auto __q = q.pairs()["__q"];
                auto seller = q.pairs()["seller"];
                auto address = q.pairs()["__address"];
                auto port = boost::lexical_cast<std::uint16_t>(
                    q.pairs()["__port"]
                );
                auto id = boost::lexical_cast<
                    std::uint32_t>(q.pairs()["id"]
                );
                auto tid = boost::lexical_cast<
                    std::uint32_t>(q.pairs()["tid"]
                );

                assert(m_id_transaction == tid);
                
                /**
                 * Check for an error.
                 */
                if (q.pairs().find("error") != q.pairs().end())
                {
                    auto error = q.pairs()["error"];
                    
                    /**
                     * Experimental
                     */
                    if (error == "processing")
                    {
                        m_id_transaction = 0;
                        m_id_buy = 0;
                        m_state = state_none;
                        
                        /**
                         * Callback
                         */
                        stack_impl_.on_bid_state(m_market, m_id, m_state);
                        
                        do_process_asks();
                    }
                    else
                    {
                        // ...
                    }
                    
                    log_error("Bid " << m_id << " got error(" << error << ").");
                }
                else if (m_state == state_pending)
                {
                    try
                    {
                        assert(m_id_buy == id);

                        if (m_id_transaction == tid)
                        {
                            log_debug(
                                "Bid " << m_id << " is setting state to "
                                "state_processing."
                            );
                            
                            m_state = state_processing;
                    
                            route_buy(seller, m_id_buy, address, port);
                            
                            timer_pending_.cancel();
                            
                            /**
                             * Cancel the store timer.
                             */
                            timer_store_.cancel();
                            
                            /**
                             * Cancel the asks timer.
                             */
                            timer_asks_.cancel();
                            
                            /**
                             * Callback
                             */
                            stack_impl_.on_bid_state(
                                m_market, m_id, m_state
                            );
                            
                            log_debug(
                                "Bid " << m_id << " trade " <<
                                m_id_transaction << " completed, "
                                "waiting for confirmation."
                            );

                            /**
                             * Generate a trade reference.
                             */
                            auto trade_reference =
                                generate_trade_reference()
                            ;
                            
                            log_debug(
                                "^^^^^^^^^^^: bid trade_reference = " <<
                                trade_reference
                            );
                            
                            /**
                             * Set the state.
                             */
                            m_state = state_confirming;

                            /**
                             * Allocate the info.
                             */
                            std::map<std::string, std::string> info;
                            
                            info["state"] = "confirming";
                            
                            info["market"] = m_market;
                            info["price"] = __p;
                            info["quantity"] = __q;
                            info["seller"] = seller;
                            info["timestamp"] = std::to_string(
                                m_timestamp
                            );
                            info["trade_reference"] = trade_reference;
  
                            /**
                             * Callback that the trade has completed but is
                             * awaiting confirmation.
                             */
                            stack_impl_.on_trade(
                                m_market, m_id_transaction, info
                            );
                            
                            /**
                             * Store the trade reference.
                             */
                            store_trade_reference(trade_reference);
                            
                            /**
                             * Lookup trade reference's.
                             */
                            find_trade_reference();
                        
                            ret = true;
                        }
                        else
                        {
                            assert(0);
                        }
                    }
                    catch (std::exception & e)
                    {
                        log_debug(
                            "Bid " << m_id << " failed to route "
                            "buy, what = " << e.what() << "."
                        );
                    }
                }
                else if (m_state == state_processing)
                {
                    log_debug(
                        "Bid " << m_id << " got sell but state is "
                        "state_processing."
                    );
#if 0 /* :TODO: */
                    route_error(seller, m_id_buy, address, port);
#endif
                    ret = false;
                }
                else
                {
                    // ...
                }
                
                return ret;
            }
        
            /**
             * Routes an error.
             * @param seller The seller.
             * @param id The id.
             * @param address The address.
             * @param port The port.
             */
            void route_error(
                const std::string & seller, const std::uint32_t & id,
                const std::string & address, const std::uint16_t & port
                )
            {
                /**
                 * Forumlate the query.
                 */
                std::string query = "buy=" + m_market;
                
                /**
                 * buy=mint/btc&buyer=xyz&seller=abc&id=123
                 * &__t=1394701668&__s==eyJ1IjoiZ3Jh...I6IjAifQ==&_l=13
                 */
                query += "&__p=" + std::to_string(m_price);
                query += "&__q=" + std::to_string(m_quantity);
                query += "&buyer=" + m_buyer;
                query += "&seller=" + seller;
                query += "&__address=" +
                    stack_impl_.endpoint().address().to_string()
                ;
                query += "&__port=" +
                    std::to_string(stack_impl_.endpoint().port())
                ;
                query += "&id=" + std::to_string(m_id_buy);
                
                /**
                 * Set the transaction id.
                 */
                query += "&tid=" + std::to_string(m_id_transaction);

                /**
                 * Set the error.
                 */
                query += "&error=processing";
                
                /**
                 * The timestamp.
                 */
                query += "&__t=" + std::to_string(m_timestamp);
                
                /**
                 * Sign the query.
                 */
                std::string signature =
                    stack_impl_.credentials_manager_->sign(query)
                ;
                
                /**
                 * The signature.
                 */
                query += "&__s=" + uri::encode(crypto::base64_encode(
                    signature.data(), signature.size())
                );

                log_debug(
                    "Bid " << m_id << " is routing buy(error) to " <<
                    seller << " (" << address << ":" << port << ")."
                );
                
                /**
                 * Route the query.
                 */
                stack_impl_.route(address.c_str(), port, seller, query);
            }
        
            /**
             * Routes a buy.
             * @param seller The seller.
             * @param id The id.
             * @param address The address.
             * @param port The port.
             */
            void route_buy(
                const std::string & seller, const std::uint32_t & id,
                const std::string & address, const std::uint16_t & port
                )
            {
                log_debug("Bid is routing buy, id = " << id << ".");
                
                /**
                 * Set the buy id.
                 */
                m_id_buy = id;
                
                /**
                 * Forumlate the query.
                 */
                std::string query = "buy=" + m_market;
                
                /**
                 * buy=mint/btc&buyer=xyz&seller=abc&id=123
                 * &__t=1394701668&__s==eyJ1IjoiZ3Jh...I6IjAifQ==&_l=13
                 */
                query += "&__p=" + std::to_string(m_price);
                query += "&__q=" + std::to_string(m_quantity);
                query += "&buyer=" + m_buyer;
                query += "&seller=" + seller;
                query += "&__address=" +
                    stack_impl_.endpoint().address().to_string()
                ;
                query += "&__port=" +
                    std::to_string(stack_impl_.endpoint().port())
                ;
                query += "&id=" + std::to_string(m_id_buy);
                
                /**
                 * Set the transaction id.
                 */
                query += "&tid=" + std::to_string(m_id_transaction);
                
                bool found = false;
                
                for (auto & i : asks_cache_)
                {
                    if (i.pairs().find("id") != i.pairs().end())
                    {
                        std::uint32_t id_ask = boost::lexical_cast<
                            std::uint32_t>(i.pairs()["id"]
                        );

                        if (m_id == m_id_buy || m_id_buy == id_ask)
                        {
                            auto __ask = i.str();

                            auto index = __ask.find("&_l=");
                        
                            __ask = __ask.substr(0, index);
                            
                            __ask = uri::encode(
                                crypto::base64_encode(
                                __ask.data(), __ask.size())
                            );
                
                            query += "&__ask=" + __ask;
                            
                            found = true;
                            
                            break;
                        }
                    }
                }

                /**
                 * Get the __bid.
                 */
                auto __bid = query_string(m_timestamp);
                
                auto index = __bid.find("&_l=");
            
                __bid = __bid.substr(0, index);
                
                __bid = uri::encode(
                    crypto::base64_encode(__bid.data(), __bid.size())
                );

                /**
                 * Set the __bid.
                 */
                query += "&__bid=" + __bid;
                
                if (found)
                {
                    /**
                     * The timestamp.
                     */
                    query += "&__t=" + std::to_string(m_timestamp);
                    
                    /**
                     * Sign the query.
                     */
                    std::string signature =
                        stack_impl_.credentials_manager_->sign(query)
                    ;
                    
                    /**
                     * The signature.
                     */
                    query += "&__s=" + uri::encode(crypto::base64_encode(
                        signature.data(), signature.size())
                    );

                    log_none(
                        "Bid " << m_id << " is routing buy to " <<
                        seller << " (" << address << ":" << port << ")."
                    );
                    
                    /**
                     * Route the query.
                     */
                    stack_impl_.route(address.c_str(), port, seller, query);
                }
            }
        
            /**
             * The state.
             */
            const state_t & state() const
            {
                return m_state;
            }
        
            /**
             * The id.
             */
            const std::uint32_t & id() const
            {
                return m_id;
            }
        
            /**
             * The transaction id.
             */
            const std::uint32_t & id_transaction() const
            {
                return m_id_transaction;
            }
        
            /**
             * The buy id.
             */
            const std::uint32_t & id_buy() const
            {
                return m_id_buy;
            }
        
            /**
             * The market.
             */
            const std::string & market() const
            {
                return m_market;
            }
        
            /**
             * The price.
             */
            const float & price() const
            {
                return m_price;
            }
        
            /**
             * The quantity.
             */
            const float & quantity() const
            {
                return m_quantity;
            }
        
            /**
             * The buyer.
             */
            const std::string & buyer() const
            {
                return m_buyer;
            }
        
            /**
             * Sets the timestamp.
             */
            void set_timestamp(const std::time_t & value)
            {
                m_timestamp = value;
            }
            
            /**
             * The timestamp.
             */
            const std::time_t & timestamp() const
            {
                return m_timestamp;
            }
        
            /**
             * Sets the expires.
             */
            void set_expires(const std::time_t & value)
            {
                m_expires = value;
            }
        
            /**
             * The expires.
             */
            const std::time_t & expires() const
            {
                return m_expires;
            }
        
            /**
             * If true we are expired.
             */
            bool is_expired()
            {
                return std::time(0) > m_expires;
            }
        
            /**
             * The lifetime interval.
             */
            enum { interval_lifetime = 8 };
        
            /**
             * The storage interval.
             */
            enum { interval_storage = interval_lifetime / 2 };
        
            /**
             * The asks process interval.
             */
            enum { interval_process_asks = 8000 };
        
            /**
             * The pending interval.
             */
            enum { interval_pending = 7500 };
        
            /**
             * The trade reference lifetime interval.
             */
            enum { interval_lifetime_trade_reference = 259200 };
        
            /**
             * The trade reference storage interval.
             */
            enum { interval_store_trade_reference = 14400 };
            
        private:
        
            /**
             * The state.
             */
            state_t m_state;
        
            /**
             * The id.
             */
            std::uint32_t m_id;
        
            /**
             * The transaction id.
             */
            std::uint32_t m_id_transaction;
        
            /**
             * The buy id.
             */
            std::uint32_t m_id_buy;
        
            /**
             * The market.
             */
            std::string m_market;
        
            /**
             * The price.
             */
            float m_price;
        
            /**
             * The quantity.
             */
            float m_quantity;
        
            /**
             * The buyer.
             */
            std::string m_buyer;
        
            /**
             * The timestamp.
             */
            std::time_t m_timestamp;
        
            /**
             * The expires.
             */
            std::time_t m_expires;
        
        protected:
        
            /**
             * Builds the query string.
             * @param timestamp The timestamp.
             */
            std::string query_string(const std::time_t & timestamp)
            {
                /**
                 * Forumlate the query.
                 */
                std::string query = "bid=" + m_market;
                
                /**
                 * bid=mint/btc&__p=0.0123&__q=7&buyer=abc&id=123
                 * &__t=1394701668&__s=eyJ1IjoiZ3Jh...I6IjAifQ==&_l=8
                 */
                query += "&__p=" + std::to_string(m_price);
                query += "&__q=" + std::to_string(m_quantity);
                query += "&buyer=" + m_buyer;
                query += "&__address=" +
                    stack_impl_.endpoint().address().to_string()
                ;
                query += "&__port=" +
                    std::to_string(stack_impl_.endpoint().port())
                ;
                query += "&id=" + std::to_string(m_id);
                
                /**
                 * The timestamp.
                 */
                query += "&__t=" + std::to_string(timestamp);
                
                /**
                 * Sign the query.
                 */
                std::string signature =
                    stack_impl_.credentials_manager_->sign(query)
                ;
                
                /**
                 * The signature.
                 */
                query += "&__s=" + uri::encode(crypto::base64_encode(
                    signature.data(), signature.size())
                );
                
                /**
                 * The lifetime.
                 */
                query += "&_l=" + std::to_string(interval_lifetime);
                
                return query;
            }
        
            /**
             * Generates a trade from a sell query.
             * @param q The sell query.
             */
            std::string generate_trade(database::query & q)
            {
                /**
                 * Allocate the return value.
                 */
                std::string ret;

                /**
                 *  Get the seller.
                 */
                auto seller = q.pairs()["seller"];
                
                /**
                 * Forumlate the query.
                 */
                std::string query_buy = "buy=" + m_market;
                
                /**
                 * buy=mint/btc&buyer=xyz&seller=abc
                 * &__address=192.168.1.1&__port=40028
                 * &id=123&tid=321&__t=1394701668&__s==eyJ1IjoiZ3Jh...I6IjAifQ==
                 * &_l=13
                 */
                query_buy += "&__p=" + std::to_string(m_price);
                query_buy += "&__q=" + std::to_string(m_quantity);
                query_buy += "&buyer=" + m_buyer;
                query_buy += "&seller=" + seller;
                query_buy += "&__address=" +
                    stack_impl_.endpoint().address().to_string()
                ;
                query_buy += "&__port=" +
                    std::to_string(stack_impl_.endpoint().port())
                ;
                query_buy += "&id=" + std::to_string(m_id_buy);
                
                /**
                 * Set the transaction id.
                 */
                query_buy += "&tid=" + std::to_string(m_id_transaction);
                
                bool found = false;
                
                for (auto & i : asks_cache_)
                {
                    if (i.pairs().find("id") != i.pairs().end())
                    {
                        std::uint32_t id_ask = boost::lexical_cast<
                            std::uint32_t>(i.pairs()["id"]
                        );

                        if (m_id == m_id_buy || m_id_buy == id_ask)
                        {
                            auto __ask = i.str();
                            
                            auto index = __ask.find("&_l=");
                        
                            __ask = __ask.substr(0, index);
                        
                            __ask = uri::encode(
                                crypto::base64_encode(
                                __ask.data(), __ask.size())
                            );
                            
                            query_buy += "&__ask=" + __ask;
                            
                            found = true;
                            
                            break;
                        }
                    }
                }
                
                assert(found);

                /**
                 * Get the __bid.
                 */
                auto __bid = query_string(m_timestamp);

                auto index = __bid.find("&_l=");
            
                __bid = __bid.substr(0, index);
                
                __bid = uri::encode(
                    crypto::base64_encode(__bid.data(), __bid.size())
                );
                
                /**
                 * Set the __bid.
                 */
                query_buy += "&__bid=" + __bid;
                
                //log_debug("bid: __bid = " << __bid);

                /**
                 * Set the timestamp.
                 */
                query_buy += "&__t=" + std::to_string(m_timestamp);
                
                /**
                 * Sign the query.
                 */
                std::string signature =
                    stack_impl_.credentials_manager_->sign(query_buy)
                ;
                
                /**
                 * The signature.
                 */
                query_buy += "&__s=" + uri::encode(crypto::base64_encode(
                    signature.data(), signature.size())
                );
                
                /**
                 * Append the trade (market name).
                 */
                ret = "trade=" + m_market;
                
                /**
                 * Append the buy query.
                 */
                ret += "&__buy=" + uri::encode(
                    crypto::base64_encode(query_buy.data(), query_buy.size())
                );

                /**
                 * Get the __sell.
                 */
                auto __sell = q.str();

                /**
                 * Append the sell query.
                 */
                ret += "&__sell=" + uri::encode(
                    crypto::base64_encode(__sell.data(), __sell.size())
                );

                /**
                 * Set the buy id.
                 */
                ret += "&id=" + std::to_string(m_id_buy);
                
                /**
                 * Set the transaction id.
                 */
                ret += "&tid=" + std::to_string(m_id_transaction);
                
                /**
                 * Set the timestamp.
                 */
                ret += "&__t=" + std::to_string(m_timestamp);
                
                /**
                 * Sign the query.
                 */
                signature =
                    stack_impl_.credentials_manager_->sign(ret)
                ;
                
                /**
                 * Set the signature.
                 */
                ret += "&__s=" + uri::encode(crypto::base64_encode(
                    signature.data(), signature.size())
                );
                
                return ret;
            }
        
            /**
             * Generates a trade reference.
             */
            std::string generate_trade_reference()
            {
                std::string ret;
                
                /**
                 * trade=LTC/BTC&__p=0.0123&__q=7&buyer=xyz&seller=abc
                 * &id=123&tid=321&__t=1394701668
                 * &__s==gHdeAJ0TIcj39%2BrMc...5y1aP8gax%2BNxiA%3D
                 */
                
                /**
                 * Append the trade (market name).
                 */
                ret = "trade=" + m_market;
                
                /**
                 * Set the price.
                 */
                ret += "&__p=" + std::to_string(m_price);
                
                /**
                 * Set the price.
                 */
                ret += "&__q=" + std::to_string(m_quantity);
                
                /**
                 * Set the price.
                 */
                ret += "&buyer=" + m_buyer;
                
                /**
                 * Set the buy id.
                 */
                ret += "&id=" + std::to_string(m_id_buy);
                
                /**
                 * Set the transaction id.
                 */
                ret += "&tid=" + std::to_string(m_id_transaction);
                
                /**
                 * Set the timestamp.
                 */
                ret += "&__t=" + std::to_string(m_timestamp);
                
                /**
                 * Sign the query.
                 */
                auto signature =
                    stack_impl_.credentials_manager_->sign(ret)
                ;
                
                /**
                 * Set the signature.
                 */
                ret += "&__s=" + uri::encode(crypto::base64_encode(
                    signature.data(), signature.size())
                );
                
                /**
                 * 72 hours
                 */
                ret += "&_l=" +
                    std::to_string(interval_lifetime_trade_reference)
                ;
  
                return ret;
            }
        
            /**
             * Performs a store operation.
             */
            void store()
            {
                if (m_state >= state_processing)
                {
                    log_error(
                        "Bid " << m_id << " attempted to stored while state is"
                        " >= state_processing."
                    );
                }
                else
                {
                    /**
                     * Store the query.
                     */
                    stack_impl_.store(query_string(m_timestamp));
                    
                    auto self(shared_from_this());

                    /**
                     * Start the store timer again.
                     */
                    timer_store_.expires_from_now(
                        std::chrono::seconds(interval_storage)
                    );
                    timer_store_.async_wait(
                        strand_.wrap(
                            [this, self](boost::system::error_code ec)
                            {
                                if (ec)
                                {
                                    // ...
                                }
                                else
                                {
                                    store();
                                }
                            }
                        )
                    );
                }
            }
        
            /**
             * Stores a trade reference.
             */
            void store_trade_reference(const std::string & trade_reference)
            {
                /**
                 * Store the query.
                 */
                stack_impl_.store(trade_reference);
                
                auto self(shared_from_this());

                /**
                 * Start the store timer again.
                 */
                timer_store_trade_.expires_from_now(
                    std::chrono::seconds(interval_store_trade_reference)
                );
                timer_store_trade_.async_wait(
                    strand_.wrap(
                        [this, self, trade_reference]
                        (boost::system::error_code ec)
                        {
                            if (ec)
                            {
                                // ...
                            }
                            else
                            {
                                store_trade_reference(trade_reference);
                            }
                        }
                    )
                );
            }
        
            /**
             * Looks up a trade reference.
             */
            void find_trade_reference()
            {
                if (m_state == state_confirmed)
                {
                    // ...
                }
                else
                {
                    log_debug(
                        "Bid " << m_id << " is looking up trade reference for "
                        "tid = " << m_id_transaction << "."
                    );
                    
                    std::string query = "trade=" + m_market;
                    
                    query += "&id=" + std::to_string(m_id);
                    query += "&tid=" + std::to_string(m_id_transaction);
                    
                    stack_impl_.find(query, 1);
                    
                    auto self(shared_from_this());

                    /**
                     * Start the lookup timer again.
                     */
                    timer_lookup_trade_.expires_from_now(
                        std::chrono::seconds(8)
                    );
                    timer_lookup_trade_.async_wait(
                        strand_.wrap(
                            [this, self]
                            (boost::system::error_code ec)
                            {
                                if (ec)
                                {
                                    // ...
                                }
                                else
                                {
                                    find_trade_reference();
                                }
                            }
                        )
                    );
                }
            }
        
            void do_process_asks()
            {
                assert(interval_pending < interval_process_asks);
                
                std::lock_guard<std::recursive_mutex> l1(mutex_asks_);
                
                if (asks_.size() > 0)
                {
                    auto & q = asks_.front();
                    
                    std::string seller = q.pairs()["seller"];
                    auto address = q.pairs()["__address"];
                    auto port = boost::lexical_cast<std::uint16_t>(
                        q.pairs()["__port"]
                    );
                    std::uint32_t id = boost::lexical_cast<std::uint32_t>(
                        q.pairs()["id"]
                    );
            
                    if (m_state == state_none)
                    {
                        static std::random_device rd;
                        static std::mt19937_64 mt(rd());
                        std::uniform_int_distribution<std::uint32_t> dist;

                        /**
                         * Generate a transaction id.
                         */
                        m_id_transaction = dist(mt);
                    
                        m_state = state_pending;
                        
                        /**
                         * Callback
                         */
                        stack_impl_.on_bid_state(
                            m_market, m_id, m_state
                        );
                        
                        auto self(shared_from_this());
                        
                        /**
                         * Start the pending timer.
                         */
                        timer_pending_.expires_from_now(
                            std::chrono::milliseconds(interval_pending)
                        );
                        timer_pending_.async_wait(
                            strand_.wrap(
                                [this, self](boost::system::error_code ec)
                                {
                                    if (ec)
                                    {
                                        // ...
                                    }
                                    else
                                    {
                                        log_debug(
                                            "Bid " << m_id << " timed out."
                                        );
                                        
                                        m_id_transaction = 0;
                                        m_id_buy = 0;
                                        m_state = state_none;
                                        
                                        /**
                                         * Callback
                                         */
                                        stack_impl_.on_bid_state(
                                            m_market, m_id, m_state
                                        );
                                        
                                        do_process_asks();
                                    }
                                }
                            )
                        );
                        
                        route_buy(seller, id, address, port);
                        
                        asks_.pop_front();
                    }
                    else if (m_state > state_none)
                    {
                        // ...
                    }
                }
            
                auto self(shared_from_this());

                /**
                 * Start the timer again.
                 */
                timer_asks_.expires_from_now(
                    std::chrono::milliseconds(interval_process_asks)
                );
                timer_asks_.async_wait(
                    strand_.wrap(
                        [this, self](boost::system::error_code ec)
                        {
                            if (ec)
                            {
                                // ...
                            }
                            else
                            {
                                do_process_asks();
                            }
                        }
                    )
                );
            }

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
             * The found asks queue mutex.
             */
            std::recursive_mutex mutex_asks_;
        
            /**
             * The found asks cache mutex.
             */
            std::recursive_mutex mutex_asks_cache_;
        
            /**
             * The found asks queue.
             */
            std::deque<database::query> asks_;
        
            /**
             * The found asks cache.
             */
            std::vector<database::query> asks_cache_;
        
            /**
             * The asks timer.
             */
            boost::asio::basic_waitable_timer<
                std::chrono::steady_clock
            > timer_asks_;
        
            /**
             * The store timer.
             */
            boost::asio::basic_waitable_timer<
                std::chrono::steady_clock
            > timer_store_;
        
            /**
             * The timeout timer.
             */
            boost::asio::basic_waitable_timer<
                std::chrono::steady_clock
            > timer_pending_;
        
            /**
             * The store trade timer.
             */
            boost::asio::basic_waitable_timer<
                std::chrono::steady_clock
            > timer_store_trade_;
        
            /**
             * The lookup trade timer.
             */
            boost::asio::basic_waitable_timer<
                std::chrono::steady_clock
            > timer_lookup_trade_;
    };
}

#endif // COINSY_BID_HPP
