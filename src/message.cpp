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

#include <clocale>

#include <coinsy/crypto.hpp>
#include <coinsy/logger.hpp>
#include <coinsy/message.hpp>

using namespace coinsy;

static inline std::string to_string(const std::wstring & str)
{
#if (defined __ANDROID__)
    std::string ret(str.length(), ' ');
    std::copy(str.begin(), str.end(), ret.begin());
    return ret;
#else
    const wchar_t * cs = str.c_str();
    const std::size_t wn = std::wcsrtombs(0, &cs, 0, 0);

    if (wn == std::size_t(-1))
    {
        return "";
    }

    std::vector<char> buf(wn + 1);
    
    const std::size_t wn_again = std::wcsrtombs(buf.data(), &cs, wn + 1, 0);

    if (wn_again == std::size_t(-1))
    {
        return "";
    }
    
    return std::string(buf.data(), wn);
#endif // __ANDROID__
}

static inline std::wstring to_wstring(const std::string & str)
{
#if (defined __ANDROID__)
    std::wstring ret(str.length(),L' ');
    std::copy(str.begin(), str.end(), ret.begin());
    return ret;
#else
    const char * cs = str.c_str();
    const std::size_t wn = std::mbsrtowcs(0, &cs, 0, 0);

    if (wn == std::size_t(-1))
    {
        return L"";
    }

    std::vector<wchar_t> buf(wn + 1);
    
    const std::size_t wn_again = std::mbsrtowcs(buf.data(), &cs, wn + 1, 0);

    if (wn_again == std::size_t(-1))
    {
        return L"";
    }

    return std::wstring(buf.data(), wn);
#endif // __ANDROID__
}

message::message()
{
    // ...
}

message::message(const std::string & json)
    : m_json(json)
{
    // ...
}

message::message(
    const std::string & type, const std::string & command,
    const std::string & to, const std::string & from
    )
    : m_type(type)
    , m_command(command)
    , m_id(crypto::random_string(32))
    , m_timestamp(generate_timestamp())
    , m_to(to)
    , m_from(from)
{
    // ...
}

void message::encode()
{
    /**
     * Put the type into property tree.
     */
    wptree_.put(L"type", to_wstring(m_type));
    
    /**
     * Put the command into property tree.
     */
    wptree_.put(L"command", to_wstring(m_command));
    
    /**
     * Put the id into property tree.
     */
    wptree_.put(L"id", to_wstring(m_id));
    
    /**
     * Put the timestamp into property tree.
     */
    wptree_.put(L"timestamp", to_wstring(m_timestamp));
    
    /**
     * Put the to into property tree.
     */
    wptree_.put(L"to", to_wstring(m_to));

    /**
     * Put the from into property tree.
     */
    wptree_.put(L"from", to_wstring(m_from));
    
    /**
     * The message attribute.
     */
    if (!m_message_attribute.empty())
    {
        /**
         * Put the attributes message into property tree.
         */
        wptree_.put(L"attributes.message", m_message_attribute);
    }
    
    /**
     * The std::wstringstream.
     */
    std::wstringstream ss;
    
    /**
     * Write property tree to json file.
     */
    write_json(ss, wptree_, false);
    
    /**
     * Assign the json.
     */
    m_json = to_string(ss.str());
}
        
void message::decode()
{
    /**
     * Allocate the json.
     */
    std::wstringstream json;
    
    /**
     * Read the json into the stream.
     */
    json << to_wstring(m_json);
    
    /**
     * Load the json stream and put it's contents into the property tree. If
     * reading fails an exception is thrown.
     */
    read_json(json, wptree_);
    
    /**
     * Get the type.
     */
    m_type = to_string(wptree_.get<std::wstring> (
        L"type", L"")
    );
    
    /**
     * Get the command.
     */
    m_command = to_string(wptree_.get<std::wstring> (
        L"command", L"")
    );

    /**
     * Get the id.
     */
    m_id = to_string(wptree_.get<std::wstring> (
        L"id", L"")
    );
    
    /**
     * Get the timestamp.
     */
    m_timestamp = to_string(wptree_.get<std::wstring> (
        L"timestamp", L"")
    );
    
    /**
     * Get the to.
     */
    m_to = to_string(wptree_.get<std::wstring> (
        L"to", L"")
    );
    
    /**
     * Get the from.
     */
    m_from = to_string(wptree_.get<std::wstring> (
        L"from", L"")
    );
    
    /**
     * Decode the attributes.
     */
    try
    {
        m_message_attribute = wptree_.get<std::wstring> (
            L"attributes.message", L""
        );
    }
    catch (...)
    {
        // ...
    }
}

const std::string & message::json() const
{
    return m_json;
}

const std::string & message::type() const
{
    return m_type;
}

const std::string & message::command() const
{
    return m_command;
}

const std::string & message::id() const
{
    return m_id;
}

const std::string & message::timestamp() const
{
    return m_timestamp;
}

const std::string & message::to() const
{
    return m_to;
}

const std::string & message::from() const
{
    return m_from;
}

void message::set_message_attribute(const std::string & val)
{
    m_message_attribute = to_wstring(val);
}

void message::set_message_attribute(const std::wstring & val)
{
    m_message_attribute = val;
}

const std::wstring & message::message_attribute() const
{
    return m_message_attribute;
}

std::string message::generate_timestamp()
{
    std::time_t current_time_seconds;
    
    std::time(&current_time_seconds);
    
    struct tm * current_time = std::gmtime(&current_time_seconds);
    
    char output[256];
    
    std::strftime(output, sizeof(output), "%Y%m%dT%H:%M:%S", current_time);
    
    return output;
}
