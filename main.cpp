
#include "main.h"
#include "collapser.h"

#define TINYOBJLOADER_IMPLEMENTATION // define this in only *one* .cc
#include "tiny_obj_loader.h"


using namespace TCLAP;
using namespace std;


float rounded( float n ){
//    return n;
    n = round(n*100000.0) / 100000.0;
    if( n == -0.0 ) n = 0.0;
    return n;
}

// https://github.com/huamulan/OpenGL-tutorial/blob/47edede8273f16b0c89c0573cf2ea198c77814e7/common/tangentspace.cpp

inline void computeBinormals( tinyobj::mesh_t *mesh, int i, float* res )
{
    int i0 = mesh->indices[i*3+0];
    int i1 = mesh->indices[i*3+1];
    int i2 = mesh->indices[i*3+2];

    glm::vec3 v0 = glm::make_vec3( &mesh->positions[i0*3] );
    glm::vec3 v1 = glm::make_vec3( &mesh->positions[i1*3] );
    glm::vec3 v2 = glm::make_vec3( &mesh->positions[i2*3] );

    glm::vec2 uv0 = glm::make_vec2( &mesh->texcoords[i0*2] );
    glm::vec2 uv1 = glm::make_vec2( &mesh->texcoords[i1*2] );
    glm::vec2 uv2 = glm::make_vec2( &mesh->texcoords[i2*2] );

    // Edges of the triangle : postion delta
    glm::vec3 deltaPos1 = v1-v0;
    glm::vec3 deltaPos2 = v2-v0;

    // UV delta
    glm::vec2 deltaUV1 = uv1-uv0;
    glm::vec2 deltaUV2 = uv2-uv0;

    float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);

