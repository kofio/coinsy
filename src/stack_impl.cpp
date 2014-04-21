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

#include <future>
#include <iostream>

#include <database/query.hpp>

#include <coinsy/ask.hpp>
#include <coinsy/authentication_manager.hpp>
#include <coinsy/balance_manager.hpp>
#include <coinsy/bid.hpp>
#include <coinsy/credentials_manager.hpp>
#include <coinsy/crypto.hpp>
#include <coinsy/filesystem.hpp>
#include <coinsy/http_transport.hpp>
#include <coinsy/location_manager.hpp>
#include <coinsy/logger.hpp>
#include <coinsy/market_manager.hpp>
#include <coinsy/message.hpp>
#include <coinsy/message_manager.hpp>
#include <coinsy/profile_manager.hpp>
#include <coinsy/stack.hpp>
#include <coinsy/stack_impl.hpp>
#include <coinsy/trollbox_manager.hpp>
#include <coinsy/uri.hpp>
#include <coinsy/version_manager.hpp>

using namespace coinsy;

stack_impl::stack_impl(coinsy::stack & owner)
    : database::stack()
    , strand_(io_service_)
    , stack_(owner)
{
    // ...
}

void stack_impl::start(const std::uint16_t & port)
{
    database_configuration_.set_port(port);
    database_configuration_.set_operation_mode(
        database::stack::configuration::operation_mode_interface
    );
    
    std::vector< std::pair<std::string, std::uint16_t> > contacts;

    contacts.push_back(std::make_pair("23.31.159.168", 40102));
    contacts.push_back(std::make_pair("23.31.159.168", 40104));
    contacts.push_back(std::make_pair("23.31.159.168", 40106));
    contacts.push_back(std::make_pair("23.31.159.168", 40108));
    contacts.push_back(std::make_pair("23.31.159.168", 40110));
    contacts.push_back(std::make_pair("23.31.159.168", 40112));
    contacts.push_back(std::make_pair("23.31.159.168", 40114));
    contacts.push_back(std::make_pair("23.31.159.168", 40116));
    contacts.push_back(std::make_pair("23.31.159.168", 40118));
    
    database::stack::start(database_configuration_);
    
    /**
     * Join the network.
     */
    database::stack::join(contacts);
}

void stack_impl::sign_in(
    const std::string & username, const std::string & password
    )
{    
    /**
     * Set the username.
     */
    m_username = username;
    
    /**
     * Set the password.
     */
    m_password = password;
    
    /**
     * Set the node id.
     */
    this->set_node_id(m_username);
    
    try
    {
        create_directories();
    }
    catch (std::exception & e)
    {
        log_error(
            "Stack failed to create directories, what = " << e.what() << "."
        );
    }
    
    /**
     * Allocate the credentials_manager.
     */
    credentials_manager_.reset(new credentials_manager(io_service_, *this));

    /**
     * Set the on started handler.
     */
    credentials_manager_->set_on_started(
        std::bind(&stack_impl::credentials_manager_on_started, this)
    );

    /**
     * Start the credentials_manager.
     */
    credentials_manager_->start();
}

void stack_impl::sign_out()
{
    work_.reset();

    if (balance_manager_)
    {
        balance_manager_->stop();
    }
    
    if (credentials_manager_)
    {
        credentials_manager_->stop();
    }

    if (location_manager_)
    {
        // ...
    }
    
    if (market_manager_)
    {
        market_manager_->stop();
    }
 
    if (message_manager_)
    {
        message_manager_->stop();
    }
    
    if (profile_manager_)
    {
        profile_manager_->stop();
    }
    
    if (trollbox_manager_)
    {
        trollbox_manager_->stop();
    }
    
    if (version_manager_)
    {
        version_manager_->stop();
    }
    
    authentication_manager_.reset();
    credentials_manager_.reset();
    market_manager_.reset();
    message_manager_.reset();
    profile_manager_.reset();
    trollbox_manager_.reset();
    version_manager_.reset();
    
    m_username.clear();
    m_password.clear();
    
    try
    {
        if (thread_.joinable())
        {
            //thread_.join();
        }
    
    }
    catch (std::exception & e)
    {
        thread_.detach();
        
        std::cerr << e.what() << std::endl;
    }
    
    /**
     * Set the node id.
     */
    this->set_node_id("");
}

const std::map<std::string, std::string> & stack_impl::profile() const
{
    if (profile_manager_)
    {
        return profile_manager_->profile();
    }
    
    static std::map<std::string, std::string> ret;
    
    return ret;
}

