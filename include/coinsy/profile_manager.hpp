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

#ifndef COINSY_profile_manager_hpp
#define COINSY_profile_manager_hpp

#include <boost/asio.hpp>

#include <cstdint>
#include <map>
#include <mutex>
#include <string>

namespace coinsy {

    class stack_impl;
    
    class profile_manager
        : public std::enable_shared_from_this<profile_manager>
    {
        public:
        
            /**
             * Two hours.
             */
            enum { republish_interval = 3600 * 2 };
        
            /**
             * Constructor
             * @param ios The boost::asio::io_service.
             * @param owner The owner.
             */
            explicit profile_manager(boost::asio::io_service &, stack_impl &);
        
            /**
             * Starts
             */
            void start();
        
            /**
             * Stops
             */
            void stop();
        
            /**
             * Sets the profile.
             */
            void set_profile(const std::map<std::string, std::string> & profile);
        
            /**
             * The profile.
             */
            const std::map<std::string, std::string> & profile() const;
        
            /**
             * Publishes the profile.
             * @param interval The interval.
             */
            std::uint16_t do_publish(
                const std::uint32_t & interval = republish_interval
            );
        
        private:
        
            /**
             * Saves to disk.
             */
            void save();
        
            /**
             * Loads from disk.
             */
            void load();
        
            /**
             * The profile.
             */
            std::map<std::string, std::string> m_profile;
        
        protected:
        
            /**
             * The boost::asio::io_service.
             */
            boost::asio::io_service & io_service_;
        
            /**
             * The boost::asio::strand.
             */
            boost::asio::strand strand_;
        
            /** 
             * The stack_impl.
             */
            stack_impl & stack_impl_;
        
            /**
             * The timer.
             */
            boost::asio::basic_waitable_timer<std::chrono::steady_clock> timer_;
        
            /**
             * The publications mutex.
             */
            mutable std::recursive_mutex mutex_;
    };
    
} // namespace coinsy

#endif // COINSY_profile_manager_hpp
