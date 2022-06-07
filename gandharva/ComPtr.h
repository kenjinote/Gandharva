#ifndef _0f79af69_848b_4a27_9bcf_28748c5ad888
#define _0f79af69_848b_4a27_9bcf_28748c5ad888

#include <utility>
#include <type_traits>
#include <new>

namespace gandharva
{

template<class T> 
class comptr
{
public:
	typedef T element_type;

	// constructors:
	comptr()
		: p_(nullptr) {}
	template<class Y> explicit comptr(Y* p)
		: p_(p) {}
	comptr(const comptr& r)
		: p_(r.get()) { if(p_) p_->AddRef(); }
	template<class Y> comptr(const comptr<Y>& r)
		: p_(r.get()) { if(p_) p_->AddRef(); }
	comptr(comptr&& r)
		: p_(r.get()) { r.detach(); }
	template<class Y> comptr(comptr<Y>&& r)
		: p_(r.get()) { r.detach(); }
	comptr(nullptr_t)
		: p_(nullptr) {}

	// destructor:
	~comptr() { this->reset(); }
	
	// assignment:
	comptr& operator=(const comptr& r){
		if(this->p_ != r.p_){
			this->reset();
			p_ = r.get(); if(p_) p_->AddRef();
		}
		return *this;
	}
	template<class Y> comptr& operator=(const comptr<Y>& r){
		if(this->p_ != r.p_){
			this->reset();
			p_ = r.get(); if(p_) p_->AddRef();
		}
		return *this;
	}
	comptr& operator=(comptr&& r){
		this->reset();
		p_ = r.get(); r.detach();
		return *this;
	}
	template<class Y> comptr& operator=(comptr<Y>&& r){
		this->reset();
		p_ = r.get(); r.detach();
		return *this;
	}

	// modifiers:
	void swap(comptr& r){
		auto temp = r.p_; r.p_ = this->p_; this->p_ = temp;
	}
	void reset(){
		if(p_){ p_->Release(); p_ = nullptr; }
	}
	template<class Y> void reset(Y* p){
		this->reset();
		p_ = p;		
	}
	void detach(){ p_ = nullptr; }

	// observers:
	T* get() const { return p_; }
//	T& operator*() const { return *p_; }
	T* operator->() const { return p_; }
	operator bool() const { return p_ != nullptr; }

	class PtrToPtr_
	{
	public:
		typedef T element_type;
		PtrToPtr_(comptr& r): r_(r) {}
		PtrToPtr_(PtrToPtr_&& other): r_(other.r_) {}

		operator void** (){
			r_.reset();
			return &(void*&)r_.p_;
		}

		operator T** (){
			r_.reset();
			return &r_.p_;
		}

	private:
		PtrToPtr_(const PtrToPtr_&);
		void operator=(const PtrToPtr_&);

		comptr& r_;
	};

	PtrToPtr_ operator&() { return PtrToPtr_(*this); }

	// pIf1->QueryInterface(&pIf2) Ç™è„ÇÃ & ââéZéqÇ≈ÇÕÇ§Ç‹Ç≠Ç¢Ç©Ç»Ç¢ÇÃÇ≈
	// pIf1.QueryInterface(&pIf2) Ç∆Ç¢Ç§ë„ë÷ï®ÇçÏÇÈ

	template <class TPtrToPtr>
	auto QueryInterface(TPtrToPtr&& pp)
		-> decltype(p_->QueryInterface(
			__uuidof(typename std::remove_reference<TPtrToPtr>::type::element_type),
			(void**)pp
		))
	{
		return p_->QueryInterface(
			__uuidof(typename std::remove_reference<TPtrToPtr>::type::element_type),
			(void**)pp
		);
	}

private:
	T*  p_;
};

// comptr creation
template<class T> comptr<T> make_comptr()
{
	return comptr<T>(new(std::nothrow) T());
}
template<class T, class Arg0> comptr<T> make_comptr(Arg0&& arg0)
{
	return comptr<T>(new(std::nothrow) T(std::forward<Arg0>(arg0)));
}
template<class T, class Arg0, class Arg1> comptr<T> make_comptr(Arg0&& arg0, Arg1&& arg1)
{
	return comptr<T>(new(std::nothrow) T(std::forward<Arg0>(arg0), std::forward<Arg1>(arg1)));
}
template<class T, class Arg0, class Arg1, class Arg2> comptr<T> make_comptr(Arg0&& arg0, Arg1&& arg1, Arg2&& arg2)
{
	return comptr<T>(new(std::nothrow) T(std::forward<Arg0>(arg0), std::forward<Arg1>(arg1), std::forward<Arg2>(arg2)));
}

// comptr comparisons:
template<class T, class U>
bool operator==(const comptr<T>& a, const comptr<U>& b){ return a.get() == b.get(); }
template<class T, class U>
bool operator!=(const comptr<T>& a, const comptr<U>& b){ return a.get() != b.get(); }
template<class T, class U>
bool operator<(const comptr<T>& a, const comptr<U>& b){ return a.get() < b.get(); }
template<class T, class U>
bool operator>(const comptr<T>& a, const comptr<U>& b){ return a.get() > b.get(); }
template<class T, class U>
bool operator<=(const comptr<T>& a, const comptr<U>& b){ return a.get() <= b.get(); }
template<class T, class U>
bool operator>=(const comptr<T>& a, const comptr<U>& b){ return a.get() >= b.get(); }
template <class T>
bool operator==(const comptr<T>& a, nullptr_t){ return a.get() == nullptr; }
template <class T>
bool operator==(nullptr_t, const comptr<T>& b){ return nullptr == b.get(); }
template <class T>
bool operator!=(const comptr<T>& a, nullptr_t){ return a.get() != nullptr; }
template <class T>
bool operator!=(nullptr_t, const comptr<T>& b){ return nullptr != b.get(); }
template <class T>
bool operator<(const comptr<T>& a, nullptr_t){ return a.get() < nullptr; }
template <class T>
bool operator<(nullptr_t, const comptr<T>& b){ return nullptr < b.get(); }
template <class T>
bool operator<=(const comptr<T>& a, nullptr_t){ return a.get() <= nullptr; }
template <class T>
bool operator<=(nullptr_t, const comptr<T>& b){ return nullptr <= b.get(); }
template <class T>
bool operator>(const comptr<T>& a, nullptr_t){ return a.get() > nullptr; }
template <class T>
bool operator>(nullptr_t, const comptr<T>& b){ return nullptr > b.get(); }
template <class T>
bool operator>=(const comptr<T>& a, nullptr_t){ return a.get() >= nullptr; }
template <class T>
bool operator>=(nullptr_t, const comptr<T>& b){ return nullptr >= b.get(); }

// comptr casts:
template<class T, class U>
comptr<T> static_pointer_cast(const comptr<U>& r)
{
	auto t = static_cast<T*>(r.get());
	if(t) t->AddRef();
	return comptr<T>(t);
}
template<class T, class U>
comptr<T> dynamic_pointer_cast(const comptr<U>& r)
{
	auto t = dynamic_cast<T*>(r.get());
	if(t) t->AddRef();
	return comptr<T>(t);
}
template<class T, class U>
comptr<T> const_pointer_cast(const comptr<U>& r)
{
	auto t = const_cast<T*>(r.get());
	if(t) t->AddRef();
	return comptr<T>(t);
}


} // namespace gandharva


// comptr specialized algorithms:
namespace std
{
	template<class T> void swap(gandharva::comptr<T>& a, gandharva::comptr<T>& b){
		return a.swap(b);
	}
}


#endif//_0f79af69_848b_4a27_9bcf_28748c5ad888