std::string stack_impl::send_message(
    const std::string & username, const std::wstring & message_attribute
    )
{
    if (message_manager_)
    {
        return message_manager_->route_chat_message(
            username, message_attribute
        );
    }
    
    return std::string();
}

void stack_impl::url_get(
    const std::string & url,
    const std::function<void (const std::map<std::string, std::string> &,
    const std::string &)> & f
    )
{
    std::shared_ptr<http_transport> t =
        std::make_shared<http_transport>(io_service_, url)
    ;

    t->start(
        [this, f](
        boost::system::error_code ec, std::shared_ptr<http_transport> t)
    {
        if (ec)
        {
            f(std::map<std::string, std::string> (), std::string());
		}
		else
		{
            f(t->headers(), t->response_body());
		}
	});
}

void stack_impl::url_post(
    const std::string & url,
    const std::map<std::string, std::string> & headers,
    const std::string & body,
    const std::function<void (const std::map<std::string, std::string> &,
    const std::string &)> & f
    )
{
    std::shared_ptr<http_transport> t =
        std::make_shared<http_transport>(io_service_, url)
    ;

    t->headers() = headers;
    
    t->set_request_body(body);
    
    t->start(
        [this, f](
        boost::system::error_code ec, std::shared_ptr<http_transport> t)
    {
        if (ec)
        {
            f(std::map<std::string, std::string> (), std::string());
		}
		else
		{
            f(t->headers(), t->response_body());
		}
	});
}

std::vector<std::string> split(const std::string & s, char seperator)
{
   std::vector<std::string> output;

    std::string::size_type prev_pos = 0, pos = 0;

    while ((pos = s.find(seperator, pos)) != std::string::npos)
    {
        std::string substring(s.substr(prev_pos, pos - prev_pos));

        output.push_back(substring);

        prev_pos = ++pos;
    }

    output.push_back(s.substr(prev_pos, pos-prev_pos));

    return output;
}

std::uint16_t stack_impl::update_profile(
    const std::map<std::string, std::string> & profile
    )
{
    if (profile_manager_)
    {
        /**
         * Set the profile.
         */
        profile_manager_->set_profile(profile);
        
        /** 
         * Perform a publish operation.
         */
        return profile_manager_->do_publish();
    }
    
    return 0;
}

bool stack_impl::market_add(const std::string & value)
{
    if (market_manager_)
    {
        return market_manager_->add(value);
    }
    
    return false;
}

bool stack_impl::market_remove(const std::string & value)
{
    if (market_manager_)
    {
        return market_manager_->remove(value);
    }
    
    return false;
}

std::uint32_t stack_impl::market_ask(
    const std::string & key, const float & price, const float & quantity
    )
{
    if (market_manager_)
    {
        return market_manager_->ask(key, price, quantity);
    }
    
    return 0;
}

std::uint32_t stack_impl::market_bid(
    const std::string & key, const float & price, const float & quantity
    )
{
    if (market_manager_)
    {
        return market_manager_->bid(key, price, quantity);
    }
    
    return 0;
}

bool stack_impl::market_cancel(const std::string & key, const std::uint32_t & id)
{
    if (market_manager_)
    {
        return market_manager_->cancel(key, id);
    }
    
    return false;
}

void stack_impl::market_troll(
    const std::string & key, const std::string & message
    )
{
    if (trollbox_manager_)
    {
        trollbox_manager_->troll(key, message);
    }
}

void stack_impl::on_connected(const char * addr, const std::uint16_t & port)
{
#if 0 // Disables anonymous tcp connections.
    /**
     * Set the endpoint to that of our local address.
     */
    m_endpoint = boost::asio::ip::tcp::endpoint(
        boost::asio::ip::address::from_string("192.168.1.4"),
        database_configuration_.port()
    );
#else
    /**
     * Set the endpoint to that of our parent peer.
     */
    m_endpoint = boost::asio::ip::tcp::endpoint(
        boost::asio::ip::address::from_string(addr), port
    );
#endif
    stack_.on_connected(addr, port);
}

void stack_impl::on_disconnected(const char * addr, const std::uint16_t & port)
{
    stack_.on_disconnected(addr, port);
}

