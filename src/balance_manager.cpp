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

#include <cstdlib>

#include <coinsy/balance_manager.hpp>
#include <coinsy/logger.hpp>
#include <coinsy/stack_impl.hpp>

using namespace coinsy;

balance_manager::balance_manager(
    boost::asio::io_service & ios, stack_impl & owner
    )
    : stack_impl_(owner)
    , io_service_(ios)
    , strand_(ios)
    , timer_balance_(ios)
{
    // ...
}

void balance_manager::start()
{
    /**
     * Maintain after delay triggering callback.
     */
    maintain(8);
}

void balance_manager::stop()
{
    timer_balance_.cancel();
}

float balance_manager::check(const std::string & symbol)
{
    float ret = 0.0f;
    
    auto it = m_balances.find(symbol);
    
    if (it == m_balances.end())
    {
        ret = 25 + static_cast <float> (std::rand()) /
            (static_cast <float> (RAND_MAX / (500 - 25))
        );
        
        /**
         * Assign a pseudorandom number between 25 and 500.
         */
        m_balances[symbol] = ret;
        
        /**
         * Maintain after a delay triggering callback.
         */
        maintain(1);
    }
    else
    {
        ret = it->second;
    }
    
    return ret;
}

void balance_manager::set_on_balance(
    const std::function<void (const std::string &, const float &)> & f
    )
{
    m_on_balance = f;
}

void balance_manager::maintain(const std::uint32_t & interval)
{
    auto self(shared_from_this());
    
    timer_balance_.expires_from_now(
        std::chrono::seconds(interval)
    );
    timer_balance_.async_wait(
        strand_.wrap(
            [this, self](boost::system::error_code ec)
            {
                if (ec)
                {
                    // ...
                }
                else
                {
                    do_maintain(interval_maintanance);
                }
            }
        )
    );
}

void balance_manager::do_maintain(const std::uint32_t & interval)
{
    log_debug("Balance manager is maintaing balance.");
    
    for (auto & i : m_balances)
    {
        /**
         * Callback
         */
        if (m_on_balance)
        {
            m_on_balance(i.first, i.second);
        }
    }
    
    maintain(interval);
}
