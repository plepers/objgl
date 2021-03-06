//
//  collapser.cpp
//  objgl
//
//  Created by Pierre Lepers on 27/07/2015.
//
//

#include "collapser.h"


// Multi threaded bucket collapsing
void* collapseBucket(  void* in )
{
    
    COLLAPSE_PARAM* params = (COLLAPSE_PARAM*) in;
    
    
    unsigned int *mRemapTable = params->mRemapTable;
    char *interleaved = params->interleaved;
    unsigned int mVertexSize = params->mVertexSize;
    
    int lindexA, lindexB;
    unsigned int lCurrent = 0;
    unsigned int lCompare = 0;
    
    int* bucket;
    
    
    for( hash i = 0; i < params->bcount; i++ )
    {
        bucket = params->buckets[i+params->boffset];
        int numVerts = params->bucketCounts[i+params->boffset];
        
        
        
        if( numVerts < 2 )
        {
            continue;
        }
        
        for ( lindexA = 0; lindexA < numVerts-1; lindexA++ )
        {
            
            lCurrent = bucket[lindexA];
            
            if( mRemapTable[lCurrent] != lCurrent )
            {
                // this vertex is already remapped to a previous one
                // skip
                // ----
                continue;
            }
            
            // compare current with all following vertices
            // -----
            for ( lindexB = lindexA+1; lindexB < numVerts; lindexB++ )
            {
                lCompare = bucket[lindexB];
                
                if(
                   memcmp(
                          &interleaved[lCurrent*mVertexSize],
                          &interleaved[lCompare*mVertexSize],
                          mVertexSize
                          ) == 0
                   )
                {
                    // vertices are equals
                    
                    mRemapTable[lCompare] = lCurrent;
                };
                
                
            }
            
        }
        
    }

    
    return NULL;
}



Collapser::Collapser(unsigned int *pIndices, unsigned int pNumIndices, unsigned int pNumVertices )
{
    mIndices 		= pIndices;
    mNumIndices     = pNumIndices;
    mNumVertices 	= pNumVertices;
    mVertexSize     = 0;
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
    for(int i=0; i < mStreams.size(); i++)
    {
        delete mStreams[i];
    }
    
    mStreams.clear();
    
    delete [] mRemapTable;
}


void Collapser::addStream(char *bytes, unsigned int numbytes)
{
    
    Stream *s = new Stream();
    s->csize = numbytes;
    s->bytes = bytes;
    
    mStreams.push_back(s);
    
    mVertexSize += numbytes;
    
}


void Collapser::collapse()
{
    

    
    int numStreams = mStreams.size();
    
    
    unsigned int lCurrent = 0;
    
    
    // first create an interleaved version of geometry
    // to compare vertices in one memcmp call
    //
    
    char *interleaved = (char*) malloc( mVertexSize * mNumVertices );
    char *ptr = interleaved;
    
    for ( lCurrent = 0; lCurrent < mNumVertices; lCurrent++ )
    {
        for ( int streamIndex = 0; streamIndex < numStreams; streamIndex++ )
        {
            unsigned int csize	= mStreams[streamIndex]->csize;
            memcpy( ptr, &mStreams[streamIndex]->bytes[lCurrent*csize], csize );
            ptr += csize;
        }
        
    }
    
    // hash and split vertices into bucket
    // for faster compare/collapse
    //
    int ** buckets;
    hash *hashList = new hash[ mNumVertices ];
    
    
    unsigned int *bucketCounts = new unsigned int[ NBUCKETS ];
    memset( bucketCounts, 0, NBUCKETS * sizeof(unsigned int) );
    
    
    // hash all vertices, store hashes and count vertices per buckets
    //
    
    for ( lCurrent = 0; lCurrent < mNumVertices; lCurrent++ )
    {
        hash vHash = hashVertex( (char*) &interleaved[lCurrent * mVertexSize], mVertexSize );
        vHash = vHash % NBUCKETS;
        hashList[lCurrent] = vHash;
        bucketCounts[vHash] ++;
    }
    
    
    // allocate buckets
    //
    buckets = (int **) malloc( NBUCKETS * sizeof(void *) );
    
    for( hash i = 0; i < NBUCKETS; i++ )
    {
        buckets[i] = new int[ bucketCounts[i] ];
    }
    
    // for each buckets, create le list of corresponding vertex indices
    //
    memset( bucketCounts, 0, NBUCKETS * sizeof(unsigned int) );
    
    for ( lCurrent = 0; lCurrent < mNumVertices; lCurrent++ )
    {
        hash vHash = hashList[lCurrent];
        buckets[vHash][ bucketCounts[vHash] ] = lCurrent;
        bucketCounts[vHash] ++;
    }
    
    
    
    // collapse each buckets
    //
    
    
    int bPerThreads = NBUCKETS / MAX_THREADS;
    pthread_t threads[MAX_THREADS];
    COLLAPSE_PARAM* cParams = (COLLAPSE_PARAM*) malloc( MAX_THREADS * sizeof(COLLAPSE_PARAM));
    
    for ( int i = 0; i < MAX_THREADS; i++ ) {
        cParams[i].buckets = buckets;
        cParams[i].bucketCounts = bucketCounts;
        cParams[i].boffset = i*bPerThreads;
        cParams[i].bcount = bPerThreads;
        cParams[i].interleaved = interleaved;
        cParams[i].mVertexSize = mVertexSize;
        cParams[i].mRemapTable = mRemapTable;
        int rc = pthread_create(&threads[i], NULL, collapseBucket, (void *) &cParams[i]);
        assert(0 == rc);
        
    }
    
    
    for (int i=0; i<MAX_THREADS; ++i) {
        // block until thread i completes
        int rc = pthread_join(threads[i], NULL);
        assert(0 == rc);
    }

    
    
    // free memory
    
    for( int i = 0; i < NBUCKETS; i++ )
    {
        delete buckets[i];
    }
    
    delete buckets;
    delete bucketCounts;
    delete hashList;
    
    
    // calculate new length
    // =====================
    
    mCollapsedNumVertices = 0;
    for (int i = 0; i < mNumVertices; i++) {
        if( mRemapTable[i] == i ) mCollapsedNumVertices++;
    }
    
    
    // remap indices
	// =============
    remap();
    
    logStats();
    
}

