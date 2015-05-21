//============================================================================
// Name        : ReliableProtocal.h
// Author      : TheRockLiuHY
// Date        : May 5, 2015
// Copyright   : liuhongyang's copyright
// Description : SmartNet
//============================================================================

#ifndef RELIABLE_CAST_RELIABLE_CAST_H_
#define RELIABLE_CAST_RELIABLE_CAST_H_

#include <stddef.h>
#include <base/base.h>
#include <utils/lock_free_ringbuf.h>

namespace ns_smart_net
{

	typedef struct
	{
		void *iov_base; /* Starting address */
		size_t iov_len; /* Number of bytes to transfer */
	} iovec_t;

	/*
	 * support unreliable/reliable unicast, multicast
	 */
	class net_endpoint
	{
		DISABLE_COPY(net_endpoint)
		DISABLE_MOVE(net_endpoint)

	public:
		enum cast_type
		{
			ECT_NONE = -1, ECT_UNICAST = 0, ECT_MULTICAST = 1, ECT_BROADCAST = 2
		};

		enum endpoint_type
		{
			EEPT_NONE = -1, EEPT_SENDER = 0, EEPT_RECVER = 1
		};

		enum reliable_or_not
		{
			ERON_NONE = -1, ERON_RELIABLE = 0, ERON_NO_RELIABLE = 1
		};

	public:
		net_endpoint(cast_type ect, endpoint_type eept, reliable_or_not eron);
		~net_endpoint();

	public:
		int32_t set_data(iovec_t &iov); ///not thread-safe, only allow one setter
		iovec_t& get_data();

	private:
		smart_utils::lock_free_ringbuf<iovec_t>::shared_ptr_t output_ringbuf_;
		smart_utils::lock_free_ringbuf<iovec_t>::shared_ptr_t input_ringbuf_;
	};

} /* namespace NSSmartNet */

#endif /* RELIABLE_CAST_RELIABLE_CAST_H_ */
