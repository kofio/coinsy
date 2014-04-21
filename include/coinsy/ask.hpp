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

#ifndef COINSY_ASK_HPP
#define COINSY_ASK_HPP

#include <cassert>
#include <cstdint>
#include <map>
#include <mutex>
#include <random>
#include <stdexcept>
#include <string>

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
     * Implements an ask order.
     */
    class ask : public std::enable_shared_from_this<ask>
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
            ask(
                boost::asio::io_service & ios, stack_impl & owner,
                const std::string & market, const float & price,
                const float & quantity, const std::string & seller,
                const std::uint32_t & id = 0,
                const std::time_t & timestamp = std::time(0),
                const std::time_t & expires = 0
                )
                : m_state(state_none)
                , m_id(id)
                , m_id_transaction(0)
                , m_id_sell(0)
                , m_market(market)
                , m_price(price)
                , m_quantity(quantity)
                , m_seller(seller)
                , m_timestamp(timestamp)
                , m_expires(expires)
                , io_service_(ios)
                , strand_(ios)
                , stack_impl_(owner)
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
                if (m_seller == stack_impl_.username())
                {
                    /**
                     * Perform a store operation.
                     */
                    store();
                }
                else
                {
                    throw std::runtime_error("Seller must be myself");
                }
            }
        
            /**
             * Stops the bid.
             */
            void stop()
            {
                m_state = state_none;
                
                timer_store_.cancel();
                timer_pending_.cancel();
                timer_store_trade_.cancel();
                timer_lookup_trade_.cancel();
            }
        
            /**
             * Called when a matching bid is found.
             * @param q The query.
             */
            void on_find_bid(database::query & q)
            {
                // ...
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
                    if (q.pairs().find("buyer") != q.pairs().end())
                    {
                        auto __p = q.pairs()["__p"];
                        auto __q = q.pairs()["__q"];
                        auto buyer = q.pairs()["buyer"];
                        auto address = q.pairs()["__address"];
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
                            info["buyer"] = buyer;
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
             * Called when a buy is received.
             * @param q The database::query.
             */
            bool on_route_buy(database::query & q)
            {
                bool ret = false;
                
                auto __p = q.pairs()["__p"];
                auto __q = q.pairs()["__q"];
                auto buyer = q.pairs()["buyer"];
                auto address = q.pairs()["__address"];
                auto port = boost::lexical_cast<std::uint16_t>(
                    q.pairs()["__port"]
                );
                auto id = boost::lexical_cast<
                    std::uint32_t>(q.pairs()["id"]
                );

                /**
                 * Check for an error.
                 */
                if (q.pairs().find("error") != q.pairs().end())
                {
                    auto error = q.pairs()["error"];
                    
                    if (error == "processing")
                    {
                        m_id_transaction = 0;
                        m_id_sell = 0;
                        m_state = state_none;
                        
                        /**
                         * Callback
                         */
                        stack_impl_.on_ask_state(m_market, m_id, m_state);
                    }
                    else
                    {
                        // ...
                    }
                    
                    log_error("Ask " << m_id << " got error(" << error << ").");
                }
                else if (m_state == state_pending)
                {
                    if (
                        q.pairs().find("__ask") != q.pairs().end() &&
                        q.pairs().find("__bid") != q.pairs().end()
                        )
                    {
                        auto __ask = q.pairs()["__ask"];
                        auto __bid = q.pairs()["__bid"];
                        
                        __bid = uri::decode(__bid);
                        __bid = crypto::base64_decode(
                            __bid.data(), __bid.size()
                        );
                        
                        __ask = uri::decode(__ask);
                        __ask = crypto::base64_decode(
                            __ask.data(), __ask.size()
                        );
                        
                        /**
                         * Allocate the ask query.
                         */
                        database::query query_ask(__ask);
                        
                        auto index = __ask.find("&_l=");
                        
                        __ask = __ask.substr(0, index);
                        
                        auto query = query_string(m_timestamp);
                        
                        index = query.find("&_l=");
                        
                        query = query.substr(0, index);

                        if (query == __ask)
                        {
                            try
                            {
                                auto id_ask = boost::lexical_cast<
                                    std::uint32_t>(query_ask.pairs()["id"]
                                );

                                if (m_id == id_ask)
                                {
                                    log_debug(
                                        "Ask " << m_id << " is setting state "
                                        "to state_processing."
                                    );
                                    
                                    auto tid = boost::lexical_cast<
                                        std::uint32_t>(q.pairs()["tid"]
                                    );
    
                                    /**
                                     * Cancel the pending timer.
                                     */
                                    timer_pending_.cancel();
                                
                                    /**
                                     * Set state to state_processing.
                                     */
                                    m_state = state_processing;
                            
                                    /**
                                     * Route the sell.
                                     */
                                    route_sell(
                                        buyer, id_ask, address, port, tid, __bid
                                    );

                                    /**
                                     * Cancel the store timer.
                                     */
                                    timer_store_.cancel();
                                    
                                    /**
                                     * Callback
                                     */
                                    stack_impl_.on_ask_state(
                                        m_market, m_id, state_processing
                                    );
                                    
                                    log_debug(
                                        "Ask " << m_id << " trade " <<
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
                                    info["buyer"] = buyer;
                                    info["timestamp"] = std::to_string(
                                        m_timestamp
                                    );
                                    info["trade_reference"] = trade_reference;
                            
                                    /**
                                     * Callback that the trade has completed
                                     * but is awaiting confirmation.
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
                                    "Ask " << m_id << " failed to route sell, "
                                    "what = " << e.what() << "."
                                );
                            }
                        }
                        else
                        {
                            // ...
                        }
                    }
                }
                else if (m_state == state_processing)
                {
                    log_debug(
                        "Ask " << m_id << " got buy but state is "
                        "state_processing."
                    );
#if 0 /* :TODO: */
                    route_error(buyer, m_id_sell, address, port);
#endif
                    ret = false;
                }
                else
                {
                    if (m_id == id)
                    {
                        if (
                            q.pairs().find("__ask") != q.pairs().end() &&
                            q.pairs().find("__bid") != q.pairs().end()
                            )
                        {
                            auto __ask = q.pairs()["__ask"];
                            auto __bid = q.pairs()["__bid"];
                            
                            __ask = uri::decode(__ask);
                            __ask = crypto::base64_decode(
                                __ask.data(), __ask.size()
                            );
                            __bid = uri::decode(__bid);
                            __bid = crypto::base64_decode(
                                __bid.data(), __bid.size()
                            );
                            
                            /**
                             * Allocate the ask query.
                             */
                            database::query query_ask(__ask);
                            
                            auto index = __ask.find("&_l=");
                            
                            __ask = __ask.substr(0, index);
                            
                            auto query = query_string(m_timestamp);
                            
                            index = query.find("&_l=");
                            
                            query = query.substr(0, index);

                            if (query == __ask)
                            {
                                log_debug(
                                    "Ask " << m_id << " is setting state to "
                                    "state_pending."
                                );
                                
                                m_state = state_pending;
                                
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
                                                    "Ask " << m_id <<
                                                    " timed out."
                                                );
                                                
                                                m_id_transaction = 0;
                                                m_id_sell = 0;
                                                m_state = state_none;
                                                
                                                /**
                                                 * Callback
                                                 */
                                                stack_impl_.on_ask_state(
                                                    m_market, m_id, m_state
                                                );
                                            }
                                        }
                                    )
                                );
                        
                                /**
                                 * Callback
                                 */
                                stack_impl_.on_ask_state(
                                    m_market, m_id, m_state
                                );
                                
                                m_id_sell = m_id;
                                
                                auto tid = boost::lexical_cast<
                                    std::uint32_t>(q.pairs()["tid"]
                                );
                        
                                try
                                {
                                    auto id_ask = boost::lexical_cast<
                                        std::uint32_t>(query_ask.pairs()["id"]
                                    );
                                    
                                    route_sell(
                                        buyer, id_ask, address, port, tid, __bid
                                    );
                                    
                                    ret = true;
                                }
                                catch (std::exception & e)
                                {
                                    log_debug(
                                        "Ask " << m_id << " failed to route "
                                        "sell, what = " << e.what() << "."
                                    );
                                }
                            }
                        }
                    }
                }
                
                return ret;
            }
        
            /**
             * Routes an error.
             * @param buyer The buyer.
             * @param id The id.
             * @param address The address.
             * @param port The port.
             */
            void route_error(
                const std::string & buyer, const std::uint32_t & id,
                const std::string & address, const std::uint16_t & port
                )
            {
                /**
                 * Forumlate the query.
                 */
                std::string query = "sell=" + m_market;
                
                /**
                 * buy=mint/btc&buyer=xyz&seller=abc&id=123
                 * &__t=1394701668&__s==eyJ1IjoiZ3Jh...I6IjAifQ==&_l=13
                 */
                query += "&__p=" + std::to_string(m_price);
                query += "&__q=" + std::to_string(m_quantity);
                query += "&buyer=" + buyer;
                query += "&seller=" + m_seller;
                query += "&__address=" +
                    stack_impl_.endpoint().address().to_string()
                ;
                query += "&__port=" +
                    std::to_string(stack_impl_.endpoint().port())
                ;
                query += "&id=" + std::to_string(m_id_sell);
                
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
                    "Ask " << m_id << " is routing sell(error) to " <<
                    buyer << " (" << address << ":" << port << ")."
                );
                
                /**
                 * Route the query.
                 */
                stack_impl_.route(address.c_str(), port, buyer, query);
            }
        
            /**
             * Routes a sell.
             * @param buyer The buyer.
             * @param id The id.
             * @param address The address.
             * @param port The port.
             * @param __bid The __bid.
             */
            void route_sell(
                const std::string & buyer, const std::uint32_t & id,
                const std::string & address, const std::uint16_t & port,
                const std::uint32_t & tid, const std::string & __bid
                )
            {
                log_debug("Ask is routing ask, id = " << id << ".");
                
                /**
                 * Set the transaction id.
                 */
                m_id_transaction = tid;
                
                /**
                 * Set the sell id.
                 */
                m_id_sell = id;

                /**
                 * Forumlate the query.
                 */
                std::string query = "sell=" + m_market;
                
                /**
                 * sell=mint/btc&buyer=xyz&seller=abc&id=123
                 * &__t=1394701668&__s==eyJ1IjoiZ3Jh...I6IjAifQ==&_l=13
                 */
                query += "&__p=" + std::to_string(m_price);
                query += "&__q=" + std::to_string(m_quantity);
                query += "&buyer=" + buyer;
                query += "&seller=" + m_seller;
                query += "&__address=" +
                    stack_impl_.endpoint().address().to_string()
                ;
                query += "&__port=" +
                    std::to_string(stack_impl_.endpoint().port())
                ;
                query += "&id=" + std::to_string(m_id_sell);

                /**
                 * Set the transaction id.
                 */
                query += "&tid=" + std::to_string(m_id_transaction);
                
                /**
                 * Get the __ask.
                 */
                auto __ask = query_string(m_timestamp);
                
                auto index = __ask.find("&_l=");
            
                __ask = __ask.substr(0, index);
                
                /**
                 * Set the __ask.
                 */
                query += "&__ask=" + uri::encode(
                    crypto::base64_encode(__ask.data(), __ask.size())
                );

                auto __bid_encoded = uri::encode(
                    crypto::base64_encode(__bid.data(), __bid.size())
                );

                /**
                 * Set the __bid.
                 */
                query += "&__bid=" + __bid_encoded;

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
                    "Ask " << m_id << " is routing sell to " << buyer << "."
                );
                
                /**
                 * Route the query.
                 */
                stack_impl_.route(address.c_str(), port, buyer, query);
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
             * The sell id.
             */
            const std::uint32_t & id_sell() const
            {
                return m_id_sell;
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
             * The seller.
             */
            const std::string & seller() const
            {
                return m_seller;
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
            std::uint32_t m_id_sell;
        
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
             * The seller.
             */
            std::string m_seller;
        
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
                std::string query = "ask=" + m_market;
                
                /**
                 * ask=mint/btc&__p=0.0123&__q=7&seller=abc&id=123
                 * &__t=1394701668&__s=eyJ1IjoiZ3Jh...I6IjAifQ==&_l=8
                 */
                query += "&__p=" + std::to_string(m_price);
                query += "&__q=" + std::to_string(m_quantity);
                query += "&seller=" + m_seller;
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
             * Generates a trade from a buy query.
             * @param q The buy query.
             */
            std::string generate_trade(database::query & q)
            {
                /**
                 * Allocate the return value.
                 */
                std::string ret;

                /**
                 *  Get the buyer.
                 */
                auto buyer = q.pairs()["buyer"];

                /**
                 * Append the trade (market name).
                 */
                ret = "trade=" + m_market;

                /**
                 * Append the buy query.
                 */
                ret += "&__buy=" + uri::encode(
                    crypto::base64_encode(q.str().data(), q.str().size())
                );
                
                /**
                 * Forumlate the query.
                 */
                std::string query_sell = "sell=" + m_market;
                
                /**
                 * sell=mint/btc&buyer=xyz&seller=abc&id=123
                 * &__t=1394701668&__s==eyJ1IjoiZ3Jh...I6IjAifQ==&_l=13
                 */
                query_sell += "&__p=" + std::to_string(m_price);
                query_sell += "&__q=" + std::to_string(m_quantity);
                query_sell += "&buyer=" + buyer;
                query_sell += "&seller=" + m_seller;
                query_sell += "&__address=" +
                    stack_impl_.endpoint().address().to_string()
                ;
                query_sell += "&__port=" +
                    std::to_string(stack_impl_.endpoint().port())
                ;
                query_sell += "&id=" + std::to_string(m_id_sell);

                /**
                 * Set the transaction id.
                 */
                query_sell += "&tid=" + std::to_string(m_id_transaction);
                
                /**
                 * Get the __ask.
                 */
                auto __ask = query_string(m_timestamp);
                
                auto index = __ask.find("&_l=");
            
                __ask = __ask.substr(0, index);
                
                __ask = uri::encode(
                    crypto::base64_encode(
                    __ask.data(), __ask.size())
                );

                /**
                 * Set the ask.
                 */
                query_sell += "&__ask=" + __ask;

                /**
                 * Get the bid.
                 */
                auto __bid = q.pairs()["__bid"];
                
                /**
                 * Set the __bid.
                 */
                query_sell += "&__bid=" + uri::encode(__bid);

                /**
                 * The timestamp.
                 */
                query_sell += "&__t=" + std::to_string(m_timestamp);
                
                /**
                 * Sign the query.
                 */
                std::string signature =
                    stack_impl_.credentials_manager_->sign(query_sell)
                ;
                
                /**
                 * The signature.
                 */
                query_sell += "&__s=" + uri::encode(crypto::base64_encode(
                    signature.data(), signature.size())
                );

                /**
                 * Append the sell query.
                 */
                ret += "&__sell=" + uri::encode(
                    crypto::base64_encode(query_sell.data(), query_sell.size())
                );

                assert(m_id == m_id_sell);
                
                /**
                 * Set the sell id.
                 */
                ret += "&id=" + std::to_string(m_id_sell);
                
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
                 * Set the seller.
                 */
                ret += "&seller=" + m_seller;
                
                /**
                 * Set the sell id.
                 */
                ret += "&id=" + std::to_string(m_id_sell);
                
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
                        "Ask " << m_id << " attempted to stored while state is"
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
                        "Ask " << m_id << " is looking up trade reference for "
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

#endif // COINSY_ASK_HPP
