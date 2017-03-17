#pragma once
#include "utils/constexpr_string/istr.hpp"
#include <string>
#include "boost/date_time/gregorian/greg_date.hpp"
#include "boost/date_time/posix_time/ptime.hpp"
#include "qgsgeometry.h"
//class QgsGeometry;
//namespace boost
//{
//	namespace posix_time
//	{
//		class ptime;
//	}
//	namespace gregorian
//	{
//		class date;
//	}
//}
namespace otl_utils
{
	template<typename T>
	struct MappedTypeOTL
	{
		constexpr static auto valid = std::false_type::value;
	};

	template<>
	struct MappedTypeOTL<int>
	{
		constexpr static auto valid = std::true_type::value;
		constexpr static auto value = make_string("int");
		template<typename TType>
		static constexpr auto check_val(const TType& val)
		{
			return (bool)val;
		}
		/*static constexpr auto check_val(const int* val)
		{
			return (bool)val;
		}*/
	};
	template<>
	struct MappedTypeOTL<double>
	{
		constexpr static auto valid = std::true_type::value;
		constexpr static auto value = make_string("double");
		static constexpr auto check_val(const double& val)
		{
			return (bool)val;
		}
		static constexpr auto check_val(const double* val)
		{
			return (bool)val;
		}
	};
	template<>
	struct MappedTypeOTL<std::string>
	{
		constexpr static auto valid = std::true_type::value;
		constexpr static auto value = make_string("char");
		static constexpr auto check_val(const std::string& val)
		{
			return !(val.size());
		}
	};
	template<>
	struct MappedTypeOTL<boost::gregorian::date>
	{
		constexpr static auto valid = std::true_type::value;
		constexpr static auto value = make_string("timestamp");
		static constexpr auto check_val(const boost::gregorian::date& val)
		{
			return !(val.is_not_a_date());
		}
	};
	template<>
	struct MappedTypeOTL<boost::posix_time::ptime>
	{
		constexpr static auto valid = std::true_type::value;
		constexpr static auto value = make_string("timestamp");
		static constexpr auto check_val(const boost::posix_time::ptime& val)
		{
			return !(val.is_not_a_date_time());
		}
	};
	template<>
	struct MappedTypeOTL<QgsGeometry>
	{
		constexpr static auto valid = std::true_type::value;
		constexpr static auto value = make_string("sdo_geometry");
		static constexpr auto check_val(const QgsGeometry* val)
		{
			return (bool)val;
		}
		static constexpr auto check_val(const QgsGeometry& val)
		{
			return (bool)val.geometry();
		}
		static constexpr auto check_val(const std::unique_ptr<QgsGeometry>& val)
		{
			return (bool)val;
		}
	};
}