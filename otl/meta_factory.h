#pragma once
#include <tuple>
#include "otl_wrapper\src\data_stream.h"
#include "field.h"
#include "domen.h"
namespace otl_utils
{
	template<typename TClass, typename TRequest, typename TTableName, typename TIDName, typename TSeqName,
		typename Fields, typename Request/*,
		typename Select,
		typename Insert,
		typename Update,
		typename Remove,
		std::size_t buffer*/>
		struct meta_factory
	{
	private:
		//make_req
		template<int N, typename TObjects, typename ... Args>
		struct requestStruct
		{
			static std::string get_where(const TObjects& object, const std::tuple<Args...>& tupple)
			{
				using type = typename std::tuple_element<N, std::tuple<Args...>>::type::type;
				using type_oracle = MappedTypeOTL<type>;
				static_assert(type_oracle::valid, "Unsupprted type!");
				std::string append;
				if(type_oracle::check_val(std::invoke(std::get<N>(tupple).getter, object)))
				{
					append = (
						make_string(" AND ") + std::get<N>(tupple).field_name + make_string(" ") + std::get<N>(tupple).oper + make_string(" :") + std::get<N>(tupple).field_name +
						make_string("<") + MappedTypeOTL<type>::value +
						std::get<N>(tupple).arr_size + make_string(">")).c_str();
				}
				return append +
					requestStruct<N - 1, TObjects, Args...>::get_where(object, tupple);
			}
		};
		template<typename TObjects, typename ... Args>
		struct requestStruct<-1, TObjects, Args...>
		{
			static std::string get_where(const TObjects& object, const std::tuple<Args...>& tupple)
			{
				return "";
			}
		};

		template<typename TObjects, typename ... Args>
		std::string get_where(const TObjects& object, const std::tuple<Args...>& tupple) const
		{
			return requestStruct<std::tuple_size<std::tuple<Args...>>::value - 1, TObjects, Args...>::get_where(object, tupple);
		}

		//write_req
		template<typename TObject, int N, typename ... Args>
		struct writeRequest
		{
			static void writeValue(const TObject& object, data_nocommit_stream& Stream, const std::tuple<Args...>& tupple)
			{
				using type = typename std::tuple_element<N, std::tuple<Args...>>::type::type;
				using type_oracle = MappedTypeOTL<type>;
				auto val = std::invoke(std::get<N>(tupple).getter, object);
				if(type_oracle::check_val(val))
					Stream << val;
				writeRequest<TObject, N - 1, Args...>::writeValue(object, Stream, tupple);
			}
		};
		template<typename TObject, typename ... Args>
		struct writeRequest<TObject, -1, Args...>
		{
			static void writeValue(const TObject& object, data_nocommit_stream& Stream, const std::tuple<Args...>& tupple)
			{

			}
		};
		template<typename T, typename ... ReqMeta>
		void fill(data_nocommit_stream& Stream, const T& request,
				  const std::tuple<ReqMeta...>& req) const
		{
			writeRequest<T, std::tuple_size<std::tuple<ReqMeta...>>::value - 1, ReqMeta...>::writeValue(request, Stream, req);
		}

		//read
		template<typename TObject, int N, typename ... Args>
		struct readStruct
		{
			static void readValue(TObject& object, data_nocommit_stream& Stream, const std::tuple<Args...>& tupple)
			{
				using type = typename std::tuple_element<N, std::tuple<Args...>>::type::type;

				type val;
				Stream >> val;
				if(!Stream.is_null())
					std::invoke(std::get<N>(tupple).setter, object, std::move(val));
				readStruct<TObject, N - 1, Args...>::readValue(object, Stream, tupple);
			}
		};
		template<typename TObject, typename ... Args>
		struct readStruct<TObject, -1, Args...>
		{
			static void readValue(TObject& object, data_nocommit_stream& Stream, const std::tuple<Args...>& tupple)
			{

			}
		};
		template<typename T, typename ... Args>
		auto read_from_stream(T& object, data_nocommit_stream& str, const std::tuple<Args...>& tupple) const
		{
			readStruct<typename DomenType::ClassName, std::tuple_size<std::tuple<Args...>>::value - 1, Args...>::readValue(*object, str, tupple);
		}

