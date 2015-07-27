//
//  collapser.cpp
//  objgl
//
//  Created by Pierre Lepers on 27/07/2015.
//
//

#include "collapser.h"



Collapser::Collapser(unsigned int *pIndices, unsigned int pNumIndices, unsigned int pNumVertices )
{
    mIndices 		= pIndices;
    mNumIndices 	= pNumIndices;
    mNumVertices 	= pNumVertices;
    mStreams		= StreamArray();
    
    mRemapTable = new unsigned int[mNumVertices];
    
    
    // identity map
    //
    for (unsigned int i = 0; i < pNumVertices; i++) {
        mRemapTable[i] = i;
    }
    
}

Collapser::~Collapser()
{
    for(int i=0; i < mStreams.size; i++)
    {
        delete mStreams.streams[i];
    }
    
    delete [] mRemapTable;
}


void Collapser::addStream(double *data, unsigned int csize)
{
    int numStreams = mStreams.size;
    
    mStreams.size++;
    
    Stream *newStream = mStreams.streams[numStreams] = new Stream();
    
    newStream->csize = csize;
    newStream->data = data;
    
}

//
// brute force but simple collapsing
// should probably be optimized...
//
void Collapser::collapse()
{
    
    int numStreams = mStreams.size;
    
    
    printf("start collapse, num verts : %i \n", mNumVertices );
    
    unsigned int lCurrent = 0;
    unsigned int lCompare = 0;
    
    
    for ( lCurrent = 0; lCurrent < mNumVertices-1; lCurrent++ )
    {
        
        if( mRemapTable[lCurrent] != lCurrent )
        {
            // this vertex is already remapped to a previous one
            // skip
            // ----
            continue;
        }
        
        // compare current with all following vertices
        // -----
        for ( lCompare = lCurrent+1; lCompare < mNumVertices; )
        {
            
            
            // for current/compare we check equality of all streams / all components
            // loop in streams then components
            // -----
            for ( int streamIndex = 0; streamIndex < numStreams; streamIndex++ ) {
                
                
                
                unsigned int csize	= mStreams.streams[streamIndex]->csize;
                double *data 	= mStreams.streams[streamIndex]->data;
                
                for (unsigned int comp = 0; comp < csize; comp++) {
                    
                    // Todo : compare with an epsilon ?
                    //
                    if( data[ lCurrent*csize + comp ] != data[ lCompare*csize + comp ] ){
                        // components are not equals
                        // skip this vertex, go to next one
                        // ------
                        goto nextVert;
                    }
                    
                    
                }
                
                
            }
            
            // lCompare is the same vertex than lCurrent
            //
            mRemapTable[lCompare] = lCurrent;
            
            nextVert :
            lCompare++;
            
        }
        
    }
    
    
    unsigned int i;
    
    for (i = 0; i < mNumIndices; i++) {
        mIndices[i] = mRemapTable[ mIndices[i] ];
    }
    
    
    //    int newLen = 0;
    //    for (i = 0; i < mNumVertices; i++) {
    //        if( mRemapTable[i] == i )
    //            newLen++;
    //    }
    //    FBXSDK_printf("complete collapse, num verts : %i \n", newLen );
    
    
}



