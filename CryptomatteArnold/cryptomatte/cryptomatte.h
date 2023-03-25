/*
API Documentation:

This no longer needs to be built into every shader as in Arnold 4, thanks to AOV
shaders. If an alternate AOV shader
is made, it should should still interact with cryptomatte entirely through the
CryptomatteData struct.
How to add cryptomatte to a shader:

1. Add a member of type *CryptomatteData to your ShaderData

    node_initialize {
        CryptomatteData* data = new CryptomatteData();
        AiNodeSetLocalData(node, data);
    }

    The constructor sets up getting the global state of things, including
getting global
    options declared on the Arnold options.

2. in node_finish:

    node_finish {
        CryptomatteData* data =
            reinterpret_cast<CryptomatteData*>(AiNodeGetLocalData(node));
        delete data;
    }

3. in node_update:

    node_update {
        CryptomatteData* data =
            reinterpret_cast<CryptomatteData*>(AiNodeGetLocalData(node));

        // set cryptomatte depth (optional)
        data->set_option_channels(AiNodeGetInt(node, "cryptomatte_depth"),
                                CRYPTO_PREVIEWINEXR_DEFAULT);

        // set namespace options (optional - see CRYPTO_NAME_* flags)
        data->set_option_namespace_stripping(obj_flags, mat_flags);

        // set option for sidecar manifest (optional)
        data->set_manifest_sidecar(sidecar);

        AtArray* uc_aov_array = AiArray(
            4, 1, AI_TYPE_STRING, AiNodeGetStr(node, "user_crypto_aov_0").c_str(),
            AiNodeGetStr(node, "user_crypto_aov_1").c_str(),
            AiNodeGetStr(node, "user_crypto_aov_2").c_str(),
            AiNodeGetStr(node, "user_crypto_aov_3").c_str());
        AtArray* uc_src_array = AiArray(
            4, 1, AI_TYPE_STRING, AiNodeGetStr(node, "user_crypto_src_0").c_str(),
            AiNodeGetStr(node, "user_crypto_src_1").c_str(),
            AiNodeGetStr(node, "user_crypto_src_2").c_str(),
            AiNodeGetStr(node, "user_crypto_src_3").c_str());

        // does all the setup work. User cryptomatte arrays are optional (can be
        // nulls).
        // The three arguments are the names of the cryptomatte AOVs. If the
        // AOVs are active (connected to EXR drivers), this does all the
        // complicated setup work of creating multiple AOVs if necessary,
        // writing metadata, etc.
        // An arnold AtUniverse must be provided, so that we can find nodes
        // in the proper universe
        data->setup_all(universe,
                        AiNodeGetStr(node, "aov_crypto_asset"),
                        AiNodeGetStr(node, "aov_crypto_object"),
                        AiNodeGetStr(node, "aov_crypto_material"), uc_aov_array,
                        uc_src_array);
    }


4. in shader_evaluate:

    shader_evaluate {
        if (sg->Rt & AI_RAY_CAMERA && sg->sc == AI_CONTEXT_SURFACE) {
            CryptomatteData* data =
                reinterpret_cast<CryptomatteData*>(AiNodeGetLocalData(node));
            // This does all the hashing and writing values to AOVs, including
            // user-defined cryptomattes. Opacity does not need to be supplied.
            data->do_cryptomattes(sg);
        }
    }


*/

#pragma once

#include "MurmurHash3.h"
#include <ai.h>
#include <algorithm>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <string>
#include <unordered_set>
#include <vector>

#define NOMINMAX // lets you keep using std::min on windows

using String = std::string;

using ManifestMap = std::map<String, float>;

using StringVector = std::vector<String>;

///////////////////////////////////////////////
//
//      Constants
//
///////////////////////////////////////////////

//      For system controls
#define CRYPTO_DEPTH_DEFAULT 6
#define CRYPTO_STRIPOBJNS_DEFAULT true
#define CRYPTO_STRIPMATNS_DEFAULT true
#define CRYPTO_ICEPCLOUDVERB_DEFAULT 1
#define CRYPTO_SIDECARMANIFESTS_DEFAULT false
#define CRYPTO_PREVIEWINEXR_DEFAULT false

// System values
#define MAX_STRING_LENGTH 2048
#define MAX_CRYPTOMATTE_DEPTH 99
#define MAX_USER_CRYPTOMATTES 16

extern uint8_t g_pointcloud_instance_verbosity;

// Internal
#define CRYPTOMATTE_METADATA_SET_FLAG "already_has_crypto_metadata_"

// User data names
extern const AtString CRYPTO_ASSET_UDATA;
extern const AtString CRYPTO_OBJECT_UDATA;
extern const AtString CRYPTO_MATERIAL_UDATA;

extern const AtString CRYPTO_ASSET_OFFSET_UDATA;
extern const AtString CRYPTO_OBJECT_OFFSET_UDATA;
extern const AtString CRYPTO_MATERIAL_OFFSET_UDATA;

extern AtCritSec g_critsec;
extern bool g_critsec_active;

// Some static AtStrings to cache
const AtString aStr_shader("shader");
const AtString aStr_list_aggregate("list_aggregate");

// Name processing flags
using CryptoNameFlag = uint8_t;

// clang-format off
#define CRYPTO_NAME_NONE          0x00
#define CRYPTO_NAME_STRIP_NS      0x01 /* remove "namespace" */
#define CRYPTO_NAME_MAYA          0x02 /* mtoa style */
#define CRYPTO_NAME_PATHS         0x04 /* path based (starting with "/") */
#define CRYPTO_NAME_OBJPATHPIPES  0x08 /* pipes are considered in paths (c4d) */
#define CRYPTO_NAME_MATPATHPIPES  0x10 /* sitoa, old-c4d style */
#define CRYPTO_NAME_LEGACY        0x20 /* sitoa, old-c4d style */
#define CRYPTO_NAME_ALL           CryptoNameFlag(-1)
// clang-format on

///////////////////////////////////////////////
//
//      Crit sec utilities
//
///////////////////////////////////////////////

inline bool crypto_crit_sec_init() {
    // Called in node_plugin_initialize. Returns true as a convenience.
    g_critsec_active = true;
    AiCritSecInit(&g_critsec);
    return true;
}

inline void crypto_crit_sec_close() {
    // Called in node_plugin_cleanup
    g_critsec_active = false;
    AiCritSecClose(&g_critsec);
}

inline void crypto_crit_sec_enter() {
    // If the crit sec has not been inited since last close, we simply do not enter.
    // (Used by Cryptomatte filter.)
    if (g_critsec_active)
        AiCritSecEnter(&g_critsec);
}

inline void crypto_crit_sec_leave() {
    // If the crit sec has not been inited since last close, we simply do not enter.
    // (Used by Cryptomatte filter.)
    if (g_critsec_active)
        AiCritSecLeave(&g_critsec);
}

///////////////////////////////////////////////
//
//      String processing
//
///////////////////////////////////////////////

inline void safe_copy_to_buffer(char buffer[MAX_STRING_LENGTH], const char* c) {
    if (c)
        strncpy(buffer, c, std::min(strlen(c), (size_t)MAX_STRING_LENGTH - 1));
    else
        buffer[0] = '\0';
}

inline bool cstr_empty(const char* c) { return !c || c[0] == '\0'; }

///////////////////////////////////////////////
//
//      Name processing
//
///////////////////////////////////////////////

inline bool sitoa_pointcloud_instance_handling(const char* obj_full_name,
                                               char obj_name_out[MAX_STRING_LENGTH]) {
    if (g_pointcloud_instance_verbosity == 0 || !strstr(obj_full_name, ".SItoA.Instance.")) {
        return false;
    }
    char obj_name[MAX_STRING_LENGTH];
    safe_copy_to_buffer(obj_name, obj_full_name);

    char* instance_start = strstr(obj_name, ".SItoA.Instance.");
    if (!instance_start)
        return false;

    char* space = strstr(instance_start, " ");
    if (!space)
        return false;

    char* instance_name = &space[1];
    char* obj_suffix2 = strstr(instance_name, ".SItoA.");
    if (!obj_suffix2)
        return false;
    obj_suffix2[0] = '\0'; // strip the suffix
    size_t chars_to_copy = strlen(instance_name);
    if (chars_to_copy >= MAX_STRING_LENGTH || chars_to_copy == 0) {
        return false;
    }
    if (g_pointcloud_instance_verbosity == 2) {
        char* frame_numbers = &instance_start[16]; // 16 chars in ".SItoA.Instance.", this gets us
                                                   // to the first number
        char* instance_ID = strstr(frame_numbers, ".");
        if (!instance_ID)
            return false;
        char* instance_ID_end = strstr(instance_ID, " ");
        if (!instance_ID_end)
            return false;
        instance_ID_end[0] = '\0';
        size_t ID_len = strlen(instance_ID);
        strncpy(&instance_name[chars_to_copy], instance_ID, ID_len);
        chars_to_copy += ID_len;
    }

    strncpy(obj_name_out, instance_name, chars_to_copy);
    return true;
}

