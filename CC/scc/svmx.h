/**
* simple virtual machine
* 
* executable file format structures
* 
*/
#pragma once
#include <stdint.h>
#include <assert.h>

// SVMX_IMAGE_SIGNATURE is 'SVMX'
#define SVMX_IMAGE_SIGNATURE 0x584D5653

/**
* POD types for file
*/

/* image data chunk types */
enum SVMX_DATA_CHUNK : uint32_t {
	SVMX_DATA_CHUNK_CODE = 0,
	SVMX_DATA_CHUNK_DATA,
	SVMX_DATA_CHUNK_IMPORTS,
	SVMX_DATA_CHUNK_EXPORTS,
	SVMX_DATA_CHUNK_GLOBALVARS,

	SVMX_DATA_MAX_CHUNKS //reserved
};

/* image data chunk */
class svmx_data_chunk_s
{
	uint32_t m_noffset;
	uint32_t m_nsize;
public:
	svmx_data_chunk_s() : m_noffset(0), m_nsize(0) {}
	~svmx_data_chunk_s() {}
	inline uint32_t get_offset() { return m_noffset; }
	inline uint32_t get_size() { return m_nsize; }

	inline bool is_multiple(uint32_t data_size) {
		return !(get_size() % data_size);
	}
	inline bool offset_is_in_image(uint32_t image_size) {
		return get_offset() < image_size;
	}

	inline bool is_valid(uint32_t image_size) {
		return offset_is_in_image(image_size);
	}
	inline bool is_valid(uint32_t image_size, uint32_t data_size) {
		return offset_is_in_image(image_size) && is_multiple(data_size);
	}
};

/* file header */
class svmx_header_s {
	uint32_t          m_nmagic;
	uint32_t          m_nversion;
	svmx_data_chunk_s m_image_data_chunks[SVMX_DATA_MAX_CHUNKS];
public:
	svmx_header_s() : m_nmagic(SVMX_IMAGE_SIGNATURE), m_nversion(1) {}
	~svmx_header_s() {}

	inline bool is_valid_signature() {
		return m_nmagic == SVMX_IMAGE_SIGNATURE;
	}

	inline svmx_data_chunk_s& get_data_chunk(SVMX_DATA_CHUNK ichk) {
		assert(ichk < SVMX_DATA_MAX_CHUNKS && "data chunk index out of bounds");
		return m_image_data_chunks[ichk];
	}
};

/* image exports info struct */
class svmx_export_info_s
{
	uint32_t m_name_addr;
	uint32_t m_func_addr;
public:
	svmx_export_info_s() :m_name_addr(0), m_func_addr(0) {}
	~svmx_export_info_s() {}
	inline uint32_t get_name_address() { return m_name_addr; }
	inline uint32_t get_func_address() { return m_func_addr; }
	inline const char* get_name_string(uint8_t *p_dataseg_base) {
		return (const char*)&p_dataseg_base[get_name_address()];
	}
};

/**
* main parser file
* 
* 
*/
enum SVMXP_STATUS {
	SVMXP_STATUS_OK = 0,
	SVMXP_STATUS_INVALID_SIGNATURE,
	SVMXP_STATUS_CHUNK_SIZE_NOT_MULTIPLE,
	SVMXP_STATUS_CHUNK_OFFSET_OUT_OF_BOUNDS,
	SVMXP_STATUS_EMPTY_CHUNK,
	SVMXP_STATUS_EMPTY_NOT_FOUND
};

class svmx_parser
{
	uint8_t* m_pbase;
	uint32_t m_nimage_size;
public:
	svmx_parser(uint8_t *p_img_data, uint32_t n_data_size);
	~svmx_parser();

	SVMXP_STATUS find_image_exports(uint32_t &dst_vmaddr, const char *p_procname);
};

