// Copyright (c) 2016 nyorain 
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt

///\file
///\brief Defines the Connection and Connectable classes needed e.g. for callbacks.

#pragma once

#ifndef NYTL_INCLUDE_CONNECTION_HPP
#define NYTL_INCLUDE_CONNECTION_HPP

#include <nytl/nonCopyable.hpp>

#include <cstdlib>
#include <memory>

namespace nytl
{

template<typename ID> class Connection;
template<typename ID> class ConnectionRef;

///Base class for objects that can be connected to in any way.
///This connection can then be controlled (i.e. destroyed) with a Connection object.
template<typename ID>
class Connectable
{
protected:
	friend class Connection<ID>;
	friend class ConnectionRef<ID>;

	virtual ~Connectable() = default;
	virtual void removeConnection(ID id) = 0;
};

///Underlaying connection data.
template<typename ID>
using ConnectionDataPtr = std::shared_ptr<ID>;

///\ingroup function
///\brief The Connection class represents a Connection to a nytl::Callback slot.
///\details A Connection object is returned when a function is registered in a Callback object
///and can then be used to unregister the function and furthermore check whether
///the Callback object is still valid and the function is still registered.
template<typename ID>
class Connection
{
public:
	Connection() = default;
	Connection(Connectable<ID>& call, const ConnectionDataPtr<ID>& data) noexcept
		: callback_(&call), data_(data) {}

	~Connection() = default;

	Connection(const Connection&) = default;
	Connection& operator=(const Connection&) = default;

	Connection(Connection&&) = default;
	Connection& operator=(Connection&&) = default;

	///Unregisters the function associated with this Connection from the Callback object.
	void destroy() { if(connected()) callback_->removeConnection(*data_); callback_ = nullptr; }

	///Returns whether the function is still registered and the Callback is still alive.
	bool connected() const { return (callback_) && (data_) && (*data_ != 0); }

protected:
	Connectable<ID>* callback_ {nullptr};
	ConnectionDataPtr<ID> data_ {nullptr};
};

///\ingroup function
///\brief Like Connection representing a registered function but can be used inside Callbacks.
///\details Sometimes it may be useful to unregister a Callback function while it is called
///(e.g. if the Callback function should be called only once) and there is no possibility to
///capture a Connection object inside the Callback (like e.g. with lambdas) then a ConnectionRef
///parameter can be added to the beggining of the Callbacks function signature with which the
///function can be unregistered from inside itself. A new class is needed for this since
///if Connection is used in a fucntion signature, the Callback object can not know if this
///Connection object is part of the signature or only there to get a Connection to itself.
///So there is no need for generally using this class outside a Callback function, Connection
///should be used instead since it proved the same functionality.
template<typename ID>
class ConnectionRef
{
public:
	ConnectionRef() = default;
	ConnectionRef(Connectable<ID>& call, const ConnectionDataPtr<ID>& data) noexcept
		: callback_(&call), data_(data) {}

	~ConnectionRef() = default;

	ConnectionRef(const ConnectionRef& other) = default;
	ConnectionRef& operator=(const ConnectionRef& other) = default;

	ConnectionRef(ConnectionRef&& other) = default;
	ConnectionRef& operator=(ConnectionRef&& other) = default;

	///Disconnected the Connection, unregisters the associated function.
	void destroy() const { if(connected()) callback_->removeConnection(*data_); }

	///Returns whether the Callback function is still registered.
	bool connected() const { return (callback_) && (*data_ != 0); }

protected:
	Connectable<ID>* callback_ {nullptr};
	ConnectionDataPtr<ID> data_ {nullptr};
};

///\ingroup function
///RAII Connection class that will disconnect automatically on destruction.
template<typename ID>
class ConnectionGuard : public NonCopyable
{
public:
	ConnectionGuard() = default;
	ConnectionGuard(const Connection<ID>& conn) : connection_(conn) {}
	~ConnectionGuard() { connection_.destroy(); }

	ConnectionGuard(ConnectionGuard&& other) : connection_(std::move(other.connection_)) {}
	ConnectionGuard& operator=(ConnectionGuard&& other)
		{ release(); connection_ = std::move(other.connection_); return *this; }

	Connection<ID>& get() { return connection_; }
	const Connection<ID>& get() const { return connection_; }
	void release(){ connection_ = {}; }

	bool connected() const { return connection_.connected(); }
	void destroy() { connection_.destroy(); }

protected:
	Connection<ID> connection_ {};
};

}

#endif //header guard
