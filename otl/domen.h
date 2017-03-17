#pragma once
namespace otl_utils
{
	template<typename TClass, typename TRequest, typename TTableName, typename TIDName, typename TSeqName>
	class TDomenDescription
	{
	public:
		using ClassName = TClass;
		using RequestName = TRequest;
		TTableName table_name;
		TIDName id_name;
		TSeqName seq_name;

		constexpr TDomenDescription(TTableName table_name_,
									TIDName id_name_,
									TSeqName seq_name_):
			table_name(table_name_),
			id_name(id_name_),
			seq_name(seq_name_)
		{

		}
	};

	template<typename TClass, typename TRequest, typename TTableName, typename TIDName, typename TSeqName>
	constexpr TDomenDescription<TClass, TRequest, TTableName, TIDName, TSeqName> make_desc(TTableName table_name,
																				 TIDName id_name,
																				 TSeqName seq_name)
	{
		return TDomenDescription<TClass, TRequest, TTableName, TIDName, TSeqName>(table_name, id_name, seq_name);
	}
}