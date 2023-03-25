#include <ai.h>
#include "lentil.h"
#include "operator_data.h"
#include "aov_data.h"

// adds the correct filter for all aov's

AI_OPERATOR_NODE_EXPORT_METHODS(LentilOperatorMtd);

const AtString atstring_filter_gaussian = AtString("gaussian_filter");
const AtString atstring_filter_closest = AtString("closest_filter");
const AtString atstring_filter_variance = AtString("variance_filter");

node_parameters 
{
    AiMetaDataSetBool(nentry, nullptr, "force_update", true);
}

operator_init
{
    AiNodeSetLocalData(op, new OperatorData());
    return true;
}

operator_cook
{
    OperatorData* operator_data = (OperatorData*)AiNodeGetLocalData(op);
    AtUniverse *universe = AiNodeGetUniverse(op);
    AtNode *camera_node = AiUniverseGetCamera(universe);

    bool cam_is_lentil = true;
    const AtNodeEntry *camera_ne = AiNodeGetNodeEntry(camera_node);
    if (AiNodeEntryGetNameAtString(camera_ne) != AtString("lentil_camera")) cam_is_lentil = false;
    if (!cam_is_lentil) return false;

    
    AtNode *filter_node = AiNodeLookUpByName(universe, AtString("lentil_replaced_filter"));
    if (!filter_node) filter_node = AiNode(universe, AtString("lentil_filter"), AtString("lentil_replaced_filter"));

    AtArray* outputs = AiNodeGetArray(AiUniverseGetOptions(universe), AtString("outputs"));
    const int elements = AiArrayGetNumElements(outputs);
    std::vector<std::string> output_strings;


    for (int i=0; i<elements; i++) {
        std::string output_string = AiArrayGetStr(outputs, i).c_str();

        AOVData aov(universe, output_string);

        AtNode *original_filter_node = AiNodeLookUpByName(universe, AtString(aov.to.filter_tok.c_str()));
        const AtNodeEntry *original_filter_ne = AiNodeGetNodeEntry(original_filter_node);
        const AtString original_filter_ne_name = AiNodeEntryGetNameAtString(original_filter_ne);

        if (original_filter_ne_name != atstring_filter_gaussian && 
            original_filter_ne_name != atstring_filter_closest && 
            original_filter_ne_name != atstring_filter_variance) {
                AiMsgWarning("[LENTIL] Specified AOV filter (%s) is incompatible with Lentil. Defaulting to gaussian_filter.", original_filter_ne_name.c_str());
                aov.original_filter = atstring_filter_gaussian;
        } else {
            aov.original_filter = original_filter_ne_name;
        }

        bool replace_filter = true;

        // lentil unsupported datatype
        if (aov.to.aov_type_tok != "RGBA" && 
            aov.to.aov_type_tok != "RGB" && 
            aov.to.aov_type_tok != "FLOAT" && 
            aov.to.aov_type_tok != "VECTOR") {
            replace_filter = false;
        }

        // never attach filter to the unranked crypto AOVs, they're just for display purposes.
        // ranked aov's are e.g: crypto_material00, crypto_material01, but those are filter swapped after they've been created by cryptomatte
        if (aov.to.aov_name_tok == "crypto_material" || 
            aov.to.aov_name_tok == "crypto_asset" || 
            aov.to.aov_name_tok == "crypto_object"){
            replace_filter = false;
        } else if (aov.to.aov_name_tok.find("crypto_") != std::string::npos){
            // aov.is_crypto = true;
            continue; // don't think this should happen anyway, since they get setup later
        }

        if (replace_filter && aov.to.aov_name_tok != "lentil_replaced_filter"){
            aov.to.filter_tok = "lentil_replaced_filter";
        }

        // identify as duplicate (eg in the case of multiple drivers)
        for (auto &element : operator_data->aovs) {
            if (aov.to.aov_name_tok == element.to.aov_name_tok) {
                aov.is_duplicate = true;
            }
        }
        
        operator_data->aovs.push_back(aov);
    }


    // make a copy of RGBA aov and use it as the basis for the lentil_debug AOV
    // doing this to make sure the whole AOV string is correct, including all the options
    // because some stuff happens in the constructor and we're skipping that here, we need to set these manually
    // AtNode *closest_filter_node = AiNode(universe, AtString("closest_filter"), AtString("closest_filter_lentil"));
    AOVData aov_lentil_debug = operator_data->aovs[0];
    aov_lentil_debug.to.aov_type_tok = "FLOAT";
    aov_lentil_debug.to.aov_name_tok = "lentil_debug";
    // aov_lentil_debug.to.filter_tok = AiNodeGetName(closest_filter_node);
    aov_lentil_debug.to = TokenizedOutputLentil(universe, AtString(aov_lentil_debug.to.rebuild_output().c_str()));
    aov_lentil_debug.name = AtString("lentil_debug");
    aov_lentil_debug.type = string_to_arnold_type(aov_lentil_debug.to.aov_type_tok);
    aov_lentil_debug.original_filter = AtString("closest_filter");
    operator_data->aovs.push_back(aov_lentil_debug);


    AOVData aov_lentil_time = operator_data->aovs[0];
    aov_lentil_time.to.aov_type_tok = "FLOAT";
    aov_lentil_time.to.aov_name_tok = "lentil_time";
    // aov_lentil_debug.to.filter_tok = AiNodeGetName(closest_filter_node);
    aov_lentil_time.to = TokenizedOutputLentil(universe, AtString(aov_lentil_time.to.rebuild_output().c_str()));
    aov_lentil_time.name = AtString("lentil_time");
    aov_lentil_time.type = string_to_arnold_type(aov_lentil_time.to.aov_type_tok);
    // aov_lentil_time.original_filter = AtString(AiNodeGetName(closest_filter_node));
    operator_data->aovs.push_back(aov_lentil_time);


    AOVData aov_lentil_raydir = operator_data->aovs[0];
    aov_lentil_raydir.to.aov_type_tok = "RGB";
    aov_lentil_raydir.to.aov_name_tok = "lentil_raydir";
    aov_lentil_raydir.to = TokenizedOutputLentil(universe, AtString(aov_lentil_raydir.to.rebuild_output().c_str()));
    aov_lentil_raydir.name = AtString("lentil_raydir");
    aov_lentil_raydir.type = string_to_arnold_type(aov_lentil_raydir.to.aov_type_tok);
    operator_data->aovs.push_back(aov_lentil_raydir);


    // need to add an entry to the aov_shaders (NODE)
    AtArray* aov_shaders_array = AiNodeGetArray(AiUniverseGetOptions(universe), AtString("aov_shaders"));
    int aov_shader_array_size = AiArrayGetNumElements(aov_shaders_array);


    AtNode *time_write = AiNode(universe, AtString("aov_write_float"), AtString("lentil_time_write"));
    AtNode *time_read = AiNode(universe, AtString("state_float"), AtString("lentil_time_read"));

    // set time node params/linking
    AiNodeSetStr(time_read, AtString("variable"), AtString("time"));
    AiNodeSetStr(time_write, AtString("aov_name"), AtString("lentil_time"));
    AiNodeLink(time_read, AtString("aov_input"), time_write);
    
    aov_shader_array_size += 1;
    AiArrayResize(aov_shaders_array, aov_shader_array_size, 1);
    AiArraySetPtr(aov_shaders_array, aov_shader_array_size-1, (void*)time_write);
    AiNodeSetArray(AiUniverseGetOptions(universe), AtString("aov_shaders"), aov_shaders_array);


    AtNode *raydir_write = AiNode(universe, AtString("aov_write_rgb"), AtString("lentil_raydir_write"));
    AtNode *raydir_read = AiNode(universe, AtString("state_vector"), AtString("lentil_raydir_read"));

    // set time node params/linking
    AiNodeSetStr(raydir_read, AtString("variable"), AtString("Rd"));
    AiNodeSetStr(raydir_write, AtString("aov_name"), AtString("lentil_raydir"));
    AiNodeLink(raydir_read, AtString("aov_input"), raydir_write);

    aov_shader_array_size += 1;
    AiArrayResize(aov_shaders_array, aov_shader_array_size, 1);
    AiArraySetPtr(aov_shaders_array, aov_shader_array_size-1, (void*)raydir_write);
    AiNodeSetArray(AiUniverseGetOptions(universe), AtString("aov_shaders"), aov_shaders_array);


    // to debug what the operator did
    // AiASSWrite(uni, "/home/cactus/lentil/pota/tests/ilyas_motion_blur_bug/everything.ass", AI_NODE_ALL, 1, 0); 

    return true;
}

operator_post_cook
{
    return true;
}

operator_cleanup
{
    OperatorData* operator_data = (OperatorData*)AiNodeGetLocalData(op);
    delete operator_data;
    return true;
}



void registerLentilOperator(AtNodeLib* node) {
    node->methods = (AtNodeMethods*)LentilOperatorMtd;
    node->name = "lentil_operator";
    node->node_type = AI_NODE_OPERATOR;
    strcpy(node->version, AI_VERSION);
}