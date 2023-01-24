#include <iostream>
using namespace std;
typedef unsigned char byte;
#define MATRIX_MAX_SIZE 16

// GF_Matrix 구조체 사용
struct GF_Matrix {
	byte M[MATRIX_MAX_SIZE][MATRIX_MAX_SIZE];
	int row;
	int col;
};

byte GF_add(byte gf1, byte gf2) {
	return gf1 ^ gf2; //XOR
}

// GF_Matrix 출력
void GF_Mat_print(GF_Matrix Mat) {
	for (int i = 0; i < Mat.row; i++) {
		printf("[");
		for (int j = 0; j < Mat.col; j++) {
			printf(" %02x", Mat.M[i][j]);
		}
		printf(" ]\n");
	}
	printf("\n");
}


//GF(2)에서의 곱셈->AND 연산
byte GF2_mul(byte f, byte g)
{ //대응되는 수가 모두 1이면 1을 반환
	byte h = 0;
	if ((f*g) == 1) 
		h = 1;  
	else
		h == 0;
	return h;
}

byte GF_xtime(byte f) {
	return (((f >> 7) & 0x01) == 1 ? (f << 1) ^ 0x1b : f << 1);
}

byte GF_mul(byte f, byte g) {
	byte h; // 곱셈 결과 h(x) = f(x)*g(x)
	int coef; // 계수 (0 또는 1)
	h = 0x00;
	for (int i = 7; i >= 0; i--) {
		coef = (f >> i) & 0x01; // a7, a6, a5, ... ,a0
		h = GF_xtime(h);
		if (coef == 1) {
			h = GF_add(h, g);
		}
	}
	return h;
}

byte GF_inv(byte f) {
	byte f_inv; // 역원(결과값)
	byte temp; // 중간에 곱할 값(a^n): a^2, a^4, a^8, a^16, ... ,a^128
	f_inv = 1;
	temp = f;
	for (int i = 0; i < 7; i++) {
		temp = GF_mul(temp, temp);
		f_inv = GF_mul(f_inv, temp);
	}
	return f_inv;
}

void GF2_Mat_Exchange_Row(GF_Matrix& A, int row1, int row2) {
	byte tmp;
	for (int i = 0; i < A.col; i++) {
		tmp = A.M[row1][i];
		A.M[row1][i] = A.M[row2][i];
		A.M[row2][i] = tmp;
	}
}

void GF2_Mat_Scalar_Mul(GF_Matrix& A, byte scalar, int row) {
	byte temp;
	for (int i = 0; i < A.col; i++) {
		temp = GF2_mul(scalar, A.M[row][i]);
		A.M[row][i] = temp;
	}
}

void GF2_Mat_Row_Add(GF_Matrix& A, byte scalar, int row_src, int row_target) {
	byte temp;
	for (int i = 0; i < A.col; i++) {
		temp = GF2_mul(scalar, A.M[row_src][i]);
		A.M[row_target][i] = GF_add(A.M[row_target][i], temp);
	}
}

GF_Matrix GF2_Mat_inverse(GF_Matrix A)
{
	GF_Matrix AA;
	int pivot_row;

	// 형렬 A와 단위행렬 I를 옆으로 붙여 행렬 AA를 만든다. AA = [ A | I ]
	AA.row = A.row;
	AA.col = A.col * 2;
	for (int i = 0; i < A.row; i++) {
		for (int j = 0; j < A.col; j++) {
			AA.M[i][j] = A.M[i][j];
			AA.M[i][A.col + j] = (i == j) ? 1.0 : 0.0;
			AA.M[i][j + A.col] = (i == j) ? 01 : 00;	//GF(2) 원소의 1과 0으로
		}
	}

	// Reduced REF(Row Echelon Form) 사다리꼴을 만들어 역행렬을 구한다.
	for (int j = 0; j < A.col; j++) {
		pivot_row = -1;  // 핵심 행(피봇 행)을 찾는다. (초기값 -1로 설정)
		for (int i = j; i < A.row; i++) {
			if (AA.M[i][j] != 0x00) { // [j][j], [j+1][j], [j+2][j] 중 0이 아닌 것
				pivot_row = i;
				break;
			}
		}
		if (pivot_row == -1) {  // 피봇 행이 없으면 (모두 0인 경우)
			cout << "Not invertible !" << endl;  // 역행렬 없음
			return A;  // 의미없는 리턴값
		}
		if (pivot_row != j) { // pivot : [0][0]		[1][1]		[2][2]
			//pivot행 <=> j행
			GF2_Mat_Exchange_Row(AA, j, pivot_row);
		}
		byte temp;
		if (AA.M[j][j] == 1) temp = 1;
		else temp = 0;	//곱셈의 역원
		GF2_Mat_Scalar_Mul(AA, temp, j); // 피봇값이 1이 되도록 조정한다
		for (int i = 0; i < A.row; i++) {
			if ((i != j) && (AA.M[i][j] != 0x00)) {
				GF2_Mat_Row_Add(AA, AA.M[i][j], j, i);
			}
		}
		GF_Mat_print(AA);
	}
	GF_Matrix Inv;
	Inv.row = A.row;
	Inv.col = A.col;
	for (int i = 0; i < A.row; i++) {
		for (int j = 0; j < A.col; j++) {
			Inv.M[i][j] = AA.M[i][A.col + j];
		}
	}
	return Inv;
}

