#pragma once
#include "utils/constexpr_string/istr.hpp"
namespace otl_utils
{
	template<typename TFieldType, typename TFieldName,
		typename TGetter,
		typename TSetter,
		std::size_t _Size,
		std::size_t _Scale,
		bool _notNull,
		bool _unique> struct FieldDescriptor
	{
	public:
		using type = TFieldType;
		const TFieldName field_name;

		const TGetter getter;

		const TSetter setter;

		constexpr static auto arr_size = make_string("");

		constexpr FieldDescriptor(TFieldName field_name_, TGetter getter_, TSetter setter_):
			field_name(field_name_), getter(getter_), setter(setter_)
		{

		}
		constexpr auto size() const noexcept
		{
			return (_Size);
		}
		constexpr auto scale() const noexcept
		{
			return (_Scale);
		}
		constexpr auto not_null() const noexcept
		{
			return (_notNull);
		}
		constexpr auto unique() const noexcept
		{
			return (_unique);
		}
	};
	template<typename TFieldName,
		typename TGetter,
		typename TSetter,
		std::size_t _Size,
		std::size_t _Scale,
		bool _notNull,
		bool _unique> struct FieldDescriptor<std::string, TFieldName, TGetter, TSetter, _Size, _Scale, _notNull, _unique>
	{
	public:
		using type = std::string;
		const TFieldName field_name;

		const TGetter getter;

		const TSetter setter;

		constexpr static auto arr_size = make_string("[") + istr<_Size - 1>() + make_string("]");

		constexpr FieldDescriptor(TFieldName field_name_, TGetter getter_, TSetter setter_):
			field_name(field_name_), getter(getter_), setter(setter_)
		{

		}
		constexpr auto size() const noexcept
		{
			return (_Size);
		}
		constexpr auto scale() const noexcept
		{
			return (_Scale);
		}
		constexpr auto not_null() const noexcept
		{
			return (_notNull);
		}
		constexpr auto unique() const noexcept
		{
			return (_unique);
		}
	};
	/*template<typename TFieldType, typename TFieldName,
		typename TGetter,
		typename TSetter> struct FieldDescriptor<TFieldType, TFieldName, TGetter, TSetter, 0u>
	{
	public:
		using type = TFieldType;
		const TFieldName field_name;

		const TGetter getter;

		const TSetter setter;

		constexpr static auto arr_size = make_string("");

		constexpr FieldDescriptor(TFieldName field_name_, TGetter getter_, TSetter setter_ ):
			field_name(field_name_), getter(getter_), setter(setter_)
		{

		}
		constexpr std::size_t size() const noexcept
		{
			return 0u;
		}
	};*/

	/*template<typename TTypeFieldType, typename TFieldName>
	constexpr SFieldDesc<TTypeFieldType, TFieldName, 0u> make_field(TFieldName fieldName)
	{
		return SFieldDesc<TTypeFieldType, TFieldName, 0u>(fieldName);
	}

	template<typename TTypeFieldType, typename TFieldName, typename TGetter>
	constexpr SFieldDesc<TTypeFieldType, TFieldName, 0u, TGetter> make_field(TFieldName fieldName, TGetter getter)
	{
		return SFieldDesc<TTypeFieldType, TFieldName, 0u, TGetter>(fieldName, getter);
	}*/
	template<typename TTypeFieldType, std::size_t Size, std::size_t Scale, bool NotNull, typename TGetter, typename TSetter, int TFieldSize>
	constexpr /*FieldDescriptor<TTypeFieldType, TFieldName, TGetter, TSetter, Size>*/
		auto make_field(char const (&cstr)[TFieldSize], TGetter getter, TSetter setter)->decltype(auto)
	{
		return FieldDescriptor<TTypeFieldType, string<TFieldSize - 1>, TGetter, TSetter, Size, Scale, NotNull, false>(string<TFieldSize - 1>(cstr), getter, setter);
	}
	template<typename TTypeFieldType, std::size_t Size, std::size_t Scale, bool NotNull, bool Unique, typename TGetter, typename TSetter, int TFieldSize>
	constexpr /*FieldDescriptor<TTypeFieldType, TFieldName, TGetter, TSetter, Size>*/
		auto make_field(char const (&cstr)[TFieldSize], TGetter getter, TSetter setter)->decltype(auto)
	{
		return FieldDescriptor<TTypeFieldType, string<TFieldSize - 1>, TGetter, TSetter, Size, Scale, NotNull, Unique>(string<TFieldSize - 1>(cstr), getter, setter);
	}
	template<typename TTypeFieldType, std::size_t Size, std::size_t Scale, typename TGetter, typename TSetter, int TFieldSize>
	constexpr /*FieldDescriptor<TTypeFieldType, TFieldName, TGetter, TSetter, Size>*/
		auto make_field(char const (&cstr)[TFieldSize], TGetter getter, TSetter setter)->decltype(auto)
	{
		return FieldDescriptor<TTypeFieldType, string<TFieldSize - 1>, TGetter, TSetter, Size, Scale, false, false>(string<TFieldSize - 1>(cstr), getter, setter);
	}

	template<typename TTypeFieldType, std::size_t Size, typename TGetter, typename TSetter, int TFieldSize>
	constexpr /*FieldDescriptor<TTypeFieldType, TFieldName, TGetter, TSetter, Size>*/
		auto make_field(char const (&cstr)[TFieldSize], TGetter getter, TSetter setter)->decltype(auto)
	{
		return FieldDescriptor<TTypeFieldType, string<TFieldSize - 1>, TGetter, TSetter, Size, 0u, false, false>(string<TFieldSize - 1>(cstr), getter, setter);
	}

	template<typename TTypeFieldType, typename TGetter, typename TSetter, int TFieldSize>
	constexpr
		auto make_field(char const (&cstr)[TFieldSize], TGetter getter, TSetter setter)
	{
		return FieldDescriptor<TTypeFieldType, string<TFieldSize - 1>, TGetter, TSetter, 0u, 0u, false, false>(string<TFieldSize - 1>(cstr), getter, setter);
	}
}