inline void mtoa_strip_namespaces(const char* obj_full_name, char obj_name_out[MAX_STRING_LENGTH]) {
    char* to = obj_name_out;
    size_t len = 0;
    size_t sublen = 0;
    const char* from = obj_full_name;
    const char* end = from + strlen(obj_full_name);
    const char* found = strchr(from, '|');
    const char* sep = nullptr;

    while (found) {
        sep = strchr(from, ':');
        if (sep && sep < found) {
            from = sep + 1;
        }
        sublen = found - from;
        memmove(to, from, sublen);
        to[sublen] = '|';

        len += sublen + 1;
        to += sublen + 1;
        from = found + 1;

        found = strchr(from, '|');
    }

    sep = strchr(from, ':');
    if (sep && sep < end) {
        from = sep + 1;
    }
    sublen = end - from;
    memmove(to, from, sublen);
    to[sublen] = '\0';
}

inline void get_clean_object_name(const char* obj_full_name, char obj_name_out[MAX_STRING_LENGTH],
                                  char ns_name_out[MAX_STRING_LENGTH], CryptoNameFlag flags) {
    if (flags == CRYPTO_NAME_NONE) {
        memmove(obj_name_out, obj_full_name, strlen(obj_full_name));
        strcpy(ns_name_out, "default");
        return;
    }

    char ns_name[MAX_STRING_LENGTH] = "";
    safe_copy_to_buffer(ns_name, obj_full_name);
    bool obj_already_done = false;

    const bool do_strip_ns = (flags & CRYPTO_NAME_STRIP_NS) != 0;
    const bool do_maya = (flags & CRYPTO_NAME_MAYA) != 0;
    const bool do_paths = (flags & CRYPTO_NAME_PATHS) != 0;
    const bool do_path_pipe = (flags & CRYPTO_NAME_OBJPATHPIPES) != 0;
    const bool do_legacy = (flags & CRYPTO_NAME_LEGACY) != 0;

    const uint8_t mode_maya = 0;
    const uint8_t mode_pathstyle = 1;
    const uint8_t mode_si = 2;
    const uint8_t mode_c4d = 3;

    uint8_t mode = mode_maya;
    if (ns_name[0] == '/') {
        // Path-style: /obj/hierarchy|obj_cache_hierarchy
        // For instance: /Null/Sphere
        //               /Null/Cloner|Null/Sphere1
        mode = mode_pathstyle;
    } else if (do_legacy && strncmp(ns_name, "c4d|", 4) == 0) {
        // C4DtoA prior 2.3: c4d|obj_hierarchy|...
        mode = mode_c4d;
        const char* nsp = ns_name + 4;
        size_t len = strlen(nsp);
        memmove(ns_name, nsp, len);
        ns_name[len] = '\0';
    } else if (do_legacy && strstr(ns_name, ".SItoA.")) {
        // in Softimage mode
        mode = mode_si;
        char* sitoa_suffix = strstr(ns_name, ".SItoA.");
        obj_already_done = sitoa_pointcloud_instance_handling(obj_full_name, obj_name_out);
        sitoa_suffix[0] = '\0'; // cut off everything after the start of .SItoA
    } else {
        mode = mode_maya;
    }

    char* nsp_separator = nullptr;
    if (mode == mode_c4d && do_legacy) {
        nsp_separator = strrchr(ns_name, '|');
    } else if (mode == mode_pathstyle && do_paths) {
        char* lastPipe = do_path_pipe ? strrchr(ns_name, '|') : nullptr;
        char* lastSlash = strrchr(ns_name, '/');
        nsp_separator = lastSlash > lastPipe ? lastSlash : lastPipe;
    } else if (mode == mode_si && do_legacy) {
        nsp_separator = strchr(ns_name, '.');
    } else if (mode == mode_maya && do_maya)
        nsp_separator = strchr(ns_name, ':');

    if (!obj_already_done) {
        if (!nsp_separator || !do_strip_ns) { // use whole name
            memmove(obj_name_out, ns_name, strlen(ns_name));
        } else if (mode == mode_maya) { // maya
            mtoa_strip_namespaces(ns_name, obj_name_out);
        } else { // take everything right of sep
            char* obj_name_start = nsp_separator + 1;
            memmove(obj_name_out, obj_name_start, strlen(obj_name_start));
        }
    }

    if (nsp_separator) {
        nsp_separator[0] = '\0';
        strcpy(ns_name_out, ns_name); // copy namespace
    } else {
        strcpy(ns_name_out, "default");
    }
}

inline void get_clean_material_name(const char* mat_full_name, char mat_name_out[MAX_STRING_LENGTH],
                                    CryptoNameFlag flags) {
    safe_copy_to_buffer(mat_name_out, mat_full_name);
    if (flags == CRYPTO_NAME_NONE)
        return;

    const bool do_strip_ns = (flags & CRYPTO_NAME_STRIP_NS) != 0;
    const bool do_maya = (flags & CRYPTO_NAME_MAYA) != 0;
    const bool do_paths = (flags & CRYPTO_NAME_PATHS) != 0;
    const bool do_strip_pipes = (flags & CRYPTO_NAME_MATPATHPIPES) != 0;
    const bool do_legacy = (flags & CRYPTO_NAME_LEGACY) != 0;

    // Path Style Names /my/mat/name|root_node_name
    if (do_paths && mat_name_out[0] == '/') {
        char* mat_name = do_strip_pipes ? strtok(mat_name_out, "|") : nullptr;
        mat_name = mat_name ? mat_name : mat_name_out;
        if (do_strip_ns) {
            char* ns_separator = strrchr(mat_name, '/');
            if (ns_separator)
                mat_name = ns_separator + 1;
        }
        if (mat_name != mat_name_out)
            memmove(mat_name_out, mat_name, strlen(mat_name) + 1);
        return;
    }

    // C4DtoA prior 2.3: c4d|mat_name|root_node_name
    if (do_legacy) {
        if (strncmp(mat_name_out, "c4d|", 4) == 0) {
            char* mat_name = strtok(mat_name_out + 4, "|");
            if (mat_name)
                memmove(mat_name_out, mat_name, strlen(mat_name) + 1);
            return;
        }
    }

    // For maya, you get something simpler, like namespace:my_material_sg.
    if (do_maya) {
        char* ns_separator = strchr(mat_name_out, ':');
        if (do_strip_ns && ns_separator) {
            ns_separator[0] = '\0';
            char* mat_name = ns_separator + 1;
            memmove(mat_name_out, mat_name, strlen(mat_name) + 1);
            return;
        }
    }

    // Softimage: Sources.Materials.myLibraryName.myMatName.Standard_Mattes.uBasic.SITOA.25000....
    if (do_legacy) {
        char* mat_postfix = strstr(mat_name_out, ".SItoA.");
        if (mat_postfix) {
            char* mat_name = mat_name_out;
            mat_postfix[0] = '\0';

            char* mat_shader_name = strrchr(mat_name, '.');
            if (mat_shader_name)
                mat_shader_name[0] = '\0';

            char* standard_mattes = strstr(mat_name, ".Standard_Mattes");
            if (standard_mattes)
                standard_mattes[0] = '\0';

            const char* prefix = "Sources.Materials.";
            char* mat_prefix_separator = strstr(mat_name, prefix);
            if (mat_prefix_separator)
                mat_name = mat_prefix_separator + strlen(prefix);

            char* nsp_separator = strchr(mat_name, '.');
            if (do_strip_ns && nsp_separator) {
                nsp_separator[0] = '\0';
                mat_name = nsp_separator + 1;
            }
            if (mat_name != mat_name_out)
                memmove(mat_name_out, mat_name, strlen(mat_name) + 1);
            return;
        }
    }
}

