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

#include <coinsy/constants.hpp>
#include <coinsy/http_transport.hpp>
#include <coinsy/logger.hpp>
#include <coinsy/stack_impl.hpp>
#include <coinsy/version_manager.hpp>

using namespace coinsy;

version_manager::version_manager(
    boost::asio::io_service & ios, stack_impl & owner
    )
    : m_proxy_tid(0)
#if (defined _WIN32 || defined WIN32) || (defined _WIN64 || defined WIN64)
    , m_platform("windows")
#elif (defined __IPHONE_OS_VERSION_MIN_REQUIRED)
    , m_platform("ios")
#elif (defined __APPLE__)
    , m_platform("osx")
#elif (defined __ANDROID__)
    , m_platform("android")
#else
    , m_platform("linux")
#endif
    , io_service_(ios)
    , strand_(ios)
    , stack_impl_(owner)
    , timer_(ios)
{
    // ...
}

void version_manager::start(
    const std::function<void (std::map<std::string, std::string>)> & f
    )
{
    m_on_complete = f;

    auto self(shared_from_this());
    
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
                    do_tick(86400);
                }
            }
        )
    );
}

void version_manager::stop()
{
    timer_.cancel();
}

void version_manager::check_version()
{
    auto self(shared_from_this());
    
    io_service_.post(strand_.wrap(
        [this, self]()
    {
        do_check_version();
    }));
}

void version_manager::on_proxy(
    const std::uint16_t & tid, const char * addr,
    const std::uint16_t & port, const std::string & value
    )
{
    if (m_proxy_tid == tid)
    {
        m_proxy_tid = 0;
        
        /**
         * Handle the json.
         */
        handle_json(value);
    }
}

void version_manager::do_check_version()
{
    /**
     * Formulate the url from one of the hostnames.
     */
    std::string url =
        "https://www." + std::string(constants::auth_hostnames[0]) +
        "/v/?p=" + m_platform +
        "&s=" + std::to_string(constants::stack_version)
    ;
    
    std::shared_ptr<http_transport> t =
        std::make_shared<http_transport>(io_service_, url)
    ;

    t->start(
        [this](
        boost::system::error_code ec, std::shared_ptr<http_transport> t)
        {
            if (ec)
            {
                log_info(
                    "Version manager failed to connect, trying proxy."
                );
                
                io_service_.post(strand_.wrap(
                    [this]()
                {
                    /**
                     * Generate a random host index.
                     */
                    auto index_host =
                        std::rand() % sizeof(constants::auth_hostnames)
                    ;
                    
                    std::stringstream ss;
                    ss <<
                        "GET" << " "  << "/v2/?p=" << m_platform <<
                        "&s=" << std::to_string(constants::stack_version) <<
                        " HTTP/1.0\r\n"
                    ;
                    ss << "Host: " <<
                        constants::auth_hostnames[index_host] <<
                    "\r\n";
                    ss << "Accept: */*\r\n";
                    ss << "Connection: close\r\n";
                    ss << "\r\n";

                    m_proxy_tid = stack_impl_.proxy(
                        constants::auth_addresses[index_host], 80,
                        ss.str().data(), ss.str().size()
                    );
                }));
            }
            else
            {
                if (t->status_code() == 200)
                {
                    /**
                     * Handle the json.
                     */
                    handle_json(t->response_body());
                }
                else
                {
                    log_error(
                        "Version manager request failed, status "
                        "code = " << t->status_code() << "."
                    );
                }
            }
        }
    );
}

void version_manager::do_tick(const std::uint32_t & interval)
{
    do_check_version();

    if (interval > 0)
    {
        auto self(shared_from_this());
        
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
                        do_tick(interval);
                    }
                }
            )
        );
    }
}

void version_manager::handle_json(const std::string & json)
{
    std::stringstream ss;

    /**
     * Example:
     * {"message":"Success","status":0,"upgrade":"1","required":"0"}
     */
    ss << json;

    /**
     * Allocate empty property tree object.
     */
    boost::property_tree::ptree pt;
    
    std::map<std::string, std::string> result;
    
    try
    {
        /**
         * Read the json.
         */
        read_json(ss, pt);

        result["status"] = pt.get<std::string> (
            "status", ""
        );
        result["message"] = pt.get<std::string> (
            "message", ""
        );
        result["upgrade"] = pt.get<std::string> (
            "upgrade", ""
        );
        result["required"] = pt.get<std::string> (
            "required", ""
        );
        
        if (result["status"] == "0")
        {
            if (result["upgrade"] == "1")
            {
                log_info("Version manager upgrade available.");
                
                if (result["required"] == "1")
                {
                    log_info(
                        "Version manager upgrade required, message = " <<
                        result["message"] << "."
                    );
                }
            }
        }
        else
        {
            std::cerr <<
                "Version manager check failure, message = " <<
                result["message"] << ", status = " << result["status"] << "." <<
            std::endl;
        }
    }
    catch (std::exception & e)
    {
        std::cerr <<
            "Version manager check failure, what = " <<
            e.what() << "." <<
        std::endl;
        
        result["status"] = "-1";
        result["message"] = e.what();
    }
    
    if (m_on_complete)
    {
        m_on_complete(result);
    }
}
