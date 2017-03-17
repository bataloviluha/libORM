#pragma once
#include "mine_structure/otl/interface/t_otl_factory_pimpl.h"
#include "domen.h"
#include "field.h"
#include "mapped_type.h"
#include "request.h"
#include "meta_factory.h"
#include <mutex>
#include "otl_wrapper/src/OTL_Wrapper.h"
#include "utils/applyh.h"
#include "mapped_type_oracle.h"
#include "mine_structure/otl/interface/CRequest.h"
namespace otl_utils
{
	template<typename meta_factory, typename ... Args>
	class PrivateDataReslisation:public TOtlFactoryPimpl<typename meta_factory::DomenType::ClassName,
		typename meta_factory::DomenType::RequestName>
	{
		//проверка существовани€ таблицы
		bool check_table_exist()
		{
			auto sql = "select count(1) from USER_TABLES WHERE TABLE_NAME=:TABLE_NAME<char[127]>";
			data_nocommit_stream Stream(1, sql, this->cn.otl_connection());
			Stream << this->table_name();
			int count(0);
			Stream >> count;
			return count;
		}

		//проверка сущестовавани€ первичного ключа
		bool check_pk()
		{
			auto sql = "select count(1) from USER_CONSTRAINTS where TABLE_NAME=:TABLE_NAME<char[127]> and CONSTRAINT_TYPE = 'P'";
			data_nocommit_stream Stream(1, sql, this->cn.otl_connection());
			Stream << this->table_name();
			int count(0);
			Stream >> count;
			return count;
		}

		//проверка существовани€ секвенса
		int check_seq()
		{
			auto sql = "select LAST_NUMBER from USER_SEQUENCES where SEQUENCE_NAME=:SEQUENCE_NAME<char[127]>";
			data_nocommit_stream Stream(1, sql, this->cn.otl_connection());
			Stream << this->seq_name();
			if(Stream.eof())
			{
				return 0;
			}
			int count(0);
			Stream >> count;
			return count;
		}

	public:
		using meta = typename meta_factory;

		meta meta_fact;

		using type_tupple = std::tuple<Args...>;
		type_tupple tupple_arguments;

		PrivateDataReslisation(std::mutex& _connectionMutex,
							   otl_connection_wrapper& _cn, meta_factory&& meta, type_tupple&& tupple_):
			TOtlFactoryPimpl(_cn, _connectionMutex), meta_fact(std::move(meta)),
			tupple_arguments(std::move(tupple_))
		{

		}

		virtual std::string seq_name() const override
		{
			return std::string(meta_fact.DomenDesc.seq_name.c_str());
		}

		virtual std::string make_where(const request_type& req) const override
		{
			return meta_fact.make_where(req);
		}

		virtual std::string make_order(const request_type& req) const override
		{
			std::string retVal;
			auto c_req = dynamic_cast<const otl_utils::CRequest*>(&req);
			if(c_req)
			{
				bool first = true;
				for(auto&& it : c_req->order_by())
				{
					if(!first)
						retVal += ",";
					retVal += it.first;
					retVal += (it.second ? " ASC " : " DESC ");
					first = false;
				}
			}
			return retVal;
		}
		virtual void fill_from_query(data_nocommit_stream& str, const request_type& req) override
		{
			meta_fact.fill_from_query(str, req);
		}

		virtual db_ptr_type create_from_stream(data_nocommit_stream& str, const request_type& req) override
		{
			db_aware::TIdValueType id;
			str >> id;
			auto iter_find = cache.find(id);
			if(iter_find != cache.end())
			{
				auto ptr = iter_find->second.lock();
				if(ptr)
				{
					str.skip_to_end_of_row();
					return ptr;
				}
			}
			auto object = this->create();
			object->set_id(id);
			meta_fact.create_from_stream(object, str);
			if(iter_find == cache.end())
			{
				cache.emplace(id, object);
			}
			else
			{
				iter_find->second = object;
			}
			return object;
		}

		template<typename ... Args>
		db_ptr_type create(const std::tuple<Args...>& tupple)
		{
			return utils::apply(&std::make_shared<typename meta::DomenType::ClassName, Args...>, tupple);
		}

		//создать объект
		virtual db_ptr_type create() override
		{
			return create(tupple_arguments);
		}

		virtual std::string table_name() const override
		{
			return std::string(meta_fact.DomenDesc.table_name.c_str());
		}

		virtual std::string select_fields_string() const override
		{
			return std::string(meta_fact.get_fields_string().c_str());
		}

		virtual std::string primary_field_name() const override
		{
			return std::string(meta_fact.DomenDesc.id_name.c_str());
		}

		virtual std::string make_update_sql(const db_const_ptr_type& ptr) override
		{
			return std::string(meta_fact.get_update_string().c_str());
		}

		virtual std::string make_insert_sql(const db_const_ptr_type& ptr) override
		{
			return std::string(meta_fact.get_insert_string().c_str());
		}

		/*virtual request_ptr_type new_request() override
		{
			return std::make_shared<typename meta_factory::DomenType::RequestName>();
		}*/

		virtual void fill_save_query(data_nocommit_stream& saveStream, const db_ptr_type& val) override
		{
			meta_fact.fill_save_query(val, saveStream);
		}

		//создать таблицу
		void create_table()
		{
			auto sql = std::string("create table ");
			sql += this->table_name();
			sql += " (" + this->primary_field_name() + " " + MappedTypeOracle<db_aware::TIdValueType, 0u, 0u, true>::value().c_str();
			sql += this->meta_fact.get_list_fields_for_create_table().c_str();
			sql += " )";

			data_nocommit_stream Stream(1, sql.c_str(), this->cn.otl_connection());
		}

		//проверка полей таблицы и их создание/модификаци€
		void validate_tab_columns(std::vector<std::string>& retVal = std::vector<std::string>())
		{
			auto sql = "select COLUMN_NAME, DATA_TYPE, DATA_LENGTH, DATA_PRECISION, DATA_SCALE, NULLABLE from USER_TAB_COLUMNS WHERE TABLE_NAME=:TABLE_NAME<char[127]>";
			data_nocommit_stream Stream(30, sql, this->cn.otl_connection());
			Stream << this->table_name();
			std::vector<std::tuple<std::string, std::string, int, int, int, bool>> tup;
			while(!Stream.eof())
			{
				std::string COLUMN_NAME, DATA_TYPE;
				int DATA_LENGTH(0), DATA_PRECISION(0), DATA_SCALE(0);
				std::string NULLABLE;

				Stream >> COLUMN_NAME >> DATA_TYPE >> DATA_LENGTH >> DATA_PRECISION >> DATA_SCALE >> NULLABLE;

				tup.emplace_back(std::move(COLUMN_NAME), std::move(DATA_TYPE),
								 DATA_LENGTH, DATA_PRECISION, DATA_SCALE, NULLABLE == "N");
			}
			auto iter_find_pk = std::find_if(tup.cbegin(), tup.cend(), [pName = this->primary_field_name()](decltype(tup)::const_reference val)
			{
				return std::get<0>(val) == pName;
			});
			if(iter_find_pk == tup.cend())
			{
				retVal.push_back("ќтсутсвует первичное поле " + this->primary_field_name() + " дл€ таблицы " + this->table_name());
				std::string sqlAddField = " alter table " + this->table_name() + " add (" + this->primary_field_name() + MappedTypeOracle<db_aware::TIdValueType, 0u, 0u, true>::value().c_str() + ")";
				data_nocommit_stream Stream(1, sqlAddField.c_str(), this->cn.otl_connection());
			}

			meta_fact.validate_tab_columns(this->cn, std::move(tup), retVal);
		}

		//проверка первичного ключа и его создание в случае отсутстви€
		void validate_pk(std::vector<std::string>& retVal = std::vector<std::string>())
		{
			if(!check_pk())
			{
				retVal.push_back("ќтсутсвует первичный ключ дл€ таблицы " + this->table_name());
				std::string sql = "alter table " + this->table_name() + " add( "
					"constraint " + this->table_name() + "_PK "
					"PRIMARY KEY "
					"(" + this->primary_field_name() + "))";
				data_nocommit_stream Stream(1, sql.c_str(), this->cn.otl_connection());
			}
		}

		//проверка всех ключей таблицы
		void validate_keys(std::vector<std::string>& retVal = std::vector<std::string>())
		{
			validate_pk(retVal);
		}

		//проверка и создание последовательности в случае отсутстви€
		void validate_seq(std::vector<std::string>& retVal = std::vector<std::string>())
		{
			db_aware::TIdValueType max(1);
			{
				auto sql_get_max = "select max(" + this->primary_field_name() + ") + 1 from " + this->table_name();
				data_nocommit_stream Stream(1, sql_get_max.c_str(), this->cn.otl_connection());
				Stream >> max;
				if(Stream.is_null())
				{
					max = 1;
				}
			}

			auto last_value = check_seq();
			if(!last_value)
			{
				retVal.push_back("ќтсутсвует последовательность "+ this->seq_name());
				std::string sql = "create sequence " + this->seq_name() +
					" start with " + std::to_string(max) + 
					" INCREMENT BY 1"
					" MINVALUE 1";
				data_nocommit_stream Stream(1, sql.c_str(), this->cn.otl_connection());
			}
			else if(last_value < max)
			{
				auto delta = max - last_value;
				retVal.push_back("¬ последовательности " + this->seq_name() +" следующее значение меньше максимального элемента на велечину " + std::to_string(delta));
				if(delta > 1)
				{
					auto sql = "alter sequence " + this->seq_name() + " increment by " + std::to_string(delta);
					data_nocommit_stream Stream(1, sql.c_str(), this->cn.otl_connection());
				}
				this->gen_new_id_value();
				if(delta > 1)
				{
					auto sql = "alter sequence " + this->seq_name() + " increment by 1";
					data_nocommit_stream Stream(1, sql.c_str(), this->cn.otl_connection());
				}
			}
		}

		//проверить все метаданные таблицы
		void validate_table_with_content(std::vector<std::string>& retVal = std::vector<std::string>())
		{
			if(!check_table_exist())
			{
				retVal.push_back("“аблица не существует " + this->table_name());
				this->create_table();
			}
			else
			{
				this->validate_tab_columns(retVal);
			}
			validate_keys(retVal);
			validate_seq(retVal);
		}

	};
	template<typename meta_factory, typename ... Args>
	auto make_private_data(std::mutex& connectionMutex,
						   otl_connection_wrapper& cn, meta_factory& meta, Args&&... args)
	{
		return PrivateDataReslisation<meta_factory, decltype(std::tie(std::forward<Args>(args)...))>(connectionMutex, cn, meta, std::tie(std::forward<Args>(args)...));
	}
}