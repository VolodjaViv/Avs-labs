#include <iostream>
#include <mmintrin.h>

using namespace std;

void mullBC(int8_t  B[], int8_t C[], __m64 resultBC[], __m64 unpackedB[], __m64 unpackedC[])
{
    __m64 BufferC = _mm_setr_pi8(C[0], C[1], C[2], C[3], C[4], C[5], C[6], C[7]); // с записана в ммх
    __m64 JK_C = _mm_cmpgt_pi16(_m_from_int(0), BufferC); //маска для ммх

    __m64 BufferB = _mm_setr_pi8(B[0], B[1], B[2], B[3], B[4], B[5], B[6], B[7]); // b записана в ммх
    __m64 JK_B = _mm_cmpgt_pi16(_m_from_int(0), BufferB); //маска для ммх

    unpackedB[0] = _mm_unpacklo_pi8(BufferB, JK_B); // распаковка от первых 4
    unpackedB[1] = _mm_unpackhi_pi8(BufferB, JK_B); // вторых 4
    unpackedC[0] = _mm_unpacklo_pi8(BufferC, JK_C); // распаковка от первых 4
    unpackedC[1] = _mm_unpackhi_pi8(BufferC, JK_C); // вторых 4
    //resultBC = _mm_setr_pi8(B[0], B[1], B[2], B[3], B[4], B[5], B[6], B[7]);
    resultBC[0] = _mm_mullo_pi16(unpackedB[0], unpackedC[0]);
    resultBC[1] = _mm_mullo_pi16(unpackedB[1], unpackedC[1]);
}

void subA_BC(int8_t A[8], __m64 unpackedA[], __m64 resultBC[], __m64 resultA_BC[])
{
    __m64 BufferA = _mm_setr_pi8(A[0], A[1], A[2], A[3], A[4], A[5], A[6], A[7]); // с записана в ммх
    __m64 JK_A = _mm_cmpgt_pi16(_m_from_int(0), BufferA); //маска для ммх
    unpackedA[0] = _mm_unpacklo_pi8(BufferA, JK_A); // распаковка от первых 4
    unpackedA[1] = _mm_unpackhi_pi8(BufferA, JK_A); // вторых 4
    resultA_BC[0] = _mm_subs_pi16(unpackedA[0], resultBC[0]);
    resultA_BC[1] = _mm_subs_pi16(unpackedA[1], resultBC[1]);
}

void addABC_D(__m64 resultA_BC[], int16_t D[], __m64 resultABC_D[]) //массив двух ммх
{
    resultABC_D[0] = _mm_add_pi16(resultA_BC[0], _mm_setr_pi16(D[0], D[1], D[2], D[3])); // преобразование
    resultABC_D[1] = _mm_add_pi16(resultA_BC[1], _mm_setr_pi16(D[4], D[5], D[6], D[7])); //
}

int main()
{
    int8_t A[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    int8_t B[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    int8_t C[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
    int16_t D[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };

    __m64 resultBC[2];
    __m64 unpackedB[2];
    __m64 unpackedC[2];
    mullBC(B, C, resultBC, unpackedB, unpackedC);
    __m64 resultA_BC[2];
    __m64 unpackedA[2];
    subA_BC(A, unpackedA, resultBC, resultA_BC);
    __m64 resultABC_D[2];
    addABC_D(resultA_BC, D, resultABC_D);

    int16_t Final_res[8] = { resultABC_D[0].m64_i16[0], resultABC_D[0].m64_i16[1], resultABC_D[0].m64_i16[2], resultABC_D[0].m64_i16[3], resultABC_D[1].m64_i16[0], resultABC_D[1].m64_i16[1], resultABC_D[1].m64_i16[2], resultABC_D[1].m64_i16[3] };

    for (int i = 0; i < 8; i++)
    {
        cout << "F[" << i << "] = " << Final_res[i] << endl;
    }
}