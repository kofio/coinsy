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

#ifndef COINSY_STACK_IMPL_HPP
#define COINSY_STACK_IMPL_HPP

#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <boost/asio.hpp>

#include <database/stack.hpp>

namespace coinsy {

    class ask;
    class authentication_manager;
    class balance_manager;
    class bid;
    class credentials_manager;
    class location_manager;
    class market_manager;
    class message;
    class message_manager;
    class profile_manager;
    class stack;
    class trollbox_manager;
    class version_manager;
    
    /**
     * The stack implementation.
     */
    class stack_impl : public database::stack
    {
        public:
        
            /**
             * Constructor
             * @param owner The stack.
             */
            stack_impl(coinsy::stack &);
            
            /**
             * Starts the stack.
             * @param port The listen port.
             */
            void start(const std::uint16_t &);
            
            /**
             * Signs into the network.
             * @@param username The username.
             * @param password The password.
             */
            void sign_in(const std::string &, const std::string &);
        
            /**
             * Signs out of the network.
             */
            void sign_out();
        
            /**
             * The username.
             */
            const std::string & username() const;
        
            /**
             * The endpoint.
             * @note Parent peer or public endpoint are ok but the later not
             * being anonymous.
             */
            const boost::asio::ip::tcp::endpoint & endpoint() const;
        
            /**
             * The profile.
             */
            const std::map<std::string, std::string> & profile() const;
        
            /**
             * Sends a message to the username.
             * @param username The username.
             * @param message_attribute The message_attribute.
             */
            std::string send_message(
                const std::string & username,
                const std::wstring & message_attribute
            );
        
            /**
             * Performs an http get operation toward the url.
             * @param url The url.
             * @param f The function.
             */
            void url_get(
                const std::string & url,
                const std::function<void (const std::map<std::string,
                std::string> &, const std::string &)> & f
            );
        
            /**
             * Performs an http post operation toward the url.
             * @param url The url.
             * @param headers The headers.
             * @param body The body.
             * @param f The function.
             */
            void url_post(
                const std::string & url,
                const std::map<std::string, std::string> & headers,
                const std::string & body,
                const std::function<void (const std::map<std::string,
                std::string> &,
                const std::string &)> & f
            );
        
            /**
             * Updates a profile.
             * @param profile The profile.
             */
            std::uint16_t update_profile(
                const std::map<std::string, std::string> & profile
            );
        
            /**
             * Adds a market to the market manager.
             * @param value The value.
             * @note The value format must be of "LEFT_SYMBOL/RIGHT_SYMBOL".
             */
            bool market_add(const std::string & value);
        
            /**
             * Removes a market from the market manager.
             * @param value The value.
             * @note The value format must be of "LEFT_SYMBOL/RIGHT_SYMBOL".
             */
            bool market_remove(const std::string & value);
        
