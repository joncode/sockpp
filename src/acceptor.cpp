// acceptor.cpp
//
// --------------------------------------------------------------------------
// This file is part of the "sockpp" C++ socket library.
//
// Copyright (c) 2014-2017 Frank Pagliughi
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
// --------------------------------------------------------------------------

#include <cstring>
#include "sockpp/acceptor.h"

using namespace std;

namespace sockpp {

/////////////////////////////////////////////////////////////////////////////

// Binds the socket to the specified address.

bool acceptor::bind(const sock_address& addr)
{
	if (check_ret_bool(::bind(handle(), addr.sockaddr_ptr(), addr.size()))) {
        addr_ = addr;
		return true;
	}
    return false;
}

// --------------------------------------------------------------------------
// This attempts to open the acceptor, bind to the requested address, and
// start listening. On any error it will be sure to leave the underlying
// socket in an unopened/invalid state.
// If the acceptor appears to already be opened, this will quietly succeed
// without doing anything.

bool acceptor::open(const sock_address& addr, int queSize /*=DFLT_QUE_SIZE*/)
{
	// TODO: What to do if we are open but bound to a different address?
	if (is_open())
		return true;

	if (addr.size() < sizeof(sa_family_t))
		return false;

	sa_family_t domain = *(reinterpret_cast<const sa_family_t*>(addr.sockaddr_ptr()));
	if (domain == AF_UNSPEC) {
		// TODO: Set last error for "address unspecified"
		return false;
	}

	socket_t h = stream_socket::create(domain);
	if (!check_ret_bool(h))
		return false;

	reset(h);

	#if !defined(WIN32)
        // TODO: This should be an option
		if (domain == AF_INET || domain == AF_INET6) {
			int reuse = 1;
			if (!set_option(SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int))) {
				close();
				return false;
			}
		}
	#endif

	if (!bind(addr) || !listen(queSize)) {
		close();
		return false;
	}

	return true;
}

// --------------------------------------------------------------------------

stream_socket acceptor::accept(sock_address* clientAddr /*=nullptr*/)
{
	sockaddr* p = clientAddr ? clientAddr->sockaddr_ptr() : nullptr;
    socklen_t len = clientAddr ? clientAddr->size() : 0;

    socket_t s = check_ret(::accept(handle(), p, &len));
	return stream_socket(s);
}

/////////////////////////////////////////////////////////////////////////////
// end namespace sockpp
}

