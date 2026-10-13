/* SPDX-License-Identifier: MS-PL */
/* Built-in vertex structures, declarations and value semantics from the C ABI. */
#include <CNA/C/cna.h>

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "CnaDemos/CApiDemo.h"

typedef struct ExpectedElement {
    int32_t offset;
    CNA_VertexElementFormat format;
    CNA_VertexElementUsage usage;
} ExpectedElement;

typedef struct VertexCase {
    CNA_VertexType type;
    uint32_t stride;
    uint64_t element_count;
    const ExpectedElement* elements;
    const char* text;
} VertexCase;

static const ExpectedElement PositionColorElements[] = {
    {0, CNA_VERTEX_ELEMENT_FORMAT_VECTOR3, CNA_VERTEX_ELEMENT_USAGE_POSITION},
    {12, CNA_VERTEX_ELEMENT_FORMAT_COLOR, CNA_VERTEX_ELEMENT_USAGE_COLOR}
};
static const ExpectedElement PositionColorTextureElements[] = {
    {0, CNA_VERTEX_ELEMENT_FORMAT_VECTOR3, CNA_VERTEX_ELEMENT_USAGE_POSITION},
    {12, CNA_VERTEX_ELEMENT_FORMAT_COLOR, CNA_VERTEX_ELEMENT_USAGE_COLOR},
    {16, CNA_VERTEX_ELEMENT_FORMAT_VECTOR2, CNA_VERTEX_ELEMENT_USAGE_TEXTURE_COORDINATE}
};
static const ExpectedElement PositionNormalTangentTextureElements[] = {
    {0, CNA_VERTEX_ELEMENT_FORMAT_VECTOR3, CNA_VERTEX_ELEMENT_USAGE_POSITION},
    {12, CNA_VERTEX_ELEMENT_FORMAT_VECTOR3, CNA_VERTEX_ELEMENT_USAGE_NORMAL},
    {24, CNA_VERTEX_ELEMENT_FORMAT_VECTOR4, CNA_VERTEX_ELEMENT_USAGE_TANGENT},
    {40, CNA_VERTEX_ELEMENT_FORMAT_VECTOR2, CNA_VERTEX_ELEMENT_USAGE_TEXTURE_COORDINATE}
};
static const ExpectedElement PositionNormalTangentTextureSkinnedElements[] = {
    {0, CNA_VERTEX_ELEMENT_FORMAT_VECTOR3, CNA_VERTEX_ELEMENT_USAGE_POSITION},
    {12, CNA_VERTEX_ELEMENT_FORMAT_VECTOR3, CNA_VERTEX_ELEMENT_USAGE_NORMAL},
    {24, CNA_VERTEX_ELEMENT_FORMAT_VECTOR4, CNA_VERTEX_ELEMENT_USAGE_TANGENT},
    {40, CNA_VERTEX_ELEMENT_FORMAT_VECTOR2, CNA_VERTEX_ELEMENT_USAGE_TEXTURE_COORDINATE},
    {48, CNA_VERTEX_ELEMENT_FORMAT_VECTOR4, CNA_VERTEX_ELEMENT_USAGE_BLEND_WEIGHT},
    {64, CNA_VERTEX_ELEMENT_FORMAT_BYTE4, CNA_VERTEX_ELEMENT_USAGE_BLEND_INDICES}
};
static const ExpectedElement PositionNormalTextureElements[] = {
    {0, CNA_VERTEX_ELEMENT_FORMAT_VECTOR3, CNA_VERTEX_ELEMENT_USAGE_POSITION},
    {12, CNA_VERTEX_ELEMENT_FORMAT_VECTOR3, CNA_VERTEX_ELEMENT_USAGE_NORMAL},
    {24, CNA_VERTEX_ELEMENT_FORMAT_VECTOR2, CNA_VERTEX_ELEMENT_USAGE_TEXTURE_COORDINATE}
};
static const ExpectedElement PositionNormalTextureSkinnedElements[] = {
    {0, CNA_VERTEX_ELEMENT_FORMAT_VECTOR3, CNA_VERTEX_ELEMENT_USAGE_POSITION},
    {12, CNA_VERTEX_ELEMENT_FORMAT_VECTOR3, CNA_VERTEX_ELEMENT_USAGE_NORMAL},
    {24, CNA_VERTEX_ELEMENT_FORMAT_VECTOR2, CNA_VERTEX_ELEMENT_USAGE_TEXTURE_COORDINATE},
    {32, CNA_VERTEX_ELEMENT_FORMAT_VECTOR4, CNA_VERTEX_ELEMENT_USAGE_BLEND_WEIGHT},
    {48, CNA_VERTEX_ELEMENT_FORMAT_BYTE4, CNA_VERTEX_ELEMENT_USAGE_BLEND_INDICES}
};
static const ExpectedElement PositionTextureElements[] = {
    {0, CNA_VERTEX_ELEMENT_FORMAT_VECTOR3, CNA_VERTEX_ELEMENT_USAGE_POSITION},
    {12, CNA_VERTEX_ELEMENT_FORMAT_VECTOR2, CNA_VERTEX_ELEMENT_USAGE_TEXTURE_COORDINATE}
};

