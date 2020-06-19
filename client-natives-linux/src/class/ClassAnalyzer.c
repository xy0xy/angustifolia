#include "ClassHeader.h"
#include "../util/util.h"

#include <memory.h>
#include <malloc.h>

void readClassHeader(ClassHeader * header, const char * data, size_t length, char ** afterHeader)
{
	memset(header, 0, sizeof(ClassHeader));
	
	long check = length;
	unsigned long current = (unsigned long) data;
	memcpyInversed(&header->magic, (const void *) current, sizeof(header->magic));
	
	check -= sizeof(header->magic);
	current += sizeof(header->magic);
	if (check <= 0)
		goto final;
	
	memcpyInversed(&header->majorVer, (const void *) current, sizeof(header->majorVer));
	check -= sizeof(header->majorVer);
	current += sizeof(header->majorVer);
	if (check <= 0)
		goto final;
	
	memcpyInversed(&header->minorVer, (const void *) current, sizeof(header->minorVer));
	check -= sizeof(header->minorVer);
	current += sizeof(header->minorVer);
	if (check <= 0)
		goto final;
	
	memcpyInversed(&header->constantPoolAmount, (const void *) current, sizeof(header->constantPoolAmount));
	check -= sizeof(header->constantPoolAmount);
	current += sizeof(header->constantPoolAmount);
	if (check <= 0)
		goto final;
	
	header->constantPool = malloc(sizeof(ConstantPoolInfo) * header->constantPoolAmount - 1);
	for (short i = 0; i < header->constantPoolAmount - 1; i ++)
	{
		ConstantPoolInfo constantPoolInfo = header->constantPool[i];
		memcpyInversed(&constantPoolInfo.tag, (const void *) current, sizeof(constantPoolInfo.tag));
		check -= sizeof(constantPoolInfo.tag);
		current += sizeof(constantPoolInfo.tag);
		if (check <= 0)
			goto final;
		
		size_t size = 0;
		switch (constantPoolInfo.tag) {
			case CONSTANT_Class:
			case CONSTANT_String:
			case CONSTANT_MethodType:
				size = 2;
				break;
			case CONSTANT_Fieldref:
			case CONSTANT_Methodref:
			case CONSTANT_InterfaceMethodref:
			case CONSTANT_Integer:
			case CONSTANT_Float:
			case CONSTANT_NameAndType:
			case CONSTANT_InvokeDynamic:
				size = 4;
				break;
			case CONSTANT_Long:
			case CONSTANT_Double:
				size = 8;
				break;
			case CONSTANT_Utf8:
				size = (size_t) (2 + *((short *)current));
				break;
			case CONSTANT_MethodHandle:
				size = 3;
				break;
			default:
				goto skip;
		}
		
		constantPoolInfo.data = malloc(size * sizeof(char));
		memcpyInversed(constantPoolInfo.data, (const void *) current, sizeof(char) * size);
		check -= size;
		current += size;
		
		if (check <= 0)
			goto final;
		
		skip:
			; // do we need to do anything?
	}
	
	memcpyInversed(&header->accessFlags, (const void *) current, sizeof(header->accessFlags));
	check -= sizeof(header->accessFlags);
	current += sizeof(header->accessFlags);
	if (check <= 0)
		goto final;
	
	memcpyInversed(&header->thisClass, (const void *) current, sizeof(header->thisClass));
	check -= sizeof(header->thisClass);
	current += sizeof(header->thisClass);
	if (check <= 0)
		goto final;
	
	memcpyInversed(&header->superClass, (const void *) current, sizeof(header->superClass));
	check -= sizeof(header->superClass);
	current += sizeof(header->superClass);
	if (check <= 0)
		goto final;
	
	memcpyInversed(&header->interfaceAmount, (const void *) current, sizeof(header->interfaceAmount));
	check -= sizeof(header->interfaceAmount);
	current += sizeof(header->interfaceAmount);
	if (check <= 0)
		goto final;
	
	header->interfaces = malloc(sizeof(unsigned short) * header->interfaceAmount);
	memcpyInversed(header->interfaces, (const void *) current, sizeof(unsigned short) * header->interfaceAmount);
	check -= sizeof(unsigned short);
	current += sizeof(unsigned short);
	
	final:
	*afterHeader = (char *) current;
}
