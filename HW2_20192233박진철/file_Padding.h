#pragma once

using namespace std;

typedef unsigned char byte;

int FileSize(const char *file_name);

void Padding(byte in[], int in_length, byte out[16]);

void FilePadding(const char* source, const char* dest);