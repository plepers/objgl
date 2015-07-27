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



class Collapser
{
public:
    Collapser( unsigned int *pIndices, unsigned int pNumIndices, unsigned int pNumVertices );
    ~Collapser();
    
    void addStream( double *data, unsigned int csize );
    void collapse();
    
private:
    
    struct Stream{
        Stream() :
        data(NULL),
        csize(0)
        {}
        
        double		 	*data;
        unsigned int 	csize;
    };
    
    struct StreamArray{
        StreamArray() :
        size(0)
        {}
        
        Stream*			streams[16];
        unsigned int 	size;
    };
    
    unsigned int 		mNumVertices;
    unsigned int 		mNumIndices;
    
    
    StreamArray			mStreams;
    unsigned int 		*mIndices;
    unsigned int 		*mRemapTable;
    
    
    
    void scanStream( unsigned int index, unsigned int comp );
};


#endif /* defined(__objgl__collapser__) */
