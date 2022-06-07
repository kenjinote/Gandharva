#ifndef  _b408c7b9_8a2b_43ad_ba92_8a567dead6b4
#define  _b408c7b9_8a2b_43ad_ba92_8a567dead6b4

#include <cstddef>
#include <utility>
#include <type_traits>
#include <string>
#include <vector>


namespace gandharva
{

// 配列の長さ
template <class T, std::size_t n>
char (&(lengthof__(T (&)[n])))[n];

#define lengthof(ary__)  sizeof(lengthof__(ary__))


// スコープを抜けたときのクリーナ
template <class TFn>
class Cleaner__;

template <class TFn>
inline
typename Cleaner__<std::remove_reference<TFn> >::type
Cleaner(TFn&& fn);

template <class TFn>
class Cleaner__
{
public:
	template <class UFn>
	friend inline
	Cleaner__<typename std::remove_reference<UFn>::type>
	Cleaner(UFn&& fn);

	Cleaner__(Cleaner__ && other)
		: fn_(std::move(other.fn_)), bValid_(other.bValid_)
	{
		other.bValid_ = false;
	}

	~Cleaner__(){
		this->Invoke();
	}

	void Invoke(){
		if(bValid_){
			fn_();
			bValid_ = false;
		}
	}

private:
	Cleaner__(TFn const& fn): fn_(fn), bValid_(true) {}
	Cleaner__(TFn     && fn): fn_(std::move(fn)), bValid_(true) {}

	Cleaner__(Cleaner__ const&);      // forbid copy
	void operator=(Cleaner__ const&); // forbid copy

	TFn  fn_    ;
	bool bValid_;
};

template <class TFn>
inline
Cleaner__<typename std::remove_reference<TFn>::type>
Cleaner(TFn&& fn)
{
	return Cleaner__<typename std::remove_reference<TFn>::type>(std::forward<TFn>(fn));
}


// 時間文字列 (hh:mm:ss) を作る
std::wstring TimeToString(long seconds);


// 文字列を分割
// 空文字列は除去される
std::vector<std::wstring> Split(std::wstring const& str, wchar_t separator);

} // namespace gandharva
#endif //_b408c7b9_8a2b_43ad_ba92_8a567dead6b4
