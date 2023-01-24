#include <iostream>
using namespace std;
typedef unsigned char byte;

byte GF_add(byte gf1, byte gf2) {
	return gf1 ^ gf2; //XOR
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

//(a)번 문제 - 생성자 x의 개수 구하기
void Find_x()
{
	byte x1;
	byte x[256] = { 0x00, };
	int sum = 0;	//생성자의 개수
	int j = 0;

	for (int i = 0; i < 256; i++) {
		x1 = i;
		for (j = 0; j < 254; j++) {
			x1 = GF_mul(i, x1);
			if (x1 == 1)  
				//만약 x^n=1(mod 255)인 n이 255 이전에 존재한다면, x는 생성자가 아님
				break;
		}
		if (x1 == 1 && j==253) x[i] = i;
		//x^255=1(mod255)이고, 255전에 1이 나오지 않는 x를 x자리에 추가
	}

	printf("x is\n");
	for (int i = 0; i < 256; i++) {
		if (x[i] != 0) { 
			//x[i]에서, 0이 아닌 값으로 저장된 원소는 생성자임
			printf("x=%02x\n", x[i]);
			sum += 1;
		}
	}
	printf("\nThe number of x : %d\n", sum);
}

//(b)번 문제 -생성자 표 완성하기
void Make_Chart()
{
	byte a1, y1; //a1: 생성자, y1=0x03
	byte a2[256] = { 0x00, }; //a2[i]: a^i 
	byte y2[256] = { 0x00, }; //y2[i]: 0x03^i

	a1 = 0x05;
	printf("Let a=%02x\n", a1);
	a2[1] = a1; //a2[1]=a^1=a
	for (int i = 0; i < 254; i++) {
		a1 = GF_mul(0x05, a1); 
		a2[i + 2] = a1; //a2[i+2] = a^(i+2) = a^(2+i)
	}
	a2[0] = a2[255]; //a2[0]=a^0=a^255=0x01
	for (int i = 0; i < 255; i++) {
		printf("a^(%d)=%02x\n", i, a2[i]);
	}

	printf("=====================================\n");

	y1 = 0x03;
	y2[1] = y1; //y2[1]=y^1=y=0x03
	for (int i = 0; i < 254; i++) {
		y1 = GF_mul(0x03, y1);
		y2[i + 2] = y1; //y2[i+2] = y^(i+2) = y^(2+i)
	}
	y2[0] = y2[255]; //y2[0]=y^0=y^255=0x01
	for (int i = 0; i < 255; i++) {
		printf("0x03^(%d)=%02x\n", i, y2[i]);
	}
}

//(d)ExpTable 구현
void Make_ExpTable(byte a, byte etable[256])
{
	byte temp;
	temp = a;
	etable[1] = a; //etable[1]=a^1=a
	for (int i = 0; i < 254; i++) {
		a=GF_mul(temp, a);
		etable[i + 2] = a; //etable [i+2]=a^i+2=a^(2+i)
	}
	etable[0] = etable[255]; //etable[0]=etable[255]=0x01
}

//(d)LogTable 구현
void Make_LogTable(byte a, byte ltable[256])
{
	byte etable[256];
	Make_ExpTable(a, etable); //ExpTable 생성
	for (int i = 0; i < 256; i++) {
		ltable[etable[i]] = i; //ltable=(etable)^(-1)
	}
}

//(e)번 문제-테이블을 이용한 곱계산
byte GF_Table_mul(byte f, byte g) //f, g: 곱해야 하는 수
{
	byte a = 0x05; //여기서 생성자 a는 0x05로 가정
	byte etable[256] = { 0x00, };
	byte ltable[256] = { 0x00, };
	byte x, y;
	byte h;

	Make_ExpTable(a, etable);

	Make_LogTable(a, ltable);

	x = ltable[f]; //LogTable에 있는 f에 대응하는 수
	y = ltable[g]; //LogTable에 있는 g에 대응하는 수
	h = x + y; //f*g=a^(x+y)
	h = etable[h]; //h를 ExpTable에서 찾아 곱한 값을 구함

	return h;
}

int main()
{
	byte a = 0x05; //생성자 a를 0x05라 가정
	byte etable[256] = { 0x00, }; //ExpTable
	byte ltable[256] = { 0x00, }; //LogTable
	byte f, g, h;
	f = 0xa1; //곱해야할 두 수를 0xa1, 0x63으로 가정
	g = 0x63;

	printf("Find_x\n");
	Find_x();

	printf("Make_chart\n");
	Make_Chart();

	printf("Make_ExpTable\n");
	Make_ExpTable(a, etable);

	printf("Make_LogTable\n");
	Make_LogTable(a, ltable);

	h = GF_Table_mul(f, g);
	printf("%02x", h);

}