static const VertexCase Cases[] = {
    {CNA_VERTEX_TYPE_POSITION_COLOR, 16U, 2U, PositionColorElements,
     "{{Position:{X:1 Y:2 Z:3} Color:{R:4 G:5 B:6 A:7}}}"},
    {CNA_VERTEX_TYPE_POSITION_COLOR_TEXTURE, 24U, 3U, PositionColorTextureElements,
     "{{Position:{X:1 Y:2 Z:3} Color:{R:4 G:5 B:6 A:7} "
     "TextureCoordinate:{X:11 Y:12}}}"},
    {CNA_VERTEX_TYPE_POSITION_NORMAL_TANGENT_TEXTURE, 48U, 4U,
     PositionNormalTangentTextureElements,
     "{{Position:{X:1 Y:2 Z:3} Normal:{X:4 Y:5 Z:6} Tangent:{X:7 Y:8 Z:9 W:10} "
     "TextureCoordinate:{X:11 Y:12}}}"},
    {CNA_VERTEX_TYPE_POSITION_NORMAL_TANGENT_TEXTURE_SKINNED, 68U, 6U,
     PositionNormalTangentTextureSkinnedElements,
     "{{Position:{X:1 Y:2 Z:3} Normal:{X:4 Y:5 Z:6} Tangent:{X:7 Y:8 Z:9 W:10} "
     "TextureCoordinate:{X:11 Y:12} BlendWeight:{X:13 Y:14 Z:15 W:16} "
     "BlendIndices:{1 2 3 4}}}"},
    {CNA_VERTEX_TYPE_POSITION_NORMAL_TEXTURE, 32U, 3U, PositionNormalTextureElements,
     "{{Position:{X:1 Y:2 Z:3} Normal:{X:4 Y:5 Z:6} "
     "TextureCoordinate:{X:11 Y:12}}}"},
    {CNA_VERTEX_TYPE_POSITION_NORMAL_TEXTURE_SKINNED, 52U, 5U,
     PositionNormalTextureSkinnedElements,
     "{{Position:{X:1 Y:2 Z:3} Normal:{X:4 Y:5 Z:6} TextureCoordinate:{X:11 Y:12} "
     "BlendWeight:{X:13 Y:14 Z:15 W:16} BlendIndices:{1 2 3 4}}}"},
    {CNA_VERTEX_TYPE_POSITION_TEXTURE, 20U, 2U, PositionTextureElements,
     "{{Position:{X:1 Y:2 Z:3} TextureCoordinate:{X:11 Y:12}}}"}
};

static int bytes_are(const void* data, size_t count, uint8_t expected)
{
    const uint8_t* bytes = (const uint8_t*)data;
    size_t index;
    for (index = 0U; index < count; ++index) {
        if (bytes[index] != expected) {
            return 0;
        }
    }
    return 1;
}

static uint32_t structure_size(CNA_VertexType type)
{
    switch (type) {
        case CNA_VERTEX_TYPE_POSITION_COLOR:
            return (uint32_t)sizeof(CNA_VertexPositionColor);
        case CNA_VERTEX_TYPE_POSITION_COLOR_TEXTURE:
            return (uint32_t)sizeof(CNA_VertexPositionColorTexture);
        case CNA_VERTEX_TYPE_POSITION_NORMAL_TANGENT_TEXTURE:
            return (uint32_t)sizeof(CNA_VertexPositionNormalTangentTexture);
        case CNA_VERTEX_TYPE_POSITION_NORMAL_TANGENT_TEXTURE_SKINNED:
            return (uint32_t)sizeof(CNA_VertexPositionNormalTangentTextureSkinned);
        case CNA_VERTEX_TYPE_POSITION_NORMAL_TEXTURE:
            return (uint32_t)sizeof(CNA_VertexPositionNormalTexture);
        case CNA_VERTEX_TYPE_POSITION_NORMAL_TEXTURE_SKINNED:
            return (uint32_t)sizeof(CNA_VertexPositionNormalTextureSkinned);
        default:
            return (uint32_t)sizeof(CNA_VertexPositionTexture);
    }
}

