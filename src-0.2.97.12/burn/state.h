#if !defined(_STATE_H)

#ifdef __cplusplus
 extern "C" {
#endif

/* Scan driver data */
int BurnAreaScan(int nAction, int* pnMin);

/* flags to use for nAction */
#define ACB_READ		 ( 1)
#define ACB_WRITE		 ( 2)

#define ACB_MEMORY_ROM	 ( 4)
#define ACB_NVRAM		 ( 8)
#define ACB_MEMCARD		 (16)
#define ACB_MEMORY_RAM	 (32)
#define ACB_DRIVER_DATA	 (64)

#define ACB_FULLSCAN	(ACB_NVRAM | ACB_MEMCARD | ACB_MEMORY_RAM | ACB_DRIVER_DATA)

#define ACB_ACCESSMASK	(ACB_READ | ACB_WRITE)
#define ACB_TYPEMASK	(ACB_MEMORY_ROM | ACB_NVRAM | ACB_MEMCARD | ACB_MEMORY_RAM | ACB_DRIVER_DATA)

#define ACB_VOLATILE    (ACB_MEMORY_RAM | ACB_DRIVER_DATA)

/* Structure used for area scanning */
struct BurnArea { void *Data; unsigned int nLen; int nAddress; char *szName; };

/* Application-defined callback for processing the area */
extern int (__cdecl *BurnAcb) (struct BurnArea* pba);

/* Scan a small variable or structure */
inline static void ScanVar(void* pv, int nSize, char* szName)
{
	struct BurnArea ba;
	memset(&ba, 0, sizeof(ba));
	ba.Data   = pv;
	ba.nLen   = nSize;
	ba.szName = szName;
	BurnAcb(&ba);
}

#define SCAN_VAR(x) ScanVar(&x, sizeof(x), #x)

#ifdef OSD_CPU_H
 /* wrappers for the MAME savestate functions (used by the FM sound cores) */
 void state_save_register_func_postload(void (*pFunction)());

 void state_save_register_INT8(const char* module, int instance, const char* name, INT8* val, unsigned size);
 void state_save_register_UINT8(const char* module, int instance, const char* name, UINT8* val, unsigned size);
 void state_save_register_INT16(const char* module, int instance, const char* name, INT16* val, unsigned size);
 void state_save_register_UINT16(const char* module, int instance, const char* name, UINT16* val, unsigned size);
 void state_save_register_INT32(const char* module, int instance, const char* name, INT32* val, unsigned size);
 void state_save_register_UINT32(const char* module, int instance, const char* name, UINT32* val, unsigned size);

 void state_save_register_int(const char* module, int instance, const char* name, int* val);
 void state_save_register_float(const char* module, int instance, const char* name, float* val, unsigned size);
 void state_save_register_double(const char* module, int instance, const char* name, double* val, unsigned size);
#endif

#ifdef __cplusplus
 }
#endif

#define _STATE_H

#endif /* _STATE_H */