void stack_impl::on_find(
    const std::uint16_t & transaction_id, const std::string & query_string
    )
{
    std::lock_guard<std::recursive_mutex> l(mutex_);

    std::string username;
    
    /**
     * Allocate the query.
     */
    database::query q(query_string);
    
    if (credentials_manager_)
    {
        std::string query = query_string;
        
        auto i = query.find("__s=");
        
        log_debug("query = " << query);
        
        if (i != std::string::npos)
        {
            query = query.substr(0, i - 1);
        }

        std::string signature = q.pairs()["__s"];
        
        signature = uri::decode(crypto::base64_decode(
            signature.data(), signature.size())
        );
        
        if (
            q.pairs().find("u") != q.pairs().end() &&
            q.pairs().find("c") != q.pairs().end()
            )
        {
            log_none(
                "Stack found credentials for " << q.pairs()["u"] << "."
            );
            
            credentials_manager_->on_find(q.pairs());
        }
        
        /**
         * :TODO: Check the lifetime and if it seems abnormal drop the packet.
         */
        
        if (q.pairs().find("u") != q.pairs().end())
        {
            username = q.pairs()["u"];
        }
        else if (q.pairs().find("buyer") != q.pairs().end())
        {
            username = q.pairs()["buyer"];
        }
        else if (q.pairs().find("seller") != q.pairs().end())
        {
            username = q.pairs()["seller"];
        }
        else if (
            q.pairs().find("trade") != q.pairs().end() &&
            q.pairs().find("buyer") != q.pairs().end() &&
            q.pairs().find("seller") != q.pairs().end()
            )
        {
            std::string buyer = q.pairs()["buyer"];
            std::string seller = q.pairs()["seller"];
            
            if (buyer == m_username)
            {
                username = seller;
            }
            else if (seller == m_username)
            {
                username = buyer;
            }
        }
        else
        {
            log_error(
                "Stack is unable to determine username for query = " <<
                query << ", u = " << q.pairs()["u"] << ", dropping."
            );
            
            return;
        }

        if (username.size() > 0)
        {
            if (
                credentials_manager_->verify(username, query, signature)
                )
            {
                // ...
            }
            else
            {
                /**
                 * If the user is us store our credentials again.
                 */
                if (username == m_username)
                {
                    if (credentials_manager_)
                    {
                        credentials_manager_->store_credentials(0);
                    }
                }
                
                /**
                 * Lookup the users credentials.
                 */
                find("u=" + username, 1);
                
                /**
                 * Do not continue if a query fails verification.
                 */
                return;
            }
        }
    }
    
    /**
     * Inform the location manager.
     */
    if (location_manager_)
    {
        location_manager_->on_find(username, q);
    }
    
    /**
     * Inform the market manager.
     */
    if (market_manager_)
    {
        market_manager_->on_find(q);
    }
    
    /**
     * Inform the trollbox_manager.
     */
    if (trollbox_manager_)
    {
        trollbox_manager_->on_find(q);
    }
}

void stack_impl::on_proxy(
    const std::uint16_t & tid, const char * addr,
    const std::uint16_t & port, const std::string & value
    )
{
    if (authentication_manager_)
    {
        authentication_manager_->on_proxy(tid, addr, port, value);
    }
    
    if (version_manager_)
    {
        version_manager_->on_proxy(tid, addr, port, value);
    }
}

void stack_impl::on_route(
    const std::string & source, const std::string & destination,
    const std::string & route_payload
    )
{
    log_debug(
        "Stack got route from " << source <<
        ", route_payload = " << route_payload
    );
    
    std::lock_guard<std::recursive_mutex> l(mutex_);
    
    /**
     * Make sure the message is for us.
     */
    if (source != m_username)
    {
        return;
    }
    
    /**
     * Check if this is a query.
     */
    if (route_payload.find("&__") != std::string::npos)
    {
        std::string username;
        
        /**
         * Allocate the query.
         */
        database::query q(route_payload);
        
        if (credentials_manager_)
        {
            std::string query = route_payload;
            
            auto i = query.find("__s=");
            
            if (i != std::string::npos)
            {
                query = query.substr(0, i - 1);
            }

            std::string signature = q.pairs()["__s"];
            
            signature = uri::decode(crypto::base64_decode(
                signature.data(), signature.size())
            );
            
            if (q.pairs().find("u") != q.pairs().end())
            {
                username = q.pairs()["u"];
            }
            else if (
                q.pairs().find("buyer") != q.pairs().end() &&
                q.pairs().find("buy") != q.pairs().end()
                )
            {
                username = q.pairs()["buyer"];
            }
            else if (
                q.pairs().find("seller") != q.pairs().end() &&
                q.pairs().find("sell") != q.pairs().end()
                )
            {
                username = q.pairs()["seller"];
            }
            else
            {
                log_error(
                    "Stack is unable to determine username for query = " <<
                    query << ", u = " << q.pairs()["u"] << ", dropping."
                );
                
                return;
            }

            if (username.size() > 0)
            {
                if (
                    credentials_manager_->verify(username, query, signature)
                    )
                {
                    // ...
                }
                else
                {
                    log_debug(
                        "Stack failed to verify signature, performing "
                        "lookup for " << username << "."
                    );
                    
                    /**
                     * Lookup the users credentials.
                     */
                    find("u=" + username, 8);
                    
                    /**
                     * Do not continue if a query fails verification.
                     */
                    return;
                }
            }
        }
        
        /**
         * Inform the location manager.
         */
        if (location_manager_)
        {
            location_manager_->on_route(username, q);
        }
    
        /**
         * Inform the market manager.
         */
        if (market_manager_)
        {
            market_manager_->on_route(q);
        }
    }
    else
    {
        /**
         * Inform the message manager.
         */
        if (message_manager_)
        {
            message_manager_->on_route(source, route_payload);
        }
    }
}

