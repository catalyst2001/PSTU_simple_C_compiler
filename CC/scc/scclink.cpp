#include "scclink.h"

/* linkers manager */
class scclinkmgr : public iscclinkmgr
{
	size_t    n_nlinkers;
	iscclink* m_registred_linkers[SCC_MAX_IMAGE_LINKERS];
public:
	scclinkmgr();
	~scclinkmgr();

	virtual iscclink* get_linker(const char* p_linker);
	virtual SCCLINK_STATUS  register_linker(iscclink* p_linker);
} linkersmgr;

/* get linker for main class */
iscclinkmgr* scc_get_linkmgr()
{
	return &linkersmgr;
}

scclinkmgr::scclinkmgr()
{
	n_nlinkers = 0;
	memset(m_registred_linkers, 0, sizeof(m_registred_linkers));
}

scclinkmgr::~scclinkmgr()
{
}

iscclink* scclinkmgr::get_linker(const char* p_linker)
{
	for (size_t i = 0; i < n_nlinkers; i++) {
		if (!stricmp(m_registred_linkers[i]->get_name(), p_linker)) {
			return m_registred_linkers[i];
		}
	}
	return nullptr;
}

SCCLINK_STATUS scclinkmgr::register_linker(iscclink* p_linker)
{
	/* chjeck image linkers count */
	if (n_nlinkers >= SCC_MAX_IMAGE_LINKERS) {
		cc_fatal("exceeded SCC_MAX_IMAGE_LINKERS limit in registreator!");
		return SCCLINK_STATUS_LINKERS_LIMIT_EXCEEDED;
	}
	m_registred_linkers[n_nlinkers++] = p_linker;
	return SCCLINK_STATUS_OK;
}
