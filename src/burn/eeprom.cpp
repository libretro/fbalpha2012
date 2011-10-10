// EEPROM module
#include "burnint.h"
#include "eeprom.h"

#define SERIAL_BUFFER_LENGTH 40
#define MEMORY_SIZE 1024

static const eeprom_interface* intf;

static int serial_count;
static unsigned char serial_buffer[SERIAL_BUFFER_LENGTH];
static unsigned char eeprom_data[MEMORY_SIZE];
static int eeprom_data_bits;
static int eeprom_read_address;
static int eeprom_clock_count;
static int latch, reset_line, clock_line, sending;
static int locked;
static int reset_delay;

static int neeprom_available = 0;

static bool eeprom_command_match(const char* buf, const char* cmd, int len)
{
	if (!buf || !cmd || len <= 0)
		return 0;

	for (; len > 0;) {
		char b = *buf;
		char c = *cmd;

		if ((b == 0) || (c == 0))
			return (b == c);

		switch (c) {
			case '0':
			case '1':
				if (b != c)
					return 0;
			case 'X':
			case 'x':
				buf++;
				len--;
				cmd++;
				break;

			case '*':
				c = cmd[1];
				switch(c) {
					case '0':
					case '1':
						if (b == c)
							cmd++;
						else
						{
							buf++;
							len--;
						}
						break;
					default:
						return 0;
				}
		}
	}

	return (*cmd == 0);
}

