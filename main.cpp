
#include "main.h"
#include "obj.h"
#include "collapser.h"

using namespace TCLAP;
using namespace std;



int main(int argc, char* argv[])
{
    
    CmdLine cmd("objgl - export obj to opengl buffers", ' ', "0.1");
    
    ValueArg<string> a_input("i","input","input obj file",true,"","string", cmd );
    ValueArg<string> a_output("o","output","output binary file",true,"","string", cmd );
    
    SwitchArg o_export("c","collapse", "collapse duplicates", false);
    
    SwitchArg p_export("p","positions", "export Position", true);
    SwitchArg t_export("t","uvs", "export Uvs", false);
    SwitchArg n_export("n","normals", "export Normals", false);
    
    cmd.add( p_export );
    cmd.add( t_export );
    cmd.add( n_export );
    cmd.add( o_export );
    
    cmd.parse( argc, argv );
    
    const char* input =  a_input.getValue().c_str();
    const char* output = a_output.getValue().c_str();
    
    bool doExportP  = p_export.getValue();
    bool doExportT  = t_export.getValue();
    bool doExportN  = n_export.getValue();
    bool doCollapse = o_export.getValue();
    
    //==================================================
    //                       Load obj file
    //==================================================
    
    
    ObjModel objfile = {};
    if (!ReadOBJModel (input, &objfile))
        exit (EXIT_FAILURE);
    
    if( ! objfile.has_normals ) 	doExportN = false;
    if( ! objfile.has_texCoords ) 	doExportT = false;
    
    
    
    // inline buffers
    
    int lPolygonVertexCount = objfile.num_faces * 3;
    
    
    int vsize = 3;
    
    if (doExportT)
        vsize += 2;
    
    if (doExportN)
        vsize += 3;
    
    int buffersize = vsize * lPolygonVertexCount;
    float *buffer = new float[ buffersize ];
    
    unsigned int *indices = new unsigned int[ lPolygonVertexCount ];
    
    int i, j;
    int c = 0;
    
    for (i = 0; i < objfile.num_faces; ++i)
    {
        indices[i*3+0] = i*3+0;
        indices[i*3+1] = i*3+1;
        indices[i*3+2] = i*3+2;
        
        for (j = 0; j < 3; ++j)
        {
            
            buffer[c++] = objfile.vertices[objfile.faces[i].vert_indices[j]].xyzw[0];
            buffer[c++] = objfile.vertices[objfile.faces[i].vert_indices[j]].xyzw[1];
            buffer[c++] = objfile.vertices[objfile.faces[i].vert_indices[j]].xyzw[2];
            
            
            if (doExportT) {
                buffer[c++] = objfile.texCoords[objfile.faces[i].uvw_indices[j]].uvw[0];
                buffer[c++] = objfile.texCoords[objfile.faces[i].uvw_indices[j]].uvw[1];
            }
            
            if (doExportN) {
                buffer[c++] = objfile.normals[objfile.faces[i].norm_indices[j]].ijk[0];
                buffer[c++] = objfile.normals[objfile.faces[i].norm_indices[j]].ijk[1];
                buffer[c++] = objfile.normals[objfile.faces[i].norm_indices[j]].ijk[2];
            }
            
        }
    }
    
    Collapser *collapser;
    unsigned short* sIndices = new unsigned short[ lPolygonVertexCount ];
    
    if( doCollapse ){
        // collapse duplicated vertices
        // before generate finals submeshes
        //
        
        collapser = new Collapser( indices, lPolygonVertexCount, lPolygonVertexCount );
        Stream* stream = collapser->addStream( buffer, vsize );
        collapser->collapse();
        
        buffer = stream->remap;
        buffersize = collapser->getCollapsedNumVertices()*vsize;
        
        for (int i = 0; i< lPolygonVertexCount; i++) {
            sIndices[i] = (unsigned int)( indices[i] );
        }
        
    }
    
    
	// save
    FILE* file;
    file = fopen(output, "wb" );
    if(!file) return 79;
    
    if( doCollapse ) {
        fwrite(&lPolygonVertexCount , 1 , sizeof(int), file);
        fwrite(&buffersize , 1 , sizeof(int), file);
        fwrite((char*)sIndices , 1 , lPolygonVertexCount*sizeof(short), file);
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