void stack_impl::on_udp_receive(
    const char * addr, const std::uint16_t & port, const char * buf,
    const std::size_t & len
    )
{
    // ...
}

void stack_impl::on_market_public_asks(
    const std::string & key, const float & lowest_price,
    const std::vector< std::shared_ptr<ask> > & public_asks
    )
{
    std::vector<::stack::market_public_ask_t> sorted;
    
    for (auto & i : public_asks)
    {
        ::stack::market_public_ask_t a;
        
        a.id = i->id();
        a.market = i->market();
        a.price = i->price();
        a.quantity = i->quantity();
        a.seller = i->seller();
        a.timestamp = i->timestamp();
        
        sorted.push_back(a);
    }
    
    /**
     * Sort the public asks by price.
     */
    std::sort(
        sorted.begin(), sorted.end(),
        [](const ::stack::market_public_ask_t & a,
        const ::stack::market_public_ask_t & b
        ) -> bool
    { 
        return a.price < b.price;
    });
    
    stack_.on_market_public_asks(key, lowest_price, sorted);
}

void stack_impl::on_market_public_bids(
    const std::string & key, const float & highest_price,
    const std::vector< std::shared_ptr<bid> > & public_bids
    )
{
    std::vector<::stack::market_public_bid_t> sorted;
    
    for (auto & i : public_bids)
    {
        ::stack::market_public_bid_t b;
        
        b.id = i->id();
        b.market = i->market();
        b.price = i->price();
        b.quantity = i->quantity();
        b.buyer = i->buyer();
        b.timestamp = i->timestamp();
        
        sorted.push_back(b);
    }
    
    /**
     * Sort the public asks by price.
     */
    std::sort(
        sorted.begin(), sorted.end(),
        [](const ::stack::market_public_bid_t & a,
        const ::stack::market_public_bid_t & b
        ) -> bool
    { 
        return a.price > b.price;
    });
    
    stack_.on_market_public_bids(key, highest_price, sorted);
}

void stack_impl::on_ask_state(
    const std::string & key, const std::uint32_t & id,
    const std::uint32_t & state
    )
{
    stack_.on_ask_state(key, id, state);
}

void stack_impl::on_bid_state(
    const std::string & key, const std::uint32_t & id,
    const std::uint32_t & state
    )
{
    stack_.on_bid_state(key, id, state);
}

void stack_impl::on_trade(
    const std::string & key, const std::uint32_t & tid,
    std::map<std::string, std::string> & info
    )
{
    stack_.on_trade(key, tid, info);
}

void stack_impl::on_trollbox(
    const std::string & key,
    const std::vector< std::map<std::string, std::string> > & trolls
    )
{
    stack_.on_trollbox(key, trolls);
}

void stack_impl::on_chat_message(const message & msg)
{
    std::map<std::string, std::wstring> params;

    params["type"] = std::wstring(msg.type().begin(), msg.type().end());
    params["command"] = std::wstring(
        msg.command().begin(), msg.command().end()
    );
    params["id"] = std::wstring(msg.id().begin(), msg.id().end());
    params["timestamp"] = std::wstring(
        msg.timestamp().begin(), msg.timestamp().end()
    );
    params["to"] = std::wstring(msg.to().begin(), msg.to().end());
    params["from"] = std::wstring(msg.from().begin(), msg.from().end());
    params["message_attribute"] = msg.message_attribute();
    
    stack_.on_chat_message(params);
}

const std::string & stack_impl::username() const
{
    return m_username;
}