inline float hash_to_float(uint32_t hash) {
    // if all exponent bits are 0 (subnormals, +zero, -zero) set exponent to 1
    // if all exponent bits are 1 (NaNs, +inf, -inf) set exponent to 254
    uint32_t exponent = hash >> 23 & 255; // extract exponent (8 bits)
    if (exponent == 0 || exponent == 255)
        hash ^= 1 << 23; // toggle bit
    float f;
    std::memcpy(&f, &hash, 4);
    return f;
}

inline AtRGB hash_name_rgb(const char* name) {
    // This puts the float ID into the red channel, and the human-readable
    // versions into the G and B channels.
    uint32_t m3hash = 0;
    AtRGB out_color;
    MurmurHash3_x86_32(name, (uint32_t)strlen(name), 0, &m3hash);
    out_color.r = hash_to_float(m3hash);
    out_color.g = ((float)((m3hash << 8)) / (float)std::numeric_limits<uint32_t>::max());
    out_color.b = ((float)((m3hash << 16)) / (float)std::numeric_limits<uint32_t>::max());
    return out_color;
}

inline AtString get_user_data(const AtShaderGlobals* sg, const AtNode* node,
                              const AtString user_data_name, bool* cachable) {
    // returns the string if the parameter is usable, modifies cachable
    const AtUserParamEntry* pentry = AiNodeLookUpUserParameter(node, user_data_name);
    if (pentry) {
        if (AiUserParamGetType(pentry) == AI_TYPE_STRING &&
            AiUserParamGetCategory(pentry) == AI_USERDEF_CONSTANT) {
            return AiNodeGetStr(node, user_data_name);
        }
    }
    if (sg) {
        // this is intentionally outside the if (pentry) block.
        // With user data declared on ginstances and such, no pentry
        // is aquirable but AiUDataGetStr still works.
        // still true in Arnold 5.
        AtString udata_value;
        if (AiUDataGetStr(user_data_name, udata_value)) {
            *cachable = false;
            return udata_value;
        }
    }
    return AtString();
}

inline int get_offset_user_data(const AtShaderGlobals* sg, const AtNode* node,
                                const AtString user_data_name, bool* cachable) {
    // returns the string if the parameter is usable, modifies cachable
    const AtUserParamEntry* pentry = AiNodeLookUpUserParameter(node, user_data_name);
    if (pentry) {
        if (AiUserParamGetType(pentry) == AI_TYPE_INT &&
            AiUserParamGetCategory(pentry) == AI_USERDEF_CONSTANT) {
            return AiNodeGetInt(node, user_data_name);
        } else if (AiUserParamGetCategory(pentry) != AI_USERDEF_CONSTANT) {
            *cachable = false;
        }
    }
    if (sg) {
        // this is intentionally outside the if (pentry) block.
        // With user data declared on ginstances and such, no pentry
        // is aquirable but AiUDataGetStr still works.
        int udata_value = 0;
        if (AiUDataGetInt(user_data_name, udata_value)) {
            *cachable = false;
            return udata_value;
        }
    }
    return 0;
}

inline void offset_name(const AtShaderGlobals* sg, const AtNode* node, const int offset,
                        char obj_name_out[MAX_STRING_LENGTH]) {
    if (offset) {
        char offset_num_str[12];
        sprintf(offset_num_str, "_%d", offset);
        strcat(obj_name_out, offset_num_str);
    }
}

inline bool get_object_names(const AtShaderGlobals* sg, const AtNode* node, CryptoNameFlag flags,
                             char nsp_name_out[MAX_STRING_LENGTH],
                             char obj_name_out[MAX_STRING_LENGTH]) {
    bool cachable = true;

    const AtString nsp_user_data = get_user_data(sg, node, CRYPTO_ASSET_UDATA, &cachable);
    const AtString obj_user_data = get_user_data(sg, node, CRYPTO_OBJECT_UDATA, &cachable);

    bool need_nsp_name = nsp_user_data.empty();
    bool need_obj_name = obj_user_data.empty();
    if (need_obj_name || need_nsp_name)
        get_clean_object_name(AiNodeGetName(node), obj_name_out, nsp_name_out, flags);

    offset_name(sg, node, get_offset_user_data(sg, node, CRYPTO_OBJECT_OFFSET_UDATA, &cachable),
                obj_name_out);
    offset_name(sg, node, get_offset_user_data(sg, node, CRYPTO_ASSET_OFFSET_UDATA, &cachable),
                nsp_name_out);

    if (nsp_user_data)
        strcpy(nsp_name_out, nsp_user_data);

    if (obj_user_data)
        strcpy(obj_name_out, obj_user_data);

    nsp_name_out[MAX_STRING_LENGTH - 1] = '\0';
    return cachable;
}

inline bool get_material_name(const AtShaderGlobals* sg, const AtNode* node, const AtNode* shader,
                              CryptoNameFlag flags, char mat_name_out[MAX_STRING_LENGTH]) {
    bool cachable = true;
    AtString mat_user_data = get_user_data(sg, node, CRYPTO_MATERIAL_UDATA, &cachable);

    get_clean_material_name(AiNodeGetName(shader), mat_name_out, flags);
    offset_name(sg, node, get_offset_user_data(sg, node, CRYPTO_MATERIAL_OFFSET_UDATA, &cachable),
                mat_name_out);

    if (!mat_user_data.empty())
        strcpy(mat_name_out, mat_user_data);

    mat_name_out[MAX_STRING_LENGTH - 1] = '\0';
    return cachable;
}

///////////////////////////////////////////////
//
//      Metadata Writing
//
///////////////////////////////////////////////

inline void write_manifest_to_string(const ManifestMap& map, String& manf_string) {
    ManifestMap::const_iterator map_it = map.begin();
    const size_t map_entries = map.size();
    const size_t max_entries = 100000;
    size_t metadata_entries = map_entries;
    if (map_entries > max_entries) {
        AiMsgWarning("Cryptomatte: %lu entries in manifest, limiting to %lu", //
                     map_entries, max_entries);
        metadata_entries = max_entries;
    }

    manf_string.append("{");
    String pair;
    pair.reserve(MAX_STRING_LENGTH);
    for (uint32_t i = 0; i < metadata_entries; i++) {
        String name = map_it->first;
        float hash_value = map_it->second;
        ++map_it;

        uint32_t float_bits;
        std::memcpy(&float_bits, &hash_value, 4);
        char hex_chars[9];
        sprintf(hex_chars, "%08x", float_bits);

        pair.clear();
        pair.append("\"");
        for (size_t j = 0; j < name.length(); j++) {
            // append the name, char by char
            const char c = name.at(j);
            if (c == '"' || c == '\\' || c == '/')
                pair += "\\";
            pair += c;
        }
        pair.append("\":\"");
        pair.append(hex_chars);
        pair.append("\"");
        if (i < map_entries - 1)
            pair.append(",");
        manf_string.append(pair);
    }
    manf_string.append("}");
}

inline void write_manifest_sidecar_file(const ManifestMap& map_md_asset,
                                        StringVector manifest_paths) {
    String encoded_manifest = "";
    write_manifest_to_string(map_md_asset, encoded_manifest);
    for (const auto& manifest_path : manifest_paths) {
        std::ofstream out(manifest_path.c_str());
        AiMsgInfo("[Cryptomatte] writing file, %s", manifest_path.c_str());
        out << encoded_manifest.c_str();
        out.close();
    }
}

inline void add_hash_to_map(const char* c_str, ManifestMap& md_map) {
    if (cstr_empty(c_str))
        return;
    String name_string = String(c_str);
    if (md_map.count(name_string) == 0) {
        AtRGB hash = hash_name_rgb(c_str);
        md_map[name_string] = hash.r;
    }
}

