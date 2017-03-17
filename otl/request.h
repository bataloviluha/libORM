#pragma once
namespace otl_utils
{
	template<typename TFieldType, typename TFieldName,
		typename TGetter,
		std::size_t _Size,
		typename TOpearator>
		struct SReqDesc
	{
		using type = TFieldType;
		const TFieldName field_name;
		const TOpearator oper;

		const TGetter getter;

		constexpr static auto arr_size = make_string("[") + istr<_Size>() + make_string("]");

		constexpr SReqDesc(TFieldName field_name_, TGetter getter_, TOpearator oper_):
			field_name(field_name_), getter(getter_), oper(oper_)
		{

		}

		constexpr std::size_t size() const noexcept
		{
			return (_Size);
		}
	};

	template<typename TFieldType, typename TFieldName,
		typename TGetter,
		typename TOpearator>
		struct SReqDesc<TFieldType, TFieldName, TGetter, 0u, TOpearator>
	{
		using type = TFieldType;
		const TFieldName field_name;
		const TOpearator oper;

		const TGetter getter;

		constexpr static auto arr_size = make_string("");

		constexpr SReqDesc(TFieldName field_name_, TGetter getter_, TOpearator oper_):
			field_name(field_name_), getter(getter_), oper(oper_)
		{

		}

		constexpr std::size_t size() const noexcept
		{
			return (_Size);
		}
	};

	template<typename TTypeFieldType, typename TGetter, int TFieldSize, int TOperatorSize = 2>
	constexpr 
		auto make_request(char const (&cstr)[TFieldSize], TGetter getter, char const (&oper)[TOperatorSize] = "=")
	{
		return SReqDesc<TTypeFieldType, string<TFieldSize - 1>, TGetter, 0u, string<TOperatorSize - 1>>(string<TFieldSize - 1>(cstr), getter, string<TOperatorSize - 1>(oper));
	}

	template<typename TTypeFieldType, std::size_t Size, typename TGetter, int TFieldSize, int TOperatorSize = 2>
	constexpr 
		auto make_request(char const (&cstr)[TFieldSize], TGetter getter, char const (&oper)[TOperatorSize] = "=")
	{
		return SReqDesc<TTypeFieldType, string<TFieldSize - 1>, TGetter, Size, string<TOperatorSize - 1>>(string<TFieldSize - 1>(cstr), getter, string<TOperatorSize - 1>(oper));
	}
}