// ==> nvram hack, added by regret
void EEPROMHack(int size)
{
	if (!strcasecmp(BurnDrvGetText(DRV_NAME), "sfa3")
			|| !strcasecmp(BurnDrvGetText(DRV_NAME), "sfa3u")
			|| !strcasecmp(BurnDrvGetText(DRV_NAME), "sfa3ur1")
			|| !strcasecmp(BurnDrvGetText(DRV_NAME), "sfz3j")
			|| !strcasecmp(BurnDrvGetText(DRV_NAME), "sfz3jr1")
			|| !strcasecmp(BurnDrvGetText(DRV_NAME), "sfz3jr2")
			|| !strcasecmp(BurnDrvGetText(DRV_NAME), "sfz3ar1")
	   ) {
		UINT8 eeprom[] = {
			0x00,0x01,0x00,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
			0x01,0x01,0x01,0x04,0x01,0x01,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
			0x98,0x06,0x29,0x00,0x53,0x5A,0x33,0x4A,0x20,0x5A,0x45,0x52,0x4F,0x20,0x33,0x20,
			0x00,0x01,0x00,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
			0x01,0x01,0x01,0x04,0x01,0x01,0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
			0x98,0x06,0x29,0x00,0x53,0x5A,0x33,0x4A,0x20,0x5A,0x45,0x52,0x4F,0x20,0x33,0x20,
			0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x9b,0x00,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
		};  /* sfz3jr2p */

		if (!strcasecmp(BurnDrvGetText(DRV_NAME), "sfz3jr1")) {
			eeprom[0x21] = eeprom[0x51] = 0x07;
			eeprom[0x22] = eeprom[0x52] = 0x27;
		}
		else if (!strcasecmp(BurnDrvGetText(DRV_NAME), "sfz3j")) {
			eeprom[0x21] = eeprom[0x51] = 0x09;
			eeprom[0x22] = eeprom[0x52] = 0x04;
		}
		else if (!strcasecmp(BurnDrvGetText(DRV_NAME), "sfa3")) {
			eeprom[0x00] = eeprom[0x30] = 0x09;
			eeprom[0x06] = eeprom[0x36] = 0x01;
			eeprom[0x08] = eeprom[0x38] = 0x02;
			eeprom[0x10] = eeprom[0x40] = 0x04;
			eeprom[0x13] = eeprom[0x43] = 0x03;
			eeprom[0x21] = eeprom[0x51] = 0x09;
			eeprom[0x22] = eeprom[0x52] = 0x04;
			eeprom[0x23] = eeprom[0x53] = 0x0A;
			eeprom[0x27] = eeprom[0x57] = 'E';
			eeprom[0x28] = eeprom[0x58] = 'S';
			eeprom[0x29] = eeprom[0x59] = 'F';
			eeprom[0x2a] = eeprom[0x5a] = 'A';
			eeprom[0x2b] = eeprom[0x5b] = 'L';
			eeprom[0x2c] = eeprom[0x5c] = 'P';
			eeprom[0x2d] = eeprom[0x5d] = 'H';
			eeprom[0x2e] = eeprom[0x5e] = 'A';
			eeprom[0x2f] = eeprom[0x5f] = '3';
		}
		else if (!strcasecmp(BurnDrvGetText(DRV_NAME), "sfa3u")) {
			eeprom[0x21] = eeprom[0x51] = 0x09;
			eeprom[0x22] = eeprom[0x52] = 0x04;
			eeprom[0x08] = eeprom[0x23] = eeprom[0x38] = eeprom[0x53] = 0x02;
			eeprom[0x06] = eeprom[0x36] = 0x01;
			eeprom[0x10] = eeprom[0x40] = 0x04;
			eeprom[0x27] = eeprom[0x57] = 'U';
			eeprom[0x28] = eeprom[0x58] = 'A';
			eeprom[0x29] = eeprom[0x59] = 'L';
			eeprom[0x2a] = eeprom[0x5a] = 'P';
			eeprom[0x2b] = eeprom[0x5b] = 'H';
			eeprom[0x2c] = eeprom[0x5c] = 'A';
		}
		else if (!strcasecmp(BurnDrvGetText(DRV_NAME), "sfa3ur1")) {
			eeprom[0x08] = eeprom[0x23] = eeprom[0x38] = eeprom[0x53] = 0x02;
			eeprom[0x06] = eeprom[0x36] = 0x01;
			eeprom[0x10] = eeprom[0x40] = 0x04;
			eeprom[0x27] = eeprom[0x57] = 'U';
			eeprom[0x28] = eeprom[0x58] = 'A';
			eeprom[0x29] = eeprom[0x59] = 'L';
			eeprom[0x2a] = eeprom[0x5a] = 'P';
			eeprom[0x2b] = eeprom[0x5b] = 'H';
			eeprom[0x2c] = eeprom[0x5c] = 'A';
		}
		else if (!strcasecmp(BurnDrvGetText(DRV_NAME), "sfz3ar1")) {
			eeprom[0x08] = eeprom[0x38] = 0x02;
			eeprom[0x06] = eeprom[0x22] = eeprom[0x36] = eeprom[0x52] = 0x01;
			eeprom[0x10] = eeprom[0x21] = eeprom[0x40] = eeprom[0x51] = 0x07;
			eeprom[0x13] = eeprom[0x43] = 0x03;
			eeprom[0x23] = eeprom[0x53] = 0x08;
			eeprom[0x27] = eeprom[0x57] = 'A';
			eeprom[0x29] = eeprom[0x59] = 'z';
			eeprom[0x2a] = eeprom[0x5a] = 'e';
			eeprom[0x2b] = eeprom[0x5b] = 'r';
			eeprom[0x2c] = eeprom[0x5c] = 'o';
		}

		memcpy(eeprom_data, eeprom, size);
		return;
	}

	if (!strcasecmp(BurnDrvGetText(DRV_NAME), "batcir4p")
			|| !strcasecmp(BurnDrvGetText(DRV_NAME), "btcirj4p")
	   ) {
		UINT8 eeprom[] = {
			0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x03,0x07,0x00,0x00,0x00,
			0x03,0x00,0x01,0x01,0x01,0x00,0x07,0x00,0x97,0x03,0x19,0x00,0x42,0x54,0x43,0x20,
			0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x03,0x07,0x00,0x00,0x00,
			0x03,0x00,0x01,0x01,0x01,0x00,0x07,0x00,0x97,0x03,0x19,0x00,0x42,0x54,0x43,0x20,
			0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
			0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff
		};  /* btcirj4p */

		if (!strcasecmp(BurnDrvGetText(DRV_NAME), "batcir4p")) {
			eeprom[0x2b] = eeprom[0x5b] = 0x08;
		}

		memcpy(eeprom_data, eeprom, size);
		return;
	}

	if (!strcasecmp(BurnDrvGetText(DRV_NAME), "avsp3p")
			|| !strcasecmp(BurnDrvGetText(DRV_NAME), "avspu3p")
			|| !strcasecmp(BurnDrvGetText(DRV_NAME), "avspj3p")
			|| !strcasecmp(BurnDrvGetText(DRV_NAME), "avspa3p")
	   ) {
		UINT8 eeprom[] = {
			0x00,0x01,0x00,0x01,0x01,0x00,0x01,0xc3,0x00,0X02,0x00,0x00,0x00,0x00,0x00,0x00,
			0x03,0x01,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
			0x94,0x05,0x20,0X02,0x92,0x10,0x06,0xff,0x50,0X72,0X65,0X64,0X61,0X74,0X6f,0X72,
			0x00,0x01,0x00,0x01,0x01,0x00,0x01,0xc3,0x00,0X02,0x00,0x00,0x00,0x00,0x00,0x00,
			0x03,0x01,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
			0x94,0x05,0x20,0X02,0x92,0x10,0x06,0xff,0x50,0X72,0X65,0X64,0X61,0X74,0X6f,0X72,
			0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
		};  /* avspu */

		if (!strcasecmp(BurnDrvGetText(DRV_NAME), "avsp3p")) {
			eeprom[0x23] = eeprom[0x53] = 0x08;
		}
		else if (!strcasecmp(BurnDrvGetText(DRV_NAME), "avspj3p")) {
			eeprom[0x09] = eeprom[0x23] = eeprom[0x39] = eeprom[0x53] = 0x00;
			eeprom[0x29] = eeprom[0x59] = 0x52;
			eeprom[0x2a] = eeprom[0x5a] = 0x45;
			eeprom[0x2b] = eeprom[0x5b] = 0x44;
			eeprom[0x2c] = eeprom[0x5c] = 0x41;
			eeprom[0x2d] = eeprom[0x5d] = 0x54;
			eeprom[0x2e] = eeprom[0x5e] = 0x4f;
			eeprom[0x2f] = eeprom[0x5f] = 0x52;
		}
		else if (!strcasecmp(BurnDrvGetText(DRV_NAME), "avspa3p")) {
			eeprom[0x03] = eeprom[0x33] = 0x00;
			eeprom[0x23] = eeprom[0x53] = 0x06;
			eeprom[0x29] = eeprom[0x59] = 0x52;
			eeprom[0x2a] = eeprom[0x5a] = 0x45;
			eeprom[0x2b] = eeprom[0x5b] = 0x44;
			eeprom[0x2c] = eeprom[0x5c] = 0x41;
			eeprom[0x2d] = eeprom[0x5d] = 0x54;
			eeprom[0x2e] = eeprom[0x5e] = 0x4f;
			eeprom[0x2f] = eeprom[0x5f] = 0x52;
		}

		memcpy(eeprom_data, eeprom, size);
		return;
	}

	if (!strcasecmp(BurnDrvGetText(DRV_NAME), "ddsom4p")
			|| !strcasecmp(BurnDrvGetText(DRV_NAME), "ddsomr4p")
			|| !strcasecmp(BurnDrvGetText(DRV_NAME), "ddsomu4p")
			|| !strcasecmp(BurnDrvGetText(DRV_NAME), "ddsmur4p")
			|| !strcasecmp(BurnDrvGetText(DRV_NAME), "ddsomj4p")
			|| !strcasecmp(BurnDrvGetText(DRV_NAME), "ddsmjr4p")
			|| !strcasecmp(BurnDrvGetText(DRV_NAME), "ddsoma4p")
	   ) {
		UINT8 eeprom[] = {
			0x00,0x0b,0x01,0x00,0x01,0x01,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
			0x03,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
			0x96,0x02,0x01,0x02,0x00,0x00,0x00,0x00,0x44,0x26,0x44,0x32,0x20,0x55,0x53,0x41,
			0x00,0x0b,0x01,0x00,0x01,0x01,0x00,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
			0x03,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
			0x96,0x02,0x01,0x02,0x00,0x00,0x00,0x00,0x44,0x26,0x44,0x32,0x20,0x55,0x53,0x41,
			0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
			0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
		};  /* ddsom4p */

		if (!strcasecmp(BurnDrvGetText(DRV_NAME), "ddsmur4p")) {
			eeprom[0x63] = 0x01;
		}
		else if (!strcasecmp(BurnDrvGetText(DRV_NAME), "ddsom4p")
				|| !strcasecmp(BurnDrvGetText(DRV_NAME), "ddsomr4p")) {
			eeprom[0x07] = eeprom[0x37] = 0x02;
			eeprom[0x23] = eeprom[0x53] = 0x08;
			eeprom[0x2d] = eeprom[0x5d] = 0x45; // E
			eeprom[0x2e] = eeprom[0x5e] = 0x55; // U
			eeprom[0x2f] = eeprom[0x5f] = 0x52; // R

			if (!strcasecmp(BurnDrvGetText(DRV_NAME), "ddsom4p")) {
				eeprom[0x63] = 0x01;
			}
		}
		else if (!strcasecmp(BurnDrvGetText(DRV_NAME), "ddsmjr4p")
				|| !strcasecmp(BurnDrvGetText(DRV_NAME), "ddsomj4p")) {
			eeprom[0x07] = eeprom[0x23] = eeprom[0x37] = eeprom[0x53] = 0x00;
			eeprom[0x2d] = eeprom[0x5d] = 0x4a; // J
			eeprom[0x2e] = eeprom[0x5e] = 0x50; // P
			eeprom[0x2f] = eeprom[0x5f] = 0x4e; // N

			if (!strcasecmp(BurnDrvGetText(DRV_NAME), "ddsomj4p")) {
				eeprom[0x63] = 0x01;
			}
		}
		else if (!strcasecmp(BurnDrvGetText(DRV_NAME), "ddsoma4p")) {
			eeprom[0x23] = eeprom[0x53] = 0x06;
			eeprom[0x06] = eeprom[0x36] = 0x01;
			eeprom[0x2d] = eeprom[0x5d] = 0x41; // A
			eeprom[0x2e] = eeprom[0x5e] = 0x53; // S
			eeprom[0x2f] = eeprom[0x5f] = 0x49; // I
			eeprom[0x63] = 0x01;
		}

		memcpy(eeprom_data, eeprom, size);
		return;
	}
}
// <== nvram hack