const boost::asio::ip::tcp::endpoint & stack_impl::endpoint() const
{
    return m_endpoint;
}

void stack_impl::create_directories()
{
    std::string path = filesystem::data_path() + m_username;
    
    log_info(
        "Stack creating path = " << path << "."
    );

    auto result = filesystem::create_path(path);
    
    if (result == 0 || result == filesystem::error_already_exists)
    {
        log_none("Stack, path already exists.");
    }
    else
    {
        throw std::runtime_error(
            "failed to create path " + filesystem::data_path()
        );
    }
}

void stack_impl::credentials_manager_on_started()
{
    /**
     * Reset the boost::asio::io_service.
     */
    io_service_.reset();
    
    /**
     * Allocate the boost::asio::io_service::work.
     */
    work_.reset(new boost::asio::io_service::work(io_service_));

    if (thread_.joinable())
    {
        //thread_.join();
    }

    /**
     * Allocate the thread.
     */
    thread_ = std::thread(
        [this]()
        {
            for (;;)
            {
                try
                {
                    io_service_.run();
                    
                    if (!work_)
                    {
                        break;
                    }
                }
                catch (const boost::system::system_error & e)
                {
                    // ...
                }
            }
        }
    );
    
    /**
     * Allocate the authentication_manager.
     */
    authentication_manager_.reset(
        new authentication_manager(io_service_, *this)
    );

    /**
     * Allocate the version_manager.
     */
    version_manager_.reset(
        new version_manager(io_service_, *this)
    );
    
    /**
     * Get the base64 encoded der representation of the public certificate.
     */
    std::string base64_cert = credentials_manager_->base64_public_cert();
 
    /**
     * Sign into the network.
     */
    authentication_manager_->sign_in(
        m_username, m_password, base64_cert,
        [this] (std::map<std::string, std::string> result)
        {
            if (result.empty())
            {
                /**
                 * Callback
                 */
                stack_.on_sign_in("-1 unknown");
                
                /**
                 * Sign out.
                 */
                io_service_.post(
                    strand_.wrap(std::bind(&stack_impl::sign_out, this))
                );
            }
            else if (result["status"] == "0")
            {
                /**
                 * If the authentication node returned a credentials envelope
                 * set it in the credentials manager.
                 */
                if (result.find("envelope") != result.end())
                {
                    credentials_manager_->set_credentials_envelope(
                        result["envelope"]
                    );
                    
                    /**
                     * Inform the credentials_manager to begin storing our
                     * credentials in the database after a short delay.
                     */
                    credentials_manager_->store_credentials(3);
                }
                
                /**
                 * Allocate the balance_manager.
                 */
                balance_manager_.reset(new balance_manager(io_service_, *this));
                
                /**
                 * Allocate the location_manager.
                 */
                location_manager_.reset(new location_manager());
                
                /**
                 * Allocate the market_manager.
                 */
                market_manager_.reset(new market_manager(io_service_, *this));
                
                /**
                 * Allocate the message_manager.
                 */
                message_manager_.reset(new message_manager(io_service_, *this));
                
                /**
                 * Allocate the profile_manager.
                 */
                profile_manager_.reset(new profile_manager(io_service_, *this));
                
                /**
                 * Allocate the trollbox_manager.
                 */
                trollbox_manager_.reset(
                    new trollbox_manager(io_service_, *this)
                );

                /**
                 * Callback
                 */
                stack_.on_sign_in(result["status"] + " " + result["message"]);
            
                /**
                 * Start the balance_manager.
                 */
                balance_manager_->start();
                
                /**
                 * Set the on balance handler.
                 */
                balance_manager_->set_on_balance(
                    [this] (const std::string & symbol, const float & balance)
                    {
                        stack_.on_balance(symbol, balance);
                    }
                );
                
                /**
                 * Start the market_manager.
                 */
                market_manager_->start();
                
                /**
                 * Start the profile_manager.
                 */
                profile_manager_->start();
                
                /**
                 * Start the trollbox_manager.
                 */
                trollbox_manager_->start();
            }
            else /* Failure */
            {
                /**
                 * Callback
                 */
                stack_.on_sign_in(result["status"] + " " + result["message"]);
                
                /**
                 * Sign out.
                 */
                io_service_.post(
                    strand_.wrap(std::bind(&stack_impl::sign_out, this))
                );
            }
        }
    );

    /**
     * Start the version_manager.
     */
    version_manager_->start(
        [this] (std::map<std::string, std::string> result)
    {
        stack_.on_version(result);
    });
}
