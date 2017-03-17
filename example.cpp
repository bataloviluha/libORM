class CProjectOMSOtlFactoryMeta
{
public:
	static auto getMeta()
	{
		return otl_utils::make_meta<CProjectOMSDBAware, CProjectOMSOtlRequest, 100>(
			"PROJECT_POMS", "ID_PROJECT_POMS", "S_PROJECT_POMS",
			std::make_tuple(
			otl_utils::make_field<int>("ID_GORN_UCH", &CProjectOMSDBAware::id_gorn_uch, &CProjectOMSDBAware::set_id_gorn_uch),
			otl_utils::make_field<int>("ID_VYR", &CProjectOMSDBAware::id_excavation, &CProjectOMSDBAware::set_id_excavation),
			otl_utils::make_field<int>("ID_PANEL", &CProjectOMSDBAware::id_panel, &CProjectOMSDBAware::set_id_panel),
			otl_utils::make_field<int>("ID_GORIZONT", &CProjectOMSDBAware::id_gorizont, &CProjectOMSDBAware::set_id_gorizont),
			otl_utils::make_field<int>("ID_PLAST", &CProjectOMSDBAware::id_plast, &CProjectOMSDBAware::set_id_plast),
			otl_utils::make_field<int>("ID_BLOCK", &CProjectOMSDBAware::id_block, &CProjectOMSDBAware::set_id_block),
			otl_utils::make_field<std::string, 100>("NOMER", &mine_structure::IProjectOMS::nomer, &mine_structure::IProjectOMS::set_nomer),
			otl_utils::make_field<int>("ID_MARK_OKMGR", &CProjectOMSDBAware::id_worker_okmgr, &CProjectOMSDBAware::set_id_worker_okmgr),
			otl_utils::make_field<int>("ID_MARK_UCH", &CProjectOMSDBAware::id_worker_uch, &CProjectOMSDBAware::set_id_worker_uch),
			otl_utils::make_field<std::string, 200>("NOTE", &mine_structure::IProjectOMS::comment, &mine_structure::IProjectOMS::set_comment),
			otl_utils::make_field<double>("OTSTAVANIE", &mine_structure::IProjectOMS::otstavanie, &mine_structure::IProjectOMS::set_otstavanie),
			otl_utils::make_field<int>("IS_OPAS_ZONA", &mine_structure::IProjectOMS::is_opas, &mine_structure::IProjectOMS::set_is_opas),
			otl_utils::make_field<boost::gregorian::date>("DATA", &mine_structure::IProjectOMS::date, &mine_structure::IProjectOMS::set_date),
			otl_utils::make_field<int>("ID_RUDNIK", &CProjectOMSDBAware::id_mine, &CProjectOMSDBAware::set_id_mine),
			otl_utils::make_field<int>("ID_KOMBAIN", &CProjectOMSDBAware::id_kombain, &CProjectOMSDBAware::set_id_kombain)
			),
			std::make_tuple(
			otl_utils::make_request<int>("ID_RUDNIK", &mine_structure::IProjectOMSRequest::mine),
			otl_utils::make_request<int>("ID_GORN_UCH", &mine_structure::IProjectOMSRequest::gorn_uch),
			otl_utils::make_request<int>("ID_PLAST", &mine_structure::IProjectOMSRequest::plast),
			otl_utils::make_request<int>("ID_PANEL", &mine_structure::IProjectOMSRequest::panel),
			otl_utils::make_request<int>("ID_BLOCK", &mine_structure::IProjectOMSRequest::block),
			otl_utils::make_request<boost::gregorian::date>("DATA", &mine_structure::IProjectOMSRequest::date_from,">=")
			));
	}
};
using CProjectOMSOtlFactoryType = otl_utils::PrivateDataReslisation<decltype(CProjectOMSOtlFactoryMeta::getMeta()),
	std::add_lvalue_reference_t<mine_otl_factory>,
	std::add_lvalue_reference_t<otl_gorn_uch_factory>,
	std::add_lvalue_reference_t<TLoadDbIterface<mine_structure::excavation>>,
	std::add_lvalue_reference_t<panel_otl_factory>,
	std::add_lvalue_reference_t<CGorizontOTLFactory>,
	std::add_lvalue_reference_t<plast_otl_factory>,
	std::add_lvalue_reference_t<block_otl_factory>,
	std::add_lvalue_reference_t<WorkerDBFactory>,
	std::add_lvalue_reference_t<CKombainOtlFactory>>;
class CProjectOMSOtlFactory::CPrivateData:public CProjectOMSOtlFactoryType
{
public:
	CPrivateData(std::mutex& mutex, otl_connection_wrapper& otl_connection,
				 mine_otl_factory& mineFact,
				 otl_gorn_uch_factory& gornUchFact,
				 TLoadDbIterface<mine_structure::excavation>& excavFact,
				 panel_otl_factory& panelFact,
				 CGorizontOTLFactory& gorizontFact,
				 plast_otl_factory& plastFact,
				 block_otl_factory& blockFact,
				 WorkerDBFactory& workerFact,
				 CKombainOtlFactory& kombainFact):
		CProjectOMSOtlFactoryType(mutex, otl_connection,
								  CProjectOMSOtlFactoryMeta::getMeta(), std::tie(mineFact, gornUchFact, excavFact, panelFact, gorizontFact, plastFact, blockFact,
								  workerFact, kombainFact))
	{

	}
};