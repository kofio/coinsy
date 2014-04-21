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

#import "NSString+Additions.hpp"

@implementation NSString (NSString_Additions)

#if TARGET_RT_BIG_ENDIAN
#define WCHAR_T_ENCODING NSUTF32BigEndianStringEncoding
#else
#define WCHAR_T_ENCODING NSUTF32LittleEndianStringEncoding
#endif

+ (NSString *)stringWithStdWString:(const std::wstring &)string
{
    int length = wcslen(string.c_str());
    char * mbs = (char *)malloc(sizeof(char)*(length+1));
    wcstombs(mbs, string.c_str(), length+1);
    NSString * ret = [[NSString alloc] initWithCString:mbs
        encoding:NSUTF8StringEncoding
    ];
    free(mbs);
    return ret;
}

+ (NSString *)stringWithStdString:(const std::string &)string;
{
    return [NSString stringWithUTF8String:string.c_str()];
}

- (std::wstring)stdWString
{
    NSData * asData = [self dataUsingEncoding:WCHAR_T_ENCODING];
    
    return std::wstring(
        (wchar_t *)[asData bytes], [asData length] / sizeof(wchar_t)
    );
}

- (std::string)stdString
{
    return [self UTF8String];
}

- (NSString *)substituteEmoticons
{ 
    //See http://www.easyapns.com/iphone-emoji-alerts for a list of emoticons available
 
    NSString *res = [self stringByReplacingOccurrencesOfString:@":)" withString:@"\ue415"];
    res = [res stringByReplacingOccurrencesOfString:@":(" withString:@"\ue403"];
    res = [res stringByReplacingOccurrencesOfString:@";-)" withString:@"\ue405"];
    res = [res stringByReplacingOccurrencesOfString:@":-x" withString:@"\ue418"];
 
    return res;
 
}

@end
