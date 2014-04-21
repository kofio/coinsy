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

#include <coinsy/credentials_manager.hpp>
#include <coinsy/crypto.hpp>
#include <coinsy/logger.hpp>
#include <coinsy/stack_impl.hpp>
#include <coinsy/trollbox.hpp>
#include <coinsy/uri.hpp>

using namespace coinsy;

trollbox::trollbox(
    boost::asio::io_service & ios, stack_impl & owner, const std::string & key
    )
    : io_service_(ios)
    , strand_(ios)
    , stack_impl_(owner)
    , timer_lookup_(ios)
    , timer_maintenance_(ios)
    , m_market(key)
{
    // ...
}

void trollbox::start()
{
    find_trolls(6);
    
    do_maintenance(3);
}

void trollbox::stop()
{
    timer_lookup_.cancel();
    timer_maintenance_.cancel();
}

void trollbox::troll(const std::string & username, const std::string & message)
{
    /**
     * Forumlate the query.
     */
    std::string query = "troll=" + m_market;
    
    /**
     * The username.
     */
    query += "&u=" + username;
    
    /**
     * The username.
     */
    query += "&m=" + message;
    
    /**
     * The timestamp.
     */
    query += "&__t=" + std::to_string(std::time(0));
    
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
    query += "&_l=" + std::to_string(12);

    log_debug(
        "Trollbox " << m_market << " is storing query = " << query
    );
    
    stack_impl_.store(query);
}

void trollbox::on_find_troll(database::query & q)
{
    auto u = q.pairs()["u"];
    auto m = q.pairs()["m"];
    auto _e = boost::lexical_cast<std::time_t>(q.pairs()["_e"]);
    
    log_debug(
        "Trollbox " << m_market << " found troll, u = " << u <<
        ", m = "<< m << ", _e = " << _e  << "."
    );

    /**
     * Allocate the troll.
     */
    troll_t t;
    
    t.username = u;
    t.message = m;
    t.expires = std::time(0) + (_e < 0 ? 0 : _e);

    std::lock_guard<std::mutex> l1(mutex_trolls_);
    
    /**
     * Insert the troll.
     */
    trolls_.push_back(t);
}

void trollbox::find_trolls(const std::uint32_t & interval)
{
    std::string query = "troll=" + m_market;
    
    stack_impl_.find(query, 50);

    auto self(shared_from_this());

    /**
     * Start the lookup timer again.
     */
    timer_lookup_.expires_from_now(
        std::chrono::seconds(interval)
    );
    timer_lookup_.async_wait(
        strand_.wrap(
            [this, self, interval]
            (boost::system::error_code ec)
            {
                if (ec)
                {
                    // ...
                }
                else
                {
                    find_trolls(interval);
                }
            }
        )
    );
}

void trollbox::do_maintenance(const std::uint32_t & interval)
{
    std::lock_guard<std::mutex> l1(mutex_trolls_);
    
    auto it = trolls_.begin();
    
    while (it != trolls_.end())
    {
        const troll_t & t = *it;

        /**
         * We use a slightly longer (300 seconds) expiration so trolls linger
         * longer.
         */
        if (std::time(0) > (t.expires + 300))
        {
            it = trolls_.erase(it);
        }
        else
        {
            ++it;
        }
    }

    /**
     * Sort the trolls.
     */
    std::sort(
        trolls_.begin(), trolls_.end(),
        [](const troll_t & a, const troll_t & b) -> bool
    { 
        return a.expires < b.expires;
    });

    /**
     * Erase duplicates.
     */
    trolls_.erase(std::unique(trolls_.begin(), trolls_.end()), trolls_.end());

    std::vector< std::map<std::string, std::string> > trolls;
    
    for (auto & i : trolls_)
    {
        std::map<std::string, std::string> troll;
        
        troll["u"] = i.username;
        troll["m"] = i.message;
        
        trolls.push_back(troll);
    }
    
    /**
     * Callback
     */
    stack_impl_.on_trollbox(m_market, trolls);

    auto self(shared_from_this());
    
    /**
     * Start the lookup timer again.
     */
    timer_maintenance_.expires_from_now(
        std::chrono::seconds(interval)
    );
    timer_maintenance_.async_wait(
        strand_.wrap(
            [this, self, interval]
            (boost::system::error_code ec)
            {
                if (ec)
                {
                    // ...
                }
                else
                {
                    do_maintenance(interval);
                }
            }
        )
    );
}
