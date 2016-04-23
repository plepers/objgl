
#include "main.h"
#include "obj.h"
#include "collapser.h"

using namespace TCLAP;
using namespace std;


// https://github.com/huamulan/OpenGL-tutorial/blob/47edede8273f16b0c89c0573cf2ea198c77814e7/common/tangentspace.cpp

void computeBinormals( ObjModel objfile, int i, float* res )
{
    glm::vec3 v0 = glm::make_vec3( objfile.vertices[objfile.faces[i].vert_indices[0]].xyzw );
    glm::vec3 v1 = glm::make_vec3( objfile.vertices[objfile.faces[i].vert_indices[1]].xyzw );
    glm::vec3 v2 = glm::make_vec3( objfile.vertices[objfile.faces[i].vert_indices[2]].xyzw );

    glm::vec2 uv0 = glm::make_vec2( objfile.texCoords[objfile.faces[i].uvw_indices[0]].uvw );
    glm::vec2 uv1 = glm::make_vec2( objfile.texCoords[objfile.faces[i].uvw_indices[1]].uvw );
    glm::vec2 uv2 = glm::make_vec2( objfile.texCoords[objfile.faces[i].uvw_indices[2]].uvw );

    // Edges of the triangle : postion delta
    glm::vec3 deltaPos1 = v1-v0;
    glm::vec3 deltaPos2 = v2-v0;

    // UV delta
    glm::vec2 deltaUV1 = uv1-uv0;
    glm::vec2 deltaUV2 = uv2-uv0;

    float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);

    glm::vec3 tangent = (deltaPos1 * deltaUV2.y   - deltaPos2 * deltaUV1.y)*r;
    glm::vec3 bitangent = (deltaPos2 * deltaUV1.x   - deltaPos1 * deltaUV2.x)*r;

    for (unsigned int j=0; j<3; j+=1 )
    {
        glm::vec3 n = glm::make_vec3( objfile.normals[objfile.faces[i].norm_indices[j]].ijk );
        glm::vec3 t;
        glm::vec3 b;

        // Gram-Schmidt orthogonalize
        t = glm::normalize(tangent   - n * glm::dot(n, tangent  ));
        b = glm::normalize(bitangent - n * glm::dot(n, bitangent));

        // Calculate handedness
        if (glm::dot(glm::cross(n, t), b) < 0.0f){
            t = t * -1.0f;
        }

        res[j*6+0] = t.x;
        res[j*6+1] = t.y;
        res[j*6+2] = t.z;
        res[j*6+3] = b.x;
        res[j*6+4] = b.y;
        res[j*6+5] = b.z;

    }
}

char* indicesToU8( unsigned int *indices, unsigned int length ){
    unsigned char *res = (unsigned char*) malloc( length*sizeof(char) );
    for (int i=0; i< length; i++ ) {
        res[i] = indices[i];
    }
    return (char*)res;
}

char* indicesToU16( unsigned int *indices, unsigned int length ){
    unsigned short *res = (unsigned short*) malloc( length*sizeof(short) );
    for (int i=0; i< length; i++ ) {
        res[i] = indices[i];
    }
    return (char*)res;
}

char* indicesToU32( unsigned int *indices, unsigned int length ){
    return (char*)indices;
}


