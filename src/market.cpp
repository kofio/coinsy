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

#include <boost/lexical_cast.hpp>

#include <coinsy/ask.hpp>
#include <coinsy/balance_manager.hpp>
#include <coinsy/bid.hpp>
#include <coinsy/credentials_manager.hpp>
#include <coinsy/crypto.hpp>
#include <coinsy/logger.hpp>
#include <coinsy/market.hpp>
#include <coinsy/stack_impl.hpp>
#include <coinsy/uri.hpp>

using namespace coinsy;

market::market(
    boost::asio::io_service & ios, stack_impl & owner,
    const std::string & symbols
    )
    : m_lowest_ask(0.0f)
    , m_highest_bid(0.0f)
    , io_service_(ios)
    , strand_(ios)
    , stack_impl_(owner)
    , timer_ask_lookup_(ios)
    , timer_bid_lookup_(ios)
    , timer_public_asks_(ios)
    , timer_public_bids_(ios)
{
    auto i = symbols.find("/");
    
    m_left_symbol = symbols.substr(0, i);
    m_right_symbol = symbols.substr(i + 1, symbols.size() - 1);
}

void market::start()
{
    assert(interval_lookup > 2);
    
    /**
     * Check the balance for the left symbol.
     */
    auto left_balance = stack_impl_.balance_manager_->check(m_left_symbol);
    
    log_debug(
        "Market " << name() << " got balance " << left_balance <<
        " for " << m_left_symbol << "."
    );
    
    /**
     * Check the balance for the right symbol.
     */
    auto right_balance = stack_impl_.balance_manager_->check(m_right_symbol);
    
    log_debug(
        "Market " << name() << " got balance " << right_balance <<
        " for " << m_right_symbol << "."
    );
    
    auto self(shared_from_this());
    
    /**
     * Perform an initial (delayed) ask lookup.
     */
    timer_ask_lookup_.expires_from_now(
        std::chrono::seconds(interval_lookup / 2)
    );
    timer_ask_lookup_.async_wait(
        strand_.wrap(
            [this, self](boost::system::error_code ec)
            {
                if (ec)
                {
                    // ...
                }
                else
                {
                    lookup_ask();
                }
            }
        )
    );
    
    /**
     * Perform an initial (delayed) bid lookup.
     */
    timer_bid_lookup_.expires_from_now(std::chrono::seconds(interval_lookup));
    timer_bid_lookup_.async_wait(
        strand_.wrap(
            [this, self](boost::system::error_code ec)
            {
                if (ec)
                {
                    // ...
                }
                else
                {
                    lookup_bid();
                }
            }
        )
    );
    
    assert(interval_maintanance == 4);
    
    /**
     * Perform an initial (delayed) public asks maintenance operation.
     */
    timer_public_asks_.expires_from_now(
        std::chrono::seconds(interval_maintanance / 2)
    );
    timer_public_asks_.async_wait(
        strand_.wrap(
            [this, self](boost::system::error_code ec)
            {
                if (ec)
                {
                    // ...
                }
                else
                {
                    maintain_public_asks();
                }
            }
        )
    );
    
    /**
     * Perform an initial (delayed) public bids maintenance operation.
     */
    timer_public_bids_.expires_from_now(
        std::chrono::seconds(interval_maintanance)
    );
    timer_public_bids_.async_wait(
        strand_.wrap(
            [this, self](boost::system::error_code ec)
            {
                if (ec)
                {
                    // ...
                }
                else
                {
                    maintain_public_bids();
                }
            }
        )
    );
}

void market::stop()
{
    timer_ask_lookup_.cancel();
    timer_bid_lookup_.cancel();
    timer_public_asks_.cancel();
    timer_public_bids_.cancel();
}

std::uint32_t market::place_ask(const float & price, const float & quantity)
{
    /**
     * Allocate the ask.
     */
    auto a = std::make_shared<ask>(
        io_service_, stack_impl_, name(), price, quantity,
        stack_impl_.username()
    );

    std::lock_guard<std::mutex> l1(mutex_open_asks_);
    
    /**
     * Retain the ask.
     */
    m_open_asks.push_back(a);
    
    /**
     * Start the ask.
     */
    a->start();
    
    return a->id();
}

std::uint32_t market::place_bid(const float & price, const float & quantity)
{
    /**
     * Allocate the bid.
     */
    auto b = std::make_shared<bid>(
        io_service_, stack_impl_, name(), price, quantity,
        stack_impl_.username()
    );
    
    std::lock_guard<std::mutex> l1(mutex_open_bids_);
    
    /**
     * Retain the bid.
     */
    m_open_bids.push_back(b);

    /**
     * Start the bid.
     */
    b->start();
    
    return b->id();
}

