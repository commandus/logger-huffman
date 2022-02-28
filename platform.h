#ifndef PLATFORM_H
#define PLATFORM_H 1

#ifdef _MSC_VER
#define ALIGN	__declspec(align(1))
#define PACKED	
#else
#define ALIGN	
#define PACKED	__attribute__((aligned(1), packed))
#endif

#endif