int EEPROMAvailable()
{
	return neeprom_available;
}

void EEPROMInit(const eeprom_interface* interface)
{
	if (!interface)
		return;

	intf = interface;

	unsigned int intfSize = (1 << intf->address_bits) * intf->data_bits / 8;
	#if 0
	if (intfSize > MEMORY_SIZE)
	{
		bprintf(0, "EEPROM larger than eeprom allows");
	}
	#endif

	memset(eeprom_data, 0xff, intfSize);
	serial_count = 0;
	latch = 0;
	reset_line = EEPROM_ASSERT_LINE;
	clock_line = EEPROM_ASSERT_LINE;
	eeprom_read_address = 0;
	sending = 0;
	if (intf->cmd_unlock) locked = 1;
	else locked = 0;

	char output[256] = "";
#if defined (_XBOX)
	sprintf(output, "GAME:\\config\\games\\%s.nv", BurnDrvGetTextA(DRV_NAME));
#else
	sprintf(output, "/dev_hdd0/game/FBAN00000/USRDIR/config/games/%s.nv", BurnDrvGetTextA(DRV_NAME));
#endif

	neeprom_available = 0;

	int len = intfSize & (MEMORY_SIZE - 1);

	FILE* file = fopen(output, "rb");
	if (file != NULL) {
		neeprom_available = 1;
		fread(eeprom_data, len, 1, file);
		fclose(file);
	} else {
		EEPROMHack(len);
	}
}

