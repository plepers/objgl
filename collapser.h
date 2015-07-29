//
//  collapser.h
//  objgl
//
//  Created by Pierre Lepers on 27/07/2015.
//
//

#ifndef __objgl__collapser__
#define __objgl__collapser__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Stream{
    Stream() :
    data(NULL),
    remap(NULL),
    csize(0)
    {}
    
    float 		 	*data;
    float 		 	*remap;
    unsigned int 	csize;
} Stream;


class Collapser
{
public:
    Collapser( unsigned int *pIndices, unsigned int pNumIndices, unsigned int pNumVertices );
    ~Collapser();
    
    Stream* addStream( float *data, unsigned int csize );
    void collapse();
    unsigned int getCollapsedNumVertices(){
        return mCollapsedNumVertices;
    }
    
    
    
private:
    
    
    struct StreamArray{
        StreamArray() :
        size(0)
        {}
        
        Stream*			streams[16];
        unsigned int 	size;
    };
    
    unsigned int 		mNumVertices;
    unsigned int 		mCollapsedNumVertices;
    unsigned int 		mNumIndices;
    
    
    StreamArray			mStreams;
    unsigned int 		*mIndices;
    unsigned int 		*mRemapTable;
    
    
    
    void scanStream( unsigned int index, unsigned int comp );
};


#endif /* defined(__objgl__collapser__) */
