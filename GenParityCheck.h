#ifndef __CS_GEN_PARITY_CHECK__
#define __CS_GEN_PARITY_CHECK__

#include "Matrix.h"
// This file contains definitions of methods to generate parity check
// matrices and corresponding generator matrices

void genRegLDPCEasy(unsigned int wc, unsigned int wr,
                    unsigned int width, Matrix& mat);

#endif