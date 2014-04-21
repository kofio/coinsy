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

#include <coinsy/logger.hpp>
#include <coinsy/market.hpp>
#include <coinsy/market_manager.hpp>
#include <coinsy/stack_impl.hpp>

using namespace coinsy;

market_manager::market_manager(
    boost::asio::io_service & ios, stack_impl & owner
    )
    : io_service_(ios)
    , strand_(ios)
    , stack_impl_(owner)
{
    // ...
}

void market_manager::start()
{
    // ...
}

void market_manager::stop()
{
    std::lock_guard<std::mutex> l1(mutex_markets_);
    
    for (auto & i : markets_)
    {
        if (i.second)
        {
            i.second->stop();
        }
    }
    
    markets_.clear();
}

bool market_manager::add(const std::string & key)
{
    std::lock_guard<std::mutex> l1(mutex_markets_);
    
    auto it = markets_.find(key);
    
    if (it == markets_.end())
    {
        auto m = std::make_shared<market> (
            io_service_, stack_impl_, key
        );
        
        markets_[key] = m;
        
        m->start();
        
        return true;
    }
    
    return false;
}

bool market_manager::remove(const std::string & key)
{
    std::lock_guard<std::mutex> l1(mutex_markets_);
    
    auto it = markets_.find(key);
    
    if (it != markets_.end())
    {
        markets_.erase(it);
        
        return true;
    }
    
    return false;
}

std::uint32_t market_manager::ask(
    const std::string & key, const float & price, const float & quantity
    )
{
    std::lock_guard<std::mutex> l1(mutex_markets_);
    
    auto it = markets_.find(key);
    
    if (it != markets_.end())
    {
        if (it->second)
        {
            return it->second->place_ask(price, quantity);
        }
    }
    
    return 0;
}

std::uint32_t market_manager::bid(
    const std::string & key, const float & price, const float & quantity
    )
{
    std::lock_guard<std::mutex> l1(mutex_markets_);
    
    auto it = markets_.find(key);
    
    if (it != markets_.end())
    {
        if (it->second)
        {
            return it->second->place_bid(price, quantity);
        }
    }
    
    return 0;
}

bool market_manager::cancel(const std::string & key, const std::uint32_t & id)
{
    std::lock_guard<std::mutex> l1(mutex_markets_);
    
    auto it = markets_.find(key);
    
    if (it != markets_.end())
    {
        if (it->second)
        {
            return it->second->cancel_order(id);
        }
    }
    
    return 0;
}

void market_manager::on_find(database::query & q)
{
    if (
        q.pairs().find("ask") != q.pairs().end() &&
        q.pairs().find("__p") != q.pairs().end() &&
        q.pairs().find("__q") != q.pairs().end() &&
        q.pairs().find("seller") != q.pairs().end() &&
        q.pairs().find("id") != q.pairs().end() &&
        q.pairs().find("__t") != q.pairs().end()
        )
    {
        auto it = markets_.find(q.pairs()["ask"]);
        
        if (it != markets_.end())
        {
            if (it->second)
            {
                it->second->on_find_ask(q);
            }
        }
    }
    else if (
        q.pairs().find("bid") != q.pairs().end() &&
        q.pairs().find("__p") != q.pairs().end() &&
        q.pairs().find("__q") != q.pairs().end() &&
        q.pairs().find("buyer") != q.pairs().end() &&
        q.pairs().find("id") != q.pairs().end() &&
        q.pairs().find("__t") != q.pairs().end()
        )
    {
        auto it = markets_.find(q.pairs()["bid"]);
        
        if (it != markets_.end())
        {
            if (it->second)
            {
                it->second->on_find_bid(q);
            }
        }
    }
    else if (
        q.pairs().find("trade") != q.pairs().end() &&
        q.pairs().find("__p") != q.pairs().end() &&
        q.pairs().find("__q") != q.pairs().end() &&
        (q.pairs().find("buyer") != q.pairs().end() ||
        q.pairs().find("seller") != q.pairs().end()) &&
        q.pairs().find("id") != q.pairs().end() &&
        q.pairs().find("tid") != q.pairs().end() &&
        q.pairs().find("__t") != q.pairs().end()
        )
    {
        auto it = markets_.find(q.pairs()["trade"]);
        
        if (it != markets_.end())
        {
            if (it->second)
            {
                it->second->on_find_trade(q);
            }
        }
    }
}

void market_manager::on_route(database::query & q)
{
    if (
        q.pairs().find("buy") != q.pairs().end() &&
        q.pairs().find("__p") != q.pairs().end() &&
        q.pairs().find("__q") != q.pairs().end() &&
        q.pairs().find("buyer") != q.pairs().end() &&
        q.pairs().find("seller") != q.pairs().end() &&
        q.pairs().find("id") != q.pairs().end() &&
        q.pairs().find("__t") != q.pairs().end()
        )
    {
        auto it = markets_.find(q.pairs()["buy"]);
        
        if (it != markets_.end())
        {
            if (it->second)
            {
                it->second->on_route_buy(q);
            }
        }
    }
    else if (
        q.pairs().find("sell") != q.pairs().end() &&
        q.pairs().find("__p") != q.pairs().end() &&
        q.pairs().find("__q") != q.pairs().end() &&
        q.pairs().find("buyer") != q.pairs().end() &&
        q.pairs().find("seller") != q.pairs().end() &&
        q.pairs().find("id") != q.pairs().end() &&
        q.pairs().find("__t") != q.pairs().end()
        )
    {
        auto it = markets_.find(q.pairs()["sell"]);
        
        if (it != markets_.end())
        {
            if (it->second)
            {
                it->second->on_route_sell(q);
            }
        }
    }
}
