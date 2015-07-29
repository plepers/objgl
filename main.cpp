
#include "main.h"
#include "obj.h"

using namespace TCLAP;
using namespace std;



int main(int argc, char* argv[])
{
    
    CmdLine cmd("objgl - export obj to opengl buffers", ' ', "0.1");
    
    ValueArg<string> a_input("i","input","input obj file",true,"","string", cmd );
    ValueArg<string> a_output("o","output","output binary file",true,"","string", cmd );
    
    SwitchArg p_export("p","positions", "export Position", true);
    SwitchArg t_export("t","uvs", "export Uvs", false);
    SwitchArg n_export("n","normals", "export Normals", false);
    cmd.add( p_export );
    cmd.add( t_export );
    cmd.add( n_export );
    
    cmd.parse( argc, argv );
    
    const char* input =  a_input.getValue().c_str();
    const char* output = a_output.getValue().c_str();
    
    bool doExportP = p_export.getValue();
    bool doExportT = t_export.getValue();
    bool doExportN = n_export.getValue();
    
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
    
    unsigned short *indices = new unsigned short[ lPolygonVertexCount ];
    
    int i, j;
    int c = 0;
    
    for (i = 0; i < objfile.num_faces; ++i)
    {
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
    
    
    
    
	// save
    FILE* file;
    file = fopen(output, "wb" );
    if(!file) return 79;
    fwrite((char*)buffer , 1 , buffersize*4, file);
    fclose(file);
    return 0;

    
    FreeModel(&objfile);
    
    
    return 0;
}