//    glm::vec3 tangent = (deltaPos1 * deltaUV2.y   - deltaPos2 * deltaUV1.y)*r;
    glm::vec3 bitangent = (deltaPos2 * deltaUV1.x   - deltaPos1 * deltaUV2.x)*r;

    for (unsigned int j=0; j<3; j+=1 )
    {
        i0 = mesh->indices[i*3+j];

        glm::vec3 n = glm::make_vec3( &mesh->normals[i0*3] );
        glm::vec3 t;
        glm::vec3 b;

        t = glm::normalize( glm::cross(n, bitangent) );
        b = glm::normalize( glm::cross(n, t ) );
        
        res[j*6+0] = rounded( t.x );
        res[j*6+1] = rounded( t.y );
        res[j*6+2] = rounded( t.z );
        res[j*6+3] = rounded( b.x );
        res[j*6+4] = rounded( b.y );
        res[j*6+5] = rounded( b.z );

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

    CmdLine cmd("objgl - export obj to opengl buffers", ' ', "0.3");

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


    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string err;
    bool ret = tinyobj::LoadObj(shapes, materials, err, input);

    if (!err.empty()) { // `err` may contain warning message.
        std::cerr << err << std::endl;
    }

    if (!ret) {
        exit(1);
    }


    bool has_normals   = shapes[0].mesh.normals.size() > 0;
    bool has_texCoords = shapes[0].mesh.texcoords.size() > 0;

    if( ! has_normals )    doExportN = false;
    if( ! has_texCoords )  doExportT = false;

    if( ! has_normals || ! has_texCoords )
        doExportB = false;



    // inline buffers

    int lPolygonVertexCount = 0;
    for (int i = 0; i<shapes.size(); i++ ) {
        lPolygonVertexCount += shapes[i].mesh.indices.size();
    }

//    int lPolygonVertexCount = objfile.num_faces * 3;


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
    //printf("alloc buffer with %i bytes", buffersize*4);

    unsigned int *indices = new unsigned int[ lPolygonVertexCount ];

    int i, j;
    int c = 0, d=0;

    float *binorms = new float[18];
    
    mat_map *matMap = new mat_map[256];
    int currMatMap = 0;
    
    printf("num shapes %lu \n", shapes.size() );
    for( int shapeIndex = 0; shapeIndex < shapes.size(); shapeIndex++ ) {

        tinyobj::mesh_t mesh = shapes[shapeIndex].mesh;
        int currMatId = -2;
        unsigned int num_faces = mesh.indices.size()/3;
        unsigned int parsed_faces = 0;
        
        while( parsed_faces < num_faces ){
            
            currMatId++;
            int beginI = d;
            
            
            for (i = 0; i < num_faces; i++ )
            {
                
                int mat = mesh.material_ids[i];

                if( mat != currMatId ){
                    continue;
                }
                
                
                indices[d++] = parsed_faces*3+0;
                indices[d++] = parsed_faces*3+1;
                indices[d++] = parsed_faces*3+2;
                
                parsed_faces ++;
                
                if( doExportB ){
                    computeBinormals( &mesh, i, binorms );
                }
                
                for (j = 0; j < 3; ++j)
                {
                    unsigned int index = mesh.indices[i*3+j];
                    
                    buffer[c++] = scaleOpt * mesh.positions[index*3+0];
                    buffer[c++] = scaleOpt * mesh.positions[index*3+1];
                    buffer[c++] = scaleOpt * mesh.positions[index*3+2];
                    
                    
                    if (doExportT) {
                        buffer[c++] = mesh.texcoords[index*2+0];
                        buffer[c++] = mesh.texcoords[index*2+1];
                    }
                    
                    if (doExportN) {
                        buffer[c++] = mesh.normals[index*3+0];
                        buffer[c++] = mesh.normals[index*3+1];
                        buffer[c++] = mesh.normals[index*3+2];
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
            
            if( d > beginI && currMatId > -1 ){
                matMap[currMatMap].matId = currMatId;
                matMap[currMatMap].begintri = beginI/3;
                matMap[currMatMap].numtris = (d-beginI)/3;
                
                currMatMap++;
                
            }
        }
    }

    //          collapse
    // ==================================
    
    Collapser *collapser;
    
    // collapse duplicated vertices
    // before generate finals submeshes
    //
    
    collapser = new Collapser( indices, lPolygonVertexCount, lPolygonVertexCount );
    collapser->addStream( (char*)buffer, vsize*sizeof(float) );
    collapser->collapse();
    
    numVertices = collapser->getCollapsedNumVertices();
    buffersize = collapser->getCollapsedNumVertices()*vsize;
    
    
    delete collapser;
    collapser = NULL;


    
    // Write output
    // =====================================


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
    
    

    printf (" nindices : %i \n nvertices : %i\n flags : %i \n iformat %i - %i \n", lPolygonVertexCount, numVertices, flags, iFormat, buffersize );
    // write headers

    fwrite(&lPolygonVertexCount , 1 , sizeof(int), file);
    fwrite(&buffersize  ,         1 , sizeof(int), file);
    fwrite(&flags ,               1 , sizeof(int), file);
    
    
    // write mats
    
    fwrite(&currMatMap ,          1 , sizeof(int), file);
    for (int cm=0; cm< currMatMap; cm++ ) {
        mat_map mmat = matMap[cm];
        if( mmat.matId == -1 ) continue;
        std::string name = materials[mmat.matId].name;
        unsigned int nsize = name.size();
        fwrite(&nsize, sizeof(int), 1, file );
        fwrite(name.c_str(), sizeof(char), name.size(), file );
        //fwrite( 0, 1, 1, file );
        fwrite(&mmat.begintri, 1,sizeof(int), file );
        fwrite(&mmat.numtris,  1,sizeof(int), file );
    }
    
    // write buffers
    if( iFormat == INT )
        fwrite( indicesToU32(indices, lPolygonVertexCount) , 1 , lPolygonVertexCount*sizeof(int), file);
    else if( iFormat == SHORT )
        fwrite( indicesToU16(indices, lPolygonVertexCount) , 1 , lPolygonVertexCount*sizeof(short), file);
    else
        fwrite( indicesToU8(indices, lPolygonVertexCount) , 1 , lPolygonVertexCount*sizeof(char), file);
    

    fwrite((char*)buffer , 1 , buffersize*sizeof(float), file);

    fclose(file);
    return 0;

    if( doCollapse ){
        delete collapser;
        collapser = NULL;
    }



    return 0;
}


