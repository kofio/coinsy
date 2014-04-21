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

#ifndef COINSY_LOCATION_MANAGER_HPP
#define COINSY_LOCATION_MANAGER_HPP

#include <map>
#include <mutex>
#include <set>
#include <string>

#include <boost/asio.hpp>

#include <database/query.hpp>

namespace coinsy {

    /**
     * Implements a location manager.
     */
    class location_manager
    {
        public:
        
            /**
             * The locations.
             * @param username The username.
             */
            std::set<boost::asio::ip::tcp::endpoint> locations(
                const std::string & username
            );
        
            /**
             * Called by the stack_impl when a query is found.
             * @param username The username.
             * @param q The database::query.
             */
            void on_find(const std::string & username, database::query & q);
        
            /**
             * Called by the stack_impl when a route message is received.
             * @param username The username.
             * @param q The database::query.
             */
            void on_route(const std::string & username, database::query & q);
        
        private:
        
            /**
             * Inserts a location for a user.
             * @param username The username.
             * @param ep The boost::asio::ip::tcp::endpoint.
             */
            void insert(
                const std::string & username,
                const boost::asio::ip::tcp::endpoint & ep
            );
        
            /**
             * The locations.
             */
            std::map<
                std::string, std::set<boost::asio::ip::tcp::endpoint> >
            m_locations;
        
        protected:
        
            /**
             * The locations mutex.
             */
            std::recursive_mutex mutex_locations_;
    };
    
} // namespace coinsy

#endif // COINSY_LOCATION_MANAGER_HPP
