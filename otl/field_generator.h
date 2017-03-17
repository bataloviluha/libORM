#pragma once
#include <tuple>
namespace otl_utils
{
	template<typename ... Args>
	constexpr auto generateFields(Args&&... _Args)
	{
		return std::make_tuple(std::forward<Args>(_Args)...);
	}
}