inline AtString add_override_udata_to_manifest(const AtNode* node, const AtString override_udata,
                                               ManifestMap& hash_map) {
    /*
    Adds override user data to manifest map, including arrays of overrides.
    Does not add offsets.
    */
    const AtUserParamEntry* pentry = AiNodeLookUpUserParameter(node, override_udata);
    if (!pentry || AiUserParamGetType(pentry) != AI_TYPE_STRING)
        return AtString();

    if (AiUserParamGetCategory(pentry) == AI_USERDEF_CONSTANT) {
        // not an array
        AtString udata = AiNodeGetStr(node, override_udata);
        add_hash_to_map(udata, hash_map);
        return udata;
    } else {
        AtArray* values = AiNodeGetArray(node, override_udata);
        if (values) {
            for (uint32_t ai = 0; ai < AiArrayGetNumElements(values); ai++)
                add_hash_to_map(AiArrayGetStr(values, ai), hash_map);
        }
        return AtString();
    }
}

inline void add_obj_to_manifest(const AtNode* node, char name[MAX_STRING_LENGTH],
                                AtString override_udata, const AtString offset_udata,
                                ManifestMap& hash_map) {
    /*
    Adds objects to the manifest, based on processed names and potentially user
    data overrides
    and offsets.

    Does not handle combining overrides and offsets, unless the overrides are
    already passed
    in as "name", as is the case with single value per node overrides.
    */
    add_hash_to_map(name, hash_map);
    add_override_udata_to_manifest(node, override_udata, hash_map);
    bool single_offset_val = true;
    get_offset_user_data(nullptr, node, offset_udata, &single_offset_val);
    if (!single_offset_val) { // means offset was an array
        AtArray* offsets = AiNodeGetArray(node, offset_udata);
        if (offsets) {
            std::unordered_set<int> visitedOffsets;
            for (uint32_t i = 0; i < AiArrayGetNumElements(offsets); i++) {
                int offset = AiArrayGetInt(offsets, i);
                if (visitedOffsets.find(offset) == visitedOffsets.end()) {
                    visitedOffsets.insert(offset);
                    char name_copy[MAX_STRING_LENGTH] = "";
                    safe_copy_to_buffer(name_copy, name);
                    offset_name(nullptr, node, offset, name_copy);
                    add_hash_to_map(name_copy, hash_map);
                }
            }
        }
    }
}

///////////////////////////////////////////////
//
//      CryptomatteCache
//
///////////////////////////////////////////////

// clang-format off
#define CACHE_LINE 64
#if defined(_WIN32) || defined(_MSC_VER)
#define CACHE_ALIGN __declspec(align(CACHE_LINE))
#else
#define CACHE_ALIGN __attribute__((aligned(CACHE_LINE)))
#endif
// clang-format on

struct CACHE_ALIGN CryptomatteCache {
    AtNode* object = nullptr;
    AtRGB nsp_hash_clr = AI_RGB_BLACK;
    AtRGB obj_hash_clr = AI_RGB_BLACK;
    AtNode* shader_object = nullptr;
    AtRGB mat_hash_clr = AI_RGB_BLACK;
};

extern CryptomatteCache CRYPTOMATTE_CACHE[AI_MAX_THREADS];

///////////////////////////////////////////////
//
//      UserCryptomatte and CryptomatteData
//
///////////////////////////////////////////////

struct UserCryptomattes {
    size_t count = 0;
    std::vector<AtArray*> aov_arrays;
    std::vector<AtString> aovs;
    std::vector<AtString> sources;

    UserCryptomattes() {}

    UserCryptomattes(const AtArray* aov_input, const AtArray* src_input) {
        if (!aov_input || !src_input)
            return;

        const uint32_t num_inputs =
            std::min(AiArrayGetNumElements(aov_input), AiArrayGetNumElements(src_input));

        for (uint32_t i = 0; i < num_inputs; i++) {
            const AtString aov = AiArrayGetStr(aov_input, i);
            const AtString src = AiArrayGetStr(src_input, i);
            if (!aov.empty() && !src.empty()) {
                AiMsgInfo("Adding user-Cryptomatte %lu: AOV: %s Source user data: %s", aovs.size(),
                          aov.c_str(), src.c_str());
                aovs.push_back(aov);
                sources.push_back(src);
            }
        }
        aov_arrays.resize(aovs.size());
        for (auto& aov_array : aov_arrays)
            aov_array = nullptr;

        count = aovs.size();
    }

    ~UserCryptomattes() {
        for (auto& aov_array : aov_arrays) {
            AiArrayDestroy(aov_array);
        }
    }
};

struct CryptomatteData {
    // Accessed during sampling, so hopefully in first cache line.
    AtString aov_cryptoasset;
    AtString aov_cryptoobject;
    AtString aov_cryptomaterial;

    AtArray* aov_array_cryptoasset = nullptr;
    AtArray* aov_array_cryptoobject = nullptr;
    AtArray* aov_array_cryptomaterial = nullptr;
    UserCryptomattes user_cryptomattes;
    // Custom output drivers need to be considered as if they 
    // were a driver_exr
    bool custom_output_driver = false;
    // Do we want to create new outputs for the "depth" AOVs
    // (e.g. crypto_material00, crypto_material01, etc...)
    bool create_depth_outputs = true;

    // User options.
    uint8_t option_depth;
    uint8_t option_aov_depth;
    bool option_exr_preview_channels;
    CryptoNameFlag option_obj_flags;
    CryptoNameFlag option_mat_flags;
    uint8_t option_pcloud_ice_verbosity;
    bool option_sidecar_manifests;

    // Vector of paths for each of the cryptomattes. Vector because each
    // cryptomatte can write to multiple drivers (stereo, multi-camera)
    StringVector manif_asset_paths;
    StringVector manif_object_paths;
    StringVector manif_material_paths;
    // Nested vector of paths for each user cryptomatte.
    std::vector<StringVector> manifs_user_paths;

    // lentil additions
    bool started = false;
    bool is_setup_completed = false;

public:
    CryptomatteData() {
        set_option_channels(CRYPTO_DEPTH_DEFAULT, CRYPTO_PREVIEWINEXR_DEFAULT);
        set_option_namespace_stripping(CRYPTO_NAME_ALL, CRYPTO_NAME_ALL);
        set_option_ice_pcloud_verbosity(CRYPTO_ICEPCLOUDVERB_DEFAULT);
        if (!g_critsec_active)
            AiMsgError("[Cryptomatte] Critical section was not initialized. ");
    }

    void setup_all(AtUniverse *universe, 
                   const AtString aov_cryptoasset_, const AtString aov_cryptoobject_,
                   const AtString aov_cryptomaterial_, 
                   AtArray* uc_aov_array,
                   AtArray* uc_src_array,
                   bool custom_output_driver_,
                   bool create_depth_outputs_) {
        
        // lentil addition
        started = true;

        aov_cryptoasset = aov_cryptoasset_;
        aov_cryptoobject = aov_cryptoobject_;
        aov_cryptomaterial = aov_cryptomaterial_;

        custom_output_driver = custom_output_driver_;
        create_depth_outputs = create_depth_outputs_;
        destroy_arrays();

        user_cryptomattes = UserCryptomattes(uc_aov_array, uc_src_array);

        crypto_crit_sec_enter();
        setup_outputs(universe);
        crypto_crit_sec_leave();

        // lentil addition
        is_setup_completed = true;
    }

    void set_option_channels(int depth, bool exr_preview_channels) {
        depth = std::min(std::max(depth, 1), MAX_CRYPTOMATTE_DEPTH);
        option_depth = depth;
        option_exr_preview_channels = exr_preview_channels;
        if (option_depth % 2 == 0)
            option_aov_depth = option_depth / 2;
        else
            option_aov_depth = (option_depth + 1) / 2;
    }

    void set_option_namespace_stripping(CryptoNameFlag obj_flags, CryptoNameFlag mat_flags) {
        option_obj_flags = obj_flags;
        option_mat_flags = mat_flags;
    }

    void set_option_ice_pcloud_verbosity(int verbosity) {
        verbosity = std::min(std::max(verbosity, 0), 2);
        option_pcloud_ice_verbosity = verbosity;
        g_pointcloud_instance_verbosity = option_pcloud_ice_verbosity; // to do: really remove this
    }

    void set_option_sidecar_manifests(bool sidecar) { option_sidecar_manifests = sidecar; }

