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
    mNumIndices     = pNumIndices;
    mNumVertices 	= pNumVertices;
    mStreams		= StreamArray();
    
    mCollapsedNumVertices = 0;
    
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


Stream* Collapser::addStream(float *data, unsigned int csize)
{
    int numStreams = mStreams.size;
    
    mStreams.size++;
    
    Stream *newStream = mStreams.streams[numStreams] = new Stream();
    
    newStream->csize = csize;
    newStream->data = data;
    
    return newStream;
    
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
                float *data 	= mStreams.streams[streamIndex]->data;
                
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
    
    unsigned int newSize = 0;
    for ( int i = 0; i < mNumVertices; i++) {
        if( mRemapTable[i] == i ) {
            newSize++;
        }
    }
    
    mCollapsedNumVertices = newSize;
    
    
    for ( int i = 0; i < mNumIndices; i++) {
        mIndices[i] = mRemapTable[ mIndices[i] ];
        
    }
    
    
    
    int lRemappedIndex = 0;
    unsigned int lCurrentIndex = 0;
    unsigned int lsgNumVertices = 0;
    
    int * tIdxMap = new int[mNumIndices];
    for( int i = 0; i < mNumIndices; i++) {
        tIdxMap[i] = -1;
    }
    
    for ( int streamIndex = 0; streamIndex < numStreams; streamIndex++ ) {
        
        unsigned int csize	= mStreams.streams[streamIndex]->csize;
        mStreams.streams[streamIndex]->remap = new float[newSize*csize];
        
    }
    
    for ( int  newIdxPtr = 0; newIdxPtr < mNumIndices; newIdxPtr++) {
        
        lCurrentIndex = mIndices[ newIdxPtr ];
        lRemappedIndex = tIdxMap[ lCurrentIndex ];
        
        if( lRemappedIndex == -1 )
        {
            
            lRemappedIndex = lsgNumVertices;
            tIdxMap[ lCurrentIndex ] = lRemappedIndex;
            
            for ( int streamIndex = 0; streamIndex < numStreams; streamIndex++ ) {
                
                
                unsigned int csize	= mStreams.streams[streamIndex]->csize;
                size_t cbytes = csize*sizeof(float);
                
                float* newdata = mStreams.streams[streamIndex]->remap;
                float* data 	= mStreams.streams[streamIndex]->data;
                
                memcpy( &newdata[lsgNumVertices*csize], &data[lCurrentIndex*csize], cbytes );
                
            }
            
            lsgNumVertices++;

        }
        
        mIndices[ newIdxPtr ] = lRemappedIndex;

        
    }
    
    printf("nv match %i %i\n", lsgNumVertices, newSize );
   
//    
//    unsigned int i;
//    for (i = 0; i < mNumIndices; i++) {
//        mIndices[i] = mRemapTable[ mIndices[i] ];
//        
//    }
//
//    int newLen = 0;
//    for (i = 0; i < mNumVertices; i++) {
//        if( mRemapTable[i] == i ) {
//            newLen++;
//        }
//    }
    //    printf("complete collapse, num verts : %i \n", newLen );
    
    
}