static CNA_VertexValue sample_vertex(CNA_VertexType type)
{
    const CNA_Vector3 position = {1.0f, 2.0f, 3.0f};
    const CNA_Vector3 normal = {4.0f, 5.0f, 6.0f};
    const CNA_Vector4 tangent = {7.0f, 8.0f, 9.0f, 10.0f};
    const CNA_Vector2 texture = {11.0f, 12.0f};
    const CNA_Vector4 weight = {13.0f, 14.0f, 15.0f, 16.0f};
    const CNA_Color color = {4U, 5U, 6U, 7U};
    CNA_VertexValue result;
    memset(&result, 0, sizeof(result));
    switch (type) {
        case CNA_VERTEX_TYPE_POSITION_COLOR:
            result.position_color = (CNA_VertexPositionColor){position, color};
            break;
        case CNA_VERTEX_TYPE_POSITION_COLOR_TEXTURE:
            result.position_color_texture =
                (CNA_VertexPositionColorTexture){position, color, texture};
            break;
        case CNA_VERTEX_TYPE_POSITION_NORMAL_TANGENT_TEXTURE:
            result.position_normal_tangent_texture =
                (CNA_VertexPositionNormalTangentTexture){position, normal, tangent, texture};
            break;
        case CNA_VERTEX_TYPE_POSITION_NORMAL_TANGENT_TEXTURE_SKINNED:
            result.position_normal_tangent_texture_skinned =
                (CNA_VertexPositionNormalTangentTextureSkinned){
                    position, normal, tangent, texture, weight, {1U, 2U, 3U, 4U}};
            break;
        case CNA_VERTEX_TYPE_POSITION_NORMAL_TEXTURE:
            result.position_normal_texture =
                (CNA_VertexPositionNormalTexture){position, normal, texture};
            break;
        case CNA_VERTEX_TYPE_POSITION_NORMAL_TEXTURE_SKINNED:
            result.position_normal_texture_skinned =
                (CNA_VertexPositionNormalTextureSkinned){
                    position, normal, texture, weight, {1U, 2U, 3U, 4U}};
            break;
        default:
            result.position_texture = (CNA_VertexPositionTexture){position, texture};
            break;
    }
    return result;
}

static void mutate_position(CNA_VertexType type, CNA_VertexValue* value)
{
    switch (type) {
        case CNA_VERTEX_TYPE_POSITION_COLOR:
            value->position_color.position.x = -1.0f;
            break;
        case CNA_VERTEX_TYPE_POSITION_COLOR_TEXTURE:
            value->position_color_texture.position.x = -1.0f;
            break;
        case CNA_VERTEX_TYPE_POSITION_NORMAL_TANGENT_TEXTURE:
            value->position_normal_tangent_texture.position.x = -1.0f;
            break;
        case CNA_VERTEX_TYPE_POSITION_NORMAL_TANGENT_TEXTURE_SKINNED:
            value->position_normal_tangent_texture_skinned.position.x = -1.0f;
            break;
        case CNA_VERTEX_TYPE_POSITION_NORMAL_TEXTURE:
            value->position_normal_texture.position.x = -1.0f;
            break;
        case CNA_VERTEX_TYPE_POSITION_NORMAL_TEXTURE_SKINNED:
            value->position_normal_texture_skinned.position.x = -1.0f;
            break;
        default:
            value->position_texture.position.x = -1.0f;
            break;
    }
}

static int all_defaults_are_zero(void)
{
    size_t index;
    for (index = 0U; index < sizeof(Cases) / sizeof(Cases[0]); ++index) {
        CNA_VertexValue value;
        memset(&value, 0xA5, sizeof(value));
        if (cna_vertex_value_init_default(Cases[index].type, &value) != CNA_RESULT_SUCCESS ||
            !bytes_are(&value, sizeof(value), 0U)) {
            return 0;
        }
    }
    return 1;
}