    void do_cryptomattes(AtShaderGlobals* sg) {
        if (sg->Rt & AI_RAY_CAMERA && sg->sc == AI_CONTEXT_SURFACE) {
            do_standard_cryptomattes(sg);
            do_user_cryptomattes(sg);
        }
    }

    void write_sidecar_manifests(AtUniverse *universe) {
        write_standard_sidecar_manifests(universe);
        write_user_sidecar_manifests(universe);
    }

    ~CryptomatteData() { destroy_arrays(); }

private:
    void do_standard_cryptomattes(AtShaderGlobals* sg) {
        if (!aov_array_cryptoasset && !aov_array_cryptoobject && !aov_array_cryptomaterial)
            return;

        AtRGB nsp_hash_clr, obj_hash_clr, mat_hash_clr;
        hash_object_rgb(sg, nsp_hash_clr, obj_hash_clr, mat_hash_clr);

        if (aov_array_cryptoasset)
            aov_array_set_flt(sg, aov_array_cryptoasset, nsp_hash_clr.r);
        if (aov_array_cryptoobject)
            aov_array_set_flt(sg, aov_array_cryptoobject, obj_hash_clr.r);
        if (aov_array_cryptomaterial)
            aov_array_set_flt(sg, aov_array_cryptomaterial, mat_hash_clr.r);

        nsp_hash_clr.r = obj_hash_clr.r = mat_hash_clr.r = 0.0f;

        AiAOVSetRGBA(sg, aov_cryptoasset, nsp_hash_clr);
        AiAOVSetRGBA(sg, aov_cryptoobject, obj_hash_clr);
        AiAOVSetRGBA(sg, aov_cryptomaterial, mat_hash_clr);
    }

    void do_user_cryptomattes(AtShaderGlobals* sg) {
        for (uint32_t i = 0; i < user_cryptomattes.count; i++) {
            AtArray* aovArray = user_cryptomattes.aov_arrays[i];
            if (aovArray) {
                AtString aov_name = user_cryptomattes.aovs[i];
                AtString src_data_name = user_cryptomattes.sources[i];
                AtRGB hash = AI_RGB_BLACK;
                AtString result;

                AiUDataGetStr(src_data_name, result);
                if (!result.empty())
                    hash = hash_name_rgb(result.c_str());

                aov_array_set_flt(sg, aovArray, hash.r);
                hash.r = 0.0f;
                AiAOVSetRGBA(sg, aov_name, hash);
            }
        }
    }

    void hash_object_rgb(AtShaderGlobals* sg, AtRGB& nsp_hash_clr, AtRGB& obj_hash_clr,
                         AtRGB& mat_hash_clr) {
        if (CRYPTOMATTE_CACHE[sg->tid].object == sg->Op) {
            nsp_hash_clr = CRYPTOMATTE_CACHE[sg->tid].nsp_hash_clr;
            obj_hash_clr = CRYPTOMATTE_CACHE[sg->tid].obj_hash_clr;
        } else {
            char nsp_name[MAX_STRING_LENGTH] = "";
            char obj_name[MAX_STRING_LENGTH] = "";
            bool cachable = get_object_names(sg, sg->Op, option_obj_flags, nsp_name, obj_name);
            nsp_hash_clr = hash_name_rgb(nsp_name);
            obj_hash_clr = hash_name_rgb(obj_name);
            if (cachable) {
                // only values that will be valid for the whole node, sg->Op,
                // are cachable.
                // the source of manually overriden values is not known and may
                // therefore not be cached.
                CRYPTOMATTE_CACHE[sg->tid].object = sg->Op;
                CRYPTOMATTE_CACHE[sg->tid].obj_hash_clr = obj_hash_clr;
                CRYPTOMATTE_CACHE[sg->tid].nsp_hash_clr = nsp_hash_clr;
            }
        }

        AtNode* shader = AiShaderGlobalsGetShader(sg);
        if (CRYPTOMATTE_CACHE[sg->tid].shader_object == sg->Op) {
            mat_hash_clr = CRYPTOMATTE_CACHE[sg->tid].mat_hash_clr;
        } else {
            AtArray* shaders = AiNodeGetArray(sg->Op, aStr_shader);
            bool cachable = shaders ? AiArrayGetNumElements(shaders) == 1 : false;

            char mat_name[MAX_STRING_LENGTH] = "";
            cachable =
                get_material_name(sg, sg->Op, shader, option_mat_flags, mat_name) && cachable;
            mat_hash_clr = hash_name_rgb(mat_name);

            if (cachable) {
                // only values that will be valid for the whole node, sg->Op,
                // are cachable.
                CRYPTOMATTE_CACHE[sg->tid].shader_object = sg->Op;
                CRYPTOMATTE_CACHE[sg->tid].mat_hash_clr = mat_hash_clr;
            }
        }
    }

    void aov_array_set_flt(AtShaderGlobals* sg, const AtArray* aov_names, float id) const {
        for (uint32_t i = 0; i < AiArrayGetNumElements(aov_names); i++) {
            const AtString aov_name = AiArrayGetStr(aov_names, i);
            if (aov_name.empty())
                return;
            AiAOVSetFlt(sg, aov_name, id);
        }
    }

    ///////////////////////////////////////////////
    //      Building Cryptomatte Arnold Nodes
    ///////////////////////////////////////////////

    struct TokenizedOutput {
        String camera_tok = "";
        String aov_name_tok = "";
        String aov_type_tok = "";
        String filter_tok = "";
        String driver_tok = "";
        String layer_tok = "";
        bool half_flag = false;
        AtNode* raw_driver = nullptr;
        AtUniverse *universe = nullptr;

    private:
        AtNode* driver = nullptr;

    public:
        TokenizedOutput() {}

        TokenizedOutput(AtUniverse *universe_in, AtNode* raw_driver_in) { universe = universe_in; raw_driver = raw_driver_in; }

        TokenizedOutput(AtUniverse *universe_in, AtString output_string) {
            universe = universe_in;
            char* temp_string = strdup(output_string.c_str());
            const String c0 = to_string_safe(strtok(temp_string, " "));
            const String c1 = to_string_safe(strtok(nullptr, " "));
            const String c2 = to_string_safe(strtok(nullptr, " "));
            const String c3 = to_string_safe(strtok(nullptr, " "));
            const String c4 = to_string_safe(strtok(nullptr, " "));
            const String c5 = to_string_safe(strtok(nullptr, " "));
            const String c6 = to_string_safe(strtok(nullptr, " "));
            free(temp_string);

            // The first token c0 can eventually be a camera name. To ensure this, we look for such a node in the current universe
            const AtNode *camNode = AiNodeLookUpByName(universe, AtString(c0.c_str()));
            const bool has_camera = (camNode && AiNodeEntryGetType(AiNodeGetNodeEntry(camNode)) == AI_NODE_CAMERA);
            camera_tok = has_camera ? c0 : "";

            // the half flag is that last one in the outputs line, it can either be c4, c5, or c6
            half_flag = ((c6 == String("HALF")) || 
                        (c6.empty() && c5 == String("HALF")) ||
                        (c5.empty() && c4 == String("HALF")));
            
            // Aov name, type, filter and driver, are mandatory tokens, that can eventually be preceded by the camera token
            aov_name_tok = has_camera ? c1 : c0;
            aov_type_tok = has_camera ? c2 : c1;
            filter_tok = has_camera ? c3 : c2;
            driver_tok = has_camera ? c4 : c3;
            layer_tok = has_camera ? c5 : c4;
            // Ensure we're not confusing the optional layer token
            //  with the (also optional) HALF flag
            if (layer_tok == String("HALF"))
                layer_tok = String("");

            driver = AiNodeLookUpByName(universe, driver_tok.c_str());
        }

        String rebuild_output() const {
            if (raw_driver)
                return String(AiNodeGetName(raw_driver));

            String output_str("");
            if (!camera_tok.empty()) {
                output_str.append(camera_tok);
                output_str.append(" ");
            }
            output_str.append(aov_name_tok);
            output_str.append(" ");
            output_str.append(aov_type_tok);
            output_str.append(" ");
            output_str.append(filter_tok);
            output_str.append(" ");
            output_str.append(driver_tok);
            if (!layer_tok.empty()) {
                output_str.append(" ");
                output_str.append(layer_tok);
            }
            if (half_flag) // output was already flagged half
                output_str.append(" HALF");
            return output_str;
        }

