#include<iostream>
#include <ctime>

using namespace std;

void fill_array_rand_val(_int8* a, _int8* b, _int8* c, _int16* d) {
    int min_8 = -128, max_8 = 127, min_16 = -32513, max_16 = 32512;
    for (int i = 0; i < 8; i++) { a[i] = min_8 + rand() % (2 * max_8 + 1); b[i] = min_8 + rand() % (2 * max_8 + 1); c[i] = min_8 + rand() % (2 * max_8 + 1); d[i] = min_16 + rand() % (2 * max_16 + 1); }
}

void print_arrays(_int8* a, _int8* b, _int8* c, _int16* d) {
    for (int i = 0; i < 8; i++) cout << "A[" << i << "]=" << int(a[i]) << "  "; cout << endl;
    for (int i = 0; i < 8; i++) cout << "B[" << i << "]=" << int(b[i]) << "  "; cout << endl;
    for (int i = 0; i < 8; i++) cout << "C[" << i << "]=" << int(c[i]) << "  "; cout << endl;
    for (int i = 0; i < 8; i++) cout << "D[" << i << "]=" << int(d[i]) << "  "; cout << endl << endl;
}

void show_streight_result(_int8* a, _int8* b, _int8* c, _int16* d) {
    cout << endl;
    int streihgt_result[8];
    for (int i = 0; i < 8; i++) {
        streihgt_result[i] = a[i] - (b[i] * c[i]) + d[i];
        cout << "Res[" << i << "]=" << streihgt_result[i] << "  ";
    }
}

void show_asm_result(_int16* f) {
    for (int i = 0; i < 8; i++) cout << "Asm[" << i << "]=" << int(f[i]) << "  ";
}

int main() {
    _int8  a[8];
    _int8  b[8];
    _int8  c[8];
    _int16 d[8];
    _int16 f[8];

    for (int i = 0; i < 10; i++) {
        fill_array_rand_val(a, b, c, d);
        print_arrays(a, b, c, d);
        _asm {
            movq mm0, qword ptr[a]
            movq mm1, qword ptr[b]
            movq mm2, qword ptr[c]
            movq mm3, qword ptr[d]

            punpcklbw mm0, mm0
            punpcklbw mm1, mm1
            punpcklbw mm2, mm2
            psraw     mm0, 8
            psraw     mm1, 8
            psraw     mm2, 8

            pmullw mm1, mm2
            psubsw mm0, mm1
            paddw mm0, mm3

            movq qword ptr[f], mm0

            movq mm4, qword ptr[a + 4]
            movq mm5, qword ptr[b + 4]
            movq mm6, qword ptr[c + 4]
            movq mm3, qword ptr[d + 8]

            punpcklbw mm4, mm4
            punpcklbw mm5, mm5
            punpcklbw mm6, mm6
            psraw     mm4, 8
            psraw     mm5, 8
            psraw     mm6, 8

            pmullw mm5, mm6
            psubw mm4, mm5
            paddw mm4, mm3

            movq qword ptr[f + 8], mm4
        }
        show_asm_result(f);
        show_streight_result(a, b, c, d);
        cout << endl << "-------------------------------------------------------------------------------------------------------------" << endl;
    }
    return 0;
}
