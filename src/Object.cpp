/*
 * Object.cpp
 *
 * Created by miles
*/

#include "Object.h"

#include <tiny_obj_loader.h>

std::map<const char*,Object> Object::xObjects;


Object::Object(const char *model)
{
    objFile = objFile + "assets/models/" + model + "/" + model + ".obj";
    obj = new Asset(objFile.c_str());
    mtlFile = mtlFile + "assets/models/" + model + "/" + model + ".mtl";
    mtl = new Asset(mtlFile.c_str());

    tinyobj::ObjReader reader;
    tinyobj::ObjReaderConfig config;
    std::string objData((char*)obj->BlockingGet());
    std::string mtlData((char*)mtl->BlockingGet());
    reader.ParseFromString(objData, mtlData, config);
    auto &attrib = reader.GetAttrib();
    auto &shapes = reader.GetShapes();
    auto &materials = reader.GetMaterials();

    std::vector<uint32_t> mm;
    mm.resize(materials.size());
    uint32_t i = 0;
    for(auto m: materials){
        Material::MaterialData md;
        md.col[0] = m.diffuse[0];
        md.col[1] = m.diffuse[1];
        md.col[2] = m.diffuse[2];
        md.col[3] = m.dissolve;

        md.roughness = m.roughness;
        md.metalness = m.metallic;
        md.emission[0] = m.emission[0];
        md.emission[1] = m.emission[1];
        md.emission[2] = m.emission[2];

        mm[i++] = Material::Create(md);
    }

    for (size_t s = 0; s < shapes.size(); s++) {
        size_t index_offset = 0;
        auto& mesh = xMesh.emplace_back();
        mesh.SetMaterial(mm[shapes[s].mesh.material_ids[0]]);
        auto vertexData = mesh.GetVertexBuffer(shapes[s].mesh.num_face_vertices.size() * 3);
        uint32_t CV = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

            // Loop over vertices in the face.
            for (size_t v = 0; v < fv; v++) {
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                vertexData[CV].pos[0] = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                vertexData[CV].pos[1] = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                vertexData[CV].pos[2] = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

                vertexData[CV].col[0] = 1;
                vertexData[CV].col[1] = 1;
                vertexData[CV].col[2] = 1;
                vertexData[CV].col[3] = 1;

                if (idx.normal_index >= 0) {
                    vertexData[CV].norm[0] = attrib.normals[3 * size_t(idx.normal_index) + 0];
                    vertexData[CV].norm[1] = attrib.normals[3 * size_t(idx.normal_index) + 1];
                    vertexData[CV].norm[2] = attrib.normals[3 * size_t(idx.normal_index) + 2];
                }

                CV++;
            }

            index_offset += fv;
        }

        mesh.GetGpuBuffer();
    }
}


void Object::Render(uint32_t count, uint32_t offset)
{
    for(auto m: xMesh){
        m.BindMaterial();
        GPU::Encoder().SetVertexBuffer(0, m.GetGpuBuffer(), 0, m.GetBufferSize());
        GPU::Encoder().Draw(m.GetVertexCount(), count, 0, offset);
    }
}
