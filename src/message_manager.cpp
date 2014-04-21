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

#include <cassert>

#include <coinsy/credentials_manager.hpp>
#include <coinsy/crypto.hpp>
#include <coinsy/envelope.hpp>
#include <coinsy/location_manager.hpp>
#include <coinsy/logger.hpp>
#include <coinsy/message.hpp>
#include <coinsy/message_manager.hpp>
#include <coinsy/stack_impl.hpp>

using namespace coinsy;

message_manager::message_manager(
    boost::asio::io_service & ios, stack_impl & owner
    )
    : io_service_(ios)
    , strand_(ios)
    , stack_impl_(owner)
{
    // ...
}

void message_manager::start()
{
    // ...
}

void message_manager::stop()
{
    // ...
}

std::string message_manager::route_chat_message(
    const std::string & to, const std::wstring & message_attribute
    )
{
    /**
     * Allocate the message.
     */
    message msg(
        "chat", "message", to, stack_impl_.username()
    );
    
    /**
     * Set the message attribute.
     */
    msg.set_message_attribute(message_attribute);
    
    /**
     * Encode the message.
     */
    msg.encode();
#if 0 // Sends messages without envelopes (no encryption or signature).
    route_message(msg);
#else
    /**
     * Allocate the envelope.
     */
    envelope env(stack_impl_, msg.json());
    
    /**
     * Encode the envelope.
     */
    env.encode(stack_impl_.credentials_manager_->credentials(to));
    
    /**
     * Route the envelope.
     */
    route_envelope(msg.to(), env);
#endif
    /**
     * Return the message id.
     */
    return msg.id();
}

void message_manager::route_message(const message & msg)
{
    if (stack_impl_.location_manager_)
    {
        auto locations = stack_impl_.location_manager_->locations(msg.to());
        
        if (locations.size() > 0)
        {
            for (auto & i : locations)
            {
                auto address = i.address();
                auto port = i.port();
                
                log_debug(
                    "Message manager is routing message to " <<
                    msg.to() << " (" << address << ":" << port << ")."
                );

                /**
                 * Route the message.
                 */
                stack_impl_.route(
                    address.to_string().c_str(), port, msg.to(), msg.json()
                );
            }
        }
        else
        {
            log_error(
                "Message manager failed to route message for " <<
                msg.to() << ", no locations found."
            );
        }
    }
    else
    {
        assert(0);
    }
}

void message_manager::on_route(
    const std::string & source, const std::string & route_payload
    )
{
    /**
     * Check if it is an envelope or a message.
     */
    auto i = route_payload.find("application");

    if (i == std::string::npos)
    {
        try
        {
            /**
             * Allocate the message.
             */
            message msg(route_payload);
            
            /**
             * Decode the message.
             */
            msg.decode();
            
            /**
             * Handle the message.
             */
            handle_message(msg);
        }
        catch (std::exception & e)
        {
            log_debug(
                "Message manager failed to decode message, what = " <<
                e.what() << "."
            );
        }
    }
    else
    {
        try
        {
            /**
             * Get the credentials for the user.
             */
            auto credentials = stack_impl_.credentials_manager_->credentials(
                source
            );
            
            if (credentials.size() > 0)
            {
                for (auto & i : credentials)
                {
                    try
                    {
                        /**
                         * Allocate the envelope.
                         */
                        envelope env(stack_impl_, route_payload);
                    
                        /**
                         * Decode the envelope.
                         */
                        if (env.decode(i->pub()))
                        {
                            /**
                             * Allocate the message.
                             */
                            message msg(env.value());
                        
                            /**
                             * Decode the message.
                             */
                            msg.decode();
                            
                            /**
                             * Handle the message.
                             */
                            handle_message(msg);
                            
                            break;
                        }
                    }
                    catch (std::exception & e)
                    {
                        log_debug(
                            "Message manager failed to decode envelope, "
                            "what = " << e.what() << "."
                        );
                    }
                }
            }
            else
            {
                log_debug(
                    "Message manager failed to decode envelope, no "
                    "credentials for " << source << "."
                );
            }
        }
        catch (std::exception & e)
        {
            log_debug(
                "Message manager failed to decode envelope, what = " <<
                e.what() << "."
            );
        }
    }
}

void message_manager::handle_message(message & msg)
{
    if (msg.type() == "chat")
    {
        if (msg.command() == "message")
        {
            stack_impl_.on_chat_message(msg);
        }
        else
        {
            // ...
        }
    }
    else
    {
        // ...
    }
}

void message_manager::route_envelope(
    const std::string & to, const envelope & env
    )
{
    if (stack_impl_.location_manager_)
    {
        auto locations = stack_impl_.location_manager_->locations(to);
        
        if (locations.size() > 0)
        {
            for (auto & i : locations)
            {
                auto address = i.address();
                auto port = i.port();
                
                log_debug(
                    "Message manager is routing message to " <<
                    to << " (" << address << ":" << port << ")."
                );

                /**
                 * Route the message.
                 */
                stack_impl_.route(
                    address.to_string().c_str(), port, to, env.json()
                );
            }
        }
        else
        {
            log_error(
                "Message manager failed to route envelope for " <<
                 to << ", no locations found."
            );
        }
    }
    else
    {
        assert(0);
    }
}
