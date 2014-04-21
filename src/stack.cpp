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

#include <coinsy/stack.hpp>
#include <coinsy/stack_impl.hpp>

using namespace coinsy;

stack::stack()
    : stack_impl_(0)
{
    // ...
}

void stack::start(const std::uint16_t & port)
{
    if (stack_impl_)
    {
        throw std::runtime_error("Stack is already allocated");
    }
    else
    {
        /**
         * Allocate the stack implementation.
         */
        stack_impl_ = new stack_impl(*this);
        
        /**
         * Start the stack implementation.
         */
        stack_impl_->start(port);
    }
}

void stack::stop()
{
    if (stack_impl_)
    {
        /**
         * Stop the stack implementation.
         */
        stack_impl_->stop();
        
        /**
         * Deallocate the stack implementation.
         */
        delete stack_impl_, stack_impl_ = 0;
    }
    else
    {
        throw std::runtime_error("Stack is not allocated");
    }
}

void stack::sign_in(
    const std::string & username, const std::string & password
    )
{
    if (stack_impl_)
    {
        stack_impl_->sign_in(username, password);
    }
    else
    {
        throw std::runtime_error("Stack is not allocated");
    }
}

void stack::sign_out()
{
    if (stack_impl_)
    {
        stack_impl_->sign_out();
    }
    else
    {
        throw std::runtime_error("Stack is not allocated");
    }
}

const std::string & stack::username() const
{
    if (stack_impl_)
    {
        return stack_impl_->username();
    }
    
    static std::string ret;
    
    return ret;
}

const std::map<std::string, std::string> & stack::profile() const
{
    if (stack_impl_)
    {
        return stack_impl_->profile();
    }
    
    static std::map<std::string, std::string> ret;
    
    return ret;
}

std::uint16_t stack::store(const std::string & query)
{
    if (stack_impl_)
    {
        return stack_impl_->store(query);
    }
    
    return 0;
}

std::uint16_t stack::find(
    const std::string & query, const std::size_t & max_results
    )
{
    if (stack_impl_)
    {
        return stack_impl_->find(query, max_results);
    }
    
    return 0;
}

std::string stack::message(
    const std::string & username, const std::wstring & message_attribute
    )
{
    if (stack_impl_)
    {
        return stack_impl_->send_message(username, message_attribute);
    }
    
    return std::string();
}

void stack::url_get(
    const std::string & url,
    const std::function<void (const std::map<std::string, std::string> &,
    const std::string &)> & f
    )
{
    if (stack_impl_)
    {
        stack_impl_->url_get(url, f);
    }
}

void stack::url_post(
    const std::string & url,
    const std::map<std::string, std::string> & headers,
    const std::string & body,
    const std::function<void (const std::map<std::string, std::string> &,
    const std::string &)> & f
    )
{
    if (stack_impl_)
    {
        stack_impl_->url_post(url, headers, body, f);
    }
}

std::uint16_t stack::update_profile(
    const std::map<std::string, std::string> & profile
    )
{
    if (stack_impl_)
    {
        return stack_impl_->update_profile(profile);
    }
    
    return 0;
}

bool stack::market_add(const std::string & key)
{
    if (stack_impl_)
    {
        return stack_impl_->market_add(key);
    }
    
    return false;
}

bool stack::market_remove(const std::string & key)
{
    if (stack_impl_)
    {
        return stack_impl_->market_remove(key);
    }
    
    return false;
}

std::uint32_t stack::market_ask(
    const std::string & key, const float & price, const float & quantity
    )
{
    if (stack_impl_)
    {
        return stack_impl_->market_ask(key, price, quantity);
    }
    
    return 0;
}

std::uint32_t stack::market_bid(
    const std::string & key, const float & price, const float & quantity
    )
{
    if (stack_impl_)
    {
        return stack_impl_->market_bid(key, price, quantity);
    }
    
    return 0;
}

bool stack::market_cancel(const std::string & key, const std::uint32_t & id)
{
    if (stack_impl_)
    {
        return stack_impl_->market_cancel(key, id);
    }
    
    return false;
}

void stack::market_troll(const std::string & key, const std::string & message)
{
    if (stack_impl_)
    {
        stack_impl_->market_troll(key, message);
    }
}
