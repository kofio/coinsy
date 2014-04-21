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

#include <boost/asio.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <coinsy/crypto.hpp>
#include <coinsy/credentials_manager.hpp>
#include <coinsy/filesystem.hpp>
#include <coinsy/logger.hpp>
#include <coinsy/profile_manager.hpp>
#include <coinsy/stack_impl.hpp>
#include <coinsy/uri.hpp>

using namespace coinsy;

profile_manager::profile_manager(
    boost::asio::io_service & ios, stack_impl & owner
    )
    : stack_impl_(owner)
    , io_service_(ios)
    , strand_(ios)
    , timer_(ios)
{
    // ...
}

void profile_manager::start()
{
    auto self(shared_from_this());
    
    load();
    
    timer_.expires_from_now(std::chrono::seconds(8));
    timer_.async_wait(
        strand_.wrap(
            [this, self](boost::system::error_code ec)
            {
                if (ec)
                {
                    // ...
                }
                else
                {
                    do_publish(republish_interval);
                }
            }
        )
    );
}

void profile_manager::stop()
{
    timer_.cancel();
    
    save();
}

void profile_manager::set_profile(
    const std::map<std::string, std::string> & profile
    )
{
    std::lock_guard<std::recursive_mutex> l(mutex_);
    
    m_profile = profile;
    
    save();
}

const std::map<std::string, std::string> & profile_manager::profile() const
{
    std::lock_guard<std::recursive_mutex> l(mutex_);
    
    return m_profile;
}

std::uint16_t profile_manager::do_publish(const std::uint32_t & interval)
{
    log_debug(
        "Profile manager is publishing profile, interval = " << interval << "."
    );
    
    if (!stack_impl_.username().empty())
    {
        auto self(shared_from_this());
        
        std::string query;
        
        /**
         * The username.
         */
        query += "u=" + stack_impl_.username();
        
        std::lock_guard<std::recursive_mutex> l(mutex_);
        
        for (auto & i : m_profile)
        {
            query += "&" +
                uri::encode(i.first) + "=" + uri::encode(i.second)
            ;
        }
        
        /**
         * The timestamp.
         */
        query += "&__t=" + std::to_string(std::time(0));
        
        /**
         * Sign the query.
         * __s = signature
         */
        std::string signature = stack_impl_.credentials_manager_->sign(query);
        
        /**
         * The signature.
         */
        query += "&__s=" + uri::encode(crypto::base64_encode(
            signature.data(), signature.size())
        );

        /**
         * 72 hours
         */
        query += "&_l=259200";

        log_debug("query = " << query);
        
        timer_.expires_from_now(std::chrono::seconds(interval));
        timer_.async_wait(
            strand_.wrap(
                [this, self, interval](boost::system::error_code ec)
                {
                    if (ec)
                    {
                        // ...
                    }
                    else
                    {
                        do_publish(interval);
                    }
                }
            )
        );
    
        return stack_impl_.store(query);
    }
    
    return 0;
}

void profile_manager::save()
{
    std::stringstream ss;
    
    try
    {
        std::lock_guard<std::recursive_mutex> l(mutex_);
        
        boost::property_tree::ptree pt;
        
        for (auto & i : m_profile)
        {
            pt.add(i.first, i.second);
        }
        
        write_json(ss, pt);
        
        std::ofstream ofs1(
            filesystem::data_path() +
            stack_impl_.username() + "/profile.json"
        );
        
        ofs1 << ss.str();
        
        ofs1.flush();
        
        std::ofstream ofs2(
            filesystem::data_path() +
            stack_impl_.username() + "/profile.json.bak"
        );
        
        ofs2 << ss.str();
        
        ofs2.flush();
    }
    catch (std::exception & e)
    {
        log_error(e.what());
    }
}

void profile_manager::load()
{
    std::lock_guard<std::recursive_mutex> l(mutex_);
    
    std::stringstream ss;
    
    try
    {
        boost::property_tree::ptree pt;

        read_json(
            filesystem::data_path() +
            stack_impl_.username() + "/profile.json", pt
        );
		
        boost::property_tree::ptree::const_iterator it = pt.begin();
        
        for (; it != pt.end(); ++it)
        {
            log_none(
                "Profile manager loaded "  << it->first << ":" <<
                it->second.get_value<std::string>() << "."
            );
            
            auto value = it->second.get_value<std::string>();
            
            if (value.size() > 0)
            {
                m_profile[it->first] = value;
            }
            
            log_debug(
                "Profile manager loaded profile from file."
            );
        }
    }
    catch (std::exception & e)
    {
        try
        {
            boost::property_tree::ptree pt;

            read_json(
                filesystem::data_path() +
                stack_impl_.username() + "/profile.json.bak", pt
            );
            
            boost::property_tree::ptree::const_iterator it = pt.begin();
            
            for (; it != pt.end(); ++it)
            {
                log_none(
                    "Profile manager loaded "  << it->first << ":" <<
                    it->second.get_value<std::string>() << "."
                );
                
                auto value = it->second.get_value<std::string>();
                
                if (value.size() > 0)
                {
                    m_profile[it->first] = value;
                }
            }
            
            log_debug(
                "Profile manager loaded profile from backup file."
            );
            
            save();
        }
        catch (std::exception & e)
        {
            log_error(e.what());
        }
    }
}