		//select
		template<int N, typename ... Args>
		struct selectStruct
		{
			constexpr static auto get_select(const std::tuple<Args...>& tupple)
			{
				return make_string(", ") + std::get<N>(tupple).field_name +
					selectStruct<N - 1, Args...>::get_select(tupple);
			}
		};
		template<typename ... Args>
		struct selectStruct<-1, Args...>
		{
			constexpr static auto get_select(const std::tuple<Args...>& tupple)
			{
				return make_string(" ");
			}
		};
		template<typename ... Args>
		constexpr auto generate_fields_string(const std::tuple<Args...>& tupple) const
		{
			return DomenDesc.id_name + selectStruct<std::tuple_size<std::tuple<Args...>>::value - 1, Args...>::get_select(tupple);
		}

		//update
		template<int N, typename ... Args>
		struct updateStruct
		{
		private:
			template<bool V>
			struct need
			{
				constexpr static auto value = make_string(" ");
			};
			template<>
			struct need<false>
			{
				constexpr static auto value = make_string(", ");
			};
		public:
			constexpr static auto get_update(const std::tuple<Args...>& tupple)
			{
				using type = typename std::tuple_element<N, std::tuple<Args...>>::type::type;
				static_assert(MappedTypeOTL<type>::valid, "Unsupprted type!");
				return need<N == std::tuple_size<std::tuple<Args...>>::value - 1>::value +
					std::get<N>(tupple).field_name + make_string(" = :") +
					std::get<N>(tupple).field_name +
					make_string("<") + MappedTypeOTL<type>::value +
					std::get<N>(tupple).arr_size + make_string(">") +
					updateStruct<N - 1, Args...>::get_update(tupple);
			}
		};
		template<typename ... Args>
		struct updateStruct<-1, Args...>
		{
			constexpr static auto get_update(const std::tuple<Args...>& tupple)
			{
				return make_string(" ");
			}
		};
		template<typename ... Args>
		constexpr auto gen_update_all_fields(const std::tuple<Args...>& tupple) const
		{
			return make_string("UPDATE ") + DomenDesc.table_name + make_string(" SET ") + updateStruct<std::tuple_size<std::tuple<Args...>>::value - 1, Args...>::get_update(tupple) +
				make_string("WHERE ") + DomenDesc.id_name + make_string(" = :id<") + MappedTypeOTL<db_aware::TIdValueType>::value + make_string(">");
		}

		//insert
		template<int N, typename ... Args>
		struct insertStruct
		{
		private:
		public:
			constexpr static auto get_insert(const std::tuple<Args...>& tupple)
			{
				using type = typename std::tuple_element<N, std::tuple<Args...>>::type::type;
				static_assert(MappedTypeOTL<type>::valid, "Unsupprted type!");

				return make_string(", :") + std::get<N>(tupple).field_name +
					make_string("<") + MappedTypeOTL<type>::value +
					std::get<N>(tupple).arr_size +
					make_string(",in>") +
					insertStruct<N - 1, Args...>::get_insert(tupple);
			}
		};
		template<typename ... Args>
		struct insertStruct<-1, Args...>
		{
			constexpr static auto get_insert(const std::tuple<Args...>& tupple)
			{
				return make_string(" ");
			}
		};
		template<typename ... Args>
		constexpr auto gen_insert_fields(const std::tuple<Args...>& tupple) const
		{
			return make_string("INSERT INTO ") + DomenDesc.table_name + make_string(" (") +
				generate_fields_string(tupple) + make_string(") VALUES( ") +
				DomenDesc.seq_name + make_string(".nextval") + insertStruct<std::tuple_size<std::tuple<Args...>>::value - 1, Args...>::get_insert(tupple) +
				make_string(") returning ") + DomenDesc.id_name + make_string(" into :id<") + MappedTypeOTL<db_aware::TIdValueType>::value + make_string(",out>");
		}

		//save
		template<typename TObject, int N, typename ... Args>
		struct writeStruct
		{
			static void writeValue(TObject& object, data_nocommit_stream& curStream, const std::tuple<Args...>& tupple)
			{
				curStream << std::invoke(std::get<N>(tupple).getter, object);
				writeStruct<TObject, N - 1, Args...>::writeValue(object, curStream, tupple);
			}
		};
		template<typename TObject, typename ... Args>
		struct writeStruct<TObject, -1, Args...>
		{
			static void writeValue(TObject& object, data_nocommit_stream& curStream, const std::tuple<Args...>& tupple)
			{

			}
		};
		template<typename T, typename ... Args>
		void fill_save_query(T& object, data_nocommit_stream& str, const std::tuple<Args...>& tupple) const
		{
			writeStruct<typename DomenType::ClassName, std::tuple_size<std::tuple<Args...>>::value - 1, Args...>::writeValue(object, str, tupple);
		}

