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

#include <coinsy/location_manager.hpp>
#include <coinsy/logger.hpp>

using namespace coinsy;

std::set<boost::asio::ip::tcp::endpoint> location_manager::locations(
    const std::string & username
    )
{
    std::lock_guard<std::recursive_mutex> l1(mutex_locations_);
    
    auto it = m_locations.find(username);
    
    if (it != m_locations.end())
    {
        return it->second;
    }
    
    return std::set<boost::asio::ip::tcp::endpoint> ();
}

void location_manager::on_find(
    const std::string & username, database::query & q
    )
{
    auto it1 = q.pairs().find("__address");
    
    if (it1 != q.pairs().end())
    {
        auto address = it1->second;
        
        auto it2 = q.pairs().find("__port");
        
        if (it2 != q.pairs().end())
        {
            auto port = boost::lexical_cast<std::uint16_t>(it2->second);
            
            insert(
                username, boost::asio::ip::tcp::endpoint(
                boost::asio::ip::address::from_string(address.c_str()), port)
            );
        }
    }
}

void location_manager::on_route(
    const std::string & username, database::query & q
    )
{
    auto it1 = q.pairs().find("__address");
    
    if (it1 != q.pairs().end())
    {
        auto address = it1->second;
        
        auto it2 = q.pairs().find("__port");
        
        if (it2 != q.pairs().end())
        {
            auto port = boost::lexical_cast<std::uint16_t>(it2->second);
 
            insert(
                username, boost::asio::ip::tcp::endpoint(
                boost::asio::ip::address::from_string(address.c_str()), port)
            );
        }
    }
}

void location_manager::insert(
    const std::string & username, const boost::asio::ip::tcp::endpoint & ep
    )
{
    std::lock_guard<std::recursive_mutex> l1(mutex_locations_);
    
    log_debug(
        "Location manager is inserting " << ep << " for " << username << "."
    );
    
    auto it = m_locations.find(username);
    
    if (it == m_locations.end())
    {
        std::set<boost::asio::ip::tcp::endpoint> endpoints;
        
        endpoints.insert(ep);
        
        m_locations[username] = endpoints;
    }
    else
    {
        it->second.insert(ep);
    }
}
