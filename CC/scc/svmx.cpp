#include "svmx.h"
#include <string>

svmx_parser::svmx_parser(uint8_t* p_img_data, uint32_t n_data_size) : m_pbase(p_img_data), m_nimage_size(n_data_size)
{
}

svmx_parser::~svmx_parser()
{
}

SVMXP_STATUS svmx_parser::find_image_exports(uint32_t& dst_vmaddr, const char* p_procname)
{
	uint32_t            n_exports;
	svmx_export_info_s *p_exps;
	uint8_t            *p_dataseg;
	svmx_header_s      *p_header = (svmx_header_s*)m_pbase;

	/* check signatuer */
	if (!p_header->is_valid_signature())
		return SVMXP_STATUS_INVALID_SIGNATURE;

	/* get exports chunk */
	svmx_data_chunk_s &export_chunk = p_header->get_data_chunk(SVMX_DATA_CHUNK_EXPORTS);
	if (!export_chunk.get_size()) {
		/* no data in chunk */
		return SVMXP_STATUS_EMPTY_CHUNK;
	}

	/* check for data in chunk is not multiple svmx_export_info_s size */
	if (!export_chunk.is_multiple(sizeof(svmx_export_info_s)))
		return SVMXP_STATUS_CHUNK_SIZE_NOT_MULTIPLE;

	/* check for offste out of bounds image */
	if (export_chunk.offset_is_in_image(m_nimage_size))
		return SVMXP_STATUS_CHUNK_OFFSET_OUT_OF_BOUNDS;

	/* get data segment */
	svmx_data_chunk_s& dataseg_chunk = p_header->get_data_chunk(SVMX_DATA_CHUNK_EXPORTS);
	if (!dataseg_chunk.get_size()) {
		/* no data in chunk */
		return SVMXP_STATUS_EMPTY_CHUNK;
	}

	/* check for offste out of bounds image */
	if (dataseg_chunk.offset_is_in_image(m_nimage_size))
		return SVMXP_STATUS_CHUNK_OFFSET_OUT_OF_BOUNDS;

	p_dataseg = (uint8_t*)&m_pbase[dataseg_chunk.get_offset()];

	/* get num exports */
	n_exports = export_chunk.get_size() / sizeof(svmx_export_info_s);
	p_exps = (svmx_export_info_s*)&m_pbase[export_chunk.get_offset()];
	for (uint32_t i = 0; i < n_exports; i++) {
		svmx_export_info_s* p_curr_exp = &p_exps[i];
		if (!strcmp(p_curr_exp->get_name_string(p_dataseg), p_procname)) {
			dst_vmaddr = p_curr_exp->get_func_address();
			/*( image export found*/
			return SVMXP_STATUS_OK;
		}
	}
	/* image export with this name not found */
	return SVMXP_STATUS_EMPTY_NOT_FOUND;
}