		//create table
		template<int N, typename ... Args>
		struct createTable
		{
			template<typename TFieldType, typename TFieldName,
				typename TGetter,
				typename TSetter,
				std::size_t _Size,
				std::size_t _Scale,
				bool _notNull,
				bool _unique>
				static auto append(const FieldDescriptor<TFieldType, TFieldName, TGetter, TSetter, _Size, _Scale, _notNull, _unique>& field)
			{
				using oracle_type = MappedTypeOracle<TFieldType, _Size, _Scale, _notNull>;
				static_assert(oracle_type::valid, "Unsupprted type!");

				return make_string(", ") + field.field_name + make_string(" ") + oracle_type::value() + oracle_type::not_null();
			}
			static auto get(const std::tuple<Args...>& tupple)
			{
				return append(std::get<N>(tupple)) +
					createTable<N - 1, Args...>::get(tupple);
			}
		};
		template<typename ... Args>
		struct createTable<-1, Args...>
		{
			static auto get(const std::tuple<Args...>& tupple)
			{
				return make_string(" ");
			}
		};
		template<typename ... Args>
		auto generate_table_fields(const std::tuple<Args...>& tupple) const
		{
			return createTable<std::tuple_size<std::tuple<Args...>>::value - 1, Args...>::get(tupple);
		}

		//add fields
		template<int N, typename T, typename ... Args>
		struct alterTableAddField
		{
			template<typename TFieldType, typename TFieldName,
				typename TGetter,
				typename TSetter,
				std::size_t _Size,
				std::size_t _Scale,
				bool _notNull,
				bool _unique, typename T>
				static auto check(const FieldDescriptor<TFieldType, TFieldName, TGetter, TSetter, _Size, _Scale, _notNull, _unique>& field, T&& val)
			{
				using oracle_type = MappedTypeOracle<TFieldType, _Size, _Scale, _notNull>;
				static_assert(oracle_type::valid, "Unsupprted type!");

				return std::get<1>(val) != oracle_type::type().c_str() ||
					std::get<2>(val) != oracle_type::len() ||
					std::get<3>(val) != oracle_type::prec() ||
					std::get<4>(val) != oracle_type::scale() ||
					std::get<5>(val) != oracle_type::is_not_null();
			}
			template<typename TFieldType, typename TFieldName,
				typename TGetter,
				typename TSetter,
				std::size_t _Size,
				std::size_t _Scale,
				bool _notNull,
				bool _unique>
				static auto get_str(const FieldDescriptor<TFieldType, TFieldName, TGetter, TSetter, _Size, _Scale, _notNull, _unique>& field)
			{
				using oracle_type = MappedTypeOracle<TFieldType, _Size, _Scale, _notNull>;
				static_assert(oracle_type::valid, "Unsupprted type!");

				return field.field_name + make_string(" ") + oracle_type::value() + oracle_type::not_null();
			}

			static auto add(otl_connection_wrapper& cn, T&& tup, const std::tuple<Args...>& tupple,const std::string& TName, std::vector<std::string>& retVal)
			{
				auto iter_find = std::find_if(tup.cbegin(), tup.cend(), [pName = std::get<N>(tupple).field_name.c_str()](T::const_reference val)
				{
					return std::get<0>(val) == pName;
				});
				if(iter_find == tup.cend())
				{
					retVal.push_back(std::string("Отсутсвует поле ") + TName + "." + std::get<N>(tupple).field_name.c_str());
					std::string sqlAddField = " alter table ";
					sqlAddField += TName;
					sqlAddField += " add (";
					sqlAddField += get_str(std::get<N>(tupple)).c_str();
					sqlAddField += ")";
					data_nocommit_stream Stream(1, sqlAddField.c_str(), cn.otl_connection());
				}
				else if(check(std::get<N>(tupple), *iter_find))
				{
					retVal.push_back(std::string("Поле ") + TName + "." + std::get<N>(tupple).field_name.c_str() + " необходимо модифицировать");
					std::string sqlAddField = " alter table ";
					sqlAddField += TName;
					sqlAddField += " modify (";
					sqlAddField += get_str(std::get<N>(tupple)).c_str();
					sqlAddField += ")";
					data_nocommit_stream Stream(1, sqlAddField.c_str(), cn.otl_connection());
				}
				alterTableAddField<N - 1, T, Args...>::add(cn, std::forward<T>(tup), tupple, TName, retVal);
			}
		};
		template<typename T, typename ... Args>
		struct alterTableAddField<-1, T, Args...>
		{
			static auto add(otl_connection_wrapper& cn, T&& tup, const std::tuple<Args...>& tupple, const std::string& TName, std::vector<std::string>& retVal)
			{
				
			}
		};
		template<typename T,typename ... Args>
		auto validate_tab_columns(otl_connection_wrapper& cn, T&& tup, const std::tuple<Args...>& tupple, std::vector<std::string>& retVal) const
		{
			return alterTableAddField<std::tuple_size<std::tuple<Args...>>::value - 1, T, Args...>::add(cn, std::forward<T>(tup), tupple, this->DomenDesc.table_name.c_str(), retVal);
		}
	public:
		using DomenType = TDomenDescription<TClass, TRequest, TTableName, TIDName, TSeqName>;
		DomenType DomenDesc;


