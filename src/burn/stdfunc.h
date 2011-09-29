// Standard ROM/input/DIP info functions

// A function to pick a rom, or return NULL if i is out of range
#define STD_ROM_PICK(Name) \
static inline struct BurnRomInfo* Name##PickRom(unsigned int i) \
{ \
	if (i >= sizeof(Name##RomDesc) / sizeof(Name##RomDesc[0])) \
		return NULL; \
	return Name##RomDesc + i; \
}

#define STDROMPICKEXT(Name, Info1, Info2) \
static inline struct BurnRomInfo* Name##PickRom(unsigned int i) \
{ \
	if (i >= 0x80) \
	{ \
		i &= 0x7F; \
		if (i >= sizeof(Info2##RomDesc) / sizeof(Info2##RomDesc[0])) \
			return NULL; \
		return Info2##RomDesc + i; \
	} \
	else \
	{ \
		if (i >= sizeof(Info1##RomDesc) / sizeof(Info1##RomDesc[0])) \
				return emptyRomDesc + 0; \
		return Info1##RomDesc + i; \
	} \
}

// Standard rom functions for returning Length, Crc, Type and one one Name
#define STD_ROM_FN(Name)													\
static inline int Name##RomInfo(struct BurnRomInfo* pri, unsigned int i)	\
{																			\
	struct BurnRomInfo* por = Name##PickRom(i);								\
	if (por == NULL) {														\
		return 1;															\
	}																		\
	if (pri) {																\
		pri->nLen = por->nLen;												\
		pri->nCrc = por->nCrc;												\
		pri->nType = por->nType;											\
	}																		\
	return 0;																\
}																			\
																			\
static inline int Name##RomName(char** pszName, unsigned int i, int nAka) \
{ \
	struct BurnRomInfo *por = Name##PickRom(i); \
	if (por == NULL) \
		return 1; \
	if(nAka) \
		return 1; \
	*pszName = por->szName; \
	return 0; \
}

#ifdef NO_COMBO

#define STDINPUTINFO(Name)													\
static inline int Name##InputInfo(struct BurnInputInfo* pii, unsigned int i) \
{ \
	if (i >= sizeof(Name##InputList) / sizeof(Name##InputList[0])) \
		return 1; \
	if (pii) \
		*pii = Name##InputList[i]; \
	return 0; \
}

#define STDINPUTINFOSPEC(Name, Info1)										\
static inline int Name##InputInfo(struct BurnInputInfo* pii, unsigned int i)\
{																			\
	if (i >= sizeof(Info1) / sizeof(Info1[0])) {							\
		return 1;															\
	}																		\
	if (pii) {																\
		*pii = Info1[i];													\
	}																		\
	return 0;																\
}

#else

#define STDINPUTINFO(Name)													\
static inline int Name##InputInfo(struct BurnInputInfo* pii, unsigned int i)\
{																			\
	if (i < sizeof(Name##InputList) / sizeof(Name##InputList[0])) {			\
		if (pii) {															\
			*pii = Name##InputList[i];										\
		}																	\
		return 0;															\
	}																		\
	else if ((i < ((sizeof(Name##InputList) / sizeof(Name##InputList[0])) + nComCount))	\
			&& nInputMacroEnabled) {										\
		if (pii) {															\
			*pii = DrvCombo[i - sizeof(Name##InputList) / sizeof(Name##InputList[0])];	\
		}																	\
		return 0;															\
	}																		\
	else return 1;															\
}

#define STDINPUTINFOSPEC(Name, Info1)										\
static inline int Name##InputInfo(struct BurnInputInfo* pii, unsigned int i)\
{																			\
	if (i < sizeof(Info1) / sizeof(Info1[0])) {								\
		if (pii) {															\
			*pii = Info1[i];												\
		}																	\
		return 0;															\
	}																		\
	else if ((i < ((sizeof(Info1) / sizeof(Info1[0])) + nComCount))			\
			&& nInputMacroEnabled) {										\
		if (pii) {															\
			*pii = DrvCombo[i - sizeof(Info1) / sizeof(Info1[0])];			\
		}																	\
		return 0;															\
	}																		\
	else return 1;															\
}

#endif

#define STDDIPINFO(Name) \
static inline int Name##DIPInfo(struct BurnDIPInfo* pdi, unsigned int i) \
{ \
	if (i >= sizeof(Name##DIPList) / sizeof(Name##DIPList[0])) \
		return 1; \
	if (pdi) \
		*pdi = Name##DIPList[i]; \
	return 0; \
}

#define STDDIPINFOEXT(Name, Info1, Info2) \
static inline int Name##DIPInfo(struct BurnDIPInfo* pdi, unsigned int i) \
{ \
	if (i >= sizeof(Info1##DIPList) / sizeof(Info1##DIPList[0])) \
	{ \
		i -= sizeof(Info1##DIPList) / sizeof(Info1##DIPList[0]); \
		if (i >= sizeof(Info2##DIPList) / sizeof(Info2##DIPList[0])) \
		return 1; \
		if (pdi) \
		*pdi = Info2##DIPList[i]; \
		return 0; \
	} \
	if (pdi) \
	*pdi = Info1##DIPList[i]; \
	return 0; \
}

// my macro for ROM/input/DIP info functions
#define INFO_FN(name)						name##RomInfo
#define NAME_FN(name)						name##RomName
#define INPUT_FN(name)						name##InputInfo
#define DIP_FN(name)						name##DIPInfo
#define ROM_FN(name)						INFO_FN(name), NAME_FN(name), INPUT_FN(name), DIP_FN(name)

#define DESC_START(name)					static struct BurnRomInfo name##RomDesc[] = {
#define DESC_END(name)						\
};											\
STD_ROM_PICK(name)							\
STD_ROM_FN(name)

#define DESC_END_EXT(name, info1, info2)	\
};											\
STDROMPICKEXT(name, info1, info2)			\
STD_ROM_FN(name)