        AtNode* get_driver() const {
            if (driver && driver_tok == String(AiNodeGetName(driver)))
                return driver;
            else if (!driver_tok.empty())
                return AiNodeLookUpByName(universe, driver_tok.c_str());
            else
                return nullptr;
        }

        bool aov_matches(const char* str) const { return aov_name_tok == String(str); }

        String to_string_safe(const char* c_str) const { return c_str ? c_str : ""; }
    };

    void setup_outputs(AtUniverse *universe) {
        const AtArray* outputs = AiNodeGetArray(AiUniverseGetOptions(universe), "outputs");
        const uint32_t prev_output_num = AiArrayGetNumElements(outputs);
        AtNode* noop_filter = option_exr_preview_channels ? nullptr : get_or_create_noop_filter(universe);

        // if a driver is set to half, it needs to be set to full,
        // and its non-cryptomatte outputs need to be set to half.
        std::unordered_set<AtNode*> modified_drivers;
        std::vector<AtNode*> driver_asset, driver_object, driver_material;
        std::vector<std::vector<AtNode*>> tmp_uc_drivers(user_cryptomattes.count);

        std::vector<TokenizedOutput> outputs_orig(prev_output_num), outputs_new;

        for (uint32_t i = 0; i < prev_output_num; i++) {
            TokenizedOutput t_output(universe, AiArrayGetStr(outputs, i));
            AtNode* driver = t_output.get_driver();

            AtArray* crypto_aovs = nullptr;
            if (t_output.aov_matches(aov_cryptoasset)) {
                crypto_aovs = aov_array_cryptoasset = allocate_aov_names();
                driver_asset.push_back(driver);
            } else if (t_output.aov_matches(aov_cryptoobject)) {
                crypto_aovs = aov_array_cryptoobject = allocate_aov_names();
                driver_object.push_back(driver);
            } else if (t_output.aov_matches(aov_cryptomaterial)) {
                crypto_aovs = aov_array_cryptomaterial = allocate_aov_names();
                driver_material.push_back(driver);
            } else {
                for (size_t j = 0; j < user_cryptomattes.count; j++) {
                    if (t_output.aov_matches(user_cryptomattes.aovs[j])) {
                        crypto_aovs = user_cryptomattes.aov_arrays[j] = allocate_aov_names();
                        tmp_uc_drivers[j].push_back(driver);
                        break;
                    }
                }
            }

            if (crypto_aovs && check_driver(driver)) {
                setup_new_outputs(universe, t_output, crypto_aovs, outputs_new);

                if (AiNodeEntryLookUpParameter(AiNodeGetNodeEntry(driver), "half_precision")) {
                    if (AiNodeGetBool(driver, "half_precision")) {
                        AiNodeSetBool(driver, "half_precision", false);
                        modified_drivers.insert(driver);
                    }
                }
                if (noop_filter)
                    t_output.filter_tok = AiNodeGetName(noop_filter);
            }

            outputs_orig[i] = t_output;
        }

        if (outputs_new.size()) {
            if (option_sidecar_manifests) {
                AtNode* manifest_driver = setup_manifest_driver(universe);
                outputs_new.push_back(TokenizedOutput(universe, manifest_driver));
            }

            for (auto& t_output : outputs_orig) {
                // if outputs are not flagged as half, and their drivers are switched
                // to full float, the output must be set to half to preserve behavior.
                if (!t_output.half_flag && modified_drivers.count(t_output.get_driver())) {
                    t_output.half_flag = true;
                }
            }

            const auto nb_outputs = outputs_orig.size() + outputs_new.size();
            AtArray* final_outputs = AiArrayAllocate((uint32_t)nb_outputs, 1, AI_TYPE_STRING);
            uint32_t i = 0;
            for (auto& t_output : outputs_orig)
                AiArraySetStr(final_outputs, i++, t_output.rebuild_output().c_str());
            for (auto& t_output : outputs_new)
                AiArraySetStr(final_outputs, i++, t_output.rebuild_output().c_str());

            AiNodeSetArray(AiUniverseGetOptions(universe), "outputs", final_outputs);
        }

        build_standard_metadata(universe, driver_asset, driver_object, driver_material);
        build_user_metadata(universe, tmp_uc_drivers);
    }

    void setup_new_outputs(AtUniverse *universe, TokenizedOutput& t_output, 
                          AtArray* crypto_aovs, std::vector<TokenizedOutput>& new_outputs) const {
        // Populates crypto_aovs and new_outputs
        AtNode* orig_filter = AiNodeLookUpByName(universe, t_output.filter_tok.c_str());

        // Outlaw RLE, dwaa, dwab
        AtNode* driver = t_output.get_driver();
        const AtNodeEntry *driverEntry = AiNodeGetNodeEntry(driver);
        const AtParamEntry* compressionParamEntry = AiNodeEntryLookUpParameter(driverEntry, "compression");

        if (compressionParamEntry) {
            const AtEnum compressions = AiParamGetEnum(compressionParamEntry);
            const int compression = AiNodeGetInt(driver, "compression");
            const bool cmp_rle = compression == AiEnumGetValue(compressions, "rle"),
                       cmp_dwa = compression == AiEnumGetValue(compressions, "dwaa") ||
                                 compression == AiEnumGetValue(compressions, "dwab");
            if (cmp_rle || cmp_dwa) {
                if (cmp_rle)
                    AiMsgWarning("Cryptomatte cannot be set to RLE compression- it "
                                 "does not work on full float. Switching to Zip.");
                if (cmp_dwa)
                    AiMsgWarning("Cryptomatte cannot be set to dwa compression- the "
                                 "compression breaks Cryptomattes. Switching to Zip.");
                AiNodeSetStr(driver, "compression", "zip");
            }
        }

        AtArray* outputs = AiNodeGetArray(AiUniverseGetOptions(universe), "outputs");

        std::unordered_set<String> output_set;
        for (uint32_t i = 0; i < AiArrayGetNumElements(outputs); i++)
            output_set.insert(String(AiArrayGetStr(outputs, i)));

        // Create filters and outputs as needed
        for (int i = 0; i < option_aov_depth; i++) {
            char rank_num[3];
            sprintf(rank_num, "%002d", i);

            const String filter_rank_name = t_output.aov_name_tok + "_filter" + rank_num;
            const String aov_rank_name = t_output.aov_name_tok + rank_num;
            if (create_depth_outputs) {
                if (AiNodeLookUpByName(universe, filter_rank_name.c_str()) == nullptr)
                    AtNode* filter = create_filter(universe, orig_filter, filter_rank_name, i);

                TokenizedOutput new_t_output = t_output;
                new_t_output.aov_name_tok = aov_rank_name;

                // also append the rank to the eventual layer name
                if (!new_t_output.layer_tok.empty())
                    new_t_output.layer_tok += rank_num;

                new_t_output.aov_type_tok = "FLOAT";
                new_t_output.filter_tok = filter_rank_name;
                new_t_output.half_flag = false;

                String new_output_str = new_t_output.rebuild_output();
                if (!output_set.count(new_output_str)) {
                    new_outputs.push_back(new_t_output);
                    output_set.insert(new_output_str);
                }
            }
            // Always call AiAOVRegister for the depth AOVs, even if we didn't create them here
            // (they could already exist in the scene outputs)
            AiAOVRegister(aov_rank_name.c_str(), AI_TYPE_FLOAT, AI_AOV_BLEND_NONE);
            AiArraySetStr(crypto_aovs, i, aov_rank_name.c_str());
        }
    }

    AtNode* create_filter(AtUniverse *universe, const AtNode* orig_filter, const String filter_name, int aovindex) const {
        const AtNodeEntry* filter_nentry = AiNodeGetNodeEntry(orig_filter);
        const auto width = AiNodeEntryLookUpParameter(filter_nentry, "width")
                               ? AiNodeGetFlt(orig_filter, "width")
                               : 2.0f;
        const String filter_type = AiNodeEntryGetName(filter_nentry);
        const String filter_param = filter_type.substr(0, filter_type.find("_filter"));

        AtNode* filter = AiNode(universe, "cryptomatte_filter", filter_name.c_str(), nullptr);
        AiNodeSetStr(filter, "filter", filter_param.c_str());
        AiNodeSetInt(filter, "rank", aovindex * 2);
        AiNodeSetFlt(filter, "width", width);
        return filter;
    }

