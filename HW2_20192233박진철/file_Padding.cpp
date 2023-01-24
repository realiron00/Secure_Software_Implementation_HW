
#include <iostream>
#include <fstream>

using namespace std;

typedef unsigned char byte;

int FileSize(const char *file_name) {
	ifstream fin;
	//fin.open("file-1.bin", ios::binary);
	fin.open(file_name, ios::binary);
	if (fin.fail()) {
		cout << "Input File Open Error" << endl;
		return -1; //오류가 나면, -1을 반환
	}
	int file_length;

	fin.seekg(0, fin.end); // get-pointer 위치시키기
	file_length = fin.tellg(); // tell me "get-pointer"
	//cout << " File size (bytes) = " << file_length << endl;
	fin.close();

	return file_length;
}

//패딩함수: 입력 (0-15바이트 데이터), 데이터 길이, 출력: 16바이트(한블록)
// 예: 01020304, 4 ---> 01020304 80000000 00000000 00000000 
// 주의: 입력 0바이트도 가능하도록 (패딩만 16바이트 출력)

//void Padding(byte in[], int in_length, byte out[]) {
void Padding(byte in[], int in_length, byte out[16]) {
	byte padding_start = 0x80;
	for (int i = 0; i < in_length; i++) {
		out[i] = in[i];
	}
	out[in_length] = padding_start;
	for (int i = in_length + 1; i < 16; i++) {
		out[i] = 0x00;
	}
}

//입력파일(source) --> [Padding] --> 출력파일(dest)
//입력파일크기 (N 바이트): 
//   16의 배수 --> 출력크기 N+16 (패딩만 한블록)
//   16의 배수가 아니면 --> N보다 큰 최초 16의 배수
void FilePadding(const char* source, const char* dest) {
	ifstream fin;
	ofstream fout;
	fin.open(source, ios::binary);
	if (fin.fail()) {
		cout << "Input File Open Error" << endl;
		return;
	}
	fout.open(dest, ios::binary);
	if (fout.fail()) {
		cout << "Output File Open Error" << endl;
		return;
	}
	int file_len;
	file_len = FileSize(source); //입력파일 길이(바이트)
	// file_len = 15 -> num_blocks = 1  (remainder = 15)
	// file_len = 16 -> num_blocks = 2  (remainder = 0)
	// file_len = 17 -> num_blocks = 2  (remainder = 1)
	// remainder     = 0, 1, 2, ... , 15
	// padding bytes =16,15,14, ... ,  1 
	// remainder + (padding bytes) = 16
	int num_blocks = file_len / 16 + 1;
	int remainder = file_len - (num_blocks - 1) * 16;

	byte buffer[16];
	// 패딩이 적용되지 않는 블록
	for (int i = 0; i < num_blocks - 1; i++) {
		fin.read((char*)buffer, 16);
		// 한블록 암호화
		fout.write((char*)buffer, 16);
	}
	byte last_in_block[16];
	byte last_out_block[16];
	byte ch;
	// 남은 데이터 읽기 (0, 1, ... , 15)
	for (int i = 0; i < remainder; i++) {
		fin.read((char*)&ch, 1);
		last_in_block[i] = ch;
	}

	Padding(last_in_block, remainder, last_out_block);
	// 한블록 암호화
	fout.write((char*)last_out_block, 16);

	fin.close();
	fout.close();
}