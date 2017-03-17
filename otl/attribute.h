#pragma once
namespace otl_utils
{
	template<typename SField, typename SObject>
	struct SAttribute
	{
		const SField& field;

		SObject& object;

		bool wasChange = false;

		template<typename T,
			typename = SField::isSetter>
			void set(T&& val)
		{
			field.setter(object, std::forward<T>(val));

			wasChange = true;
		}

		template<typename T = SField::isGetter>
		auto set()->decltype(field.getter(object))
		{
			return field.getter(object);
		}
	};
}