    AtNode* get_or_create_noop_filter(AtUniverse *universe) const {
        const static AtString noop_filter_name("cryptomatte_noop_filter");
        AtNode* filter = AiNodeLookUpByName(universe, noop_filter_name);
        if (!filter) {
            filter = AiNode(universe, "cryptomatte_filter", noop_filter_name, nullptr);
            AiNodeSetBool(filter, "noop", true);
        }
        return filter;
    }

    AtArray* allocate_aov_names() const {
        // allocates and blanks an AOV array
        AtArray* aovs = AiArrayAllocate(option_aov_depth, 1, AI_TYPE_STRING);
        for (uint32_t i = 0; i < option_aov_depth; i++)
            AiArraySetStr(aovs, i, "");
        return aovs;
    }

    AtNode* setup_manifest_driver(AtUniverse *universe) {
        AtString manifest_driver_name("cryptomatte_manifest_driver");
        AtNode* manifest_driver = AiNodeLookUpByName(universe, manifest_driver_name);
        if (!manifest_driver)
            manifest_driver = AiNode(universe, "cryptomatte_manifest_driver", manifest_driver_name, nullptr);
        AiNodeSetLocalData(manifest_driver, this);
        return manifest_driver;
    }
    
    ///////////////////////////////////////////////
    //      Manifests and metadata
    ///////////////////////////////////////////////

    void setup_deferred_manifest(AtNode* driver, AtString token, String& path_out,
                                 String& metadata_path_out) {
        path_out = "";
        metadata_path_out = "";
        if (check_driver(driver) && option_sidecar_manifests) {

            if (AiNodeEntryLookUpParameter(AiNodeGetNodeEntry(driver), "filename")) {
                String filepath = String(AiNodeGetStr(driver, "filename").c_str());
                const size_t exr_found = filepath.find(".exr");
                if (exr_found != String::npos)
                    filepath = filepath.substr(0, exr_found);

                path_out = filepath + "." + token.c_str() + ".json";
                const size_t last_partition = path_out.find_last_of("/\\");
                if (last_partition == String::npos)
                    metadata_path_out += path_out;
                else
                    metadata_path_out += path_out.substr(last_partition + 1);
            }
        }
    }

    void write_standard_sidecar_manifests(AtUniverse *universe) {
        const bool do_md_asset = manif_asset_paths.size() > 0;
        const bool do_md_object = manif_object_paths.size() > 0;
        const bool do_md_material = manif_material_paths.size() > 0;

        if (!do_md_asset && !do_md_object && !do_md_material)
            return;

        ManifestMap map_md_asset, map_md_object, map_md_material;
        compile_standard_manifests(universe, do_md_asset, do_md_object, do_md_material, map_md_asset,
                                   map_md_object, map_md_material);

        if (do_md_asset)
            write_manifest_sidecar_file(map_md_asset, manif_asset_paths);
        if (do_md_object)
            write_manifest_sidecar_file(map_md_object, manif_object_paths);
        if (do_md_material)
            write_manifest_sidecar_file(map_md_material, manif_material_paths);

        // reset sidecar writers
        manif_asset_paths = StringVector();
        manif_object_paths = StringVector();
        manif_material_paths = StringVector();
    }

    void compile_standard_manifests(AtUniverse *universe, bool do_md_asset, bool do_md_object, 
                                    bool do_md_material, ManifestMap& map_md_asset, 
                                    ManifestMap& map_md_object, ManifestMap& map_md_material) {
        AtNodeIterator* shape_iterator = AiUniverseGetNodeIterator(universe, AI_NODE_SHAPE);
        while (!AiNodeIteratorFinished(shape_iterator)) {
            AtNode* node = AiNodeIteratorGetNext(shape_iterator);
            if (!node || AiNodeIsDisabled(node))
                continue;

            // skip any list aggregate nodes
            if (AiNodeIs(node, aStr_list_aggregate))
                continue;

            char nsp_name[MAX_STRING_LENGTH] = "";
            char obj_name[MAX_STRING_LENGTH] = "";

            get_object_names(nullptr, node, option_obj_flags, nsp_name, obj_name);

            if (do_md_asset || do_md_object) {
                add_obj_to_manifest(node, nsp_name, CRYPTO_ASSET_UDATA, CRYPTO_ASSET_OFFSET_UDATA,
                                    map_md_asset);
                add_obj_to_manifest(node, obj_name, CRYPTO_OBJECT_UDATA, CRYPTO_OBJECT_OFFSET_UDATA,
                                    map_md_object);
            }
            if (do_md_material) {
                // Process all shaders from the objects into the manifest.
                // This includes cluster materials.
                AtArray* shaders = AiNodeGetArray(node, "shader");
                if (!shaders)
                    continue;
                for (uint32_t i = 0; i < AiArrayGetNumElements(shaders); i++) {
                    char mat_name[MAX_STRING_LENGTH] = "";
                    AtNode* shader = static_cast<AtNode*>(AiArrayGetPtr(shaders, i));
                    if (!shader)
                        continue;
                    get_material_name(nullptr, node, shader, option_mat_flags, mat_name);
                    add_obj_to_manifest(node, mat_name, CRYPTO_MATERIAL_UDATA,
                                        CRYPTO_MATERIAL_OFFSET_UDATA, map_md_material);
                }
            }
        }
        AiNodeIteratorDestroy(shape_iterator);
    }

    void write_user_sidecar_manifests(AtUniverse *universe) {
        std::vector<bool> do_metadata;
        do_metadata.resize(user_cryptomattes.count);
        std::vector<ManifestMap> manf_maps;
        manf_maps.resize(user_cryptomattes.count);

        for (size_t i = 0; i < user_cryptomattes.count; i++) {
            do_metadata[i] = true;
            for (size_t j = 0; j < manifs_user_paths[i].size(); j++)
                do_metadata[i] = do_metadata[i] && manifs_user_paths[i][j].length() > 0;
        }
        compile_user_manifests(universe, do_metadata, manf_maps);

        for (size_t i = 0; i < manifs_user_paths.size(); i++)
            if (do_metadata[i])
                write_manifest_sidecar_file(manf_maps[i], manifs_user_paths[i]);

        manifs_user_paths = std::vector<StringVector>();
    }

    void compile_user_manifests(AtUniverse *universe, std::vector<bool>& do_metadata,
                                std::vector<ManifestMap>& manf_maps) {
        if (user_cryptomattes.count == 0)
            return;
        AtNodeIterator* shape_iterator = AiUniverseGetNodeIterator(universe, AI_NODE_SHAPE);
        while (!AiNodeIteratorFinished(shape_iterator)) {
            AtNode* node = AiNodeIteratorGetNext(shape_iterator);
            if (!node || AiNodeIsDisabled(node))
                continue;
            for (uint32_t i = 0; i < user_cryptomattes.count; i++) {
                if (do_metadata[i])
                    add_override_udata_to_manifest(node, user_cryptomattes.sources[i],
                                                   manf_maps[i]);
            }
        }
        AiNodeIteratorDestroy(shape_iterator);
    }