            /**
             * Adds an ask order to the market.
             * @param key The key.
             * @param price The price.
             * @param quantity The quantity.
             * @note The key format must be of "LEFT_SYMBOL/RIGHT_SYMBOL".
             */
            std::uint32_t market_ask(
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
            std::uint32_t market_bid(
                const std::string & key, const float & price,
                const float & quantity
            );
        
            /**
             * Cancels an order.
             * @param key The key.
             * @param id The id.
             * @note The key format must be of "LEFT_SYMBOL/RIGHT_SYMBOL".
             */
            bool market_cancel(
                const std::string & key, const std::uint32_t & id
            );
        
            /**
             * Performs a troll operation.
             * @param key The key.
             * @param message The message.
             */
            void market_troll(
                const std::string & key, const std::string & message
            );
        
            /**
             * Called when connected to the network.
             * @param addr The address.
             * @param port The port.
             */
            virtual void on_connected(
                const char * addr, const std::uint16_t & port
            );
        
            /**
             * Called when disconnected from the network.
             * @param addr The address.
             * @param port The port.
             */
            virtual void on_disconnected(
                const char * addr, const std::uint16_t & port
            );
        
            /**
             * Called when a search result is received.
             * @param transaction_id The transaction id.
             * @param query The query.
             */
            virtual void on_find(
                const std::uint16_t & transaction_id,
                const std::string & query
            );
        
            /**
             * Called when a proxy (response) is received.
             * @param tid The transaction identifier.
             * @param addr The address.
             * @param The port.
             * @param value The value.
             */
            virtual void on_proxy(
                const std::uint16_t & tid, const char * addr,
                const std::uint16_t & port, const std::string & value
            );
        
            /**
             * Called when a a route is received.
             * @param source The source.
             * @param destination The destination.
             * @param route_payload The route payload.
             */
            virtual void on_route(
                const std::string & source,
                const std::string & destination,
                const std::string & route_payload
            );
        
            /**
             * Called when a udp packet doesn't match the protocol fingerprint.
             * @param addr The address.
             * @param port The port.
             * @param buf The buffer.
             * @param len The length.
             */
            virtual void on_udp_receive(
                const char * addr, const std::uint16_t & port, const char * buf,
                const std::size_t & len
            );
        
            /**
             * Called when a market has updated the public asks.
             * @param key The key.
             * @param lowest_price The lowest_price.
             * @param public_asks The public asks.
             */
            void on_market_public_asks(
                const std::string & key, const float & lowest_price,
                const std::vector< std::shared_ptr<ask> > & public_asks
                );
        
            /**
             * Called when a market has updated the public bids.
             * @param key The key.
             * @param highest_price The highest_price
             * @param public_bids The public bids.
             */
            void on_market_public_bids(
                const std::string & key, const float & highest_price,
                const std::vector< std::shared_ptr<bid> > & public_bids
            );
        
            /**
             * Called when an ask state changes.
             * @param key The key.
             * @param id The id.
             * @param state The state.
             */
            void on_ask_state(
                const std::string & key, const std::uint32_t & id,
                const std::uint32_t & state
            );
        
            /**
             *
             * @param key The key.
             * @param id The id.
             * @param state The state.
             */
            void on_bid_state(
                const std::string & key, const std::uint32_t & id,
                const std::uint32_t & state
            );
        
            /**
             * Called when a trade is updated.
             * @param key The key.
             * @param tid The transaction id.
             * @param info The info.
             */
            void on_trade(
                const std::string & key, const std::uint32_t & tid,
                std::map<std::string, std::string> & info
            );
        
            /**
             * Called when a trollbox is updated.
             * @param key The key.
             * @param trolls The trolls.
             */
            void on_trollbox(
                const std::string & key,
                const std::vector< std::map<std::string, std::string> > & trolls
            );
        
            /**
             * Called when a chat message is received.
             * @param msg The message.
             */
            void on_chat_message(const message & msg);
        
        private:
        
            friend class ask;
            friend class bid;
            friend class envelope;
            friend class market;
            friend class message_manager;
            friend class publish_manager;
            friend class profile_manager;
            friend class trollbox;
        
            /**
             * The username.
             */
            std::string m_username;
        
            /**
             * The password.
             */
            std::string m_password;
        
            /**
             * The endpoint.
             * @note Parent peer or public endpoint are ok but the later not
             * being anonymous.
             */
            boost::asio::ip::tcp::endpoint m_endpoint;
        
        protected:

            /**
             * Creates suport directories.
             */
            void create_directories();
        
            /**
             * Called when the credentials_manager has started.
             */
            void credentials_manager_on_started();
        
            /**
             * The stack.
             */
            coinsy::stack & stack_;
        
            /**
             * The boost::asio::io_service.
             */
            boost::asio::io_service io_service_;
        
            /**
             * The boost::asio::strand.
             */
            boost::asio::strand strand_;
        
            /**
             * The boost::asio::io_service::work.
             */
            std::shared_ptr<boost::asio::io_service::work> work_;
        
            /**
             * The thread.
             */
            std::thread thread_;
        
            /**
             * The std::recursive_mutex.
             */
            std::recursive_mutex mutex_;
        
            /**
             * The database configuration.
             */
            database::stack::configuration database_configuration_;
            
            /**
             * The authentication_manager.
             */
            std::shared_ptr<authentication_manager> authentication_manager_;
        
            /**
             * The balance_manager.
             */
            std::shared_ptr<balance_manager> balance_manager_;
        
            /**
             * The credentials_manager.
             */
            std::shared_ptr<credentials_manager> credentials_manager_;
        
            /**
             * The location_manager.
             */
            std::shared_ptr<location_manager> location_manager_;
        
            /**
             * The market_manager.
             */
            std::shared_ptr<market_manager> market_manager_;
        
            /**
             * The message_manager.
             */
            std::shared_ptr<message_manager> message_manager_;
        
            /**
             * The profile_manager.
             */
            std::shared_ptr<profile_manager> profile_manager_;
        
            /**
             * The trollbox_manager.
             */
            std::shared_ptr<trollbox_manager> trollbox_manager_;
        
            /**
             * The version_manager.
             */
            std::shared_ptr<version_manager> version_manager_;
    };
    
} // namespace coinsy

#endif // COINSY_STACK_IMPL_HPP