int main(int argc, char* argv[])
{

    CmdLine cmd("objgl - export obj to opengl buffers", ' ', "0.1");

    ValueArg<string> a_input ( "i", "input","input obj file",     true,  "", "string", cmd );
    ValueArg<string> a_output( "o", "output","output binary file",true,  "", "string", cmd );
    ValueArg<float>  a_scale ( "s", "scale", "scale positions",   false, 1.0,"number", cmd );

    SwitchArg o_export("c","collapse", "collapse duplicates", true);

    SwitchArg p_export("p","positions", "export Position", false);
    SwitchArg t_export("t","uvs",       "export Uvs", false);
    SwitchArg n_export("n","normals",   "export Normals", false);
    SwitchArg b_export("b","binormals", "generate tangents and binormals", false);
    SwitchArg C_export("C","compress",  "compress normals tangents and binormals", false);

    cmd.add( p_export );
    cmd.add( t_export );
    cmd.add( n_export );
    cmd.add( o_export );
    cmd.add( b_export );
    cmd.add( C_export );

    cmd.parse( argc, argv );

    const char* input    = a_input.getValue().c_str();
    const char* output   = a_output.getValue().c_str();
    const float scaleOpt = a_scale.getValue();

    bool doExportP  = p_export.getValue();
    bool doExportT  = t_export.getValue();
    bool doExportN  = n_export.getValue();
    bool doExportB  = b_export.getValue();
    bool doCollapse = o_export.getValue();
    bool doCompress = C_export.getValue();

    //==================================================
    //                       Load obj file
    //==================================================


    ObjModel objfile = {};
    if (!ReadOBJModel (input, &objfile))
        exit (EXIT_FAILURE);


    if( ! objfile.has_normals ) 	  doExportN = false;
    if( ! objfile.has_texCoords ) doExportT = false;

    if( ! objfile.has_normals && ! objfile.has_texCoords )
        doExportB = false;



    // inline buffers

    int lPolygonVertexCount = objfile.num_faces * 3;


    int vsize = 3;

    if (doExportT)
        vsize += 2;

    if (doExportN)
        vsize += 3;

    if (doExportB)
        vsize += 6;

    int numVertices = lPolygonVertexCount;
    int buffersize = vsize * lPolygonVertexCount;
    float *buffer = new float[ buffersize ];

    unsigned int *indices = new unsigned int[ lPolygonVertexCount ];

    int i, j;
    int c = 0;

    float *binorms = new float[18];

    for (i = 0; i < objfile.num_faces; ++i)
    {
        indices[i*3+0] = i*3+0;
        indices[i*3+1] = i*3+1;
        indices[i*3+2] = i*3+2;

        if( doExportB ){
            computeBinormals( objfile, i, binorms );
        }

        for (j = 0; j < 3; ++j)
        {

            buffer[c++] = scaleOpt * objfile.vertices[objfile.faces[i].vert_indices[j]].xyzw[0];
            buffer[c++] = scaleOpt * objfile.vertices[objfile.faces[i].vert_indices[j]].xyzw[1];
            buffer[c++] = scaleOpt * objfile.vertices[objfile.faces[i].vert_indices[j]].xyzw[2];


            if (doExportT) {
                buffer[c++] = objfile.texCoords[objfile.faces[i].uvw_indices[j]].uvw[0];
                buffer[c++] = objfile.texCoords[objfile.faces[i].uvw_indices[j]].uvw[1];
            }

            if (doExportN) {
                buffer[c++] = objfile.normals[objfile.faces[i].norm_indices[j]].ijk[0];
                buffer[c++] = objfile.normals[objfile.faces[i].norm_indices[j]].ijk[1];
                buffer[c++] = objfile.normals[objfile.faces[i].norm_indices[j]].ijk[2];
            }

            if( doExportB ){
                //tangent
                buffer[c++] = binorms[j*6+0];
                buffer[c++] = binorms[j*6+1];
                buffer[c++] = binorms[j*6+2];
                //binormal
                buffer[c++] = binorms[j*6+3];
                buffer[c++] = binorms[j*6+4];
                buffer[c++] = binorms[j*6+5];
            }

        }
    }

    Collapser *collapser;
    unsigned int* sIndices = new unsigned int[ lPolygonVertexCount ];

    if( doCollapse ){
        // collapse duplicated vertices
        // before generate finals submeshes
        //

        collapser = new Collapser( indices, lPolygonVertexCount, lPolygonVertexCount );
        Stream* stream = collapser->addStream( buffer, vsize );
        collapser->collapse();

        buffer = stream->remap;
        numVertices = collapser->getCollapsedNumVertices();
        buffersize = collapser->getCollapsedNumVertices()*vsize;

        for (int i = 0; i< lPolygonVertexCount; i++) {
            sIndices[i] = (unsigned int)( indices[i] );
        }

    }


	// save
    FILE* file;
    file = fopen(output, "wb" );
    if(!file) return 79;
    
    IndexFormat iFormat;
    
    if( numVertices > 0xFFFF )
        iFormat = INT;
    else if( numVertices > 0xFF )
        iFormat = SHORT;
    else
        iFormat = CHAR;
    
    unsigned int flags =
        ((doExportP ? 1 : 0)     ) |
        ((doExportT ? 1 : 0) << 1) |
        ((doExportN ? 1 : 0) << 2) |
        ((doExportB ? 1 : 0) << 3) |
        (char)iFormat << 4;

    if( doCollapse ) {

        printf ("nindices : %i \n nvertices : %i\n flags : %i \n iformat %i - %i\n", lPolygonVertexCount, buffersize, flags, iFormat, numVertices );
        fwrite(&lPolygonVertexCount , 1 , sizeof(int), file);
        fwrite(&buffersize ,          1 , sizeof(int), file);
        fwrite(&flags ,               1 , sizeof(int), file);
        
        if( iFormat == INT )
            fwrite( indicesToU32(sIndices, lPolygonVertexCount) , 1 , lPolygonVertexCount*sizeof(int), file);
        else if( iFormat == SHORT )
            fwrite( indicesToU16(sIndices, lPolygonVertexCount) , 1 , lPolygonVertexCount*sizeof(short), file);
        else
            fwrite( indicesToU8(sIndices, lPolygonVertexCount) , 1 , lPolygonVertexCount*sizeof(char), file);
    }

    fwrite((char*)buffer , 1 , buffersize*sizeof(float), file);

    fclose(file);
    return 0;

    if( doCollapse ){
        delete collapser;
        collapser = NULL;
    }

    FreeModel(&objfile);


    return 0;
}