void EEPROMExit()
{
	char output[256] = "";
#if defined (_XBOX)
	sprintf(output, "GAME:\\config\\games\\%s.nv", BurnDrvGetTextA(DRV_NAME));
#else
	sprintf(output, "/dev_hdd0/game/FBAN00000/USRDIR/config/games/%s.nv", BurnDrvGetTextA(DRV_NAME));	
#endif

	neeprom_available = 0;

	int len = ((1 << intf->address_bits) * (intf->data_bits >> 3)) & (MEMORY_SIZE - 1);

#ifndef __LIBSNES__
	FILE* file = fopen(output, "wb");
	fwrite(eeprom_data, len, 1, file);
	fclose(file);
#endif
}

static void eeprom_write(int bit)
{
	if (serial_count >= SERIAL_BUFFER_LENGTH - 1)
	{
		//bprintf(0, "error: EEPROM serial buffer overflow\n");
		return;
	}

	serial_buffer[serial_count++] = (bit ? '1' : '0');
	serial_buffer[serial_count] = 0;

	if (serial_count > intf->address_bits
		&& eeprom_command_match((char*)serial_buffer, intf->cmd_read, strlen((char*)serial_buffer) - intf->address_bits)) {
		int address = 0;
		for (int i = serial_count - intf->address_bits; i < serial_count; i++) {
			address <<= 1;
			if (serial_buffer[i] == '1') {
				address |= 1;
			}
		}

		if (intf->data_bits == 16) {
			eeprom_data_bits = (eeprom_data[2 * address + 0] << 8) + eeprom_data[2 * address + 1];
		} else {
			eeprom_data_bits = eeprom_data[address];
		}
		eeprom_read_address = address;
		eeprom_clock_count = 0;
		sending = 1;
		serial_count = 0;
	}
	else if (serial_count > intf->address_bits
		&& eeprom_command_match((char*)serial_buffer, intf->cmd_erase, strlen((char*)serial_buffer)-intf->address_bits)) {
		int address = 0;
		for (int i = serial_count - intf->address_bits; i < serial_count; i++) {
			address <<= 1;
			if (serial_buffer[i] == '1') {
				address |= 1;
			}
		}

		if (locked == 0) {
			if (intf->data_bits == 16) {
				eeprom_data[2 * address + 0] = 0xff;
				eeprom_data[2 * address + 1] = 0xff;
			} else {
				eeprom_data[address] = 0xff;
			}
		} else {
			serial_count = 0;
		}
	}
	else if (serial_count > (intf->address_bits + intf->data_bits)
		&& eeprom_command_match((char*)serial_buffer, intf->cmd_write, strlen((char*)serial_buffer) - (intf->address_bits + intf->data_bits))) {
		int address = 0;
		for (int i = serial_count-intf->data_bits - intf->address_bits; i < (serial_count-intf->data_bits); i++) {
			address <<= 1;
			if (serial_buffer[i] == '1') {
				address |= 1;
			}
		}
		int data = 0;
		for (int i = serial_count-intf->data_bits; i < serial_count; i++) {
			data <<= 1;
			if (serial_buffer[i] == '1') {
				data |= 1;
			}
		}

		if (locked == 0) {
			if (intf->data_bits == 16) {
				eeprom_data[2 * address + 0] = data >> 8;
				eeprom_data[2 * address + 1] = data & 0xff;
			} else {
				eeprom_data[address] = data;
			}
		} else {
			serial_count = 0;
		}
	}
	else if (eeprom_command_match((char*)serial_buffer, intf->cmd_lock, strlen((char*)serial_buffer))) {
		locked = 1;
		serial_count = 0;
	}
	else if (eeprom_command_match((char*)serial_buffer, intf->cmd_unlock, strlen((char*)serial_buffer))) {
		locked = 0;
		serial_count = 0;
	}
}

