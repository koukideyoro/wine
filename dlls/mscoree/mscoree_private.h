/*
 *
 * Copyright 2008 Alistair Leslie-Hughes
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef __MSCOREE_PRIVATE__
#define __MSCOREE_PRIVATE__

extern char *WtoA(LPCWSTR wstr);

extern HRESULT CLRMetaHost_CreateInstance(REFIID riid, void **ppobj);

extern HRESULT WINAPI CLRMetaHost_GetVersionFromFile(ICLRMetaHost* iface,
    LPCWSTR pwzFilePath, LPWSTR pwzBuffer, DWORD *pcchBuffer);

typedef struct tagASSEMBLY ASSEMBLY;

HRESULT assembly_create(ASSEMBLY **out, LPCWSTR file);
HRESULT assembly_release(ASSEMBLY *assembly);
HRESULT assembly_get_runtime_version(ASSEMBLY *assembly, LPSTR *version);

typedef struct RuntimeHost RuntimeHost;

typedef struct CLRRuntimeInfo
{
    const struct ICLRRuntimeInfoVtbl *ICLRRuntimeInfo_vtbl;
    LPCWSTR mono_libdir;
    DWORD major;
    DWORD minor;
    DWORD build;
    int mono_abi_version;
    WCHAR mono_path[MAX_PATH];
    WCHAR mscorlib_path[MAX_PATH];
    struct RuntimeHost *loaded_runtime;
} CLRRuntimeInfo;

extern HRESULT get_runtime_info(LPCWSTR exefile, LPCWSTR version, LPCWSTR config_file,
    DWORD startup_flags, DWORD runtimeinfo_flags, BOOL legacy, ICLRRuntimeInfo **result);

extern HRESULT force_get_runtime_info(ICLRRuntimeInfo **result);

extern HRESULT ICLRRuntimeInfo_GetRuntimeHost(ICLRRuntimeInfo *iface, RuntimeHost **result);

extern HRESULT MetaDataDispenser_CreateInstance(IUnknown **ppUnk);

typedef struct parsed_config_file
{
    struct list supported_runtimes;
} parsed_config_file;

typedef struct supported_runtime
{
    struct list entry;
    LPWSTR version;
} supported_runtime;

extern HRESULT parse_config_file(LPCWSTR filename, parsed_config_file *result);

extern void free_parsed_config_file(parsed_config_file *file);

/* Mono embedding */
typedef struct _MonoDomain MonoDomain;
typedef struct _MonoAssembly MonoAssembly;
typedef struct _MonoAssemblyName MonoAssemblyName;
typedef struct _MonoType MonoType;
typedef struct _MonoImage MonoImage;
typedef struct _MonoClass MonoClass;
typedef struct _MonoObject MonoObject;
typedef struct _MonoMethod MonoMethod;
typedef struct _MonoProfiler MonoProfiler;

typedef enum {
	MONO_IMAGE_OK,
	MONO_IMAGE_ERROR_ERRNO,
	MONO_IMAGE_MISSING_ASSEMBLYREF,
	MONO_IMAGE_IMAGE_INVALID
} MonoImageOpenStatus;

typedef MonoAssembly* (*MonoAssemblyPreLoadFunc)(MonoAssemblyName *aname, char **assemblies_path, void *user_data);

typedef void (*MonoProfileFunc)(MonoProfiler *prof);

typedef struct loaded_mono
{
    HMODULE mono_handle;
    HMODULE glib_handle;

    BOOL is_started;
    BOOL is_shutdown;

    MonoImage* (CDECL *mono_assembly_get_image)(MonoAssembly *assembly);
    MonoAssembly* (CDECL *mono_assembly_open)(const char *filename, MonoImageOpenStatus *status);
    MonoClass* (CDECL *mono_class_from_mono_type)(MonoType *type);
    MonoClass* (CDECL *mono_class_from_name)(MonoImage *image, const char* name_space, const char *name);
    MonoMethod* (CDECL *mono_class_get_method_from_name)(MonoClass *klass, const char *name, int param_count);
    void (CDECL *mono_config_parse)(const char *filename);
    MonoAssembly* (CDECL *mono_domain_assembly_open) (MonoDomain *domain, const char *name);
    void (CDECL *mono_free)(void *);
    void (CDECL *mono_install_assembly_preload_hook)(MonoAssemblyPreLoadFunc func, void *user_data);
    int (CDECL *mono_jit_exec)(MonoDomain *domain, MonoAssembly *assembly, int argc, char *argv[]);
    MonoDomain* (CDECL *mono_jit_init)(const char *file);
    int (CDECL *mono_jit_set_trace_options)(const char* options);
    MonoDomain* (CDECL *mono_object_get_domain)(MonoObject *obj);
    MonoObject* (CDECL *mono_object_new)(MonoDomain *domain, MonoClass *klass);
    void* (CDECL *mono_object_unbox)(MonoObject *obj);
    void (CDECL *mono_profiler_install)(MonoProfiler *prof, MonoProfileFunc shutdown_callback);
    MonoType* (CDECL *mono_reflection_type_from_name)(char *name, MonoImage *image);
    MonoObject* (CDECL *mono_runtime_invoke)(MonoMethod *method, void *obj, void **params, MonoObject **exc);
    void (CDECL *mono_runtime_object_init)(MonoObject *this_obj);
    void (CDECL *mono_runtime_quit)(void);
    void (CDECL *mono_runtime_set_shutting_down)(void);
    void (CDECL *mono_set_dirs)(const char *assembly_dir, const char *config_dir);
    char* (CDECL *mono_stringify_assembly_name)(MonoAssemblyName *aname);
    void (CDECL *mono_thread_pool_cleanup)(void);
    void (CDECL *mono_thread_suspend_all_other_threads)(void);
    void (CDECL *mono_threads_set_shutting_down)(void);
} loaded_mono;

/* loaded runtime interfaces */
extern void unload_all_runtimes(void);

extern void expect_no_runtimes(void);

extern HRESULT RuntimeHost_Construct(const CLRRuntimeInfo *runtime_version,
    loaded_mono *loaded_mono, RuntimeHost** result);

extern HRESULT RuntimeHost_GetInterface(RuntimeHost *This, REFCLSID clsid, REFIID riid, void **ppv);

extern HRESULT RuntimeHost_GetIUnknownForObject(RuntimeHost *This, MonoObject *obj, IUnknown **ppUnk);

extern HRESULT RuntimeHost_CreateManagedInstance(RuntimeHost *This, LPCWSTR name,
    MonoDomain *domain, MonoObject **result);

extern HRESULT RuntimeHost_Destroy(RuntimeHost *This);

#endif   /* __MSCOREE_PRIVATE__ */