		Fields listFields;

		Request request;

		/*Select selectSql;
		Insert insertSql;
		Update updateSql;
		Remove deleteSql;*/

		constexpr meta_factory(TTableName table_name,
							   TIDName id_name,
							   TSeqName seq_name,
							   Fields fields_, Request request_):
			DomenDesc(make_desc<TClass, TRequest>(table_name, id_name, seq_name)),
			listFields(fields_), request(request_)/*,
			selectSql(otl_utils::gen_select(DomenDesc, listFields)),
			insertSql(otl_utils::gen_insert(DomenDesc, listFields)),
			updateSql(otl_utils::gen_update_all(DomenDesc, listFields)),
			deleteSql(otl_utils::gen_delete(DomenDesc))*/
		{

		}
		constexpr auto size_buffer() const noexcept
		{
			return (buffer);
		}
		template<typename Map, typename Req, typename ...TArgs>
		void read_list(otl_connection_wrapper& otl_connection, Map& map, const Req& req, TArgs&&... args)
		{
			return lst = otl_utils::read_list(DomenDesc, otl_connection, map, selectSql,
											  listFields, request, req, size_buffer(),
											  std::forward<TArgs>(args)...);
		}
		template<typename T>
		std::string make_where(const T& req) const
		{
			return this->get_where(req, this->request);
		}
		template<typename T>
		void fill_from_query(data_nocommit_stream& Stream, const T& req) const
		{
			this->fill(Stream, req, this->request);
		}

		template<typename T>
		void create_from_stream(T& object, data_nocommit_stream& str) const
		{
			this->read_from_stream(object, str, this->listFields);
		}

		constexpr auto get_fields_string() const
		{
			return generate_fields_string(listFields);
		}

		constexpr auto get_update_string() const
		{
			return gen_update_all_fields(listFields);
		}

		constexpr auto get_insert_string() const
		{
			return gen_insert_fields(listFields);
		}

		template<typename T>
		void fill_save_query(T& object, data_nocommit_stream& str) const
		{
			this->fill_save_query(*object, str, this->listFields);
		}

		constexpr auto get_list_fields_for_create_table() const
		{
			return this->generate_table_fields(listFields);
		}

		template<typename T>
		void validate_tab_columns(otl_connection_wrapper& cn, T&& tup, std::vector<std::string>& retVal)
		{
			this->validate_tab_columns(cn, std::forward<T>(tup), listFields, retVal);
		}
	};

	template<typename TClass, typename TRequest, std::size_t buffer, int TTableNameSize, int TIDNameSize, int TSeqNameSize,
		typename ... Fields, typename ... Request>
		constexpr auto make_meta(char const (&table_name)[TTableNameSize],
								 char const (&id_name)[TIDNameSize],
								 char const (&seq_name)[TSeqNameSize],
								 std::tuple<Fields...> fields_, std::tuple<Request...> request_)
	{
		return meta_factory<TClass, TRequest, string<TTableNameSize - 1>, string<TIDNameSize - 1>, string<TSeqNameSize - 1>,
			std::tuple<Fields...>, std::tuple<Request...>>(
				string<TTableNameSize - 1>(table_name), string<TIDNameSize - 1>(id_name), string<TSeqNameSize - 1>(seq_name), fields_, request_);
	}

	template<typename TClass, typename TRequest, std::size_t buffer, int TTableNameSize, int TIDNameSize, int TSeqNameSize,
		typename ... Fields>
		constexpr auto make_meta(char const (&table_name)[TTableNameSize],
								 char const (&id_name)[TIDNameSize],
								 char const (&seq_name)[TSeqNameSize],
								 std::tuple<Fields...> fields_)
	{
		return meta_factory<TClass, TRequest, string<TTableNameSize - 1>, string<TIDNameSize - 1>, string<TSeqNameSize - 1>,
			std::tuple<Fields...>, std::tuple<>>(
				string<TTableNameSize - 1>(table_name), string<TIDNameSize - 1>(id_name), string<TSeqNameSize - 1>(seq_name), fields_, std::tuple<>());
	}
}