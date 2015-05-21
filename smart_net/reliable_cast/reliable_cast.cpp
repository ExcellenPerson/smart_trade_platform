//============================================================================
// Name        : ReliableProtocal.cpp
// Author      : TheRockLiuHY
// Date        : May 5, 2015
// Copyright   : liuhongyang's copyright
// Description : SmartNet
//============================================================================

#include "reliable_cast.h"

#include <string.h>
#include <errno.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
//#include <unistd.h>
//#include <fcntl.h>
#include <netdb.h>

namespace
{
	using namespace smart_utils;

	int32_t is_multicast_addr(const struct sockaddr &sa/*network byte order*/)
	{
		int ret = EC_SUC;
		switch (sa.sa_family)
		{
			case AF_INET:
			{
				struct sockaddr_in s4;
				memcpy(&s4, &sa, sizeof(s4));
				ret = (IN_MULTICAST(ntohl(s4.sin_addr.s_addr))) ? EC_SUC : EC_ERR;
				break;
			}
			case AF_INET6:
			{
				struct sockaddr_in6 s6;
				memcpy(&s6, &sa, sizeof(s6));
				ret = (IN6_IS_ADDR_MULTICAST(&s6.sin6_addr)) ? EC_SUC : EC_ERR;
				break;
			}
			default:
			{
				ret = EC_ERR;
				break;
			}
		}

		return ret;
	}

	enum join_or_leave
	{
		JOL_NONE = -1, JOL_JOIN = 0, JOL_LEAVE = 1
	};

