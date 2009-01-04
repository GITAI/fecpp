/*
 * fec.c -- forward error correction based on Vandermonde matrices
 * (C) 1997-98 Luigi Rizzo (luigi@iet.unipi.it)
 * (C) 2009 Jack Lloyd (lloyd@randombit.net)
 * Distributed under the terms given in license.txt
 */

#ifndef FECPP_H__
#define FECPP_H__

/*
 * The following parameter defines how many bits are used for
 * field elements. The code supports any value from 2 to 16
 * but fastest operation is achieved with 8 bit elements
 * This is the only parameter you may want to change.
 */
#ifndef GF_BITS
#define GF_BITS  8	/* code over GF(2**GF_BITS) - change to suit */
#endif

#if (GF_BITS <= 8)
typedef unsigned char gf;
#else
typedef unsigned short gf;
#endif

struct fec_parms {
    unsigned int magic ;
    int k, n ;		/* parameters of the code */
    gf *enc_matrix ;
} ;

#define	GF_SIZE ((1 << GF_BITS) - 1)	/* powers of \alpha */
void fec_free(struct fec_parms *p);
struct fec_parms* fec_new(int k, int n);

void init_fec() ;
void fec_encode(struct fec_parms *code, gf *src[], gf *fec, int index, int sz);
int fec_decode(struct fec_parms *code, gf *pkt[], int index[], int sz);

#endif
