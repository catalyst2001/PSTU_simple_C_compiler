/**
* Simple Virtual Machine Interpreter
* 
* 
* 
*/
#pragma once
#include "SVM.h"
#include <assert.h>
#include <string>
#include <map>
#include <mutex>
#include <thread>
#include <vector>

/**
* virtual machine complete statuses
*/
enum SVMI_STATUS {
	SVMI_STATUS_OK = 0,
	SVMI_STATUS_INVALID_PARAM,
	SVMI_STATUS_ABNORMAL_TERMINATE,
	SVMI_STATUS_INVALID_INSTRUCTION,
	SVMI_STATUS_FLOATING_POINT_DIVISION_BY_ZERO,
	SVMI_STATUS_INT_DIVISION_BY_ZERO,
	SVMI_STATUS_EXECUTION_HALTED,
	SVMI_STATUS_STACK_OVERFLOW,
	SVMI_STATUS_CALL_CONTEXT_STACK_OVERFLOW,
	SVMI_STATUS_IMPORT_INDEX_OUT_OF_BOUNDS
};

/* reference counter base class */
class SVMI_reference_counted
{
	size_t m_references;
protected:
	SVMI_reference_counted() : m_references(0) {}
	~SVMI_reference_counted() {}

	inline void use() {
		m_references++;
	}

	inline bool unuse() {
		if (!m_references)
			return false;

		m_references--;
		return true;
	}
};

struct SVMI_VCPU_registers
{
	SVMI_VCPU_registers() {
		memset(this, 0, sizeof(SVMI_VCPU_registers));
	}
	~SVMI_VCPU_registers() {}
	
	uint32_t SR; //state register
	union {
		struct {
			cell_t   A, B, C, D;
			cell_t   X, Y, Z, W;
			ucell_t  IP, SP, CS, DS, SS;
		};
		cell_t regs[4 + 4 + 5];
	};

	inline void IP_add(uint32_t v) { IP += v; }

	/* SR methods */
	inline void SR_reset() { SR = 0; }
	inline void SR_set_flags(cell_t val) {
		/* zero flag */
		if (!val)
			SR |= VMSRF_ZF;
		/* sign */
		if (val < 0)
			SR |= VMSRF_SF;
	}
	inline void SR_set_flags(float fval) {
		/* zero flag */
		if (fabsf(fval) < FLT_EPSILON)
			SR |= VMSRF_ZF;
		/* sign */
		if (fval < 0.f)
			SR |= VMSRF_SF;
	}
	inline bool SR_is_set(cell_t flag) {
		return SR & flag;
	}
};

/* base image info */
class SVMI_image_info
{
protected:
	uint8_t* m_pimage;
	uint8_t* m_pdataseg;
	uint8_t* m_pcodeseg;
	uint8_t* m_pimage_imports_table;
	size_t   m_nimage_size;
	size_t   m_ndata_size;
	size_t   m_ncode_size;
	size_t   m_nimage_imports_table_size;

public:
	SVMI_image_info() : m_pimage(nullptr), m_pdataseg(nullptr), m_pcodeseg(nullptr), m_pimage_imports_table(nullptr),
		m_nimage_size(0), m_ndata_size(0), m_ncode_size(0), m_nimage_imports_table_size(0) {}
	~SVMI_image_info() {}
	inline size_t   get_image_size() { return m_nimage_size; }
	inline size_t   get_data_size() { return m_ndata_size; }
	inline size_t   get_code_size() { return m_ncode_size; }
	inline size_t   get_num_imports() { return m_nimage_imports_table_size; }
	inline uint8_t* get_image_base() { return m_pimage; }
	inline uint8_t* get_data() { return m_pdataseg; }
	inline uint8_t* get_code() { return m_pcodeseg; }
	inline uint8_t* get_imports_table() { return m_pimage_imports_table; }
};

class SVMI;

struct SVMI_call_context {
	uint32_t return_address;
	uint32_t previous_SP;
};

/* for each thread SVMI context */
class SVMI_context : private SVMI_VCPU_registers, public SVMI_reference_counted
{
	friend class SVMI;
	std::vector<SVMI_call_context> m_call_contexts;

	/* stack */
	ucell_t   m_nstack_size;
	cell_t   *m_pstack;

protected:
	/* for SVMI */
	inline ucell_t  get_stack_size() { return m_nstack_size; }
	inline cell_t  *get_stack() { return m_pstack; }
	inline bool push_call_context(SVMI_call_context &callctx) {
		m_call_contexts.push_back(callctx);
		return true;
	}
	inline bool pop_call_context(SVMI_call_context& callctx) {
		if (!m_call_contexts.empty()) {
			callctx = m_call_contexts.back();
			m_call_contexts.pop_back();
			return true;
		}
		return false;	
	}
public:
	SVMI_context() {}
	~SVMI_context() {}

	SVMI_VCPU_registers* get_regs() {
		return this;
	}

	/* stack and calls */
	void call_push_arg_cell(int32_t cell) {
		m_pstack[SP] = cell;
		SP++;
	}
	void call_push_arg_address(void* p_address) {
		m_pstack[SP] = (int32_t)p_address; //HACKHACK: only 32-bit addresses currently support!
		SP++;
	}
	void call_finish() {
		SP = 0;
	}
};

class SVMI_image : public SVMI_image_info
{
public:
	SVMI_image();
	~SVMI_image();

	SVMI_STATUS load_from_disk(const char *p_filename);
	SVMI_STATUS load_from_memory(uint8_t *p_image_data, size_t image_size);
};

/* SVMI natives */
typedef cell_t (*SVMI_native)(SVMI_context *p_ctx);

/* debug proc */
enum SVMI_DBG_PROC_STATUS {
	SVMI_DBG_PROC_STATUS_CONTINUE = 0,
	SVMI_DBG_PROC_STATUS_STOP
};

typedef SVMI_DBG_PROC_STATUS (*SVMI_dbg_proc)(SVMI_context* p_ctx);

struct SVMI_native_decl
{
	SVMI_native p_nativefunc;
	const char* p_nativename;
};

/* main VM API */
class SVMI
{
	std::mutex                 m_mutex;
	std::vector<SVMI_context*> m_free_contexts;
	std::vector<SVMI_context*> m_busy_contexts;
	const SVMI_native_decl    *m_pnatives;
	SVMI_image_info           *m_pimage_info;
	std::vector<uint32_t>      m_vnatives_idxs;
	SVMI_dbg_proc              m_pdbg_proc;

	/* interpreter */
	SVMI_STATUS exec(SVMI_context *p_ctx);

public:
	SVMI();
	~SVMI();
	SVMI_STATUS   init(SVMI_image_info* p_image_info, const SVMI_native_decl* p_natives, SVMI_dbg_proc p_dbgproc);
	SVMI_STATUS   shutdown();

	/* context dependend calls */
	SVMI_context *ctx_create();
	SVMI_STATUS   call(SVMI_context* p_ctx, uint32_t vm_func_address);
	void          ctx_destroy(SVMI_context *p_ctx);
};

class SVMI_context_scope
{
	SVMI&         m_vmi;
	SVMI_context* p_ctx;
public:
	SVMI_context_scope(SVMI& vmi) : m_vmi(vmi) {
		p_ctx = m_vmi.ctx_create();
		assert(p_ctx && "context creating failed");
	}
	~SVMI_context_scope() {
		m_vmi.ctx_destroy(p_ctx);
	}

	inline SVMI_context* get_context() { return p_ctx; }
};