	int32_t set_multicast_group(int32_t sock, const int32_t local_bind_ifx_idx, const in_addr_t local_bind_addr/*net byte order*/, struct sockaddr &multicast_ip_addr/*network byte order*/,
			const join_or_leave jol)
	{

		if (-1 == sock || (JOL_JOIN != jol && JOL_LEAVE != jol) || (EC_SUC != is_multicast_addr(multicast_ip_addr)))
		{
			SU_ASSERT(false)
			return EC_ERR;
		}

		int32_t ret = EC_SUC;
		switch (multicast_ip_addr.sa_family)
		{
			case AF_INET:
			{
				ip_mreqn mreq;
				mreq.imr_ifindex = local_bind_ifx_idx;
				mreq.imr_address.s_addr = local_bind_addr;
				memcpy(&mreq.imr_multiaddr, &((*(reinterpret_cast<struct sockaddr_in*>(&multicast_ip_addr))).sin_addr.s_addr), sizeof(mreq.imr_multiaddr));
				if (setsockopt(sock, IPPROTO_IP, JOL_JOIN == jol ? IP_ADD_MEMBERSHIP : IP_DROP_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
				{
					ret = errno;
				}

				break;
			}
			case AF_INET6:
			{
				ipv6_mreq mreq;
				mreq.ipv6mr_interface = local_bind_ifx_idx;
				memcpy(&mreq.ipv6mr_multiaddr, &((*(reinterpret_cast<struct sockaddr_in6*>(&multicast_ip_addr))).sin6_addr), sizeof(mreq.ipv6mr_multiaddr));
				if (setsockopt(sock, IPPROTO_IPV6, JOL_JOIN == jol ? IPV6_JOIN_GROUP : IPV6_LEAVE_GROUP, &mreq, sizeof(mreq)) < 0)
				{
					ret = errno;
				}
				break;
			}
			default:
			{
				ret = EC_ERR;
				SU_ASSERT(false);
				break;
			}
		}

		return ret;
	}

#if 0

/* Join multicast group.
 * NB: IPV6_JOIN_GROUP == IPV6_ADD_MEMBERSHIP
 *
 * If no error occurs, pgm_sockaddr_join_group returns zero.  Otherwise, a
 * value of SOCKET_ERROR is returned, and a specific error code can be
 * retrieved by calling pgm_get_last_sock_error().
 */

PGM_GNUC_INTERNAL
int pgm_sockaddr_join_group(const SOCKET s, const sa_family_t sa_family, const struct group_req* gr)
{
	int retval = SOCKET_ERROR;
#if defined( HAVE_STRUCT_GROUP_REQ ) || ( defined( _WIN32 ) && ( _WIN32_WINNT >= 0x0600 ) )
	/* Solaris:ip(7P) "The following options take a struct ip_mreq_source as the
	 * parameter."  Presumably with source field zeroed out.
	 * Solaris:ip6(7P) "Takes a struct group_req as the parameter."
	 * Different type for each family, however group_req is protocol-independent.
	 *
	 * WinSock2:MSDN(GROUP_REQ Structure) "The GROUP_REQ structure is used with the
	 * MCAST_JOIN_GROUP and MCAST_LEAVE_GROUP socket options."
	 * Minimum supported client: none supported.
	 * Minimum supported server: Windows Server 2008.
	 *
	 * Stevens: "MCAST_JOIN_GROUP has datatype group_req{}."
	 *
	 * RFC3678: Argument type struct group_req
	 *
	 * Note that not all platforms with MCAST_JOIN_GROUP defined actually support the
	 * socket option, such that testing is deferred to Autoconf.
	 */
	const int recv_level = (AF_INET == sa_family) ? SOL_IP : SOL_IPV6;
	retval = setsockopt (s, recv_level, MCAST_JOIN_GROUP, (const char*)gr, sizeof(struct group_req));
#else
	switch (sa_family)
	{
		case AF_INET:
		{
			/* Solaris:ip(7P) Just mentions "Join a multicast group."
			 * No further details provided.
			 *
			 * Linux:ip(7) "Argument is an ip_mreqn structure.  For compatibility, the old
			 * ip_mreq structure (present since Linux 1.2) is still supported."
			 *
			 * FreeBSD,OS X:IP(4) provided by example "struct ip_mreq mreq;"
			 *
			 * WinSock2:MSDN(IPPROTO_IP Socket Options) "ip_mreq"
			 * Also, can set ip_mreq.imr_interface to be 0.0.0.<imr_ifindex>
			 *
			 * Stevens: "IP_ADD_MEMBERSHIP has datatype ip_mreq{}."
			 *
			 * RFC3678: Argument type struct ip_mreq
			 */
#	ifdef HAVE_STRUCT_IP_MREQN
			struct ip_mreqn mreqn;
			struct sockaddr_in ifaddr;
			memset (&mreqn, 0, sizeof(mreqn));
			mreqn.imr_multiaddr.s_addr = ((const struct sockaddr_in*)&gr->gr_group)->sin_addr.s_addr;
			if (!pgm_if_indextoaddr (gr->gr_interface, AF_INET, 0, (struct sockaddr*)&ifaddr, NULL))
			return -1;
			mreqn.imr_address.s_addr = ifaddr.sin_addr.s_addr;
			mreqn.imr_ifindex = gr->gr_interface;
			retval = setsockopt (s, SOL_IP, IP_ADD_MEMBERSHIP, (const char*)&mreqn, sizeof(mreqn));
#	else
			struct ip_mreq mreq;
			struct sockaddr_in ifaddr;
			memset(&mreq, 0, sizeof(mreq));
			mreq.imr_multiaddr.s_addr = ((const struct sockaddr_in*) &gr->gr_group)->sin_addr.s_addr;
			if (!pgm_if_indextoaddr(gr->gr_interface, AF_INET, 0, (struct sockaddr*) &ifaddr, NULL))
			return -1;
			mreq.imr_interface.s_addr = ifaddr.sin_addr.s_addr;
			retval = setsockopt(s, SOL_IP, IP_ADD_MEMBERSHIP, (const char*) &mreq, sizeof(mreq));
#	endif /* !HAVE_STRUCT_IP_MREQN */
			break;
		}

		case AF_INET6:
		{
			/* Solaris:ip6(7P) "Takes a struct ipv6_mreq as the parameter;"
			 *
			 * Linux:ipv6(7) "Argument is a pointer to a struct ipv6_mreq structure."
			 *
			 * OS X:IP6(4) "IPV6_JOIN_GROUP struct ipv6_mreq *"
			 *
			 * WinSock2:MSDN(IPPROTO_IP Socket Options) "ipv6_mreq"
			 *
			 * Stevens: "IPV6_JOIN_GROUP has datatype ipv6_mreq{}."
			 */
			struct ipv6_mreq mreq6;
			memset(&mreq6, 0, sizeof(mreq6));
			mreq6.ipv6mr_multiaddr = ((const struct sockaddr_in6*) &gr->gr_group)->sin6_addr;
			mreq6.ipv6mr_interface = gr->gr_interface;
			retval = setsockopt(s, SOL_IPV6, IPV6_ADD_MEMBERSHIP, (const char*) &mreq6, sizeof(mreq6));
			break;
		}

		default:
		break;
	}
#endif /* HAVE_STRUCT_GROUP_REQ */
	return retval;
}

/* leave a joined group
 */

PGM_GNUC_INTERNAL
int pgm_sockaddr_leave_group(const SOCKET s, const sa_family_t sa_family, const struct group_req* gr)
{
	int retval = SOCKET_ERROR;
#if defined( HAVE_STRUCT_GROUP_REQ ) || ( defined( _WIN32 ) && ( _WIN32_WINNT >= 0x0600 ) )
	const int recv_level = (AF_INET == sa_family) ? SOL_IP : SOL_IPV6;
	retval = setsockopt (s, recv_level, MCAST_LEAVE_GROUP, (const char*)gr, sizeof(struct group_req));
#else
	switch (sa_family)
	{
		case AF_INET:
		{
#	ifdef HAVE_STRUCT_IP_MREQN
			struct ip_mreqn mreqn;
			struct sockaddr_in ifaddr;
			memset (&mreqn, 0, sizeof(mreqn));
			mreqn.imr_multiaddr.s_addr = ((const struct sockaddr_in*)&gr->gr_group)->sin_addr.s_addr;
			if (!pgm_if_indextoaddr (gr->gr_interface, AF_INET, 0, (struct sockaddr*)&ifaddr, NULL))
			return -1;
			mreqn.imr_address.s_addr = ifaddr.sin_addr.s_addr;
			mreqn.imr_ifindex = gr->gr_interface;
			retval = setsockopt (s, SOL_IP, IP_DROP_MEMBERSHIP, (const char*)&mreqn, sizeof(mreqn));
#	else
			struct ip_mreq mreq;
			struct sockaddr_in ifaddr;
			memset(&mreq, 0, sizeof(mreq));
			mreq.imr_multiaddr.s_addr = ((const struct sockaddr_in*) &gr->gr_group)->sin_addr.s_addr;
			if (!pgm_if_indextoaddr(gr->gr_interface, AF_INET, 0, (struct sockaddr*) &ifaddr, NULL))
			return -1;
			mreq.imr_interface.s_addr = ifaddr.sin_addr.s_addr;
			retval = setsockopt(s, SOL_IP, IP_DROP_MEMBERSHIP, (const char*) &mreq, sizeof(mreq));
#	endif /* !HAVE_STRUCT_IP_MREQN */
			break;
		}

		case AF_INET6:
		{
			struct ipv6_mreq mreq6;
			memset(&mreq6, 0, sizeof(mreq6));
			mreq6.ipv6mr_multiaddr = ((const struct sockaddr_in6*) &gr->gr_group)->sin6_addr;
			mreq6.ipv6mr_interface = gr->gr_interface;
			retval = setsockopt(s, SOL_IPV6, IPV6_DROP_MEMBERSHIP, (const char*) &mreq6, sizeof(mreq6));
			break;
		}

		default:
		break;
	}
#endif /* HAVE_STRUCT_GROUP_REQ */
	return retval;
}

/* block either at the NIC or kernel, packets from a particular source
 */

PGM_GNUC_INTERNAL
int pgm_sockaddr_block_source(const SOCKET s, const sa_family_t sa_family, const struct group_source_req* gsr)
{
	int retval = SOCKET_ERROR;
#if defined( HAVE_STRUCT_GROUP_REQ ) || ( defined( _WIN32 ) && ( _WIN32_WINNT >= 0x0600 ) )
	const int recv_level = (AF_INET == sa_family) ? SOL_IP : SOL_IPV6;
	retval = setsockopt (s, recv_level, MCAST_BLOCK_SOURCE, (const char*)gsr, sizeof(struct group_source_req));
#elif defined( IP_BLOCK_SOURCE )
	switch (sa_family)
	{
		case AF_INET:
		{
			struct ip_mreq_source mreqs;
			struct sockaddr_in ifaddr;
			memset(&mreqs, 0, sizeof(mreqs));
			mreqs.imr_multiaddr.s_addr = ((const struct sockaddr_in*) &gsr->gsr_group)->sin_addr.s_addr;
			mreqs.imr_sourceaddr.s_addr = ((const struct sockaddr_in*) &gsr->gsr_source)->sin_addr.s_addr;
			pgm_if_indextoaddr(gsr->gsr_interface, AF_INET, 0, (struct sockaddr*) &ifaddr, NULL);
			mreqs.imr_interface.s_addr = ifaddr.sin_addr.s_addr;
			retval = setsockopt(s, SOL_IP, IP_BLOCK_SOURCE, (const char*) &mreqs, sizeof(mreqs));
			break;
		}

		case AF_INET6:
		/* No IPv6 API implemented, MCAST_BLOCK_SOURCE should be available instead.
		 */
		break;

		default:
		break;
	}
#else
	/* unused parameters, operation not supported on this platform. */
	(void)s;
	(void)sa_family;
	(void)gsr;
#endif /* HAVE_STRUCT_GROUP_REQ */
	return retval;
}

/* unblock a blocked multicast source.
 */

PGM_GNUC_INTERNAL
int pgm_sockaddr_unblock_source(const SOCKET s, const sa_family_t sa_family, const struct group_source_req* gsr)
{
	int retval = SOCKET_ERROR;
#if defined( HAVE_STRUCT_GROUP_REQ ) || ( defined( _WIN32 ) && ( _WIN32_WINNT >= 0x0600 ) )
	const int recv_level = (AF_INET == sa_family) ? SOL_IP : SOL_IPV6;
	retval = setsockopt (s, recv_level, MCAST_UNBLOCK_SOURCE, (const char*)gsr, sizeof(struct group_source_req));
#elif defined( IP_UNBLOCK_SOURCE )
	switch (sa_family)
	{
		case AF_INET:
		{
			struct ip_mreq_source mreqs;
			struct sockaddr_in ifaddr;
			memset(&mreqs, 0, sizeof(mreqs));
			mreqs.imr_multiaddr.s_addr = ((const struct sockaddr_in*) &gsr->gsr_group)->sin_addr.s_addr;
			mreqs.imr_sourceaddr.s_addr = ((const struct sockaddr_in*) &gsr->gsr_source)->sin_addr.s_addr;
			pgm_if_indextoaddr(gsr->gsr_interface, AF_INET, 0, (struct sockaddr*) &ifaddr, NULL);
			mreqs.imr_interface.s_addr = ifaddr.sin_addr.s_addr;
			retval = setsockopt(s, SOL_IP, IP_UNBLOCK_SOURCE, (const char*) &mreqs, sizeof(mreqs));
			break;
		}

		case AF_INET6:
		/* No IPv6 API implemented, MCAST_UNBLOCK_SOURCE should be available instead.
		 */
		break;

		default:
		break;
	}
#else
	/* unused parameters */
	(void)s;
	(void)sa_family;
	(void)gsr;
#endif /* HAVE_STRUCT_GROUP_REQ */
	return retval;
}

/* Join source-specific multicast.
 * NB: Silently reverts to ASM if SSM not supported.
 *
 * If no error occurs, pgm_sockaddr_join_source_group returns zero.
 * Otherwise, a value of SOCKET_ERROR is returned, and a specific error
 * code can be retrieved by calling pgm_get_last_sock_error().
 */

PGM_GNUC_INTERNAL
int pgm_sockaddr_join_source_group(const SOCKET s, const sa_family_t sa_family, const struct group_source_req* gsr)
{
	int retval = SOCKET_ERROR;
#if defined( HAVE_STRUCT_GROUP_REQ ) || ( defined( _WIN32 ) && ( _WIN32_WINNT >= 0x0600 ) )
	/* Solaris:ip(7P) "The following options take a struct ip_mreq_source as the
	 * parameter."
	 * Solaris:ip6(7P) "Takes a struct group_source_req as the parameter."
	 * Different type for each family, however group_source_req is protocol-
	 * independent.
	 *
	 * WinSock2:MSDN(GROUP_SOURCE_REQ Structure) "The GROUP_SOURCE_REQ structure is
	 * used with the MCAST_BLOCK_SOURCE, MCAST_JOIN_SOURCE_GROUP,
	 * MCAST_LEAVE_SOURCE_GROUP, and MCAST_UNBLOCK_SOURCE socket options."
	 *
	 * Stevens: "MCAST_JOIN_SOURCE_GROUP has datatype group_source_req{}."
	 *
	 * RFC3678: Argument type struct group_source_req
	 */
	const int recv_level = (AF_INET == sa_family) ? SOL_IP : SOL_IPV6;
	retval = setsockopt (s, recv_level, MCAST_JOIN_SOURCE_GROUP, (const char*)gsr, sizeof(struct group_source_req));
#elif defined( IP_ADD_SOURCE_MEMBERSHIP )
	switch (sa_family)
	{
		case AF_INET:
		{
			/* Solaris:ip(7P) "The following options take a struct ip_mreq as the
			 * parameter."  Incorrect literature wrt RFC.
			 *
			 * Linux:ip(7) absent.
			 *
			 * OS X:IP(4) absent.
			 *
			 * WinSock2:MSDN(IPPROTO_IP Socket Options) "ip_mreq_source"
			 *
			 * Stevens: "IP_ADD_SOURCE_MEMBERSHIP has datatype ip_mreq_source{}."
			 *
			 * RFC3678: Argument type struct ip_mreq_source
			 */
			struct ip_mreq_source mreqs;
			struct sockaddr_in ifaddr;
			memset(&mreqs, 0, sizeof(mreqs));
			mreqs.imr_multiaddr.s_addr = ((const struct sockaddr_in*) &gsr->gsr_group)->sin_addr.s_addr;
			mreqs.imr_sourceaddr.s_addr = ((const struct sockaddr_in*) &gsr->gsr_source)->sin_addr.s_addr;
			pgm_if_indextoaddr(gsr->gsr_interface, AF_INET, 0, (struct sockaddr*) &ifaddr, NULL);
			mreqs.imr_interface.s_addr = ifaddr.sin_addr.s_addr;
			retval = setsockopt(s, SOL_IP, IP_ADD_SOURCE_MEMBERSHIP, (const char*) &mreqs, sizeof(mreqs));
			break;
		}

		case AF_INET6:
		/* No IPv6 API implemented, MCAST_JOIN_SOURCE_GROUP should be available instead.
		 */
		retval = pgm_sockaddr_join_group(s, sa_family, (const struct group_req*) gsr);
		break;

		default:
		break;
	}
#else
	retval = pgm_sockaddr_join_group (s, sa_family, (const struct group_req*)gsr);
#endif /* HAVE_STRUCT_GROUP_REQ */
	return retval;
}

/* drop a SSM source
 */

PGM_GNUC_INTERNAL
int pgm_sockaddr_leave_source_group(const SOCKET s, const sa_family_t sa_family, const struct group_source_req* gsr)
{
	int retval = SOCKET_ERROR;
#if defined( HAVE_STRUCT_GROUP_REQ ) || ( defined( _WIN32 ) && ( _WIN32_WINNT >= 0x0600 ) )
	const int recv_level = (AF_INET == sa_family) ? SOL_IP : SOL_IPV6;
	retval = setsockopt (s, recv_level, MCAST_LEAVE_SOURCE_GROUP, (const char*)gsr, sizeof(struct group_source_req));
#elif defined( IP_ADD_SOURCE_MEMBERSHIP )
	switch (sa_family)
	{
		case AF_INET:
		{
			struct ip_mreq_source mreqs;
			struct sockaddr_in ifaddr;
			memset(&mreqs, 0, sizeof(mreqs));
			mreqs.imr_multiaddr.s_addr = ((const struct sockaddr_in*) &gsr->gsr_group)->sin_addr.s_addr;
			mreqs.imr_sourceaddr.s_addr = ((const struct sockaddr_in*) &gsr->gsr_source)->sin_addr.s_addr;
			pgm_if_indextoaddr(gsr->gsr_interface, AF_INET, 0, (struct sockaddr*) &ifaddr, NULL);
			mreqs.imr_interface.s_addr = ifaddr.sin_addr.s_addr;
			retval = setsockopt(s, SOL_IP, IP_DROP_SOURCE_MEMBERSHIP, (const char*) &mreqs, sizeof(mreqs));
			break;
		}

		case AF_INET6:
		/* No IPv6 API implemented, MCAST_LEAVE_SOURCE_GROUP should be available instead.
		 */
		retval = pgm_sockaddr_leave_group(s, sa_family, (const struct group_req*) gsr);
		break;

		default:
		break;
	}
#else
	retval = pgm_sockaddr_leave_group (s, sa_family, (const struct group_req*)gsr);
#endif /* HAVE_STRUCT_GROUP_REQ */
	return retval;
}

/* Batch block and unblock sources.
 */

PGM_GNUC_INTERNAL
int pgm_sockaddr_msfilter(const SOCKET s, const sa_family_t sa_family, const struct group_filter* gf_list)
{
	int retval = SOCKET_ERROR;
#if defined( MCAST_MSFILTER )
	/* Linux 2.6 API pre-empting RFC3678 naming scheme */
	const int recv_level = (AF_INET == sa_family) ? SOL_IP : SOL_IPV6;
	const socklen_t len = GROUP_FILTER_SIZE(gf_list->gf_numsrc);
	retval = setsockopt(s, recv_level, MCAST_MSFILTER, (const char*) gf_list, len);
#elif defined( _WIN32 ) && ( _WIN32_WINNT >= 0x600 )
	/* Windows Server 2008+, note MSDN(GROUP_FILTER Structure) does not list
	 * desktop support.  This contrasts to MSDN(Final-State-Based Multicast Programming)
	 * which does list support for Vista+.
	 *
	 * RFC3678 API for struct group_filter.
	 */
	const socklen_t len = GROUP_FILTER_SIZE(gf_list->gf_numsrc);
	u_long* filter = pgm_alloca (len);
	memcpy (filter, gf_list, len);
	retval = ioctlsocket (s, SIOCSMSFILTER, filter);
#elif defined( HAVE_STRUCT_IP_MSFILTER )
	/* IPv4-only filter API alternative */
	if (AF_INET == sa_family)
	{
		const socklen_t len = IP_MSFILTER_SIZE(gf_list->gf_numsrc);
		struct ip_msfilter* filter = pgm_alloca (len);
		struct sockaddr_in sa4;
		unsigned i;
		memcpy (&sa4, &gf_list->gf_group, sizeof (sa4));
		filter->imsf_multiaddr.s_addr = sa4.sin_addr.s_addr;
		pgm_if_indextoaddr (gf_list->gf_interface, AF_INET, 0, (struct sockaddr*)&sa4, NULL);
		filter->imsf_interface.s_addr = sa4.sin_addr.s_addr;
		filter->imsf_fmode = gf_list->gf_fmode;
		filter->imsf_numsrc = gf_list->gf_numsrc;
		for (i = 0; i < gf_list->gf_numsrc; i++)
		{
			memcpy (&sa4, &gf_list->gf_slist[i], sizeof (sa4));
			filter->imsf_slist[i].s_addr = sa4.sin_addr.s_addr;
		}
#	if defined( SIO_SET_MULTICAST_FILTER )
		/* Windows XP */
		retval = ioctlsocket (s, SIO_SET_MULTICAST_FILTER, (u_long*)filter);
#	elif defined( SIOCSIPMSFILTER )
		/* RFC3678 API for struct ip_msfilter */
		retval = ioctlsocket (s, SIOCSIPMSFILTER, (const char*)filter);
#	elif defined( IP_MSFILTER )
		/* NB: Windows SDK for Vista+ defines a typedef IP_MSFILTER */
		retval = ioctlsocket (s, IP_MSFILTER, (const char*)filter);
#	else
		/* Cygwin has no socket option defined */
#	endif
	}
#endif
	return retval;
}

/* Specify outgoing interface.
 *
 * If no error occurs, pgm_sockaddr_multicast_if returns zero.  Otherwise, a
 * value of SOCKET_ERROR is returned, and a specific error code can be
 * retrieved by calling pgm_get_last_sock_error().
 */

PGM_GNUC_INTERNAL
int pgm_sockaddr_multicast_if(const SOCKET s, const struct sockaddr* address, const unsigned ifindex)
{
	int retval = SOCKET_ERROR;

	switch (address->sa_family)
	{
		case AF_INET:
		{
			/* Solaris:ip(7P) "This option takes a struct in_addr as an argument, and it
			 * selects that interface for outgoing IP multicast packets."
			 *
			 * Linux:ip(7) "Argument is an ip_mreqn or ip_mreq structure similar to
			 * IP_ADD_MEMBERSHIP."
			 *
			 * OS X:IP(4) provided by example "struct in_addr addr;"
			 *
			 * WinSock2:MSDN(IPPROTO_IP Socket Options) "DWORD; Any IP address in the
			 * 0.x.x.x block (first octet of 0) except IPv4 address 0.0.0.0 is treated
			 * as an interface index."
			 * NB: 24-bit interface index size cf. 8-bit of ip_mreq.
			 *
			 * Stevens: "IP_MULTICAST_IF has datatype struct in_addr{}."
			 */
			struct sockaddr_in s4;
			memcpy(&s4, address, sizeof(struct sockaddr_in));
			retval = setsockopt(s, IPPROTO_IP, IP_MULTICAST_IF, (const char*) &s4.sin_addr, sizeof(s4.sin_addr));
			break;
		}

		case AF_INET6:
		{
#ifndef _WIN32
			/* Solaris:ip6(7P) "This option takes an integer as an argument; the integer
			 * is the interface index of the selected interface."
			 *
			 * Linux:ipv6(7) "The argument is a pointer to an interface index (see
			 * netdevice(7)) in an integer."
			 *
			 * OS X:IP6(4) "IPV6_MULTICAST_IF u_int *"
			 *
			 * Stevens: "IPV6_MULTICAST_IF has datatype u_int."
			 */
			const unsigned int optval = ifindex;
#else
			/* WinSock2:MSDN(IPPROTO_IPV6 Socket Options) "DWORD; The input value for
			 * setting this option is a 4-byte interface index of the desired outgoing
			 * interface."
			 * NB: 32-bit interface index cf. 24-bit of IPv4 and 8-bit of ip_mreq.
			 */
			const DWORD optval = ifindex;
#endif
			retval = setsockopt(s, IPPROTO_IPV6, IPV6_MULTICAST_IF, (const char*) &optval, sizeof(optval));
			break;
		}

		default:
		break;
	}
	return retval;
}

/* Specify multicast loop, other applications on the same host may receive
 * outgoing packets.  This does not affect unicast packets such as NAKs.
 *
 * If no error occurs, pgm_sockaddr_multicast_loop returns zero.  Otherwise, a
 * value of SOCKET_ERROR is returned, and a specific error code can be
 * retrieved by calling pgm_get_last_sock_error().
 */

PGM_GNUC_INTERNAL
int pgm_sockaddr_multicast_loop(const SOCKET s, const sa_family_t sa_family, const bool v)
{
	int retval = SOCKET_ERROR;

	switch (sa_family)
	{
		case AF_INET:
		{
#ifndef _WIN32
			/* Solaris:ip(7P) "Setting the unsigned character argument to 0 causes the
			 * opposite behavior, meaning that when multiple zones are present, the
			 * datagrams are delivered to all zones except the sending zone."
			 *
			 * Linux:ip(7) "Sets or reads a boolean integer argument"
			 *
			 * OS X:IP(4) provided by example "u_char loop;"
			 *
			 * Stevens: "IP_MULTICAST_LOOP has datatype u_char."
			 */
			const unsigned char optval = v ? 1 : 0;
#else
			/* WinSock2:MSDN(IPPROTO_IP Socket Options) "DWORD (boolean)"
			 */
			const DWORD optval = v ? 1 : 0;
#endif
			retval = setsockopt(s, IPPROTO_IP, IP_MULTICAST_LOOP, (const char*) &optval, sizeof(optval));
			break;
		}

		case AF_INET6:
		{
#ifndef _WIN32
			/* Solaris:ip(7P) "Setting the unsigned character argument to 0 will cause the opposite behavior."
			 *
			 * Linux:ipv6(7) "Argument is a pointer to boolean."
			 *
			 * OS X:IP6(7) "IPV6_MULTICAST_LOOP u_int *"
			 *
			 * Stevens: "IPV6_MULTICAST_LOOP has datatype u_int."
			 */
			const unsigned int optval = v ? 1 : 0;
#else
			/* WinSock2:MSDN(IPPROTO_IPV6 Socket Options) "DWORD (boolean)"
			 */
			const DWORD optval = v ? 1 : 0;
#endif
			retval = setsockopt(s, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, (const char*) &optval, sizeof(optval));
			break;
		}

		default:
		break;
	}
	return retval;
}

/* Specify TTL or outgoing hop limit.
 * NB: Only affects multicast hops, unicast hop-limit is not changed.
 *
 * If no error occurs, pgm_sockaddr_multicast_hops returns zero.  Otherwise, a
 * value of SOCKET_ERROR is returned, and a specific error code can be
 * retrieved by calling pgm_get_last_sock_error().
 *
 * Requires Wine 1.3, supported in Windows 9x/Me.
 */

PGM_GNUC_INTERNAL
int pgm_sockaddr_multicast_hops(const SOCKET s, const sa_family_t sa_family, const unsigned hops)
{
	int retval = SOCKET_ERROR;

	switch (sa_family)
	{
		case AF_INET:
		{
#ifndef _WIN32
			/* Solaris:ip(7P) "This option takes an unsigned character as an argument."
			 *
			 * Linux:ip(7) "Argument is an integer."
			 *
			 * OS X:IP(4) provided by example for SOCK_DGRAM with IP_TTL: "int ttl = 60;",
			 * or for SOCK_RAW & SOCK_DGRAM with IP_MULTICAST_TTL: "u_char ttl;"
			 *
			 * Stevens: "IP_MULTICAST_TTL has datatype u_char."
			 */
			const unsigned char optval = hops;
#else
			/* WinSock2:MSDN(IPPROTO_IP Socket Options) "DWORD"
			 */
			const DWORD optval = hops;
#endif
			retval = setsockopt(s, IPPROTO_IP, IP_MULTICAST_TTL, (const char*) &optval, sizeof(optval));
			break;
		}

		case AF_INET6:
		{
#ifndef _WIN32
			/* Solaris:ip6(7P) "This option takes an integer as an argument."
			 *
			 * Linux:ipv6(7) "Argument is a pointer to an integer."
			 *
			 * OS X:IP6(7) "IPV6_MULTICAST_HOPS int *"
			 *
			 * Stevens: "IPV6_MULTICAST_HOPS has datatype int."
			 */
			const int optval = hops;
#else
			/* WinSock2:MSDN(IPPROTO_IPV6 Socket Options) "DWORD"
			 */
			const DWORD optval = hops;
#endif
			retval = setsockopt(s, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, (const char*) &optval, sizeof(optval));
			break;
		}

		default:
		break;
	}
	return retval;
}

PGM_GNUC_INTERNAL
void pgm_sockaddr_nonblocking(const SOCKET s, const bool v)
{
#ifndef _WIN32
	int flags = fcntl(s, F_GETFL);
	if (!v)
	flags &= ~O_NONBLOCK;
	else
	flags |= O_NONBLOCK;
	fcntl(s, F_SETFL, flags);
#else
	u_long mode = v;
	ioctlsocket (s, FIONBIO, &mode);
#endif
}

/* Note that are sockaddr structure is not passed these functions inherently
 * cannot support IPv6 Zone Indices and hence are rather limited for the
 * link-local scope.
 */

PGM_GNUC_INTERNAL
const char*
pgm_inet_ntop (
		int af,
		const void* restrict src,
		char* restrict dst,
		socklen_t size
)
{
	pgm_assert (AF_INET == af || AF_INET6 == af);
	pgm_assert (NULL != src);
	pgm_assert (NULL != dst);
	pgm_assert (size > 0);

	switch (af)
	{
		case AF_INET:
		{
			struct sockaddr_in sin;
			memset (&sin, 0, sizeof (sin));
			sin.sin_family = AF_INET;
			sin.sin_addr = *(const struct in_addr*)src;
			getnameinfo ((struct sockaddr*)&sin, sizeof (sin),
					dst, size,
					NULL, 0,
					NI_NUMERICHOST);
			return dst;
		}
		case AF_INET6:
		{
			struct sockaddr_in6 sin6;
			memset (&sin6, 0, sizeof (sin6));
			sin6.sin6_family = AF_INET6;
			sin6.sin6_addr = *(const struct in6_addr*)src;
			getnameinfo ((struct sockaddr*)&sin6, sizeof (sin6),
					dst, size,
					NULL, 0,
					NI_NUMERICHOST);
			return dst;
		}
	}

#ifndef _WIN32
	errno = EAFNOSUPPORT;
#else
	WSASetLastError (WSAEAFNOSUPPORT);
#endif
	return NULL;
}

PGM_GNUC_INTERNAL
int
pgm_inet_pton (
		int af,
		const char* restrict src,
		void* restrict dst
)
{
	pgm_assert (AF_INET == af || AF_INET6 == af);
	pgm_assert (NULL != src);
	pgm_assert (NULL != dst);

	struct addrinfo hints =
	{
		.ai_family = af,
		.ai_socktype = SOCK_STREAM, /* not really */
		.ai_protocol = IPPROTO_TCP, /* not really */
		.ai_flags = AI_NUMERICHOST
	}, *result = NULL;

	const int e = getaddrinfo (src, NULL, &hints, &result);
	if (0 != e)
	{
		return 0; /* error */
	}

	pgm_assert (NULL != result->ai_addr);
	pgm_assert (0 != result->ai_addrlen);

	switch (result->ai_addr->sa_family)
	{
		case AF_INET:
		{
			struct sockaddr_in s4;
			memcpy (&s4, result->ai_addr, sizeof(s4));
			memcpy (dst, &s4.sin_addr.s_addr, sizeof(struct in_addr));
			break;
		}

		case AF_INET6:
		{
			struct sockaddr_in6 s6;
			memcpy (&s6, result->ai_addr, sizeof(s6));
			memcpy (dst, &s6.sin6_addr, sizeof(struct in6_addr));
			break;
		}

		default:
		pgm_assert_not_reached();
		break;
	}

	freeaddrinfo (result);
	return 1; /* success */
}

PGM_GNUC_INTERNAL
int
pgm_nla_to_sockaddr (
		const void* restrict nla,
		struct sockaddr* restrict sa
)
{
	uint16_t nla_family;
	int retval = 0;

	memcpy (&nla_family, nla, sizeof(nla_family));
	sa->sa_family = ntohs (nla_family);
	switch (sa->sa_family)
	{
		case AFI_IP:
		sa->sa_family = AF_INET;
		((struct sockaddr_in*)sa)->sin_addr.s_addr = ((const struct in_addr*)((const char*)nla + sizeof(uint32_t)))->s_addr;
		break;

		case AFI_IP6:
		sa->sa_family = AF_INET6;
		memcpy (&((struct sockaddr_in6*)sa)->sin6_addr, (const struct in6_addr*)((const char*)nla + sizeof(uint32_t)), sizeof(struct in6_addr));
		break;

		default:
		retval = -EINVAL;
		break;
	}

	return retval;
}

PGM_GNUC_INTERNAL
int
pgm_sockaddr_to_nla (
		const struct sockaddr* restrict sa,
		void* restrict nla
)
{
	int retval = 0;

	*(uint16_t*)nla = sa->sa_family;
	*(uint16_t*)((char*)nla + sizeof(uint16_t)) = 0; /* reserved 16bit space */
	switch (sa->sa_family)
	{
		case AF_INET:
		*(uint16_t*)nla = htons (AFI_IP);
		((struct in_addr*)((char*)nla + sizeof(uint32_t)))->s_addr = ((const struct sockaddr_in*)sa)->sin_addr.s_addr;
		break;

		case AF_INET6:
		*(uint16_t*)nla = htons (AFI_IP6);
		memcpy ((struct in6_addr*)((char*)nla + sizeof(uint32_t)), &((const struct sockaddr_in6*)sa)->sin6_addr, sizeof(struct in6_addr));
		break;

		default:
		retval = -EINVAL;
		break;
	}

	return retval;
}

/* eof */
#endif

}

namespace ns_smart_net
{

net_endpoint::net_endpoint(cast_type ect, endpoint_type eept, reliable_or_not eron)
{
	// TODO Auto-generated constructor stub
}

net_endpoint::~net_endpoint()
{
	// TODO Auto-generated destructor stub
}

int32_t net_endpoint::set_data(iovec_t& iov)
{
	return EC_SUC;
}

iovec_t& net_endpoint::get_data()
{
	static iovec_t tmp;

	return tmp;
}

} /* namespace NSSmartNet */
