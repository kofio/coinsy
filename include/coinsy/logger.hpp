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
 
#ifndef COINSY_LOGGER_HPP
#define COINSY_LOGGER_HPP

#if (defined __ANDROID__)
#include <android/log.h>
#endif
  
#if (defined _WIN32 || defined WIN32) || (defined _WIN64 || defined WIN64)
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif // (defined _WIN32 || defined WIN32) || (defined _WIN64 || defined WIN64)

#include <iostream>
#include <memory>
#include <sstream>

#include <mutex>

namespace coinsy {

    /**
     * Implements a logger.
     */
    class logger
    {
        public:
            
			/**
			 * @param severity_none
			 * @param severity_debug
			 * @param severity_error
			 * @param severity_info
			 * @param severity_warning
			 */
			typedef enum severity
			{
				severity_none,
				severity_debug,
				severity_error,
				severity_info,
				severity_warning,
			} severity_t;
			
            /**
             * Singleton accessor.
             */
			static logger & instance()
			{
			    static logger g_logger;

			    return g_logger;
			}
            
            /**
             * operator <<
             */
            template <class T>
            logger & operator << (T const & val)
            {
                std::stringstream ss;
                
                ss << val;
                
                log(ss);
                
                ss.str(std::string());
                
                return logger::instance();
            }

            /**
             * Perform the actual logging.
             * @param val
             */
			void log(std::stringstream & val)
			{
				std::lock_guard<std::recursive_mutex> lock(mutex_);
				
			    static const bool use_file = false;

			    if (use_file)
			    {
                    // ...
			    }

			    static bool use_cout = true;

			    if (use_cout)
			    {
#if (defined _WIN32 || defined WIN32) || (defined _WIN64 || defined WIN64)
#if defined(_UNICODE)
			        DWORD len = MultiByteToWideChar(
			            CP_ACP, 0, val.str().c_str(), -1, NULL, 0
			        );

			        std::unique_ptr<wchar_t> buf(new wchar_t[len]);

			        MultiByteToWideChar(
			            CP_ACP, 0, val.str().c_str(), -1, buf.get(), len
			        );

			        OutputDebugString(buf.get());
			        OutputDebugString(L"\n");

			        std::cerr << val.str() << std::endl;
#else
			        OutputDebugString(val.str().c_str());
			        OutputDebugString(L"\n");

			        std::cerr << val.str() << std::endl;
#endif // _UNICODE
#else // Not Windows.
#if (defined __ANDROID__)
					__android_log_print(ANDROID_LOG_DEBUG, "logger", val.str().c_str());
#else
			        std::cerr << val.str() << std::endl;
#endif
#endif // defined _WIN32 || defined WIN32) || (defined _WIN64 || defined WIN64
			    }
			}
            
        private:
        
			// ...
            
        protected:
        
			/**
			 * The mutex.
			 */
			std::recursive_mutex mutex_;
    };
    
    #define log_xx(severity, strm) \
    { \
        std::stringstream __ss; \
        switch (severity) \
        { \
            case coinsy::logger::severity_debug: \
                __ss << "[DEBUG] - "; \
            break; \
            case coinsy::logger::severity_error: \
                __ss << "[ERROR] - "; \
            break; \
            case coinsy::logger::severity_info: \
                __ss << "[INFO] - "; \
            break; \
            case coinsy::logger::severity_warning: \
                __ss << "[WARNING] - "; \
            break; \
            default: \
                __ss << "[UNKNOWN] - "; \
        } \
		__ss << __FUNCTION__ << ": "; \
        __ss << strm; \
        coinsy::logger::instance() << __ss.str(); \
        __ss.str(std::string()); \
    } \
	
#define log_none(strm) /** */
#if (defined NDEBUG)
#define log_debug(strm) log_none(strm)
#else
#define log_debug(strm) log_xx(coinsy::logger::severity_debug, strm)
#endif
#define log_error(strm) log_xx(coinsy::logger::severity_error, strm)
#define log_info(strm) log_xx(coinsy::logger::severity_info, strm)
#define log_warn(strm) log_xx(coinsy::logger::severity_warning, strm)

} // namespace coinsy

#endif // COINSY_LOGGER_HPP

        