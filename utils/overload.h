#pragma once
namespace utils
{
	struct overload
	{
		template<typename C, typename R,typename ... Args>
		static constexpr auto get_overload_const(R(C::*pmf)(Args...) const)-> decltype(pmf)
		{
			return pmf;
		}
		template<typename C, typename R, typename ... Args>
		static constexpr auto get_overload_not_const(R(C::*pmf)(Args...))-> decltype(pmf)
		{
			return pmf;
		}
	};
}