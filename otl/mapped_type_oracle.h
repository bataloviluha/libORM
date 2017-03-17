#pragma once
#include "utils/constexpr_string/istr.hpp"
#include <xtr1common>
#include <string>
class QgsGeometry;
namespace boost
{
	namespace posix_time
	{
		class ptime;
	}
	namespace gregorian
	{
		class date;
	}
}
namespace otl_utils
{
	template<typename TName, std::size_t Len, std::size_t Prec, bool _not_null>
	struct MappedTypeOracle
	{
		constexpr static auto valid = std::false_type::value;
	};

	template<bool _not_null>
	struct MappedTypeOracle<int, 0u, 0u, _not_null>
	{
		constexpr static auto valid = std::true_type::value;
		static auto value()
		{
			return make_string("NUMBER(38)");
		}
		static auto type()
		{
			return make_string("NUMBER");
		}
		static auto len()
		{
			return 22;
		}
		static auto prec()
		{
			return 38;
		}
		static auto scale()
		{
			return 0;
		}
		static auto not_null()
		{
			return _not_null ? make_string(" NOT NULL") : make_string("         ");
		}
		static auto is_not_null()
		{
			return (_not_null);
		}
	};
	template<std::size_t Length, std::size_t Prec, bool _not_null>
	struct MappedTypeOracle<double, Length, Prec, _not_null>
	{
		constexpr static auto valid = std::true_type::value;
		static auto value()
		{
			return make_string("NUMBER(") + istr<Length>() + make_string(",") +
				istr<Prec>() + make_string(")");
		}
		static auto type()
		{
			return make_string("NUMBER");
		}
		static auto len()
		{
			return 22;
		}
		static auto prec()
		{
			return (Length);
		}
		static auto scale()
		{
			return (Prec);
		}
		static auto not_null()
		{
			return _not_null ? make_string(" NOT NULL") : make_string("         ");
		}
		static auto is_not_null()
		{
			return (_not_null);
		}
	};
	template<std::size_t Len, bool _not_null>
	struct MappedTypeOracle<std::string, Len, 0u, _not_null>
	{
		constexpr static auto valid = std::true_type::value;
		static auto value()
		{
			return make_string("VARCHAR2(") + istr<Len>() + make_string(")");
		}
		static auto type()
		{
			return make_string("VARCHAR2");
		}
		static auto len()
		{
			return (Len);
		}
		static auto prec()
		{
			return 0;
		}
		static auto scale()
		{
			return 0;
		}
		static auto not_null()
		{
			return _not_null ? make_string(" NOT NULL") : make_string("         ");
		}
		static auto is_not_null()
		{
			return (_not_null);
		}
	};

	template<bool _not_null>
	struct MappedTypeOracle<boost::gregorian::date, 0u, 0u, _not_null>
	{
		constexpr static auto valid = std::true_type::value;
		static auto value()
		{
			return make_string("DATE");
		}
		static auto type()
		{
			return make_string("DATE");
		}
		static auto len()
		{
			return 7;
		}
		static auto prec()
		{
			return 0;
		}
		static auto scale()
		{
			return 0;
		}
		static auto not_null()
		{
			return _not_null ? make_string(" NOT NULL") : make_string("         ");
		}
		static auto is_not_null()
		{
			return (_not_null);
		}
	};

	template<bool _not_null>
	struct MappedTypeOracle<boost::posix_time::ptime, 0u, 0u, _not_null>
	{
		constexpr static auto valid = std::true_type::value;
		static auto value()
		{
			return make_string("DATE");
		}
		static auto type()
		{
			return make_string("DATE");
		}
		static auto len()
		{
			return 7;
		}
		static auto prec()
		{
			return 0;
		}
		static auto scale()
		{
			return 0;
		}
		static auto not_null()
		{
			return _not_null ? make_string(" NOT NULL") : make_string("         ");
		}
		static auto is_not_null()
		{
			return (_not_null);
		}
	};

	template<bool _not_null>
	struct MappedTypeOracle<QgsGeometry, 0u, 0u, _not_null>
	{
		constexpr static auto valid = std::true_type::value;
		static auto value()
		{
			return make_string("MDSYS.SDO_GEOMETRY");
		}
		static auto type()
		{
			return make_string("SDO_GEOMETRY");
		}
		static auto len()
		{
			return 1;
		}
		static auto prec()
		{
			return 0;
		}
		static auto scale()
		{
			return 0;
		}
		static auto not_null()
		{
			return _not_null ? make_string(" NOT NULL") : make_string("         ");
		}
		static auto is_not_null()
		{
			return (_not_null);
		}
	};
}