static int all_layouts_are_canonical(void)
{
    size_t case_index;
    for (case_index = 0U; case_index < sizeof(Cases) / sizeof(Cases[0]); ++case_index) {
        const VertexCase* test_case = &Cases[case_index];
        CNA_VertexElement elements[6];
        CNA_VertexElement sentinel[6];
        uint32_t stride = 0U;
        uint64_t count = 0U;
        uint64_t index;
        memset(elements, 0x5A, sizeof(elements));
        memcpy(sentinel, elements, sizeof(elements));
        if (cna_vertex_type_get_stride(test_case->type, &stride) != CNA_RESULT_SUCCESS ||
            stride != test_case->stride || stride != structure_size(test_case->type) ||
            cna_vertex_type_copy_elements(test_case->type, elements,
                                          test_case->element_count - 1U, &count) !=
                CNA_RESULT_BUFFER_TOO_SMALL ||
            count != test_case->element_count ||
            memcmp(elements, sentinel, sizeof(elements)) != 0 ||
            cna_vertex_type_copy_elements(test_case->type, elements, 6U, &count) !=
                CNA_RESULT_SUCCESS ||
            count != test_case->element_count) {
            return 0;
        }
        for (index = 0U; index < count; ++index) {
            if (elements[index].offset != test_case->elements[index].offset ||
                elements[index].format != test_case->elements[index].format ||
                elements[index].usage != test_case->elements[index].usage ||
                elements[index].usage_index != 0U) {
                return 0;
            }
        }
    }
    return 1;
}

static int all_value_semantics_hold(void)
{
    size_t index;
    for (index = 0U; index < sizeof(Cases) / sizeof(Cases[0]); ++index) {
        CNA_VertexValue value = sample_vertex(Cases[index].type);
        CNA_VertexValue same = value;
        CNA_VertexValue different = value;
        CNA_Bool predicate = CNA_FALSE;
        int32_t hash = 0;
        int32_t same_hash = 1;
        mutate_position(Cases[index].type, &different);
        if (cna_vertex_value_equals(Cases[index].type, &value, &same, &predicate) !=
                CNA_RESULT_SUCCESS ||
            predicate != CNA_TRUE ||
            cna_vertex_value_equals(Cases[index].type, &value, &different, &predicate) !=
                CNA_RESULT_SUCCESS ||
            predicate != CNA_FALSE ||
            cna_vertex_value_not_equals(Cases[index].type, &value, &different, &predicate) !=
                CNA_RESULT_SUCCESS ||
            predicate != CNA_TRUE ||
            cna_vertex_value_get_hash_code(Cases[index].type, &value, &hash) !=
                CNA_RESULT_SUCCESS ||
            cna_vertex_value_get_hash_code(Cases[index].type, &same, &same_hash) !=
                CNA_RESULT_SUCCESS ||
            hash != same_hash) {
            return 0;
        }
    }
    return 1;
}

static int all_value_strings_are_exact(void)
{
    size_t index;
    for (index = 0U; index < sizeof(Cases) / sizeof(Cases[0]); ++index) {
        const CNA_VertexValue value = sample_vertex(Cases[index].type);
        const uint64_t expected = (uint64_t)strlen(Cases[index].text);
        char output[256];
        uint64_t count = 0U;
        memset(output, 0x5A, sizeof(output));
        if (cna_vertex_value_get_string_byte_count(Cases[index].type, &value, &count) !=
                CNA_RESULT_SUCCESS ||
            count != expected ||
            cna_vertex_value_copy_string(Cases[index].type, &value, output, expected, &count) !=
                CNA_RESULT_SUCCESS ||
            count != expected || memcmp(output, Cases[index].text, (size_t)expected) != 0 ||
            (uint8_t)output[expected] != UINT8_C(0x5A)) {
            return 0;
        }
    }
    return 1;
}

static int short_value_string_is_atomic(void)
{
    const CNA_VertexValue value =
        sample_vertex(CNA_VERTEX_TYPE_POSITION_NORMAL_TANGENT_TEXTURE_SKINNED);
    char output[8];
    uint64_t count = 0U;
    memset(output, 0x5A, sizeof(output));
    return cna_vertex_value_copy_string(
               CNA_VERTEX_TYPE_POSITION_NORMAL_TANGENT_TEXTURE_SKINNED,
               &value, output, sizeof(output), &count) == CNA_RESULT_BUFFER_TOO_SMALL &&
           count == strlen(Cases[3].text) && bytes_are(output, sizeof(output), UINT8_C(0x5A));
}