void Collapser::remap()
{
    unsigned int i;
    
    // remap
    for (i = 0; i < mNumIndices; i++) {
        mIndices[i] = mRemapTable[ mIndices[i] ];
    }
    
    // collpase holes
    // --------------
    
    // alloc remap
    for ( int streamIndex = 0; streamIndex < mStreams.size(); streamIndex++ )
    {
        unsigned int csize	= mStreams[streamIndex]->csize;
        mStreams[streamIndex]->remap = (char*)malloc( mCollapsedNumVertices * csize );
    }
    
    // alloc vertex lookup
    int * tIdxMap = new int[mNumIndices];
    for( i = 0; i < mNumIndices; i++)
    {
        tIdxMap[i] = -1;
    }
    
    
    int lRemappedIndex = 0;
    unsigned int lCurrentIndex = 0;
    unsigned int lsgNumVertices = 0;
    
    
    // for every vertex index in indices
    // if vertex not already pushed, push it to collapsed buffer
    // and remap indices
    for( int newIdxPtr =0; newIdxPtr < mNumIndices; newIdxPtr++ )
    {
        
        lCurrentIndex = mIndices[ newIdxPtr ]; // for a vertex
        lRemappedIndex = tIdxMap[ lCurrentIndex ]; // his new collapsed position
        
        // first time we found this vertex
        // in this subGeom. add it to the temp VBO
        // and store the remapped index to IdxMap
        if( lRemappedIndex == -1 )
        {
            
            lRemappedIndex = lsgNumVertices; // increments -> new collapsed position
            tIdxMap[ lCurrentIndex ] = lRemappedIndex; // store it
            
            
            for ( int streamIndex = 0; streamIndex < mStreams.size(); streamIndex++ )
            {
                Stream* stream = mStreams[streamIndex];
                unsigned int csize	= stream->csize;
                memcpy( &stream->remap[lRemappedIndex*csize], &stream->bytes[lCurrentIndex*csize], csize );
            }
            
            lsgNumVertices++;
        }
        
        mIndices[ newIdxPtr ] = lRemappedIndex;
    }
    
    
    // copy back remap to bytes
    // free remap
    for ( int streamIndex = 0; streamIndex < mStreams.size(); streamIndex++ )
    {
        Stream* stream = mStreams[streamIndex];
        unsigned int csize	= stream->csize;
        memcpy( stream->bytes, stream->remap, mCollapsedNumVertices * csize );
        free( stream->remap );
    }
    
    
}

/*
 * FNV hash
 */
hash Collapser::hashVertex( char * vPtr, int len )
{
    unsigned char *p = (unsigned char*) vPtr;
    hash h = 2166136261;
    int i;
    
    for ( i = 0; i < len; i++ )
        h = ( h * 16777619 ) ^ p[i];
    
    return h;
}

void Collapser::logStats()
{
    int newLen = 0;
    for (int i = 0; i < mNumVertices; i++) {
        //FBXSDK_printf("     %i  -  %i \n", i , mRemapTable[i]  );
        if( mRemapTable[i] == i )
            newLen++;
    }
    
    printf("complete collapse, num verts : %i vs %i --- %i\n", newLen, mNumVertices, mCollapsedNumVertices );
}