    void build_standard_metadata(AtUniverse *universe, 
                                 const std::vector<AtNode*>& driver_asset,
                                 const std::vector<AtNode*>& driver_object,
                                 const std::vector<AtNode*>& driver_material) {
        clock_t metadata_start_time = clock();

        bool do_md_asset = metadata_needed_on_drivers(driver_asset, aov_cryptoasset),
             do_md_object = metadata_needed_on_drivers(driver_object, aov_cryptoobject),
             do_md_material = metadata_needed_on_drivers(driver_material, aov_cryptomaterial);

        if (!do_md_asset && !do_md_object && !do_md_material)
            return;

        ManifestMap map_md_asset, map_md_object, map_md_material;

        if (!option_sidecar_manifests)
            compile_standard_manifests(universe, do_md_asset, do_md_object, do_md_material, 
                                       map_md_asset, map_md_object, map_md_material);

        String manif_asset_m, manif_object_m, manif_material_m;
        manif_asset_paths.resize(driver_asset.size());
        for (size_t i = 0; i < driver_asset.size(); i++) {
            setup_deferred_manifest(driver_asset[i], aov_cryptoasset, manif_asset_paths[i],
                                    manif_asset_m);
            write_metadata_to_driver(driver_asset[i], aov_cryptoasset, map_md_asset, manif_asset_m);
        }
        manif_object_paths.resize(driver_object.size());
        for (size_t i = 0; i < driver_object.size(); i++) {
            setup_deferred_manifest(driver_object[i], aov_cryptoobject, manif_object_paths[i],
                                    manif_object_m);
            write_metadata_to_driver(driver_object[i], aov_cryptoobject, map_md_object,
                                     manif_object_m);
        }
        manif_material_paths.resize(driver_material.size());
        for (size_t i = 0; i < driver_material.size(); i++) {
            setup_deferred_manifest(driver_material[i], aov_cryptomaterial, manif_material_paths[i],
                                    manif_material_m);
            write_metadata_to_driver(driver_material[i], aov_cryptomaterial, map_md_material,
                                     manif_material_m);
        }

        if (!option_sidecar_manifests)
            AiMsgInfo("Cryptomatte manifest created - %f seconds",
                      (float(clock() - metadata_start_time) / CLOCKS_PER_SEC));
        else
            AiMsgInfo("Cryptomatte manifest creation deferred - sidecar file "
                      "written at end of render.");
    }

    void build_user_metadata(AtUniverse *universe, const std::vector<std::vector<AtNode*>>& drivers_vv) {
        std::vector<StringVector> manifs_user_m;
        manifs_user_paths = std::vector<StringVector>();
        manifs_user_m.resize(drivers_vv.size());
        manifs_user_paths.resize(drivers_vv.size());

        const bool sidecar = option_sidecar_manifests;
        if (user_cryptomattes.count == 0 || drivers_vv.size() == 0)
            return;

        const clock_t metadata_start_time = clock();
        std::vector<bool> do_metadata;
        do_metadata.resize(user_cryptomattes.count);
        std::vector<ManifestMap> manf_maps;
        manf_maps.resize(user_cryptomattes.count);

        bool do_anything = false;
        for (uint32_t i = 0; i < drivers_vv.size(); i++) {
            do_metadata[i] = false;
            for (size_t j = 0; j < drivers_vv[i].size(); j++) {
                AtNode* driver = drivers_vv[i][j];
                AtString user_aov = user_cryptomattes.aovs[i];
                do_metadata[i] = do_metadata[i] || metadata_needed(driver, user_aov);
                do_anything = do_anything || do_metadata[i];

                String manif_user_m;
                if (sidecar) {
                    String manif_asset_paths;
                    setup_deferred_manifest(driver, user_aov, manif_asset_paths, manif_user_m);
                    manifs_user_paths[i].push_back(driver ? manif_asset_paths : "");
                }
                manifs_user_m[i].push_back(driver ? manif_user_m : "");
            }
        }

        if (!do_anything)
            return;

        if (!sidecar)
            compile_user_manifests(universe, do_metadata, manf_maps);

        for (uint32_t i = 0; i < drivers_vv.size(); i++) {
            if (!do_metadata[i])
                continue;
            AtString aov_name = user_cryptomattes.aovs[i];
            for (size_t j = 0; j < drivers_vv[i].size(); j++) {
                AtNode* driver = drivers_vv[i][j];
                if (driver) {
                    metadata_set_unneeded(driver, aov_name);
                    write_metadata_to_driver(driver, aov_name, manf_maps[i], manifs_user_m[i][j]);
                }
            }
        }
        AiMsgInfo("User Cryptomatte manifests created - %f seconds",
                  float(clock() - metadata_start_time) / CLOCKS_PER_SEC);
    }

    void write_metadata_to_driver(AtNode* driver, const AtString cryptomatte_name,
                                  const ManifestMap& map, const String sidecar_manif_file) const {
        if (!check_driver(driver))
            return;

        if (!AiNodeEntryLookUpParameter(AiNodeGetNodeEntry(driver), "custom_attributes") &&
            !AiNodeLookUpUserParameter(driver, "custom_attributes")) {
            AiNodeDeclare(driver, "custom_attributes", "constant ARRAY STRING");
        }
        AtArray* orig_md = AiNodeGetArray(driver, "custom_attributes");
        const uint32_t orig_num_entries = orig_md ? AiArrayGetNumElements(orig_md) : 0;

        const String metadata_id = compute_metadata_ID(cryptomatte_name);
        const String prefix = String("STRING cryptomatte/") + metadata_id + "/";

        for (uint32_t i = 0; i < orig_num_entries; i++) {
            if (prefix.compare(AiArrayGetStr(orig_md, i)) == 0) {
                AiMsgWarning("Cryptomatte: Unable to write metadata. EXR metadata "
                             "key, %s, already in use.",
                             prefix.c_str());
                return;
            }
        }

        const String metadata_hash = prefix + String("hash MurmurHash3_32");
        const String metadata_conv = prefix + String("conversion uint32_to_float32");
        const String metadata_name = prefix + String("name ") + cryptomatte_name.c_str();
        String metadata_manf;
        if (sidecar_manif_file.empty()) {
            metadata_manf = prefix + String("manifest ");
            write_manifest_to_string(map, metadata_manf);
        } else {
            metadata_manf = prefix + String("manif_file ") + sidecar_manif_file;
        }

        AtArray* combined_md = AiArrayAllocate(orig_num_entries + 4, 1, AI_TYPE_STRING);
        for (uint32_t i = 0; i < orig_num_entries; i++)
            AiArraySetStr(combined_md, i, AiArrayGetStr(orig_md, i));
        AiArraySetStr(combined_md, orig_num_entries + 0, metadata_manf.c_str());
        AiArraySetStr(combined_md, orig_num_entries + 1, metadata_hash.c_str());
        AiArraySetStr(combined_md, orig_num_entries + 2, metadata_conv.c_str());
        AiArraySetStr(combined_md, orig_num_entries + 3, metadata_name.c_str());

        AiNodeSetArray(driver, "custom_attributes", combined_md);
    }

    bool check_driver(AtNode* driver) const {
        return driver && (custom_output_driver || AiNodeIs(driver, AtString("driver_exr")));
    }

    bool metadata_needed_on_drivers(const std::vector<AtNode*>& drivers, const AtString aov_name) {
        for (auto& driver : drivers) {
            if (metadata_needed(driver, aov_name)) {
                metadata_set_unneeded(driver, aov_name);
                return true;
            }
        }
        return false;
    }

    bool metadata_needed(AtNode* driver, const AtString aov_name) const {
        String flag = String(CRYPTOMATTE_METADATA_SET_FLAG) + aov_name.c_str();
        return check_driver(driver) && !AiNodeLookUpUserParameter(driver, flag.c_str());
    }

    void metadata_set_unneeded(AtNode* driver, const AtString aov_name) const {
        if (!driver)
            return;
        String flag = String(CRYPTOMATTE_METADATA_SET_FLAG) + aov_name.c_str();
        if (!AiNodeLookUpUserParameter(driver, flag.c_str()))
            AiNodeDeclare(driver, flag.c_str(), "constant BOOL");
    }

    String compute_metadata_ID(AtString cryptomatte_name) const {
        const float float_id = hash_name_rgb(cryptomatte_name.c_str()).r;
        uint32_t int_id;
        std::memcpy(&int_id, &float_id, 4);
        char hex_chars[9];
        sprintf(hex_chars, "%08x", int_id);
        return String(hex_chars).substr(0, 7);
    }

    ///////////////////////////////////////////////
    //      Cleanup
    ///////////////////////////////////////////////

    void destroy_arrays() {
        if (aov_array_cryptoasset)
            AiArrayDestroy(aov_array_cryptoasset);
        if (aov_array_cryptoobject)
            AiArrayDestroy(aov_array_cryptoobject);
        if (aov_array_cryptomaterial)
            AiArrayDestroy(aov_array_cryptomaterial);
        aov_array_cryptoasset = nullptr;
        aov_array_cryptoobject = nullptr;
        aov_array_cryptomaterial = nullptr;
        user_cryptomattes = UserCryptomattes();
    }
};