void EEPROMReset()
{
	serial_count = 0;
	sending = 0;
	reset_delay = intf->reset_delay;
}

void EEPROMWriteBit(int bit)
{
	latch = bit;
}

int EEPROMRead()
{
	int res;

	if (sending) {
		res = (eeprom_data_bits >> intf->data_bits) & 1;
	} else {
		if (reset_delay > 0) {
			/* this is needed by wbeachvl */
			reset_delay--;
			res = 0;
		} else {
			res = 1;
		}
	}

	return res;
}

void EEPROMSetCSLine(int state)
{
	reset_line = state;
	if (reset_line != EEPROM_CLEAR_LINE) {
		EEPROMReset();
	}
}

void EEPROMSetClockLine(int state)
{
	if (state == EEPROM_PULSE_LINE || (clock_line == EEPROM_CLEAR_LINE && state != EEPROM_CLEAR_LINE)) {
		if (reset_line == EEPROM_CLEAR_LINE) {
			if (sending) {
				if (eeprom_clock_count == intf->data_bits && intf->enable_multi_read) {
					eeprom_read_address = (eeprom_read_address + 1) & ((1 << intf->address_bits) - 1);
					if (intf->data_bits == 16) {
						eeprom_data_bits = (eeprom_data[2 * eeprom_read_address + 0] << 8) + eeprom_data[2 * eeprom_read_address + 1];
					} else {
						eeprom_data_bits = eeprom_data[eeprom_read_address];
					}
					eeprom_clock_count = 0;
				}
				eeprom_data_bits = (eeprom_data_bits << 1) | 1;
				eeprom_clock_count++;
			} else {
				eeprom_write(latch);
			}
		}
	}

	clock_line = state;
}

void EEPROMFill(const unsigned char *data, int offset, int length)
{
	memcpy(eeprom_data + offset, data, length);
}

void EEPROMScan(int nAction, int* pnMin)
{
	BurnArea ba;

	if (nAction & ACB_DRIVER_DATA)
	{
		if (pnMin && *pnMin < 0x020902)
			*pnMin = 0x029705;

		memset(&ba, 0, sizeof(ba));
		ba.Data = serial_buffer;
		ba.nLen = SERIAL_BUFFER_LENGTH;
		ba.szName = "Serial Buffer";
		BurnAcb(&ba);

		SCAN_VAR(serial_count);
		SCAN_VAR(eeprom_data_bits);
		SCAN_VAR(eeprom_read_address);
		SCAN_VAR(eeprom_clock_count);
		SCAN_VAR(latch);
		SCAN_VAR(reset_line);
		SCAN_VAR(clock_line);
		SCAN_VAR(sending);
		SCAN_VAR(locked);
		SCAN_VAR(reset_delay);
	}

//	if (nAction & ACB_NVRAM) {
//
//		if (pnMin && (nAction & ACB_TYPEMASK) == ACB_NVRAM) {
//			*pnMin = 0x02705;
//		}
//
//		memset(&ba, 0, sizeof(ba));
//  	ba.Data = eeprom_data;
//		ba.nLen = MEMORY_SIZE;
//		ba.szName = "EEPROM memory";
//		BurnAcb(&ba);
//	}
}