bool market::cancel_order(const std::uint32_t & id)
{
    bool found = false;
    
    std::lock_guard<std::mutex> l1(mutex_open_asks_);
    
    auto it = m_open_asks.begin();
    
    for (; it != m_open_asks.end(); ++it)
    {
        if ((*it)->id() == id)
        {
            log_debug("Market is canceling ask " << id << ".");
            
            (*it)->stop();
            
            m_open_asks.erase(it);
            
            found = true;
            
            break;
        }
    }
    
    if (found == false)
    {
        std::lock_guard<std::mutex> l1(mutex_open_bids_);
        
        auto it = m_open_bids.begin();
        
        for (; it != m_open_bids.end(); ++it)
        {
            if ((*it)->id() == id)
            {
                log_debug("Market is canceling bid " << id << ".");
                
                (*it)->stop();
                
                m_open_bids.erase(it);
                
                found = true;
                
                break;
            }
        }
    }
    
    return found;
}

void market::on_find_ask(database::query & q)
{
    auto price = std::stof(q.pairs()["__p"]);
    auto quantity = std::stof(q.pairs()["__q"]);
    auto seller = q.pairs()["seller"];
    auto id = boost::lexical_cast<std::uint32_t>(q.pairs()["id"]);
    auto __t = boost::lexical_cast<std::time_t>(q.pairs()["__t"]);
    auto _e = boost::lexical_cast<std::time_t>(q.pairs()["_e"]);
    
    log_debug(
        "Market " << name() << " found ask, price = " << price <<
        ", quantity = "<< quantity << ", seller = " << seller <<
        ", _e = " << _e  << "."
    );
    
    std::lock_guard<std::mutex> l1(mutex_public_asks_);
    
    bool found = false;
    
    for (auto & i: m_public_asks)
    {
        if (i->id() == id)
        {
            found = true;
            
            i->set_expires(std::time(0) + (_e < 0 ? 0 : _e));
            
            break;
        }
    }
    
    if (found == false)
    {
        /**
         * Allocate the ask.
         */
        auto a = std::make_shared<ask>(
            io_service_, stack_impl_, name(), price, quantity, seller, id, __t,
            _e
        );

        /**
         * Retain the ask.
         */
        m_public_asks.push_back(a);
    }

    std::lock_guard<std::mutex> l2(mutex_open_bids_);
    
    /**
     * Look for the first matching bid.
     */
    for (auto & i : m_open_bids)
    {
        if (
            i->id_transaction() == 0 && i->price() == price &&
            i->quantity() == quantity
            )
        {
            /**
             * Inform the bid.
             */
            i->on_find_ask(q);
        }
    }
}

void market::on_find_bid(database::query & q)
{
    auto price = std::stof(q.pairs()["__p"]);
    auto quantity = std::stof(q.pairs()["__q"]);
    auto buyer = q.pairs()["buyer"];
    auto id = boost::lexical_cast<std::uint32_t>(q.pairs()["id"]);
    auto __t = boost::lexical_cast<std::time_t>(q.pairs()["__t"]);
    auto _e = boost::lexical_cast<std::time_t>(q.pairs()["_e"])
    
    log_none(
        "Market " << name() << " found bid, price = " << price <<
        ", quantity = "<< quantity << ", buyer = " << buyer <<
        ", _e = " << _e << "."
    );
    
    std::lock_guard<std::mutex> l1(mutex_public_bids_);
    
    bool found = false;
    
    for (auto & i: m_public_bids)
    {
        if (i->id() == id)
        {
            found = true;
            
            i->set_expires(std::time(0) + (_e < 0 ? 0 : _e));
            
            break;
        }
    }
    
    if (found == false)
    {
        /**
         * Allocate the bid.
         */
        auto b = std::make_shared<bid>(
            io_service_, stack_impl_, name(), price, quantity, buyer, id, __t,
            _e
        );

        /**
         * Retain the bid.
         */
        m_public_bids.push_back(b);
    }

    std::lock_guard<std::mutex> l2(mutex_open_asks_);
    
    for (auto & i : m_open_asks)
    {
        if (i->price() == price &&i->quantity() == quantity)
        {
            /**
             * We found a matching bid for one of our open asks, inform the ask.
             */
            i->on_find_bid(q);
        }
    }
}

void market::on_route_buy(database::query & q)
{
    std::lock_guard<std::mutex> l1(mutex_open_asks_);
    
    for (auto & i : m_open_asks)
    {
        float price = std::stof(q.pairs()["__p"]);
        float quantity = std::stof(q.pairs()["__q"]);
        auto tid = boost::lexical_cast<std::uint32_t>(
            q.pairs()["tid"]
        );
        
        if (
            (i->id_transaction() == 0 || i->id_transaction() == tid)
            && i->price() == price && i->quantity() == quantity
            )
        {
            /**
             * Inform the ask.
             */
            i->on_route_buy(q);
            
            break;
        }
    }
}

void market::on_route_sell(database::query & q)
{
    std::lock_guard<std::mutex> l1(mutex_open_bids_);
    
    for (auto & i : m_open_bids)
    {
        auto tid = boost::lexical_cast<std::uint32_t>(
            q.pairs()["tid"]
        );
        
        if (i->id_transaction() == tid)
        {
            /**
             * Inform the bid.
             */
            i->on_route_sell(q);
        }
    }
}