void Get_Inv_A()
{
	byte A[8][8] = {
	{1, 0, 0, 0, 1, 1, 1, 1},
	{1, 1, 0, 0, 0, 1, 1, 1},
	{1, 1, 1, 0, 0, 0, 1, 1},
	{1, 1, 1, 1, 0, 0, 0, 1},
	{1, 1, 1, 1, 1, 0, 0, 0},
	{0, 1, 1, 1, 1, 1, 0, 0},
	{0, 0, 1, 1, 1, 1, 1, 0},
	{0, 0, 0, 1, 1, 1, 1, 1}
	};
	GF_Matrix AA, InvA;

	AA.row = 8;
	AA.col = 8;

	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			AA.M[i][j] = A[i][j];	//구조체에 행렬 A 대입
		}
	}
	cout << "A Matrix AA" << endl;
	GF_Mat_print(AA);

	cout << "Inverse A Matrix AA = " << endl;
	InvA = GF2_Mat_inverse(AA);
	GF_Mat_print(InvA);
}


//x={A^(-1)y+A^(-1)b}^(-1)
byte Inv_Affine(byte w)
{
	byte InvA[8][8] = {
	{0, 0, 1, 0, 0, 1, 0, 1},
	{1, 0, 0, 1, 0, 0, 1, 0},
	{0, 1, 0, 0, 1, 0, 0, 1},
	{1, 0, 1, 0, 0, 1, 0, 0},
	{0, 1, 0, 1, 0, 0, 1, 0},
	{0, 0, 1, 0, 1, 0, 0, 1},
	{1, 0, 0, 1, 0, 1, 0, 0},
	{0, 1, 0, 0, 1, 0, 1, 0}
	};	//Get_Inv_A를 통해 나온 역행렬 A^(-1)
	byte b_vec[8] = { 1, 1, 0, 0, 0, 1, 1, 0 };
	byte w_vec[8], ab_vec[8], x_vec[8], x;	
	//w_vec: y, ab_vec: A^(-1)*b, x_vec=A^(-1)*y+A^(-1)*b, x: x in GF(2^8)

	for (int i = 0; i < 8; i++) {
		w_vec[i] = (w >> i) & 0x01;
		ab_vec[i] = 0; //계산 값을 넣기 위해 초기화
	}

	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			ab_vec[i] ^= InvA[i][j] * b_vec[j]; //ab_vec=A^(-1)*b
		}
	}

	for (int i = 0; i < 8; i++) {
		x_vec[i] = ab_vec[i];
		for (int j = 0; j < 8; j++) {
			x_vec[i] ^= InvA[i][j] * w_vec[j]; //x_vec=A^(-1)*y+A^(-1)*b
		}
	}

	x = 0;
	byte temp_bit;
	for (int i = 0; i < 8; i++) {
		temp_bit = x_vec[i] << i;
		x ^= temp_bit;
	}
	x = GF_inv(x); //구한 x의 곱셈의 역원이 y에 대응하는 Inv Sbox의 값
	return x;
}

void GET_ISbox(byte isbox[256])
{
	byte temp;
	isbox[0] = Inv_Affine(0); //0^(-1) = 0 으로 간주
	for (int i = 1; i < 256; i++) {
		temp = i;
		isbox[i] = Inv_Affine(i);
	}
}

void Inv_Sbox()
{
	byte isbox[256];
	GET_ISbox(isbox);
	printf("ISbox=[\n");
	for (int i = 0; i < 256; i++) {
		printf("%02x, ", isbox[i]);
		if ((i % 16) == 15)
			printf("\n");
	}
	printf("]");
}

int main()
{
	Get_Inv_A();
	Inv_Sbox();
}