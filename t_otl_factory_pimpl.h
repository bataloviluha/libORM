#pragma once


#include "mine_structure/otl/traits/otl_types_definition.h"
#include "otl_wrapper/src/OTL_Wrapper.h"
#include <map>
#include <mutex>
#include <set>
#include <sstream>
#include "generators/otl/otl_generator_logic.h"
#include "utils/otl/vector_cursor_container.h"
#include "mine_structure/traits/types_definition.h"


template<typename TRealType, typename TRealReq> class TOtlFactoryPimpl
{
	public:
		using type_traits = db_aware_def<TRealType>;
		using parent_type_traits = mine_structure::std_def<typename TRealType::parent_class>;
		using type_req = mine_structure::std_requ_def_impl<typename TRealReq::parent_class>;

		using db_object_type = typename type_traits::db_object_type;
		using db_const_object_type = typename type_traits::db_object_const_type;
		using request_ptr_type = typename type_req::request_ptr_type;
		using request_type = typename type_req::request_type;
		using ptr_type = typename parent_type_traits::ptr_type;
		using const_ptr_type = typename parent_type_traits::ptr_const_type;
		using cursor_ptr_type = typename parent_type_traits::cursor_ptr_type;
		using db_ptr_type = typename type_traits::db_ptr_type;
		using db_weak_ptr_type = typename type_traits::db_weak_ptr_type;
		using db_const_ptr_type = typename type_traits::db_ptr_const_type;
		using id_type = typename type_traits::id_type;
		using db_ptr_list_type = typename type_traits::db_ptr_list_type;

		using ids_set = std::set<id_type>;

		using list_type = typename parent_type_traits::list_type;

		otl_connection_wrapper& cn;
		std::mutex& connectionMutex;
		std::map<id_type, db_weak_ptr_type> cache;

		size_t bufferSize = 100;

		std::string primaryOtlConstant = "int";

		TOtlFactoryPimpl(otl_connection_wrapper& _cn, std::mutex& _connectionMutex) :cn(_cn), connectionMutex(_connectionMutex)
		{
		}
		
		virtual std::string seq_name()const abstract;

		virtual std::string make_where(const request_type& req)const abstract;
		
		virtual std::string make_order(const request_type& req)const abstract;

		virtual void fill_from_query(data_nocommit_stream& str, const request_type& req)abstract;

		virtual db_ptr_type create_from_stream(data_nocommit_stream& str, const request_type& req)abstract;

		virtual db_ptr_type create()abstract;

		virtual std::string table_name()const abstract;

		virtual std::string select_fields_string()const abstract;
		
		virtual std::string primary_field_name()const abstract;

		virtual std::string make_update_sql(const db_const_ptr_type&)abstract;
		virtual std::string make_insert_sql(const db_const_ptr_type&)abstract;
		//virtual request_ptr_type new_request()abstract;

		virtual void fill_save_query(data_nocommit_stream& saveStream, const db_ptr_type& val)abstract;

		//Метод вызываемый перед сохранением объекта
		virtual void before_save(const db_ptr_type&){}
		//Метода вызываемый после сохранения объекта
		virtual void after_save(const db_ptr_type&) {}

		//Метод вызываемый перед удалением объета
		virtual void before_remove(const db_ptr_type&) {};
		//Метод вызываемый после удаления объекта, объект уже удален из кеша и у него убран id
		virtual void after_remove(const db_ptr_type&) {};

		//Метод вызывается после прогрузки очередного объекта из базы
		virtual void after_load_ptr(const db_ptr_type& ptr, const request_type& req)
		{
		}

		virtual std::string make_delete_row_sql(const db_const_ptr_type&)const
		{
			return "delete from " + table_name() + " where " + primary_field_name() + "=:id_value<" + primaryOtlConstant + ">";
		}

		virtual std::string make_select_row_sql()const
		{
			return "select " + select_fields_string() + " from " + from_part() + " where " + primary_field_name() + "=:id_value<" + primaryOtlConstant + ">";
		}

		virtual std::string from_part()const
		{
			return table_name();
		}

		virtual std::string make_select(const request_type& req)
		{
			std::string returnValue= "select " + select_fields_string() + " from " + from_part() + " where 1=1 " + make_where(req);
			std::string orderBy = make_order(req);
			if (orderBy!="")
			{
				returnValue += " order by " + orderBy;
			}
			//return "select " + select_fields_string() + " from " + from_part() + " where 1=1 " + make_where(req);
			return returnValue;
		}

		virtual std::string make_count_string(const request_type& req)
		{
			return "select count(1) from " + from_part() + " where 1=1 " + make_where(req);
		}

		virtual std::string make_select_id_string(const request_type& req)
		{
			return "select " + primary_field_name() + " from " + from_part() + " where 1=1 " + make_where(req);
		}

		virtual list_type list(const request_ptr_type& req = request_ptr_type())
		{
			return list(req ? *req : *this->new_request());
		}

		void reserve_collection(typename list_type::size_type count, std::vector<ptr_type>& collection)
		{
			collection.reserve(count);
		}

		template<typename ... TArgs> void reserve_collection(TArgs&& ...args)
		{

		}

		bool reserve_collection_for_request(const request_type& req,std::vector<ptr_type>& collection)
		{
			auto countRecords = count(req);
			if (countRecords == 0)
			{
				return false;
			}
			collection.reserve(countRecords);
			return true;
		}

		template<typename ... TArgs> bool reserve_collection_for_request(TArgs&& ...args)
		{
			return true;
		}

		virtual list_type list(const request_type& req)
		{
			list_type returnValue;
			if (!reserve_collection_for_request(req, returnValue))
			{
				return returnValue;
			}
			std::string selectSql= make_select(req);
			data_nocommit_stream selectStream(bufferSize, selectSql.c_str(), cn.otl_connection());
			fill_from_query(selectStream, req);
			while (selectStream.eof()!=1)
			{
				auto ptr = create_from_stream(selectStream, req);
				this->after_load_ptr(ptr, req);
				returnValue.push_back(std::move(ptr));
			}
			return returnValue;
		}

		virtual size_t count(const request_type& req)
		{
			std::string countSql = make_count_string(req);
			data_nocommit_stream selectStream(1, countSql.c_str(), cn.otl_connection());
			fill_from_query(selectStream, req);
			size_t returnValue = 0;
			selectStream >> returnValue;
			return returnValue;
		}

		cursor_ptr_type cursor(const request_ptr_type& req)
		{
			if (!req)
			{
				return nullptr;
			}
			std::string cursorSql = "begin open :open_cur for " + make_select(*req) + "; end;";
			auto queryHolder = std::make_unique<data_nocommit_stream>(1, cursorSql.c_str(), cn.otl_connection(), "open_cur");
			fill_from_query(*queryHolder, *req);
			return std::make_shared<TVectorCursorContainer<ptr_type> >(connectionMutex, queryHolder.release(), [thisProxy = this, req](data_nocommit_stream& stream)->ptr_type
			{
				auto ptr = thisProxy->create_from_stream(stream, *req);
				thisProxy->after_load_ptr(ptr, *req);
				return ptr;
			});

		}

		id_type gen_new_id_value()
		{
			std::string sql = "select " + seq_name() + ".NEXTVAL from dual";
			data_nocommit_stream selectStream(1, sql.c_str(), cn.otl_connection());
			id_type val;
			selectStream >> val;
			return val;
		}

		/*template<typename T>  void write_value_or_null(data_nocommit_stream& saveStream, const T* val)
		{
			if (val)
			{
				saveStream << *val;
			}
			else
			{
				saveStream << data_null();
			}
		}*/

		template<typename T,typename TStr> std::string add_if_not_null(T&& obj, TStr&& strValue)const
		{
			if (!obj)
			{
				return "";
			}
			auto dbAware = to_const_db_aware(obj);
			if (!dbAware||!dbAware->id())
			{
				return "";
			}
			return strValue;
		}

		/*template<typename T> void write_id_or_null(data_nocommit_stream& saveStream, T&& obj)
		{
			if (!obj)
			{
				saveStream << data_null();
				return;
			}
			auto dbAware = to_const_db_aware(obj);
			if (!dbAware || !dbAware->id())
			{
				saveStream << data_null();
				return;
			}
			saveStream << *dbAware->id();
		}*/

		void save(const list_type& lst)
		{
			for (auto& ptr : lst)
			{
				save(ptr);
			}
		}

		void save(const ptr_type& val)
		{
			if (!val)
			{
				return;
			}
			auto dbAware = std::dynamic_pointer_cast<db_object_type>(val);
			if (!dbAware)
			{
				return;
			}
			before_save(dbAware);
			std::string sql(dbAware->id() ? make_update_sql(dbAware) : make_insert_sql(dbAware));
			data_nocommit_stream saveStream(1, sql.c_str(), cn.otl_connection());
			fill_save_query(saveStream,dbAware);
			if (dbAware->id())
			{
				saveStream << *(dbAware->id());
				cache[*(dbAware->id())] = dbAware;
			}
			else
			{
				auto id = db_aware::null_id;
				saveStream >> id;
				cache.emplace(id, dbAware);
				dbAware->set_id(id);
			}
			after_save(dbAware);
		}

		void remove(const ptr_type& val)
		{
			if (!val)
			{
				return;
			}
			auto dbAware = std::dynamic_pointer_cast<db_object_type>(val);
			if (!dbAware||!dbAware->id())
			{
				return;
			}
			this->before_remove(dbAware);
			data_nocommit_stream removeStream(1, make_delete_row_sql(dbAware).c_str(), cn.otl_connection());
			removeStream << *dbAware->id();
			cache.erase(*dbAware->id());
			dbAware->set_null_id();
			this->after_remove(dbAware);
		}

		void remove(const list_type& lst)
		{
			for (auto& ptr:lst)
			{
				remove(ptr);
			}
		}

		void remove(const request_ptr_type& req)
		{
			if (!req)
			{
				return;
			}
			data_nocommit_stream selectStream(bufferSize, this->make_select_id_string(*req).c_str(), cn.otl_connection());
			fill_from_query(selectStream, *req);
			db_ptr_list_type objForDelete;
			while (!selectStream.eof())
			{
				id_type id;
				selectStream >> id;
				auto iter = cache.find(id);
				if (iter!= cache.end())
				{
					auto ptr = iter->second.lock();
					if (ptr)
					{
						this->before_remove(ptr);
						objForDelete.push_back(ptr);
					}
					cache.erase(iter);
				}
			}
			std::string deleteSql = "delete from " + this->from_part() + " where 1=1 " + make_where(*req);
			data_nocommit_stream deleteStream(1, deleteSql.c_str(), cn.otl_connection());
			fill_from_query(selectStream, *req);

			for (auto& item: objForDelete)
			{
				item->set_null_id();
				this->after_remove(item);
			}
		}

		void clear_cache()
		{
			cache.clear();
		}

		void clear_cache(const ptr_type& val)
		{
			if (!val)
			{
				return;
			}
			auto dbAware = std::dynamic_pointer_cast<db_object_type>(val);
			if (!dbAware||!dbAware->id())
			{
				return;
			}
			cache.erase(*dbAware->id());
		}

		void clear_cache(const list_type& need_list)
		{
			for (auto& ptr:need_list)
			{
				clear_cache(ptr);
			}
		}

		db_ptr_type load(id_type id)
		{
			if (id == db_aware::null_id)
			{
				return nullptr;
			}
			auto iter = cache.find(id);
			if (iter!= cache.end())
			{
				auto ptr = iter->second.lock();
				if(ptr)
					return ptr;
			}
			data_nocommit_stream selectStream(1, make_select_row_sql().c_str(), cn.otl_connection());
			auto empty_request = new_request();
			selectStream << id;
			return create_from_stream(selectStream, *empty_request);
		}

		db_ptr_list_type load(const ids_set& idSet)
		{
			if (idSet.empty())
			{
				return db_ptr_list_type();
			}
			db_ptr_list_type returnValue;
			reserve_collection(idSet.size(), returnValue);

			ids_set notFindIds;
			for (auto &id : idSet)
			{
				if (id == db_aware::null_id)
				{
					continue;
				}
				auto iter = cache.find(id);
				if (iter != cache.end())
				{
					auto ptr = iter->second.lock();
					if(ptr)
					{
						returnValue.push_back(ptr);
					}
					else
					{
						notFindIds.insert(id);
					}
				}
				else
				{
					notFindIds.insert(id);
				}
			}
			if (notFindIds.empty())
			{
				return returnValue;
			}
			auto selectFields = this->select_fields_string();
			auto primaryName = this->primary_field_name();
			if (selectFields==""|| primaryName=="")
			{
				return returnValue;
			}
			data_int_vec ids_array;
			ids_array.set_len(notFindIds.size());
			list_type::size_type i = 0;
			//for (auto iter = notFindIds.begin(); iter != notFindIds.end(); iter++)
			for(auto& id: notFindIds)
			{
				ids_array[i] = id;
				ids_array.set_non_null(i);
				i++;
			}
			std::stringstream selectByIdSql; 
			selectByIdSql << "begin open :cursor for select " << selectFields << " from " + from_part() + " where " << primaryName << " in (select column_value from table(:ids<int[" << notFindIds.size() << "]>); end;";

			data_nocommit_stream selectStream(bufferSize, selectByIdSql.str().c_str() , cn.otl_connection(),"cursor");
			selectStream << ids_array;
			auto empty_request = new_request();
			while (selectStream.eof() != 1)
			{
				returnValue.push_back(create_from_stream(selectStream, *empty_request));
			}
			return returnValue;
		}

		ptr_type clone(const const_ptr_type& ptr)const
		{
			if (!ptr)
			{
				return nullptr;
			}
			auto dbAware = std::dynamic_pointer_cast<db_const_object_type>(ptr);
			if (!dbAware)
			{
				return nullptr;
			}
			return std::make_shared<db_object_type>(*dbAware);
		}

		auto generator_logic(const request_ptr_type& request)
		{
			std::string selectSql = make_select(request ? *request : *this->new_request());
			data_nocommit_stream selectStream(bufferSize, selectSql.c_str(), cn.otl_connection());
			fill_from_query(selectStream, *request);

			return utils::make_generator_logic(std::move(selectStream),
										[thisProxy = this, request](data_nocommit_stream& str) ->ptr_type
			{
				auto ptr =  thisProxy->create_from_stream(str, *request);
				thisProxy->after_load_ptr(ptr, *request);
				return ptr;
			});
		}

		template<typename...TArg> request_ptr_type new_request(TArg&& ...args)const
		{
			return std::make_shared<TRealReq>(std::forward<TArg>(args)...);
		}
};