static int vertex_element_semantics_hold(void)
{
    const CNA_VertexElement value = {
        20, CNA_VERTEX_ELEMENT_FORMAT_VECTOR4, CNA_VERTEX_ELEMENT_USAGE_TANGENT, 3U
    };
    CNA_VertexElement different = value;
    static const char Expected[] =
        "{Offset:20 Format:Vector4 Usage:Tangent UsageIndex:3}";
    CNA_Bool predicate = CNA_FALSE;
    int32_t hash = 0;
    int32_t same_hash = 1;
    uint64_t count = 0U;
    char output[sizeof(Expected)];
    different.usage_index = 4U;
    memset(output, 0x5A, sizeof(output));
    return cna_vertex_element_equals(value, value, &predicate) == CNA_RESULT_SUCCESS &&
           predicate == CNA_TRUE &&
           cna_vertex_element_equals(value, different, &predicate) == CNA_RESULT_SUCCESS &&
           predicate == CNA_FALSE &&
           cna_vertex_element_not_equals(value, different, &predicate) == CNA_RESULT_SUCCESS &&
           predicate == CNA_TRUE &&
           cna_vertex_element_get_hash_code(value, &hash) == CNA_RESULT_SUCCESS &&
           cna_vertex_element_get_hash_code(value, &same_hash) == CNA_RESULT_SUCCESS &&
           hash == same_hash &&
           cna_vertex_element_get_string_byte_count(value, &count) == CNA_RESULT_SUCCESS &&
           count == sizeof(Expected) - 1U &&
           cna_vertex_element_copy_string(value, output, sizeof(Expected) - 1U, &count) ==
               CNA_RESULT_SUCCESS &&
           count == sizeof(Expected) - 1U &&
           memcmp(output, Expected, sizeof(Expected) - 1U) == 0 &&
           (uint8_t)output[sizeof(Expected) - 1U] == UINT8_C(0x5A);
}

static int invalid_identities_are_atomic(void)
{
    const CNA_VertexType invalid_type = CNA_VERTEX_TYPE_POSITION_TEXTURE + 1U;
    const CNA_VertexValue input = sample_vertex(CNA_VERTEX_TYPE_POSITION_TEXTURE);
    CNA_VertexValue output;
    CNA_VertexValue sentinel;
    CNA_VertexElement element = {
        1, CNA_VERTEX_ELEMENT_FORMAT_SINGLE, CNA_VERTEX_ELEMENT_USAGE_POSITION, 0U
    };
    CNA_Bool predicate = UINT8_C(77);
    int32_t hash = INT32_C(77);
    uint32_t stride = UINT32_C(77);
    uint64_t count = UINT64_C(77);
    memset(&output, 0xA5, sizeof(output));
    sentinel = output;
    if (cna_vertex_value_init_default(invalid_type, &output) != CNA_RESULT_INVALID_ARGUMENT ||
        memcmp(&output, &sentinel, sizeof(output)) != 0 ||
        cna_vertex_value_equals(invalid_type, &input, &input, &predicate) !=
            CNA_RESULT_INVALID_ARGUMENT ||
        predicate != UINT8_C(77) ||
        cna_vertex_value_get_hash_code(invalid_type, &input, &hash) !=
            CNA_RESULT_INVALID_ARGUMENT ||
        hash != INT32_C(77) ||
        cna_vertex_type_get_stride(invalid_type, &stride) != CNA_RESULT_INVALID_ARGUMENT ||
        stride != UINT32_C(77)) {
        return 0;
    }
    element.format = UINT32_C(99);
    return cna_vertex_element_not_equals(element, element, &predicate) ==
               CNA_RESULT_INVALID_ARGUMENT &&
           predicate == UINT8_C(77) &&
           cna_vertex_element_get_string_byte_count(element, &count) ==
               CNA_RESULT_INVALID_ARGUMENT &&
           count == UINT64_C(77);
}

int main(int argc, char** argv)
{
    CnaDemo demo;
    cna_demo_init(&demo, "c_api.graphics.vertex_values", argc, argv, 0);
    cna_demo_check(&demo, all_defaults_are_zero(),
                   "all seven built-in vertex defaults initialize their complete union to zero");
    cna_demo_check(&demo, all_layouts_are_canonical(),
                   "all packed strides match their C structures and exact canonical elements");
    cna_demo_check(&demo, all_value_semantics_hold(),
                   "all vertex families compare complete values and hash equal values equally");
    cna_demo_check(&demo, all_value_strings_are_exact(),
                   "all vertex families expose exact field-bearing text without a terminator");
    cna_demo_check(&demo, short_value_string_is_atomic(),
                   "a short vertex-text buffer reports its size without partial output");
    cna_demo_check(&demo, vertex_element_semantics_hold(),
                   "a declaration element compares, hashes and formats all four fields");
    cna_demo_check(&demo, invalid_identities_are_atomic(),
                   "invalid vertex and element identities preserve every caller output");
    return cna_demo_finish(&demo);
}