void market::on_find_trade(database::query & q)
{
    auto tid = boost::lexical_cast<std::uint32_t>(q.pairs()["tid"]);
    
    std::lock_guard<std::mutex> l1(mutex_open_asks_);
    
    for (auto & i : m_open_asks)
    {
        if (i->id_transaction() == tid)
        {
            i->on_find_trade(q);
            
            break;
        }
    }
    
    std::lock_guard<std::mutex> l2(mutex_open_bids_);
    
    for (auto & i : m_open_bids)
    {
        if (i->id_transaction() == tid)
        {
            i->on_find_trade(q);
            
            break;
        }
    }
}

const std::string market::name() const
{
    return m_left_symbol + "/" + m_right_symbol;
}

const std::string & market::left_symbol() const
{
    return m_left_symbol;
}

const std::string & market::right_symbol() const
{
    return m_right_symbol;
}

bool market::operator==(const market & other) const
{
    return
        m_left_symbol == other.left_symbol() &&
        m_right_symbol == other.right_symbol()
    ;
}

void market::lookup_ask()
{
    std::string query = "ask=" + name();

    /**
     * ask=mint/btc
     */
    stack_impl_.find(query, max_lookup_results);
    
    auto self(shared_from_this());
    
    timer_ask_lookup_.expires_from_now(std::chrono::seconds(interval_lookup));
    timer_ask_lookup_.async_wait(
        strand_.wrap(
            [this, self](boost::system::error_code ec)
            {
                if (ec)
                {
                    // ...
                }
                else
                {
                    lookup_ask();
                }
            }
        )
    );
}

void market::lookup_bid()
{
    std::string query = "bid=" + name();

    /**
     * bid=mint/btc
     */
    stack_impl_.find(query, max_lookup_results);

    auto self(shared_from_this());
    
    timer_bid_lookup_.expires_from_now(std::chrono::seconds(interval_lookup));
    timer_bid_lookup_.async_wait(
        strand_.wrap(
            [this, self](boost::system::error_code ec)
            {
                if (ec)
                {
                    // ...
                }
                else
                {
                    lookup_bid();
                }
            }
        )
    );
}

void market::maintain_public_asks()
{
    log_none("Market " << name() << " is maintaining public asks.");
    
    std::lock_guard<std::mutex> l1(mutex_public_asks_);
    
    /**
     * Callback
     */
    stack_impl_.on_market_public_asks(name(), m_lowest_ask, m_public_asks);
    
    /**
     * Maintain
     */
    auto it = m_public_asks.begin();
    
    /**
     * Set the initial lowest ask.
     */
    if (it == m_public_asks.end())
    {
        m_lowest_ask = 0.0f;
    }
    else
    {
        m_lowest_ask = (*it)->price();
    }
    
    while (it != m_public_asks.end())
    {
        auto a = *it;
        
        if (a)
        {
            if (a->is_expired())
            {
                log_none(
                    "Market " << name() << " public ask " << a->id() <<
                    " expired."
                );
                
                it = m_public_asks.erase(it);
            }
            else
            {
                /**
                 * Update the lowest ask.
                 */
                if (a->price() < m_lowest_ask)
                {
                    m_lowest_ask = a->price();
                }
            
                ++it;
            }
        }
        else
        {
            it = m_public_asks.erase(it);
        }
    }
    
    log_none("Market lowest ask = " << m_lowest_ask << ".");
    
    auto self(shared_from_this());
    
    timer_public_asks_.expires_from_now(
        std::chrono::seconds(interval_maintanance)
    );
    timer_public_asks_.async_wait(
        strand_.wrap(
            [this, self](boost::system::error_code ec)
            {
                if (ec)
                {
                    // ...
                }
                else
                {
                    maintain_public_asks();
                }
            }
        )
    );
}

void market::maintain_public_bids()
{
    log_none("Market " << name() << " is maintaining public bids.");
    
    std::lock_guard<std::mutex> l1(mutex_public_bids_);
    
    /**
     * Callback
     */
    stack_impl_.on_market_public_bids(name(), m_highest_bid, m_public_bids);
    
    /**
     * Maintain
     */
    auto it = m_public_bids.begin();
    
    /**
     * Set the initial lowest ask.
     */
    if (it == m_public_bids.end())
    {
        m_highest_bid = 0;
    }
    else
    {
        m_highest_bid = (*it)->price();
    }
    
    while (it != m_public_bids.end())
    {
        auto b = *it;
        
        if (b)
        {
            if (b->is_expired())
            {
                log_none(
                    "Market " << name() << " public bid " << b->id() <<
                    " expired."
                );
                
                it = m_public_bids.erase(it);
            }
            else
            {
                /**
                 * Update the highest bid.
                 */
                if (b->price() > m_highest_bid)
                {
                    m_highest_bid = b->price();
                }
            
                ++it;
            }
        }
        else
        {
            it = m_public_bids.erase(it);
        }
    }
    
    log_none("Market highest bid = " << m_highest_bid << ".");
    
    auto self(shared_from_this());
    
    timer_public_bids_.expires_from_now(
        std::chrono::seconds(interval_maintanance)
    );
    timer_public_bids_.async_wait(
        strand_.wrap(
            [this, self](boost::system::error_code ec)
            {
                if (ec)
                {
                    // ...
                }
                else
                {
                    maintain_public_bids();
                }
            }
        )
    );
}
