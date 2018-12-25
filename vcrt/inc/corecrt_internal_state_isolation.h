
#undef _DEFINE_SET_FUNCTION	//!!!?


#ifdef __cplusplus

extern "C++" {

namespace  __crt_state_management {

class scoped_global_state_reset {
public:
	scoped_global_state_reset() {}
	~scoped_global_state_reset() {}
	//!!!?
};

const size_t state_index_count = 1;
bool __cdecl initialize_global_state_isolation();
bool __cdecl uninitialize_global_state_isolation();
size_t __cdecl get_current_state_index();

template <typename T>
class dual_state_global {
public:
	dual_state_global()
	{}

	void initialize(const T& v) {
		m_v = v;
	}

	template <class F>
	void uninitialize(F fun) {
		fun(m_v);		//!!!?
	}

	T& value() {
		return m_v;
	}

	T *dangerous_get_state_array() {
		return &m_v;
	}
private:
	T m_v;
};


} // __crt_state_management::

} // "C++"

#endif // __cplusplus




