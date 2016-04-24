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
#include <vector>




class Collapser
{
public:
    Collapser( unsigned int *pIndices, unsigned int pNumIndices, unsigned int pNumVertices );
    ~Collapser();
    
    void addStream( char *data, unsigned int csize );
    void collapse();
    
    unsigned int getCollapsedNumVertices(){
        return mCollapsedNumVertices;
    }
    
private:
    
    typedef unsigned hash;
    
    struct Stream{
        Stream() :
        bytes(NULL),
        remap(NULL),
        csize(0)
        {}
        
        char 	*bytes;
        char 	*remap;
        unsigned int 	csize;
    };
    
    hash                hashVertex( char *vPtr, int len );
    void                remap();
    void                logStats();
    
    unsigned int 		mNumVertices;
    unsigned int 		mCollapsedNumVertices;
    unsigned int 		mVertexSize;
    unsigned int 		mNumIndices;
    
    
    std::vector<Stream*>	mStreams;
    unsigned int 		*mIndices;
    unsigned int 		*mRemapTable;
    

    
    
    void scanStream( unsigned int index, unsigned int comp );
};


#endif /* defined(__objgl__collapser__) */
