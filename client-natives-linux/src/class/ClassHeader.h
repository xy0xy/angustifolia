#ifndef MCRES_LICENSE_CLIENT_CLASS_HEADER_H
#define MCRES_LICENSE_CLIENT_CLASS_HEADER_H

#include <stdlib.h>

typedef struct constant_pool_info
{
	unsigned char tag;
	void * data;
} ConstantPoolInfo;

typedef struct class_header
{
	unsigned int magic;
	unsigned short majorVer;
	unsigned short minorVer;
	
	unsigned short constantPoolAmount;
	ConstantPoolInfo * constantPool;
	
	unsigned short accessFlags;
	unsigned short thisClass;
	unsigned short superClass;
	
	unsigned short interfaceAmount;
	unsigned short * interfaces;
	
	// rest of them are encrypted :P
} ClassHeader;

#define CONSTANT_Class 	7
#define CONSTANT_Fieldref 	9
#define CONSTANT_Methodref 	10
#define CONSTANT_InterfaceMethodref 	11
#define CONSTANT_String 	8
#define CONSTANT_Integer 	3
#define CONSTANT_Float 	4
#define CONSTANT_Long 	5
#define CONSTANT_Double 	6
#define CONSTANT_NameAndType 	12
#define CONSTANT_Utf8 	1
#define CONSTANT_MethodHandle 	15
#define CONSTANT_MethodType 	16
#define CONSTANT_InvokeDynamic 	18

void readClassHeader(ClassHeader * header, const char * data, size_t length, char ** afterHeader);

#endif //MCRES_LICENSE_CLIENT_CLASS_HEADER_H
