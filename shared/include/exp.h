#ifndef CUSTOM_EXP
#define CUSTOM_EXP
#include "expl.h"

static const long double Cc[] = {
/* Smallest integer x for which e^x overflows.  */
#define c_himark Cc[0]
 11356.523406294143949491931077970765L,
/* Largest integer x for which e^x underflows.  */
#define c_lomark Cc[1]
 -11433.4627433362978788372438434526231L,
/* 3x2^96 */
#define c_THREEp96 Cc[2]
 59421121885698253195157962752.0L,
/* 3x2^103 */
#define c_THREEp103 Cc[3]
 30423614405477505635920876929024.0L,
/* 3x2^111 */
#define c_THREEp111 Cc[4]
 7788445287802241442795744493830144.0L,
/* 1/ln(2) */
#define c_M_1_LN2 Cc[5]
 1.44269504088896340735992468100189204L,
/* first 93 bits of ln(2) */
#define c_M_LN2_0 Cc[6]
 0.693147180559945309417232121457981864L,
/* ln2_0 - ln(2) */
#define c_M_LN2_1 Cc[7]
 -1.94704509238074995158795957333327386E-31L,
/* very small number */
#define c_TINY Cc[8]
 1.0e-4900L,
/* 2^16383 */
#define c_TWO16383 Cc[9]
 5.94865747678615882542879663314003565E+4931L,
/* 256 */
#define c_TWO8 Cc[10]
 256,
/* 32768 */
#define c_TWO15 Cc[11]
 32768,
/* Chebyshev polynom coefficients for (exp(x)-1)/x */
#define c_P1 Cc[12]
#define c_P2 Cc[13]
#define c_P3 Cc[14]
#define c_P4 Cc[15]
#define c_P5 Cc[16]
#define c_P6 Cc[17]
 0.5L,
 1.66666666666666666666666666666666683E-01L,
 4.16666666666666666666654902320001674E-02L,
 8.33333333333333333333314659767198461E-03L,
 1.38888888889899438565058018857254025E-03L,
 1.98412698413981650382436541785404286E-04L,
};


inline double custom_exp (double x) {
    int tval1, tval2;
    long double x22, n, t, xl;
    long double ex2_u;
    /* Calculate n.  */
    n = x * c_M_1_LN2 + c_THREEp111;
    n -= c_THREEp111;
    x = x - n * c_M_LN2_0;
    xl = n * c_M_LN2_1;
    /* Calculate t/256.  */
    t = x + c_THREEp103;
    t -= c_THREEp103;
    /* Compute tval1 = t.  */
    tval1 = (int) (t * c_TWO8);
    x -= expl_table[c_T_EXPL_ARG1+2*tval1];
    xl -= expl_table[c_T_EXPL_ARG1+2*tval1+1];
    /* Calculate t/32768.  */
    t = x + c_THREEp96;
    t -= c_THREEp96;
    /* Compute tval2 = t.  */
    tval2 = (int) (t * c_TWO15);
    x -= expl_table[c_T_EXPL_ARG2+2*tval2];
    xl -= expl_table[c_T_EXPL_ARG2+2*tval2+1];
    x = x + xl;
    /* Compute ex2 = 2^n_0 e^(argtable[tval1]) e^(argtable[tval2]).  */
    ex2_u = expl_table[c_T_EXPL_RES1 + tval1] * expl_table[c_T_EXPL_RES2 + tval2];
    x22 = x + x*x*(c_P1+x*(c_P2+x*(c_P3+x*(c_P4+x*(c_P5+x*c_P6)))));
    return x22 * ex2_u + ex2_u;
}

#endif