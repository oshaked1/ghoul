notrace void hide_module(void);
notrace void show_module(void);
notrace void unload_module(void);
notrace void free_allocations(void);

inline int register_hooks(void);
inline void unregister_hooks(void);