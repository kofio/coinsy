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

#ifndef COINSY_MESSAGE_HPP
#define COINSY_MESSAGE_HPP

#include <string>

#include <boost/property_tree/json_parser.hpp>

namespace coinsy {

    /**
     * Implements a message.
     * {
     *     "type":"chat",
     *     "command":"message",
     *     "id":"xyz",
     *     "timestamp":"20140117T07:34:25",
     *     "to":"a",
     *     "from":"b",
     * }
     */
    class message
    {
        public:
        
            /**
             * Constructor
             */
            message();
        
            /**
             * Constructor
             * @param json The JSON.
             */
            message(const std::string &);
        
            /**
             * Constructor
             * @param type The type.
             * @param command The command.
             * @param to The to.
             * @param from The from.
             */
            message(
                const std::string &, const std::string &,
                const std::string &, const std::string &
            );
        
            /**
             * Encodes
             */
            void encode();
        
            /**
             * Decodes
             */
            void decode();
        
            /**
             * The JSON.
             */
            const std::string & json() const;
        
            /**
             * The type.
             */
            const std::string & type() const;
            
            /**
             * The command.
             */
            const std::string & command() const;

            /**
             * The id.
             */
            const std::string & id() const;
            
            /**
             * The timestamp.
             */
            const std::string & timestamp() const;
            
            /**
             * The to.
             */
            const std::string & to() const;
            
            /**
             * The from.
             */
            const std::string & from() const;
        
            /**
             * Set the message attribute.
             * @param val The value.
             */
            void set_message_attribute(const std::string &);
        
            /**
             * Set the message attribute.
             * @param val The value.
             */
            void set_message_attribute(const std::wstring &);
        
            /**
             * The message attribute.
             */
            const std::wstring & message_attribute() const;
        
        private:
        
            /**
             * Generates a timestamp.
             */
            std::string generate_timestamp();
        
            /**
             * The JSON.
             */
            std::string m_json;
        
            /**
             * The type.
             */
            std::string m_type;
            
            /**
             * The command.
             */
            std::string m_command;

            /**
             * The id.
             */
            std::string m_id;
            
            /**
             * The timestamp.
             */
            std::string m_timestamp;
            
            /**
             * The to.
             */
            std::string m_to;
            
            /**
             * The from.
             */
            std::string m_from;
        
            /**
             * The message attribute.
             */
            std::wstring m_message_attribute;
        
        protected:
        
            /**
             * The boost::property_tree::wptree.
             */
            boost::property_tree::wptree wptree_;
    };
    
} // namespace coinsy

#endif // COINSY_MESSAGE_HPP
