// Copyright (c) 2017-2018 nyorain
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt

/// \file Helpers for making types observable or observe the lifetime of objects.

#pragma once

#ifndef NYTL_INCLUDE_OBSERVE
#define NYTL_INCLUDE_OBSERVE

#include <vector> // std::vector
#include <algorithm> // std::find
#include <utility> // std::move

namespace nytl {

/// \brief Interface for classes that observe the lifetime of Observable objects.
/// Note that a observer implementation must unregister itself on destruction or
/// assure that it will always outlive all objects it observes, otherwise undefined
/// behavior might be triggered.
/// \module utility
template<typename T>
class Observer {
public:
	virtual ~Observer() = default;

	/// Will be called when an object observed by this observer will be destructed.
	/// The destructed object will be passed to it.
	virtual void observeDestruction(T&) = 0;
};

/// \brief Utility class to make the objects lifetime observable.
/// \details Base class that can be derived from if the lifetime of objects of this class should be
/// observable by others.
/// Note that this class is not thread-safe, and calls to move/add/remove Observer must not interfer
/// in any way. Adding the same Observer for one Observable object more than once is undefined
/// behavior. Adding an Observer for an Observable during its destruction callback is
/// undefined behavior.
/// \tparam T The class deriving from this class using the CRTP idiom.
/// \module utility
template<typename T>
class Observable {
public:
	using ObservableDerived = T;

public:
	~Observable()
	{
		// needed if a Observer implementation removes itself during observeDestruction.
		auto cpy = std::move(observer_);
		for(auto& obs : cpy) {
			obs->observeDestruction(*reinterpret_cast<T*>(this));
		}
	}

	/// \brief Adds the given observer to the list of observers.
	/// The same observer can be added multiple times, in which case it will also be
	/// called multiple times.
	void addObserver(Observer<T>& obs)
	{
		observer_.push_back(&obs);
	}

	/// \brief Removes the given observer from the list of observers.
	/// Removed the observer entirely, i.e. removes all entries if the observer
	/// was added multiple times.
	/// Will always return false if an observer calls this during its callback.
	/// \returns The number of elements found. So returns 0 if no elements could be found.
	unsigned int removeObserver(const Observer<T>& obs)
	{
		auto before = observer_.size();
		observer_.erase(std::remove(observer_.begin(), observer_.end(), &obs), observer_.end());
		return (before - observer_.size());
	}

	/// \brief Changes a given observer to a new one.
	/// Might be more efficient than first removing the oldone and then adding
	/// the new one.
	/// Will move the first found observer of oldone to newone.
	/// So if oldone is registered multiple times, will only move the
	/// first entry.
	/// \returns Whether the given observable object could be found. If this returns
	/// false the old observer could not be found and the new one was not added.
	bool moveObserver(Observer<T>& oldone, Observer<T>& newone) noexcept
	{
		auto it = std::find(observer_.begin(), observer_.end(), &oldone);
		if(it == observer_.cend()) return false;
		*it = &newone;
		return true;
	}

	/// Returns a constant reference to the list of observers.
	const std::vector<Observer<T>*>& observers() const noexcept { return observer_; }

protected:
	std::vector<Observer<T>*> observer_;
};

/// \brief Wrapper to make already defined classes observable.
/// Useful for e.g. stl classes or other not-changeable classes whose objects
/// should be observable in a certain context.
/// Can be used like this (for the already defined class 'SomeClass'):
/// ```cpp
/// struct MyObserver : public nytl::Observer<SomeClass> {
/// 	void observeDestruction(SomeClass& obj) override
/// 		{ std::cout << &obj << " was destructed!\n"; }
/// };
///
/// using ObsClass = nytl::ObservableWrapper<SomeClass>;
///
/// int main()
/// {
/// 	auto observer = MyObserver {};
/// 	auto object = new ObsClass {};
/// 	object->addObserver(observer);
/// 	delete object; // will trigger observer.observeDestruction(*object)
/// }
/// ```
/// \module utility
template<typename T>
struct ObservableWrapper : public T, public Observable<T> {
	using T::T;
};

/// \brief Smart pointer class that observes the lifetime of its object.
/// \details Basically a smart pointer that does always know whether the object it points to is
/// alive or not. Does only work with objects of classes that are derived from nytl::Observable.
/// Semantics are related to std::unique_ptr/std::shared_ptr.
/// \requires Type 'T' must be derived from [nytl::Observable]() or fulfill its syntax.
/// \module utility
template<typename T>
class ObservingPtr : public Observer<typename T::ObservableDerived> {
public:
	ObservingPtr() = default;
	ObservingPtr(T* obj) : object_(obj) { if(object_) object_->addObserver(*this); }
	ObservingPtr(T& obj) : object_(&obj) { object_->addObserver(*this); }
	~ObservingPtr(){ if(object_) object_->removeObserver(*this); }

	ObservingPtr(const ObservingPtr& other) : object_(other.object_)
	{
		if(object_) object_->addObserver(*this);
	}
	ObservingPtr& operator=(const ObservingPtr& other)
	{
		reset(other.object_);
		return *this;
	}

	ObservingPtr(ObservingPtr&& other) noexcept : object_(other.object_)
	{
		if(object_) object_->moveObserver(other, *this);
		other.object_ = nullptr;
	}
	ObservingPtr& operator=(ObservingPtr&& other) noexcept
	{
		reset();
		object_ = other.object_;
		if(object_) object_->moveObserver(other, *this);
		other.object_ = nullptr;
		return *this;
	}

	void reset(T* obj = nullptr)
	{
		if(obj) obj->addObserver(*this);
		if(object_) object_->removeObserver(*this);
		object_ = obj;
	}
	void reset(T& obj)
	{
		obj.addObserver(*this);
		if(object_) object_->removeObserver(*this);
		object_ = &obj;
	}

	T* get() const { return object_; }
	T& operator*() const { return *object_; }
	T* operator->() const { return object_; }

	operator bool() const { return (object_ != nullptr); }
	friend void swap(ObservingPtr& a, ObservingPtr& b) noexcept
	{
		if(a.object_) a.object_->moveObserver(a, b);
		if(b.object_) b.object_->moveObserver(b, a);
		std::swap(a.object_, b.object_);
	}

private:
	T* object_ {nullptr};
	void observeDestruction(typename T::ObservableDerived&) override { object_ = nullptr; }
};

} // namespace nytl

#endif // header guard
