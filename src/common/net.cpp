#include "common/net.h"

#include "common/logger.h"

#ifdef OS_WIN
#include <windows.h>
#include <iphlpapi.h>
#elif defined OS_POSIX
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <net/if.h>
#endif

#ifdef OS_WIN
namespace common
{
    namespace net
    {
        namespace details
        {
            bool get_mac_address_host_impl(IPAddr destination, IPAddr source, std::string &out_mac_address)
            {
                ULONG MacAddr[2]; /* for 6-byte hardware addresses */
                ULONG PhysAddrLen = 6; /* default to length of six bytes */

                DWORD dwRetVal = SendARP(destination, source, reinterpret_cast<PULONG>(&MacAddr), &PhysAddrLen);

                if (dwRetVal == NO_ERROR) {
                    BYTE *bPhysAddr = (BYTE *)&MacAddr;
                    for (int i = 0; i < PhysAddrLen; i++) {
                        static const uint8_t size_buff = 4;
                        char tmp[size_buff] = {0};
                        if (i == (PhysAddrLen - 1))
                            strings::SafeSNPrintf(tmp, size_buff, "%.2X", bPhysAddr[i]);
                        else
                            strings::SafeSNPrintf(tmp, size_buff, "%.2X-", bPhysAddr[i]);

                        out_mac_address += tmp;
                    }
                }
                else if(dwRetVal == ERROR_BAD_NET_NAME){

                }
                else if(dwRetVal == ERROR_BUFFER_OVERFLOW){

                }
                else if(dwRetVal == ERROR_GEN_FAILURE){

                }
                else if(dwRetVal == ERROR_INVALID_PARAMETER){

                }
                else if(dwRetVal == ERROR_INVALID_USER_BUFFER){

                }
                else if(dwRetVal == ERROR_NOT_FOUND){

                }
                else if(dwRetVal == ERROR_NOT_SUPPORTED){

                }

                return true;
            }
        }

        bool getRemoteMacAddress(const std::string &host, std::string &out_mac_address)
        {
            hostent * record = gethostbyname(host.c_str());
            if(record == NULL){
                return false;
            }

            in_addr * address = (in_addr * )record->h_addr;
            const char* ip_address = inet_ntoa(* address);
            IPAddr DestIp = inet_addr(ip_address);
            return details::get_mac_address_host_impl(DestIp, INADDR_ANY, out_mac_address);
        }
    }
}
#elif defined(OS_POSIX)
namespace common
{
    namespace net
    {
        bool getRemoteMacAddress(const string16& host, string16& out_mac_address)
        {
            // Socket to send ARP packet
            int udp_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);//socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ARP));

            if( udp_sock == ERROR_RESULT_VALUE){
                DEBUG_MSG_PERROR("socket");
                return false;
            }

            struct sockaddr_in udp_sin;
            bzero (&udp_sin, sizeof(udp_sin));
            udp_sin.sin_family = AF_INET;
            udp_sin.sin_addr.s_addr = inet_addr(host.c_str());
            udp_sin.sin_port = htons(5232);

            int enabled = 1;
            int i = setsockopt(udp_sock, SOL_SOCKET, SO_BROADCAST, &enabled, sizeof(enabled));
            if(i == ERROR_RESULT_VALUE){
                DEBUG_MSG_PERROR("setsockopt");
                return false;
            }

            
            i = sendto(udp_sock, "TEST", 5, 0, (struct sockaddr *)&udp_sin, sizeof(udp_sin));
            if(i == ERROR_RESULT_VALUE){
                DEBUG_MSG_PERROR("sendto");
                return false;
            }

            socklen_t slen = sizeof(udp_sin);
            char16 buff[256] = {0};
            recvfrom(udp_sock, buff, 256, 0, (struct sockaddr *)&udp_sin, &slen);

            return true;
        }
    }
}
#endif

namespace common
{
    std::string convertToString(const net::hostAndPort& host)
    {
        static const uint16_t size_buff = 512;
        char buff[size_buff] = {0};
        strings::SafeSNPrintf(buff, size_buff, "%s:%u", host.first.c_str(), host.second);
        return buff;
    }

    template<>
    net::hostAndPort convertFromString(const std::string& host)
    {
        net::hostAndPort res;
        size_t del = host.find_first_of(':');
        if(del != std::string::npos){
            res.first = host.substr(0, del);
            res.second = convertFromString<uint16_t>(host.substr(del + 1));
        }

        return res;
    }
}
