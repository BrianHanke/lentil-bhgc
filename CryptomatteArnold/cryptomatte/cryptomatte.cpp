#include "cryptomatte.h"
#include "MurmurHash3.h"
#include <ai.h>

AtMutex g_crypto_mutex;

// User data names
const AtString CRYPTO_ASSET_UDATA("crypto_asset");
const AtString CRYPTO_OBJECT_UDATA("crypto_object");
const AtString CRYPTO_MATERIAL_UDATA("crypto_material");

const AtString CRYPTO_ASSET_OFFSET_UDATA("crypto_asset_offset");
const AtString CRYPTO_OBJECT_OFFSET_UDATA("crypto_object_offset");
const AtString CRYPTO_MATERIAL_OFFSET_UDATA("crypto_material_offset");

// AtString to cache
const AtString aStr_aov_crypto_asset("aov_crypto_asset");
const AtString aStr_aov_crypto_material("aov_crypto_material");
const AtString aStr_aov_crypto_object("aov_crypto_object");
const AtString aStr_compression("compression");
const AtString aStr_create_depth_outputs("create_depth_outputs");
const AtString aStr_cryptomatte_depth("cryptomatte_depth");
const AtString aStr_cryptomatte_filter("cryptomatte_filter");
const AtString aStr_custom_attributes("custom_attributes");
const AtString aStr_custom_output_driver("custom_output_driver");
const AtString aStr_driver_exr("driver_exr");
const AtString aStr_dwaa("dwaa");
const AtString aStr_dwab("dwab");
const AtString aStr_filename("filename");
const AtString aStr_filter("filter");
const AtString aStr_half_precision("half_precision");
const AtString aStr_list_aggregate("list_aggregate");
const AtString aStr_noop("noop");
const AtString aStr_outputs("outputs");
const AtString aStr_preview_in_exr("preview_in_exr");
const AtString aStr_process_legacy("process_legacy");
const AtString aStr_process_mat_path_pipes("process_mat_path_pipes");
const AtString aStr_process_maya("process_maya");
const AtString aStr_process_obj_path_pipes("process_obj_path_pipes");
const AtString aStr_process_paths("process_paths");
const AtString aStr_rank("rank");
const AtString aStr_rle("rle");
const AtString aStr_shader("shader");
const AtString aStr_sidecar_manifests("sidecar_manifests");
const AtString aStr_strip_mat_namespaces("strip_mat_namespaces");
const AtString aStr_strip_obj_namespaces("strip_obj_namespaces");
const AtString aStr_user_crypto_aov_0("user_crypto_aov_0");
const AtString aStr_user_crypto_aov_1("user_crypto_aov_1");
const AtString aStr_user_crypto_aov_2("user_crypto_aov_2");
const AtString aStr_user_crypto_aov_3("user_crypto_aov_3");
const AtString aStr_user_crypto_src_0("user_crypto_src_0");
const AtString aStr_user_crypto_src_1("user_crypto_src_1");
const AtString aStr_user_crypto_src_2("user_crypto_src_2");
const AtString aStr_user_crypto_src_3("user_crypto_src_3");
const AtString aStr_width("width");
const AtString aStr_zip("zip");


uint8_t g_pointcloud_instance_verbosity = 0; // to do: remove this.

CryptomatteCache CRYPTOMATTE_CACHE[AI_MAX_THREADS];
