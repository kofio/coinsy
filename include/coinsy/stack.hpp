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
 
#ifndef COINSY_STACK_HPP
#define COINSY_STACK_HPP

#include <cstdint>
#include <ctime>
#include <functional>
#include <list>
#include <map>
#include <string>
#include <vector>

namespace coinsy {

    class stack_impl;
    
    /**
     * The stack.
     */
    class stack
    {
        public:
        
            /**
             * Implements a market public ask.
             */
            typedef struct
            {
                /**
                 * The id.
                 */
                std::uint32_t id;
            
                /**
                 * The market.
                 */
                std::string market;
            
                /**
                 * The price.
                 */
                float price;
            
                /**
                 * The quantity.
                 */
                float quantity;
            
                /**
                 * The seller.
                 */
                std::string seller;
            
                /**
                 * The timestamp.
                 */
                std::time_t timestamp;
                
            } market_public_ask_t;
        
            /**
             * Implements a market public bid.
             */
            typedef struct
            {
                /**
                 * The id.
                 */
                std::uint32_t id;
            
                /**
                 * The market.
                 */
                std::string market;
            
                /**
                 * The price.
                 */
                float price;
            
                /**
                 * The quantity.
                 */
                float quantity;
            
                /**
                 * The buyer.
                 */
                std::string buyer;
            
                /**
                 * The timestamp.
                 */
                std::time_t timestamp;
                
            } market_public_bid_t;
        
            /**
             * Constructor
             */
            stack();
            
            /**
             * Starts the stack.
             * @param port The listen port.
             */
            void start(const std::uint16_t & port = 0);
            
            /**
             * Stops the stack.
             */
            void stop();
            
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
             * The profile.
             */
            const std::map<std::string, std::string> & profile() const;
            
            /**
             * Performs a store operation.
             * @param query The query.
             */
            std::uint16_t store(const std::string &);
        
            /**
             * Performs a find operation.
             * @param query The query.
             * @param max_results The maximum number of results.
             */
            std::uint16_t find(const std::string &, const std::size_t &);
        
            /**
             * Sends a message to the username.
             * @param username The username.
             * @param message_attribute The message_attribute.
             */
            std::string message(
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
             * Refreshes the subscriptions.
             */
            void update();
        
            /**
             * Updates a profile.
             * @param profile The profile.
             */
            std::uint16_t update_profile(
                const std::map<std::string, std::string> & profile
            );
        
            /**
             * Adds a market to the market manager.
             * @param key The value.
             * @note The key format must be of "LEFT_SYMBOL/RIGHT_SYMBOL".
             */
            bool market_add(const std::string & key);
        
            /**
             * Removes a market from the market manager.
             * @param key The value.
             * @note The key format must be of "LEFT_SYMBOL/RIGHT_SYMBOL".
             */
            bool market_remove(const std::string & key);
        
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
            ) = 0;
        
            /**
             * Called when disconnected from the network.
             * @param addr The address.
             * @param port The port.
             */
            virtual void on_disconnected(
                const char * addr, const std::uint16_t & port
            ) = 0;
        
            /**
             * Called when sign in has completed.
             * @param status The status.
             */
            virtual void on_sign_in(const std::string &) = 0;
        
            /**
             * Called when the ballance is updated.
             * @pram symbol The symbol.
             * @param balance The balance.
             */
            virtual void on_balance(
                const std::string & symbol, const float & balance
            ) = 0;
        
            /**
             * Called when a market has updated the public asks.
             * @param key The key.
             * @param lowest_price The lowest_price.
             * @param public_asks The public asks.
             */
            virtual void on_market_public_asks(
                const std::string & key, const float & lowest_price,
                const std::vector<market_public_ask_t> & public_asks
            ) = 0;
        
            /**
             * Called when a market has updated the public bids.
             * @param key The key.
             * @param highest_price The highest_price.
             * @param public_bids The public bids.
             */
            virtual void on_market_public_bids(
                const std::string & key, const float & highest_price,
                const std::vector<market_public_bid_t> & public_bids
            ) = 0;
        
            /**
             * Called when an ask state changes.
             * @param key The key.
             * @param id The id.
             * @param state The state.
             */
            virtual void on_ask_state(
                const std::string & key, const std::uint32_t & id,
                const std::uint32_t & state
            ) = 0;
        
            /**
             *
             * @param key The key.
             * @param id The id.
             * @param state The state.
             */
            virtual void on_bid_state(
                const std::string & key, const std::uint32_t & id,
                const std::uint32_t & state
            ) = 0;
        
            /**
             * Called when a trade is updated.
             * @param key The key.
             * @param tid The transaction id.
             * @param info The info.
             */
            virtual void on_trade(
                const std::string & key, const std::uint32_t & tid,
                std::map<std::string, std::string> & info
            ) = 0;
        
            /**
             * Called when a trollbox is updated.
             * @param key The key.
             * @param trolls The trolls.
             */
            virtual void on_trollbox(
                const std::string & key,
                const std::vector< std::map<std::string, std::string> > & trolls
            ) = 0;
        
            /**
             * Called when a chat message is received.
             * @param params The params,
             */
            virtual void on_chat_message(
                const std::map<std::string, std::wstring> & params) = 0
            ;
        
            /**
             *
             */
            virtual void on_find_message(
                const std::uint16_t & transaction_id,
                const std::map<std::string, std::string> & pairs,
                const std::vector<std::string> & tags
            ) = 0;
        
            /**
             *
             */
            virtual void on_find_profile(
                const std::uint16_t & transaction_id,
                const std::map<std::string, std::string> & pairs
            ) = 0;
        
            /**
             * Called when a version check completes.
             */
            virtual void on_version(
                const std::map<std::string, std::string> &
            ) = 0;
        
        private:
        
            // ...
            
        protected:
        
            /**
             * The stack implementation.
             */
            stack_impl * stack_impl_;
    };

} // namespace coinsy

#endif // COINSY_STACK_HPP
